#pragma once

#include <wx/wx.h>

#include <atomic>
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
    void StartScan(const wxFileName& fileName, const ScanOptions& options, wxEvtHandler* eventTarget);
    void CancelScan();
    bool IsScanning() const;
    static std::vector<FileEntry> SortEntries(const std::vector<FileEntry>& entries);

   private:
    // Plain std::thread + atomic flag instead of std::jthread/std::stop_token:
    // Apple's libc++ only ships jthread from Xcode 26.4, too new to require.
    std::thread m_workerThread;
    std::atomic<bool> m_stopRequested{false};
    std::atomic<bool> m_isScanning{false};
    void ScanThreadLogic(const wxFileName& fileName, const ScanOptions& options, wxEvtHandler* eventTarget);
};
