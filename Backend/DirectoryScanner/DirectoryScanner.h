#pragma once

#include <wx/wx.h>  // Required for wxThreadEvent and Event Macros

#include <atomic>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "HelperFunctions.h"

namespace fs = std::filesystem;

// Structure to hold data
struct FileEntry {
    fs::path path;
    std::string name;
    bool isDirectory;
    uintmax_t size;
};

// 1. Declare the custom event type so the rest of your app can see it
wxDECLARE_EVENT(wxEVT_DIRECTORY_SCAN_COMPLETE, wxThreadEvent);

class DirectoryScanner : public std::enable_shared_from_this<DirectoryScanner> {
   public:
    DirectoryScanner();
    ~DirectoryScanner();

    // The callback parameter is gone. It now just takes the path and filters.
    void StartScan(const fs::path& rootPath, const std::vector<std::string>& extensions, wxEvtHandler* eventTarget);

    void CancelScan();
    bool IsScanning() const;

   private:
    void ScanThreadLogic(std::stop_token stoken, fs::path rootPath, std::unordered_set<std::string> extSet, wxEvtHandler* eventTarget);

    std::jthread m_workerThread;
    std::atomic<bool> m_isScanning{false};
};