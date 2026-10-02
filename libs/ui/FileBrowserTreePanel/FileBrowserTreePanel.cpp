#include "FileBrowserTreePanel.h"

#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/imaglist.h>
#include <wx/menu.h>

#include <algorithm>
#include <cstddef>
#include <utility>

#include "FileManagerBackend.h"

FileBrowserTreePanel::FileBrowserTreePanel(wxWindow* parent, Callbacks callbacks,
                                           std::vector<FileTypeFilter> fileTypeFilter)
    : FileBrowserTreePanelWx(parent), m_fileTypeFilters(std::move(fileTypeFilter)), m_callbacks(std::move(callbacks)) {
    Bind(wxEVT_DIRECTORY_SCAN_COMPLETE, &FileBrowserTreePanel::HandleDirectoryScanComplete, this);
    m_dataViewTreeCtrl1->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &FileBrowserTreePanel::HandleItemActivated, this);
    m_dataViewTreeCtrl1->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &FileBrowserTreePanel::HandleItemContextMenu, this);

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

    if (!m_savedSelectionText.IsEmpty()) {
        wxDataViewItem selection = FindChildByText(m_savedSelectionText);
        if (selection.IsOk()) {
            m_dataViewTreeCtrl1->Select(selection);
            // Pulling the selection into view would defeat KeepPosition: the
            // user may have scrolled away from it deliberately.
            if (m_scrollBehavior == ScrollBehavior::ResetToTop) {
                m_dataViewTreeCtrl1->EnsureVisible(selection);
            }
        }
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

    wxDataViewItem currentSelection = m_dataViewTreeCtrl1->GetSelection();
    if (currentSelection.IsOk()) {
        m_savedSelectionText = m_dataViewTreeCtrl1->GetItemText(currentSelection);
    } else {
        m_savedSelectionText.clear();
    }

    m_savedTopItemText.clear();
    if (scrollBehavior == ScrollBehavior::KeepPosition) {
        wxDataViewItem topItem = m_dataViewTreeCtrl1->GetTopItem();
        if (topItem.IsOk()) {
            m_savedTopItemText = m_dataViewTreeCtrl1->GetItemText(topItem);
        }
    }

    // Force the path to interpret its entire string structure as a directory.
    // It is probably not needed when working with `wxFileName` API ...
    m_currentPath = wxFileName::DirName(fileName.GetFullPath());

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
    m_savedSelectionText = absoluteFile.GetFullName();
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

void FileBrowserTreePanel::DeletePath(const wxFileName& path) {
    if (!m_callbacks.confirmDelete) {
        return;
    }

    const bool isDirectory = !path.FileExists() && path.DirExists();
    if (!m_callbacks.confirmDelete({path, isDirectory})) {
        return;
    }

    const auto result = FileManagerBackend::Delete(ToFilesystemPath(path.GetFullPath()));
    if (result.Succeeded()) {
        if (m_callbacks.onPathDeleted) {
            m_callbacks.onPathDeleted({path, isDirectory});
        }
    } else if (m_callbacks.onDeleteError) {
        m_callbacks.onDeleteError({path, isDirectory, result.error});
    }
    ReloadCurrentDir();
}

void FileBrowserTreePanel::HandleItemContextMenu(wxDataViewEvent& event) {
    const wxDataViewItem item = event.GetItem();
    const wxFileName path = ResolveItemPath(item);
    if (path.IsOk()) {
        m_dataViewTreeCtrl1->Select(item);
    }

    const bool canDelete = m_callbacks.confirmDelete && path.IsOk() && m_dataViewTreeCtrl1->GetItemText(item) != "..";
    ShowBrowserContextMenu(m_dataViewTreeCtrl1, path, canDelete);
}

void FileBrowserTreePanel::ShowBrowserContextMenu(wxWindow* owner, const wxFileName& path, bool canDelete) {
    wxMenu menu;
    const int closeId = rottools::ui::PrependCloseMenuItem(menu)->GetId();
    menu.AppendSeparator();

    int openId = wxID_NONE;
    int copyPathId = wxID_NONE;
    int deleteId = wxID_NONE;
    // Custom IDs avoid macOS applying responder-chain validation for stock
    // commands such as wxID_COPY and disabling the item.
    if (path.IsOk()) {
        openId = menu.Append(wxID_ANY, _("Open"))->GetId();
        copyPathId = menu.Append(wxID_ANY, _("Copy Path"))->GetId();
        if (canDelete) {
            deleteId = menu.Append(wxID_ANY, _("Delete"))->GetId();
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
    } else if (canDelete && selection == deleteId) {
        DeletePath(path);
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
