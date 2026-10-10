#include "FileBrowserTreePanel.h"

#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/filefn.h>
#include <wx/imaglist.h>
#include <wx/menu.h>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <iterator>
#include <utility>

#include "FileManagerBackend.h"

#if defined(__WXGTK__) && !defined(wxHAS_MOVE_TO_TRASH)
#include <gio/gio.h>
#endif

namespace {
std::error_code MovePathToTrash(const wxFileName& path) {
#ifdef wxHAS_MOVE_TO_TRASH
    return wxMoveToTrash(path.GetFullPath()) ? std::error_code{} : std::make_error_code(std::errc::io_error);
#elif defined(__WXGTK__)
    const std::filesystem::path nativePath = ToFilesystemPath(path.GetFullPath());
    GFile* file = g_file_new_for_path(nativePath.c_str());
    GError* nativeError = nullptr;
    const bool moved = g_file_trash(file, nullptr, &nativeError);
    g_object_unref(file);
    if (moved) {
        return {};
    }

    std::error_code error = std::make_error_code(std::errc::io_error);
    if (g_error_matches(nativeError, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED)) {
        error = std::make_error_code(std::errc::operation_not_supported);
    } else if (g_error_matches(nativeError, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED)) {
        error = std::make_error_code(std::errc::permission_denied);
    } else if (g_error_matches(nativeError, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
        error = std::make_error_code(std::errc::no_such_file_or_directory);
    }
    g_clear_error(&nativeError);
    return error;
#else
    (void)path;
    return std::make_error_code(std::errc::operation_not_supported);
#endif
}
}  // namespace

FileBrowserTreePanel::FileBrowserTreePanel(wxWindow* parent, Callbacks callbacks,
                                           std::vector<FileTypeFilter> fileTypeFilter)
    : FileBrowserTreePanelWx(parent), m_fileTypeFilters(std::move(fileTypeFilter)), m_callbacks(std::move(callbacks)) {
    Bind(wxEVT_DIRECTORY_SCAN_COMPLETE, &FileBrowserTreePanel::HandleDirectoryScanComplete, this);
    m_dataViewTreeCtrl1->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &FileBrowserTreePanel::HandleItemActivated, this);
    m_dataViewTreeCtrl1->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &FileBrowserTreePanel::HandleItemContextMenu, this);
    m_dataViewTreeCtrl1->Bind(wxEVT_CHAR_HOOK, &FileBrowserTreePanel::HandleTreeKeyDown, this);

    // Use the first supplied file type for the initial scan.
    ApplySelectedFileType();

    // 2. Create an Image List (16x16 is the standard size for tree nodes)
    // The 'true' parameter means it supports transparency (alpha channels)
    wxImageList* imageList = new wxImageList(16, 16, true);

    // 3. Fetch native OS icons and add them to the list
    // Index 0: The Folder Icon
    imageList->Add(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16, 16)));

    // Index 1: The File Icon
    imageList->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16, 16)));

    // 4. Assign the image list to your tree control
    // 'AssignImageList' tells the tree to take ownership, so you don't need to 'delete' it later
    m_dataViewTreeCtrl1->AssignImageList(imageList);
}

FileBrowserTreePanel::~FileBrowserTreePanel() {
    Unbind(wxEVT_DIRECTORY_SCAN_COMPLETE, &FileBrowserTreePanel::HandleDirectoryScanComplete, this);
}

wxString FileBrowserTreePanel::GetTrashName() {
#ifdef __WXMSW__
    return _("Recycle Bin");
#else
    return _("Trash");
#endif
}

wxDataViewItem FileBrowserTreePanel::FindChildByText(const wxString& text) const {
    wxDataViewItem root;
    int count = m_dataViewTreeCtrl1->GetChildCount(root);
    for (int i = 0; i < count; ++i) {
        wxDataViewItem child = m_dataViewTreeCtrl1->GetNthChild(root, i);
        if (child.IsOk() && m_dataViewTreeCtrl1->GetItemText(child) == text) {
            return child;
        }
    }
    return wxDataViewItem();
}

