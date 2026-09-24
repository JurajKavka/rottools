#pragma once

#include <functional>
#include <string>
#include <vector>

#include "DirectoryScanner.h"
#include "FileBrowserTreePanelWx.h"
#include "HelperFunctions.h"

class wxContextMenuEvent;

class FileBrowserTreePanel : public FileBrowserTreePanelWx {
   public:
    static constexpr char kFilterAllFiles[] = "*";

    struct FileTypeFilter {
        wxString label;
        std::vector<std::string> extensions;
    };

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
     * @param fileTypeFilter File type choices in display order. Each choice has a
     *        label and case-insensitive extensions (with or without a leading
     *        dot). The first choice is selected initially. An empty collection
     *        hides the file type dropdown and shows all ordinary files. Use
     *        kFilterAllFiles for an explicit unfiltered choice.
     */
    explicit FileBrowserTreePanel(wxWindow* parent, Callbacks callbacks = {},
                                  std::vector<FileTypeFilter> fileTypeFilter = {});
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
    std::vector<FileTypeFilter> m_fileTypeFilters;
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

    void UpdateTree(const std::vector<FileEntry>& entries);
    /// Finds the top-level row with the given text; invalid item if none match
    wxDataViewItem FindChildByText(const wxString& text) const;
    /// Resolves a file-browser row, including "..", to its filesystem path.
    [[nodiscard]] wxFileName ResolveItemPath(const wxDataViewItem& item) const;
    void OpenPath(const wxFileName& path);
    void CopyPath(const wxFileName& path);
    void HandleDirectoryScanComplete(DirectoryScannerEvent& event);
    void HandleHiddenFilesCheckbox(wxCommandEvent& event);
    void HandleFileTypeChoice(wxCommandEvent& event);
    void ApplySelectedFileType();
    void HandleItemActivated(wxDataViewEvent& event);
    void HandleItemContextMenu(wxDataViewEvent& event);
    void HandleHeaderContextMenu(wxContextMenuEvent& event);
    void ShowBrowserContextMenu(wxWindow* owner, const wxFileName& path);
};
