#pragma once

#include <functional>

#include "DirectoryScanner.h"
#include "FileBrowserTreePanelWx.h"
#include "HelperFunctions.h"

class FileBrowserTreePanel : public FileBrowserTreePanelWx {
   public:
    using FileOpenedCallback = std::function<void(const wxFileName&)>;
    using DirectoryChangedCallback = std::function<void(const wxFileName&)>;
    explicit FileBrowserTreePanel(wxWindow* parent, FileOpenedCallback onFileOpened = nullptr,
                                  DirectoryChangedCallback onDirectoryChanged = nullptr);
    ~FileBrowserTreePanel();
    void ListDir(const wxFileName& fileName);
    void ReloadCurrentDir();
    bool IsShowingDir(const wxFileName& dir) const;

   private:
    DirectoryScanner m_directoryScanner;
    ScanOptions m_scanOptions;
    wxFileName m_currentPath;
    wxString m_savedSelectionText;

    FileOpenedCallback m_onFileOpened;
    DirectoryChangedCallback m_onDirectoryChanged;

    void UpdateTree(const std::vector<FileEntry>& entries);
    void HandleDirectoryScanComplete(DirectoryScannerEvent& event);
    void HandleHiddenFilesCheckbox(wxCommandEvent& event);
    void HandleItemActivated(wxDataViewEvent& event);
};