void FileBrowserTreePanel::UpdateTree(const std::vector<FileEntry>& entries) {
    auto sortedData = DirectoryScanner::SortEntries(entries);

    m_dataViewTreeCtrl1->DeleteAllItems();

    wxDataViewItem root;

    m_dataViewTreeCtrl1->AppendItem(root, "..", 0);

    for (const auto& entry : sortedData) {
        m_dataViewTreeCtrl1->AppendItem(root, entry.name, entry.isDirectory ? 0 : 1);
    }

    wxDataViewItemArray selections;
    for (const wxString& text : m_savedSelectionTexts) {
        wxDataViewItem selection = FindChildByText(text);
        if (selection.IsOk()) {
            selections.Add(selection);
        }
    }
    m_dataViewTreeCtrl1->SetSelections(selections);
    // Pulling a selection into view would defeat KeepPosition: the user may
    // have scrolled away from it deliberately.
    if (m_scrollBehavior == ScrollBehavior::ResetToTop && !selections.IsEmpty()) {
        m_dataViewTreeCtrl1->EnsureVisible(selections[0]);
    }

    if (m_scrollBehavior == ScrollBehavior::KeepPosition && !m_savedTopItemText.IsEmpty()) {
        wxDataViewItem topItem = FindChildByText(m_savedTopItemText);
        if (topItem.IsOk()) {
            // EnsureVisible alone would leave the row at the bottom edge (the
            // rebuilt view starts at the top). Scrolling to the end first makes
            // the second call approach from below, which puts the row back at
            // the top edge. Both run before the next paint, so only the final
            // position is ever drawn.
            int count = m_dataViewTreeCtrl1->GetChildCount(root);
            if (count > 0) {
                m_dataViewTreeCtrl1->EnsureVisible(m_dataViewTreeCtrl1->GetNthChild(root, count - 1));
            }
            m_dataViewTreeCtrl1->EnsureVisible(topItem);
        }
    }
}

void FileBrowserTreePanel::ListDir(const wxFileName& fileName, ScrollBehavior scrollBehavior) {
    m_scrollBehavior = scrollBehavior;

    // Item names identify selections only within their current directory.
    const wxFileName requestedDirectory = wxFileName::DirName(fileName.GetFullPath());
    const bool sameDirectory = m_currentPath.IsOk() && m_currentPath.SameAs(requestedDirectory);
    m_savedSelectionTexts.clear();
    if (sameDirectory) {
        wxDataViewItemArray selections;
        m_dataViewTreeCtrl1->GetSelections(selections);
        std::transform(selections.begin(), selections.end(), std::back_inserter(m_savedSelectionTexts),
                       [this](const wxDataViewItem& selection) { return m_dataViewTreeCtrl1->GetItemText(selection); });
    }

    m_savedTopItemText.clear();
    if (sameDirectory && scrollBehavior == ScrollBehavior::KeepPosition) {
        wxDataViewItem topItem = m_dataViewTreeCtrl1->GetTopItem();
        if (topItem.IsOk()) {
            m_savedTopItemText = m_dataViewTreeCtrl1->GetItemText(topItem);
        }
    }

    // Force the path to interpret its entire string structure as a directory.
    // It is probably not needed when working with `wxFileName` API ...
    m_currentPath = requestedDirectory;

    m_directoryScanner.StartScan(fileName, m_scanOptions, this);
}

/**
 * Lists a file's containing directory and selects that file after the scan.
 *
 * Use this when another part of the application opens or saves a document so
 * the browser follows the active file even when it lives in another directory.
 */
void FileBrowserTreePanel::ShowFile(const wxFileName& fileName) {
    wxFileName absoluteFile(fileName);
    absoluteFile.MakeAbsolute();

    m_scrollBehavior = ScrollBehavior::ResetToTop;
    m_savedSelectionTexts = {absoluteFile.GetFullName()};
    m_savedTopItemText.clear();
    m_currentPath = wxFileName::DirName(absoluteFile.GetPath());
    m_directoryScanner.StartScan(m_currentPath, m_scanOptions, this);
}

