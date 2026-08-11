#include "FileBrowserTreePanel.h"

#include <wx/artprov.h>
#include <wx/imaglist.h>

#include <algorithm>

FileBrowserTreePanel::FileBrowserTreePanel(wxWindow* parent, FileOpenedCallback onFileOpened,
                                           DirectoryChangedCallback onDirectoryChanged,
                                           std::vector<std::string> extensions)
    : FileBrowserTreePanelWx(parent),
      m_onFileOpened(std::move(onFileOpened)),
      m_onDirectoryChanged(std::move(onDirectoryChanged)) {
    Bind(wxEVT_DIRECTORY_SCAN_COMPLETE, &FileBrowserTreePanel::HandleDirectoryScanComplete, this);
    m_hiddenFilesCheckbox->Bind(wxEVT_CHECKBOX, &FileBrowserTreePanel::HandleHiddenFilesCheckbox, this);
    m_dataViewTreeCtrl1->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &FileBrowserTreePanel::HandleItemActivated, this);

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
    wxDataViewItem item = event.GetItem();
    if (!item.IsOk()) {
        return;
    }

    // Extract the string label of the row that was clicked
    wxString itemText = m_dataViewTreeCtrl1->GetItemText(item);

    // Case 1: Going up a level ("..")
    if (itemText == "..") {
        wxFileName parentPath = m_currentPath;

        // wxFileName::Up() cleanly pops the last directory component off the path stack
        if (parentPath.GetDirCount() > 0) {
            parentPath.RemoveLastDir();
        }

        ListDir(parentPath);
    }
    // Case 2: Attempting to go down into a folder
    else {
        // Construct the combined prospective target path purely using the wxFileName API
        wxFileName targetPath = m_currentPath;
        targetPath.AppendDir(itemText);

        // Only trigger a re-scan if the clicked item is actually an accessible folder
        if (targetPath.DirExists()) {
            ListDir(targetPath);
        } else {
            wxFileName filePath = m_currentPath;
            filePath.SetFullName(itemText);

            if (filePath.FileExists()) {
                if (m_onFileOpened) {
                    m_onFileOpened(filePath);
                }
            }
        }
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
