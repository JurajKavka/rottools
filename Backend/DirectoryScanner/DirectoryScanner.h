#pragma once

#include <wx/wx.h>

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>
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
    void StartScan(const wxFileName fileName, const ScanOptions& options, wxEvtHandler* eventTarget);
    void CancelScan();
    bool IsScanning() const;
    std::vector<FileEntry> SortEntries(const std::vector<FileEntry>& entries) const;

   private:
    std::jthread m_workerThread;
    std::atomic<bool> m_isScanning{false};
    void ScanThreadLogic(std::stop_token stoken, const wxFileName fileName, ScanOptions options, wxEvtHandler* eventTarget);
};