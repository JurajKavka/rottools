#pragma once

#include <functional>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include "DirectoryScanner.h"
#include "FileBrowserTreePanelWx.h"
#include "HelperFunctions.h"

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
    using SelectNewFolderNameCallback =
        std::function<std::optional<wxString>(const std::optional<wxString>& previousName)>;

    struct DeletePrompt {
        wxFileName path;
        bool isDirectory = false;
    };
    enum class DeleteMode { Trash, Permanent };
    using ConfirmDeleteCallback = std::function<bool(const std::vector<DeletePrompt>&, DeleteMode)>;
    using PathDeletedCallback = std::function<void(const DeletePrompt&)>;

    struct CreateFolderError {
        wxString name;
        std::error_code error;
    };
    using CreateFolderErrorCallback = std::function<void(const CreateFolderError&)>;

    struct DeleteError {
        wxFileName path;
        bool isDirectory = false;
        std::error_code error;
    };
    using DeleteErrorCallback = std::function<void(const std::vector<DeleteError>&, DeleteMode)>;

    struct Callbacks {
        FileOpenedCallback onFileOpened;
        DirectoryChangedCallback onDirectoryChanged;
        ActionRequestedCallback onHomeRequested;
        ActionRequestedCallback onCloseRequested;
        /** Returning nullopt cancels creation. previousName is nullopt on the first prompt. */
        SelectNewFolderNameCallback selectNewFolderName;
        /** Reports a failed folder creation so the host can present the error. */
        CreateFolderErrorCallback onCreateFolderError;
        /** The host confirms each Trash or permanent removal; false or an omitted callback cancels it. */
        ConfirmDeleteCallback confirmDelete;
        /** Called for each file or directory moved to Trash or completely deleted. */
        PathDeletedCallback onPathDeleted;
        /** Reports all failed removals together so the host can present one error. */
        DeleteErrorCallback onDeleteError;
    };

    /**
     * @param fileTypeFilter File type choices in context menu order. Each choice has a
     *        label and case-insensitive extensions (with or without a leading
     *        dot). The first choice is selected initially. An empty collection
     *        omits the File type submenu and shows all ordinary files. Use
     *        kFilterAllFiles for an explicit unfiltered choice.
     */
    explicit FileBrowserTreePanel(wxWindow* parent, Callbacks callbacks = {},
                                  std::vector<FileTypeFilter> fileTypeFilter = {});
    ~FileBrowserTreePanel();

    [[nodiscard]] static wxString GetTrashName();

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
    int m_fileTypeSelection = 0;
    wxFileName m_currentPath;
    std::vector<wxString> m_savedSelectionTexts;
    /// Text of the row at the top of the viewport, saved so a KeepPosition
    /// re-list can scroll back to it (item handles do not survive the rebuild)
    wxString m_savedTopItemText;
    /// Behavior for the scan in flight, applied when its results arrive
    ScrollBehavior m_scrollBehavior = ScrollBehavior::ResetToTop;

    Callbacks m_callbacks;

    void UpdateTree(const std::vector<FileEntry>& entries);
    /// Finds the top-level row with the given text; invalid item if none match
    wxDataViewItem FindChildByText(const wxString& text) const;
    /// Resolves a file-browser row, including "..", to its filesystem path.
    [[nodiscard]] wxFileName ResolveItemPath(const wxDataViewItem& item) const;
    void OpenPath(const wxFileName& path);
    void CopyPath(const wxFileName& path);
    void CreateFolder();
    [[nodiscard]] std::vector<DeletePrompt> GetSelectedDeletePaths() const;
    void DeletePaths(const std::vector<DeletePrompt>& paths, DeleteMode mode);
    void HandleDirectoryScanComplete(DirectoryScannerEvent& event);
    void SetShowHiddenFiles(bool showHiddenFiles);
    void SetFileTypeSelection(int selection);
    void ApplySelectedFileType();
    void HandleItemActivated(wxDataViewEvent& event);
    void HandleItemContextMenu(wxDataViewEvent& event);
    void HandleTreeKeyDown(wxKeyEvent& event);
    void ShowBrowserContextMenu(wxWindow* owner, const wxFileName& path, const std::vector<DeletePrompt>& deletePaths);
};
