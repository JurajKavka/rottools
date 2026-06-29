#pragma once

#include "FileBrowserTreePanelWx.h"
#include "HelperFunctions.h"
#include "DirectoryScanner.h"

class DirectoryScanner;

class FileBrowserTreePanel : public FileBrowserTreePanelWx {
   private:
    std::shared_ptr<DirectoryScanner> m_directoryScanner;
    std::vector<FileEntry> SortEntries(const std::vector<FileEntry>& entries) const;
    void UpdateTree(const std::vector<FileEntry>& entries);
    void OnDirectoryScanComplete(wxThreadEvent& event);

   public:
    explicit FileBrowserTreePanel(wxWindow* parent);
    ~FileBrowserTreePanel();
    void ListDir(const fs::path& filePath);
};
