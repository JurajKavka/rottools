#include "DirectoryScanner.h"

#include <system_error>

#include "HelperFunctions.h"

wxDEFINE_EVENT(wxEVT_DIRECTORY_SCAN_COMPLETE, wxThreadEvent);

DirectoryScanner::DirectoryScanner() = default;

DirectoryScanner::~DirectoryScanner() {
    CancelScan();
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

    std::weak_ptr<DirectoryScanner> weakThis = weak_from_this();

    std::filesystem::path rootPath = fileName.GetFullPath().ToStdWstring();

    m_workerThread = std::jthread([weakThis, rootPath, options, eventTarget](std::stop_token stoken) {
        if (auto sharedThis = weakThis.lock()) {
            sharedThis->ScanThreadLogic(stoken, rootPath, options, eventTarget);
        }
    });
}

// Pass options by value to the background thread to safely copy the configurations
void DirectoryScanner::ScanThreadLogic(std::stop_token stoken, fs::path rootPath, ScanOptions options,
                                       wxEvtHandler* eventTarget) {
    std::vector<FileEntry> results;

    // Convert the vector into an unordered_set right inside the thread for efficient O(1) matching
    std::unordered_set<std::string> extSet(options.extensions.begin(), options.extensions.end());

    if (fs::exists(rootPath) && fs::is_directory(rootPath)) {
        // Explicitly disable following symlinks to prevent circular reference errors
        auto dirOptions = fs::directory_options::skip_permission_denied;
        for (const auto& entry : fs::directory_iterator(rootPath, dirOptions)) {
            try {
                if (stoken.stop_requested()) {
                    break;
                }

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

                    std::error_code ec;
                    fileEntry.size = isDir ? 0 : fs::file_size(entry.path(), ec);

                    results.push_back(fileEntry);
                }
            } catch (const fs::filesystem_error& e) {
                printError("[Error] {} - {}", e.code().message(), e.what());
            }
        }
    }

    // 3. Queue the event to the Main Thread if not cancelled
    if (!stoken.stop_requested()) {
        // Create the event
        wxThreadEvent* event = new wxThreadEvent(wxEVT_DIRECTORY_SCAN_COMPLETE);

        // Safely copy our std::vector into the wxWidgets event payload
        event->SetPayload(results);

        wxQueueEvent(eventTarget, event);
    }

    m_isScanning = false;
}

std::vector<FileEntry> DirectoryScanner::SortEntries(const std::vector<FileEntry>& entries) const {
    // 1. Create a copy so we don't destroy the original data
    // mozem pouzit std::move a zahodit povodne data
    std::vector<FileEntry> sorted = entries;

    std::sort(sorted.begin(), sorted.end(), [](const FileEntry& a, const FileEntry& b) {
        // Tier 1: Group Directories at the top
        if (a.isDirectory != b.isDirectory) {
            return a.isDirectory > b.isDirectory;
        }
        // Tier 2: Case-insensitive alphabetical sort
        bool isLess = std::lexicographical_compare(
            a.name.begin(), a.name.end(), b.name.begin(), b.name.end(),
            [](unsigned char c1, unsigned char c2) { return std::tolower(c1) < std::tolower(c2); });

        bool isGreater = std::lexicographical_compare(
            b.name.begin(), b.name.end(), a.name.begin(), a.name.end(),
            [](unsigned char c1, unsigned char c2) { return std::tolower(c1) < std::tolower(c2); });

        // Tie-breaker: If names are identical case-insensitively (e.g., "Apple" vs "apple"),
        // fall back to a case-sensitive check to preserve strict weak ordering requirements.
        if (!isLess && !isGreater) {
            return a.name < b.name;
        }

        return isLess;
    });

    // 3. Return the sorted vector by value
    // C++11 and later use "Move Semantics," so this is very efficient!
    return sorted;
}