#pragma once

#include "DirectoryScanner.h"
#include "FileBrowserTreePanelWx.h"
#include "HelperFunctions.h"

class DirectoryScanner;

class FileBrowserTreePanel : public FileBrowserTreePanelWx {
   public:
    using FileOpenedCallback = std::function<void(const wxFileName&)>;
    explicit FileBrowserTreePanel(wxWindow* parent, FileOpenedCallback onFileOpened = nullptr);
    ~FileBrowserTreePanel();
    void ListDir(const wxFileName fileName);

   private:
    std::shared_ptr<DirectoryScanner> m_directoryScanner;
    ScanOptions m_scanOptions;
    wxFileName m_currentPath;

    FileOpenedCallback m_onFileOpened;

    void UpdateTree(const std::vector<FileEntry>& entries);
    void OnDirectoryScanComplete(wxThreadEvent& event);
    void OnHiddenFilesCheckbox(wxCommandEvent& event);
    void OnItemActivated(wxDataViewEvent& event);
};
