#pragma once

#include <wx/wx.h>  
#include <wx/filename.h>
#include <atomic>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

// #include "HelperFunctions.h"

namespace fs = std::filesystem;

struct FileEntry {
    fs::path path;
    std::string name;
    bool isDirectory;
    uintmax_t size;
};

struct ScanOptions {
    std::vector<std::string> extensions;
    bool showHiddenFiles = false;
};

wxDECLARE_EVENT(wxEVT_DIRECTORY_SCAN_COMPLETE, wxThreadEvent);

class DirectoryScanner : public std::enable_shared_from_this<DirectoryScanner> {
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
    void ScanThreadLogic(std::stop_token stoken, fs::path rootPath, ScanOptions options,
                         wxEvtHandler* eventTarget);
};