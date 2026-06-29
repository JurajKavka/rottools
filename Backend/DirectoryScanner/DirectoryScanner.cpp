#include "DirectoryScanner.h"
#include <system_error>

// 2. Define the custom event (allocates the event ID)
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

void DirectoryScanner::StartScan(const fs::path& rootPath, const std::vector<std::string>& extensions) {
    CancelScan();
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    m_isScanning = true;
    std::unordered_set<std::string> extSet(extensions.begin(), extensions.end());
    std::weak_ptr<DirectoryScanner> weakThis = weak_from_this();

    m_workerThread = std::jthread([weakThis, rootPath, extSet](std::stop_token stoken) {
        if (auto sharedThis = weakThis.lock()) {
            sharedThis->ScanThreadLogic(stoken, rootPath, extSet);
        }
    });
}

void DirectoryScanner::ScanThreadLogic(std::stop_token stoken, fs::path rootPath, std::unordered_set<std::string> extSet) {
    std::vector<FileEntry> results;

    try {
        if (fs::exists(rootPath) && fs::is_directory(rootPath)) {
            for (const auto& entry : fs::directory_iterator(rootPath)) {
                if (stoken.stop_requested()) break;

                bool isDir = entry.is_directory();
                std::string ext = entry.path().extension().string();

                if (isDir || extSet.empty() || extSet.contains(ext)) {
                    FileEntry fileEntry;
                    fileEntry.path = entry.path();
                    fileEntry.name = entry.path().filename().string();
                    fileEntry.isDirectory = isDir;

                    std::error_code ec;
                    fileEntry.size = isDir ? 0 : fs::file_size(entry.path(), ec);

                    results.push_back(fileEntry);
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        // Silently skip unreadable folders
    }

    // 3. Queue the event to the Main Thread if not cancelled
    if (!stoken.stop_requested()) {
        // Create the event
        wxThreadEvent* event = new wxThreadEvent(wxEVT_DIRECTORY_SCAN_COMPLETE);
        
        // Safely copy our std::vector into the wxWidgets event payload
        event->SetPayload(results);

        // Queue it to wxTheApp. This makes it a GLOBAL broadcast.
        wxQueueEvent(wxTheApp, event);
    }

    m_isScanning = false;
}