wxFileName FileBrowserTreePanel::GetCurrentDirectory() const {
    return m_currentPath;
}

void FileBrowserTreePanel::HandleDirectoryScanComplete(DirectoryScannerEvent& event) {
    UpdateTree(event.files);
    if (m_callbacks.onDirectoryChanged) {
        m_callbacks.onDirectoryChanged(event.currentDirectory);
    }
}

void FileBrowserTreePanel::SetShowHiddenFiles(bool showHiddenFiles) {
    m_scanOptions.showHiddenFiles = showHiddenFiles;

    if (m_currentPath.IsOk()) {
        ListDir(m_currentPath);
    }
}

void FileBrowserTreePanel::SetFileTypeSelection(int selection) {
    if (selection < 0 || static_cast<std::size_t>(selection) >= m_fileTypeFilters.size()) {
        return;
    }

    m_fileTypeSelection = selection;
    ApplySelectedFileType();
    if (m_currentPath.IsOk()) {
        ListDir(m_currentPath);
    }
}

void FileBrowserTreePanel::ApplySelectedFileType() {
    m_scanOptions.extensions.clear();

    if (m_fileTypeFilters.empty()) {
        return;
    }

    const auto& extensions = m_fileTypeFilters[static_cast<std::size_t>(m_fileTypeSelection)].extensions;
    if (std::ranges::find(extensions, kFilterAllFiles) != extensions.end()) {
        return;
    }

    for (const std::string& extension : extensions) {
        if (!extension.empty()) {
            // DirectoryScanner compares against path::extension(), which includes the dot.
            m_scanOptions.extensions.push_back(extension.front() == '.' ? extension : "." + extension);
        }
    }
}

void FileBrowserTreePanel::HandleItemActivated(wxDataViewEvent& event) {
    const wxFileName path = ResolveItemPath(event.GetItem());
    if (!path.IsOk()) {
        return;
    }

    OpenPath(path);
}

wxFileName FileBrowserTreePanel::ResolveItemPath(const wxDataViewItem& item) const {
    if (!item.IsOk() || !m_currentPath.IsOk()) {
        return {};
    }

    const wxString itemText = m_dataViewTreeCtrl1->GetItemText(item);
    if (itemText == "..") {
        wxFileName parentPath = m_currentPath;
        if (parentPath.GetDirCount() > 0) {
            parentPath.RemoveLastDir();
        }
        return parentPath;
    }

    wxFileName directoryPath = m_currentPath;
    directoryPath.AppendDir(itemText);
    if (directoryPath.DirExists()) {
        return directoryPath;
    }

    wxFileName filePath = m_currentPath;
    filePath.SetFullName(itemText);
    if (filePath.FileExists()) {
        return filePath;
    }

    return {};
}

void FileBrowserTreePanel::OpenPath(const wxFileName& path) {
    // DirExists() checks the directory portion of a wxFileName, which also
    // exists for an ordinary file. Test the complete file path first.
    if (path.FileExists()) {
        if (m_callbacks.onFileOpened) {
            m_callbacks.onFileOpened(path);
        }
    } else if (path.DirExists()) {
        ListDir(path);
    }
}

void FileBrowserTreePanel::CopyPath(const wxFileName& path) {
    wxClipboardLocker clipboard;
    if (!clipboard) {
        return;
    }

    wxTheClipboard->SetData(new wxTextDataObject(path.GetFullPath()));
}

void FileBrowserTreePanel::CreateFolder() {
    if (!m_callbacks.selectNewFolderName) {
        return;
    }

    const wxFileName parent = m_currentPath;
    std::optional<wxString> previousName;
    while (true) {
        const std::optional<wxString> name = m_callbacks.selectNewFolderName(previousName);
        if (!name) {
            return;
        }
        previousName = name;
        const auto result =
            FileManagerBackend::CreateNewDirectory(ToFilesystemPath(parent.GetFullPath()), ToFilesystemPath(*name));
        if (result.Succeeded()) {
            if (IsShowingDir(parent)) {
                ReloadCurrentDir();
            }
            return;
        }

        if (!m_callbacks.onCreateFolderError) {
            return;
        }
        m_callbacks.onCreateFolderError({*name, result.error});
    }
}

