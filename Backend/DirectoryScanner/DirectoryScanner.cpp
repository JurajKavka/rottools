#include "DirectoryScanner.h"

#include <algorithm>
#include <cctype>
#include <system_error>

#include "HelperFunctions.h"

wxDEFINE_EVENT(wxEVT_DIRECTORY_SCAN_COMPLETE, DirectoryScannerEvent);

DirectoryScanner::DirectoryScanner() = default;

DirectoryScanner::~DirectoryScanner() {
    CancelScan();
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void DirectoryScanner::CancelScan() {
    if (m_workerThread.joinable()) {
        m_workerThread.request_stop();
    }
}

bool DirectoryScanner::IsScanning() const {
    return m_isScanning;
}

void DirectoryScanner::StartScan(const wxFileName fileName, const ScanOptions& options, wxEvtHandler* eventTarget) {
    CancelScan();
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    m_isScanning = true;

    // Capture a raw `this` rather than extending our own lifetime via a shared_ptr.
    // The scanner is owned by its parent panel; ~DirectoryScanner (running on the
    // owner's thread) requests stop and joins this thread before we are destroyed,
    // so the worker can never end up destroying/joining itself.
    m_workerThread = std::jthread([this, fileName, options, eventTarget](std::stop_token stoken) {
        ScanThreadLogic(stoken, fileName, options, eventTarget);
    });
}

// Pass options by value to the background thread to safely copy the configurations
void DirectoryScanner::ScanThreadLogic(std::stop_token stoken, const wxFileName fileName, ScanOptions options,
                                       wxEvtHandler* eventTarget) {
    std::vector<FileEntry> results;

    // Convert the vector into an unordered_set right inside the thread for efficient O(1) matching
    std::unordered_set<std::string> extSet(options.extensions.begin(), options.extensions.end());

    std::filesystem::path rootPath = fileName.GetFullPath().ToStdWstring();

    // The outer try/catch is mandatory: an exception escaping a jthread calls
    // std::terminate. It also covers the iterator increment, which a range-for
    // performs outside the loop body (so outside the inner try/catch).
    try {
        if (fs::exists(rootPath) && fs::is_directory(rootPath)) {
            auto dirOptions = fs::directory_options::skip_permission_denied;
            for (const auto& entry : fs::directory_iterator(rootPath, dirOptions)) {
                if (stoken.stop_requested()) {
                    break;
                }

                // A failure on a single entry only skips that entry
                try {
                    // Skip symlinks to prevent circular reference errors
                    if (entry.is_symlink()) {
                        continue;
                    }

                    std::string filename = entry.path().filename().string();
                    // 🛠️ Check 1: Handle Hidden Files Option
                    if (!options.showHiddenFiles && filename.starts_with('.')) {
                        continue;
                    }

                    bool isDir = entry.is_directory();
                    std::string ext = entry.path().extension().string();

                    if (isDir || extSet.empty() || extSet.contains(ext)) {
                        FileEntry fileEntry;
                        fileEntry.path = entry.path();
                        fileEntry.name = filename;
                        fileEntry.isDirectory = isDir;
                        fileEntry.size = isDir ? 0 : fs::file_size(entry.path());

                        results.push_back(fileEntry);
                    }
                } catch (const fs::filesystem_error& e) {
                    printError("[Error] Skipping entry: {} - {}", e.code().message(), e.what());
                }
            }
        }
    } catch (const std::exception& e) {
        printError("[Error] Directory scan aborted: {}", e.what());
    }

    // 3. Queue the event to the Main Thread if not cancelled
    if (!stoken.stop_requested()) {
        // Create the event

        DirectoryScannerEvent* event = new DirectoryScannerEvent(wxEVT_DIRECTORY_SCAN_COMPLETE, wxID_ANY);
        event->files = std::move(results);
        event->currentDirectory = fileName;

        wxQueueEvent(eventTarget, event);
    }

    m_isScanning = false;
}

std::vector<FileEntry> DirectoryScanner::SortEntries(const std::vector<FileEntry>& entries) const {
    // Sort an explicit local copy; the caller's data stays untouched
    std::vector<FileEntry> sorted = entries;

    std::sort(sorted.begin(), sorted.end(), [](const FileEntry& a, const FileEntry& b) {
        // Tier 1: Group Directories at the top
        if (a.isDirectory != b.isDirectory) {
            return a.isDirectory > b.isDirectory;
        }
        // Tier 2: Case-insensitive alphabetical sort (single pass over both names)
        auto itA = a.name.begin();
        auto itB = b.name.begin();
        for (; itA != a.name.end() && itB != b.name.end(); ++itA, ++itB) {
            int lowerA = std::tolower(static_cast<unsigned char>(*itA));
            int lowerB = std::tolower(static_cast<unsigned char>(*itB));
            if (lowerA != lowerB) {
                return lowerA < lowerB;
            }
        }
        // Common prefix matched: the shorter name sorts first
        if (a.name.size() != b.name.size()) {
            return a.name.size() < b.name.size();
        }
        // Tie-breaker: If names are identical case-insensitively (e.g., "Apple" vs "apple"),
        // fall back to a case-sensitive check to preserve strict weak ordering requirements.
        return a.name < b.name;
    });

    return sorted;
}