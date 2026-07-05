#include "DirectoryScanner.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <system_error>
#include <unordered_set>

#include "HelperFunctions.h"

namespace fs = std::filesystem;

wxDEFINE_EVENT(wxEVT_DIRECTORY_SCAN_COMPLETE, DirectoryScannerEvent);

// Extension matching is case-insensitive, so ".md" also catches "README.MD"
static std::string ToLowerCopy(const std::string& text) {
    std::string lowered = text;
    std::ranges::transform(lowered, lowered.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lowered;
}

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

void DirectoryScanner::StartScan(const wxFileName& fileName, const ScanOptions& options, wxEvtHandler* eventTarget) {
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

// The references point into the lambda's by-value captures, which stay alive
// for the whole run of the worker thread.
void DirectoryScanner::ScanThreadLogic(std::stop_token stoken, const wxFileName& fileName, const ScanOptions& options,
                                       wxEvtHandler* eventTarget) {
    std::vector<FileEntry> results;

    // Convert the vector into an unordered_set right inside the thread for efficient O(1) matching
    std::unordered_set<std::string> extSet;
    for (const std::string& extension : options.extensions) {
        extSet.insert(ToLowerCopy(extension));
    }

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

                    if (isDir || extSet.empty() || extSet.contains(ToLowerCopy(ext))) {
                        results.push_back(FileEntry{
                            .path = entry.path(),
                            .name = std::move(filename),
                            .isDirectory = isDir,
                            .size = isDir ? 0 : fs::file_size(entry.path()),
                        });
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

std::vector<FileEntry> DirectoryScanner::SortEntries(const std::vector<FileEntry>& entries) {
    // Sort an explicit local copy; the caller's data stays untouched
    std::vector<FileEntry> sorted = entries;

    std::ranges::sort(sorted, [](const FileEntry& a, const FileEntry& b) {
        // Tier 1: Group Directories at the top
        if (a.isDirectory != b.isDirectory) {
            return a.isDirectory > b.isDirectory;
        }
        // Tier 2: Case-insensitive alphabetical sort
        auto lower = [](char c) { return std::tolower(static_cast<unsigned char>(c)); };
        if (std::ranges::lexicographical_compare(a.name, b.name, {}, lower, lower)) {
            return true;
        }
        if (std::ranges::lexicographical_compare(b.name, a.name, {}, lower, lower)) {
            return false;
        }
        // Tie-breaker: If names are identical case-insensitively (e.g., "Apple" vs "apple"),
        // fall back to a case-sensitive check to preserve strict weak ordering requirements.
        return a.name < b.name;
    });

    return sorted;
}