std::vector<FileBrowserTreePanel::DeletePrompt> FileBrowserTreePanel::GetSelectedDeletePaths() const {
    wxDataViewItemArray selections;
    m_dataViewTreeCtrl1->GetSelections(selections);

    std::vector<DeletePrompt> paths;
    paths.reserve(selections.size());
    for (const wxDataViewItem& item : selections) {
        if (m_dataViewTreeCtrl1->GetItemText(item) == "..") {
            continue;
        }

        const wxFileName path = ResolveItemPath(item);
        if (path.IsOk()) {
            paths.push_back({path, !path.FileExists() && path.DirExists()});
        }
    }
    return paths;
}

void FileBrowserTreePanel::DeletePaths(const std::vector<DeletePrompt>& paths, DeleteMode mode) {
    if (paths.empty() || !m_callbacks.confirmDelete || !m_callbacks.confirmDelete(paths, mode)) {
        return;
    }

    std::vector<FileManagerBackend::Result> permanentResults;
    if (mode == DeleteMode::Permanent) {
        std::vector<std::filesystem::path> sources;
        sources.reserve(paths.size());
        std::transform(paths.begin(), paths.end(), std::back_inserter(sources),
                       [](const DeletePrompt& path) { return ToFilesystemPath(path.path.GetFullPath()); });
        permanentResults = FileManagerBackend::DeleteMany(sources);
    }

    std::vector<DeleteError> errors;
    for (std::size_t index = 0; index < paths.size(); ++index) {
        std::error_code error;
        if (mode == DeleteMode::Trash) {
            error = MovePathToTrash(paths[index].path);
        } else {
            error = permanentResults[index].error;
        }
        if (!error) {
            if (m_callbacks.onPathDeleted) {
                m_callbacks.onPathDeleted(paths[index]);
            }
        } else {
            errors.push_back({paths[index].path, paths[index].isDirectory, error});
        }
    }
    if (!errors.empty() && m_callbacks.onDeleteError) {
        m_callbacks.onDeleteError(errors, mode);
    }
    ReloadCurrentDir();
}

void FileBrowserTreePanel::HandleItemContextMenu(wxDataViewEvent& event) {
    const wxDataViewItem item = event.GetItem();
    const wxFileName path = ResolveItemPath(item);
    if (path.IsOk() && (m_dataViewTreeCtrl1->GetItemText(item) == ".." || !m_dataViewTreeCtrl1->IsSelected(item))) {
        m_dataViewTreeCtrl1->UnselectAll();
        m_dataViewTreeCtrl1->Select(item);
    }

    ShowBrowserContextMenu(m_dataViewTreeCtrl1, path,
                           path.IsOk() ? GetSelectedDeletePaths() : std::vector<DeletePrompt>{});
}

void FileBrowserTreePanel::HandleTreeKeyDown(wxKeyEvent& event) {
    const int key = event.GetKeyCode();
    std::optional<DeleteMode> mode;
#ifdef __WXOSX__
    // Finder: Command+Delete moves to Trash; Option+Command+Delete removes immediately.
    if (key == WXK_BACK && event.CmdDown() && !event.ShiftDown() && !event.ControlDown()) {
        mode = event.AltDown() ? DeleteMode::Permanent : DeleteMode::Trash;
    }
#else
    // Windows and Linux file managers: Delete uses Trash, Shift+Delete bypasses it.
    if (key == WXK_DELETE && !event.CmdDown() && !event.AltDown()) {
        mode = event.ShiftDown() ? DeleteMode::Permanent : DeleteMode::Trash;
    }
#endif
    if (mode && m_callbacks.confirmDelete) {
        const auto paths = GetSelectedDeletePaths();
        if (!paths.empty()) {
            DeletePaths(paths, *mode);
            return;
        }
    }
    event.Skip();
}

