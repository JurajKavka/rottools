#include "FileBrowserTreePanel.h"

#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/imaglist.h>
#include <wx/menu.h>

#include <algorithm>

FileBrowserTreePanel::FileBrowserTreePanel(wxWindow* parent, Callbacks callbacks, std::vector<std::string> extensions)
    : FileBrowserTreePanelWx(parent),
      m_onFileOpened(std::move(callbacks.onFileOpened)),
      m_onDirectoryChanged(std::move(callbacks.onDirectoryChanged)),
      m_onHomeRequested(std::move(callbacks.onHomeRequested)),
      m_onCloseRequested(std::move(callbacks.onCloseRequested)) {
    Bind(wxEVT_DIRECTORY_SCAN_COMPLETE, &FileBrowserTreePanel::HandleDirectoryScanComplete, this);
    m_hiddenFilesCheckbox->Bind(wxEVT_CHECKBOX, &FileBrowserTreePanel::HandleHiddenFilesCheckbox, this);
    m_dataViewTreeCtrl1->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &FileBrowserTreePanel::HandleItemActivated, this);
    m_dataViewTreeCtrl1->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &FileBrowserTreePanel::HandleItemContextMenu, this);
    m_homeButton->Bind(wxEVT_BUTTON, &FileBrowserTreePanel::HandleHomeButtonClick, this);
    m_closeButton->Bind(wxEVT_BUTTON, &FileBrowserTreePanel::HandleCloseButtonClick, this);

    m_scanOptions.extensions = std::move(extensions);
    m_scanOptions.showHiddenFiles = m_hiddenFilesCheckbox->IsChecked();

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
    if (m_onDirectoryChanged) {
        m_onDirectoryChanged(event.currentDirectory);
    }
}

void FileBrowserTreePanel::HandleHiddenFilesCheckbox(wxCommandEvent& event) {
    // 1. Update the configuration state with the checkbox value
    m_scanOptions.showHiddenFiles = event.IsChecked();

    // 2. If a valid directory is currently being shown, re-scan it immediately
    // with the updated configuration layout
    if (m_currentPath.IsOk()) {
        ListDir(m_currentPath);
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
        if (m_onFileOpened) {
            m_onFileOpened(path);
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

void FileBrowserTreePanel::HandleItemContextMenu(wxDataViewEvent& event) {
    const wxDataViewItem item = event.GetItem();
    const wxFileName path = ResolveItemPath(item);
    if (!path.IsOk()) {
        return;
    }

    m_dataViewTreeCtrl1->Select(item);

    wxMenu menu;
    // Custom IDs avoid macOS applying responder-chain validation for stock
    // commands such as wxID_COPY and disabling the item.
    const int openId = menu.Append(wxID_ANY, "Open")->GetId();
    const int copyPathId = menu.Append(wxID_ANY, "Copy Path")->GetId();

    const int selection = m_dataViewTreeCtrl1->GetPopupMenuSelectionFromUser(menu);
    if (selection == openId) {
        OpenPath(path);
    } else if (selection == copyPathId) {
        CopyPath(path);
    }
}

void FileBrowserTreePanel::HandleHomeButtonClick(wxCommandEvent& event) {
    if (m_onHomeRequested) {
        m_onHomeRequested();
    }
}

void FileBrowserTreePanel::HandleCloseButtonClick(wxCommandEvent& event) {
    if (m_onCloseRequested) {
        m_onCloseRequested();
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
