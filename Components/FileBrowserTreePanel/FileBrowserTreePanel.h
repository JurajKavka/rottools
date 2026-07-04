#pragma once

#include <wx/fswatcher.h>

#include "DirectoryScanner.h"
#include "FileBrowserTreePanelWx.h"
#include "HelperFunctions.h"

class DirectoryScanner;

class FileBrowserTreePanel : public FileBrowserTreePanelWx {
   public:
    using FileOpenedCallback = std::function<void(const wxFileName&)>;
    using DirectoryChangedCallback = std::function<void(const wxFileName&)>;
    explicit FileBrowserTreePanel(wxWindow* parent, FileOpenedCallback onFileOpened = nullptr, DirectoryChangedCallback onDirectoryChanged = nullptr);
    ~FileBrowserTreePanel();
    void ListDir(const wxFileName fileName);
    const void ReloadCurrentDir();

   private:
    std::shared_ptr<DirectoryScanner> m_directoryScanner;
    ScanOptions m_scanOptions;
    wxFileName m_currentPath;
    wxString m_savedSelectionText;

    FileOpenedCallback m_onFileOpened;
    DirectoryChangedCallback m_onDirectoryChanged;

    void UpdateTree(const std::vector<FileEntry>& entries);
    void OnDirectoryScanComplete(DirectoryScannerEvent& event);
    void OnHiddenFilesCheckbox(wxCommandEvent& event);
    void OnItemActivated(wxDataViewEvent& event);
    void OnFileSystemEvent(wxFileSystemWatcherEvent& event);
};
