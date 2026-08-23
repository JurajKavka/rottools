#pragma once

#include <wx/wx.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "DirectoryScannerEvent.h"

struct ScanOptions {
    std::vector<std::string> extensions;
    bool showHiddenFiles = false;
};

wxDECLARE_EVENT(wxEVT_DIRECTORY_SCAN_COMPLETE, DirectoryScannerEvent);

class DirectoryScanner {
   public:
    DirectoryScanner();
    ~DirectoryScanner();
    [[nodiscard]] std::uint64_t StartScan(const wxFileName& fileName, const ScanOptions& options,
                                          wxEvtHandler* eventTarget);
    void CancelScan();
    bool IsScanning() const;
    static std::vector<FileEntry> SortEntries(const std::vector<FileEntry>& entries);

   private:
    struct ScanRequest {
        wxFileName directory;
        ScanOptions options;
        wxEvtHandler* eventTarget = nullptr;
        std::uint64_t scanId = 0;
        std::shared_ptr<std::atomic<bool>> cancellation;
    };

    mutable std::mutex m_mutex;
    std::condition_variable m_requestAvailable;
    bool m_stopping = false;
    std::optional<ScanRequest> m_pendingRequest;
    std::shared_ptr<std::atomic<bool>> m_activeCancellation;
    std::uint64_t m_nextScanId = 1;
    std::thread m_workerThread;

    void WorkerLoop() noexcept;
    static void ScanThreadLogic(const ScanRequest& request) noexcept;
};