void FileBrowserTreePanel::ShowBrowserContextMenu(wxWindow* owner, const wxFileName& path,
                                                  const std::vector<DeletePrompt>& deletePaths) {
    wxMenu menu;
    const int closeId = rottools::ui::PrependCloseMenuItem(menu)->GetId();
    menu.AppendSeparator();

    int openId = wxID_NONE;
    int copyPathId = wxID_NONE;
    int trashId = wxID_NONE;
    // Custom IDs avoid macOS applying responder-chain validation for stock
    // commands such as wxID_COPY and disabling the item.
    if (path.IsOk()) {
        if (m_dataViewTreeCtrl1->GetSelectedItemsCount() == 1) {
            openId = menu.Append(wxID_ANY, _("Open"))->GetId();
            copyPathId = menu.Append(wxID_ANY, _("Copy Path"))->GetId();
        }
        if (m_callbacks.confirmDelete && !deletePaths.empty()) {
            const wxString trashName = GetTrashName();
            wxString label = wxString::Format(_("Move to %s"), trashName.c_str());
            if (deletePaths.size() > 1) {
                label = wxString::Format(_("Move %zu Items to %s"), deletePaths.size(), trashName.c_str());
            }
            trashId = menu.Append(wxID_ANY, label)->GetId();
        }
        menu.AppendSeparator();
    }
    const int newFolderId = menu.Append(wxID_ANY, _("New Folder"))->GetId();
    menu.Enable(newFolderId, m_callbacks.selectNewFolderName && m_currentPath.IsOk() && m_currentPath.DirExists());
    menu.AppendSeparator();

    const int homeId = menu.Append(wxID_ANY, _("Home"))->GetId();
    menu.AppendSeparator();

    std::vector<int> fileTypeIds;
    if (!m_fileTypeFilters.empty()) {
        auto* fileTypeMenu = new wxMenu;
        for (std::size_t index = 0; index < m_fileTypeFilters.size(); ++index) {
            wxMenuItem* item = fileTypeMenu->AppendRadioItem(wxID_ANY, m_fileTypeFilters[index].label);
            fileTypeIds.push_back(item->GetId());
            if (static_cast<int>(index) == m_fileTypeSelection) {
                item->Check(true);
            }
        }
        menu.AppendSubMenu(fileTypeMenu, _("File type"));
    }

    const int showHiddenFilesId = menu.AppendCheckItem(wxID_ANY, _("Show hidden files"))->GetId();
    menu.Check(showHiddenFilesId, m_scanOptions.showHiddenFiles);

    const int selection = owner->GetPopupMenuSelectionFromUser(menu);
    if (selection == closeId && m_callbacks.onCloseRequested) {
        m_callbacks.onCloseRequested();
    } else if (path.IsOk() && selection == openId) {
        OpenPath(path);
    } else if (path.IsOk() && selection == copyPathId) {
        CopyPath(path);
    } else if (trashId != wxID_NONE && selection == trashId) {
        DeletePaths(deletePaths, DeleteMode::Trash);
    } else if (selection == newFolderId) {
        CreateFolder();
    } else if (selection == homeId && m_callbacks.onHomeRequested) {
        m_callbacks.onHomeRequested();
    } else if (selection == showHiddenFilesId) {
        SetShowHiddenFiles(!m_scanOptions.showHiddenFiles);
    } else if (const auto fileType = std::ranges::find(fileTypeIds, selection); fileType != fileTypeIds.end()) {
        SetFileTypeSelection(static_cast<int>(fileType - fileTypeIds.begin()));
    }
}

bool FileBrowserTreePanel::IsShowingDir(const wxFileName& dir) const {
    return m_currentPath.IsOk() && m_currentPath.SameAs(wxFileName::DirName(dir.GetFullPath()));
}

void FileBrowserTreePanel::ReloadCurrentDir() {
    if (m_currentPath.IsOk() && m_currentPath.DirExists()) {
        // A live reload after an fs event: the list refreshes under the user,
        // so the view must not jump to the top.
        ListDir(m_currentPath, ScrollBehavior::KeepPosition);
    }
}
