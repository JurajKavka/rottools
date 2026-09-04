#pragma once

#include <functional>
#include <string>
#include <vector>

#include "DirectoryScanner.h"
#include "FileBrowserTreePanelWx.h"
#include "HelperFunctions.h"

class FileBrowserTreePanel : public FileBrowserTreePanelWx {
   public:
    using FileOpenedCallback = std::function<void(const wxFileName&)>;
    using DirectoryChangedCallback = std::function<void(const wxFileName&)>;
    using ActionRequestedCallback = std::function<void()>;

    struct Callbacks {
        FileOpenedCallback onFileOpened;
        DirectoryChangedCallback onDirectoryChanged;
        ActionRequestedCallback onHomeRequested;
        ActionRequestedCallback onCloseRequested;
    };

    /**
     * @param extensions Case-insensitive file extensions to show. An empty
     *        collection shows all ordinary files.
     */
    explicit FileBrowserTreePanel(wxWindow* parent, Callbacks callbacks = {}, std::vector<std::string> extensions = {});
    ~FileBrowserTreePanel();

    /**
     * @brief Lists a directory in the tree.
     *
     * @param fileName Directory to list
     * @param scrollBehavior KeepPosition holds the scroll when re-listing the
     *        same directory (a live reload); the default starts at the top, as
     *        navigating to a new directory should.
     */
    void ListDir(const wxFileName& fileName, ScrollBehavior scrollBehavior = ScrollBehavior::ResetToTop);
    /** List the containing directory and select the given file when the scan completes. */
    void ShowFile(const wxFileName& fileName);
    [[nodiscard]] wxFileName GetCurrentDirectory() const;
    void ReloadCurrentDir();
    bool IsShowingDir(const wxFileName& dir) const;

   private:
    DirectoryScanner m_directoryScanner;
    ScanOptions m_scanOptions;
    wxFileName m_currentPath;
    wxString m_savedSelectionText;
    /// Text of the row at the top of the viewport, saved so a KeepPosition
    /// re-list can scroll back to it (item handles do not survive the rebuild)
    wxString m_savedTopItemText;
    /// Behavior for the scan in flight, applied when its results arrive
    ScrollBehavior m_scrollBehavior = ScrollBehavior::ResetToTop;

    FileOpenedCallback m_onFileOpened;
    DirectoryChangedCallback m_onDirectoryChanged;
    ActionRequestedCallback m_onHomeRequested;
    ActionRequestedCallback m_onCloseRequested;

    void UpdateTree(const std::vector<FileEntry>& entries);
    /// Finds the top-level row with the given text; invalid item if none match
    wxDataViewItem FindChildByText(const wxString& text) const;
    /// Resolves a file-browser row, including "..", to its filesystem path.
    [[nodiscard]] wxFileName ResolveItemPath(const wxDataViewItem& item) const;
    void OpenPath(const wxFileName& path);
    void CopyPath(const wxFileName& path);
    void HandleDirectoryScanComplete(DirectoryScannerEvent& event);
    void HandleHiddenFilesCheckbox(wxCommandEvent& event);
    void HandleItemActivated(wxDataViewEvent& event);
    void HandleItemContextMenu(wxDataViewEvent& event);
    void HandleHomeButtonClick(wxCommandEvent& event);
    void HandleCloseButtonClick(wxCommandEvent& event);
};
