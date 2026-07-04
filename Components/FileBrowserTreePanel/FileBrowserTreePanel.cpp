#include "FileBrowserTreePanel.h"

#include <wx/artprov.h>
#include <wx/imaglist.h>

#include <algorithm>

FileBrowserTreePanel::FileBrowserTreePanel(wxWindow* parent, FileOpenedCallback onFileOpened, DirectoryChangedCallback onDirectoryChanged)
    : FileBrowserTreePanelWx(parent), m_onFileOpened(onFileOpened), m_onDirectoryChanged(onDirectoryChanged) {
    Bind(wxEVT_DIRECTORY_SCAN_COMPLETE, &FileBrowserTreePanel::OnDirectoryScanComplete, this);
    m_hiddenFilesCheckbox->Bind(wxEVT_CHECKBOX, &FileBrowserTreePanel::OnHiddenFilesCheckbox, this);
    m_dataViewTreeCtrl1->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &FileBrowserTreePanel::OnItemActivated, this);

    m_scanOptions.extensions = {".md"};
    m_scanOptions.showHiddenFiles = m_hiddenFilesCheckbox->IsChecked();

    m_directoryScanner = std::make_shared<DirectoryScanner>();

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


};

FileBrowserTreePanel::~FileBrowserTreePanel() {
    Unbind(wxEVT_DIRECTORY_SCAN_COMPLETE, &FileBrowserTreePanel::OnDirectoryScanComplete, this);
};

void FileBrowserTreePanel::UpdateTree(const std::vector<FileEntry>& entries) {
    auto sortedData = m_directoryScanner->SortEntries(entries);

    m_dataViewTreeCtrl1->DeleteAllItems();

    wxDataViewItem root;

    m_dataViewTreeCtrl1->AppendItem(root, "..", 0);

    for (const auto& entry : sortedData) {
        m_dataViewTreeCtrl1->AppendItem(root, entry.name, entry.isDirectory ? 0 : 1);
    }

    if (!m_savedSelectionText.IsEmpty()) {
        int count = m_dataViewTreeCtrl1->GetChildCount(root);
        for (int i = 0; i < count; ++i) {
            wxDataViewItem child = m_dataViewTreeCtrl1->GetNthChild(root, i);
            if (child.IsOk() && m_dataViewTreeCtrl1->GetItemText(child) == m_savedSelectionText) {
                m_dataViewTreeCtrl1->Select(child);
                m_dataViewTreeCtrl1->EnsureVisible(child);
                break;
            }
        }
    }
}

void FileBrowserTreePanel::ListDir(const wxFileName fileName) {
    wxDataViewItem currentSelection = m_dataViewTreeCtrl1->GetSelection();
    if (currentSelection.IsOk()) {
        m_savedSelectionText = m_dataViewTreeCtrl1->GetItemText(currentSelection);
    } else {
        m_savedSelectionText.clear();
    }
    // Force the path to interpret its entire string structure as a directory.
    // It is probably not needed when working with `wxFileName` API ...
    m_currentPath = wxFileName::DirName(fileName.GetFullPath());

    m_directoryScanner->StartScan(fileName, m_scanOptions, this);
}

void FileBrowserTreePanel::OnDirectoryScanComplete(DirectoryScannerEvent& event) {
    UpdateTree(event.files);
    if (m_onDirectoryChanged) {
        m_onDirectoryChanged(event.currentDirectory);
    }
}

void FileBrowserTreePanel::OnHiddenFilesCheckbox(wxCommandEvent& event) {
    // 1. Update the configuration state with the checkbox value
    m_scanOptions.showHiddenFiles = event.IsChecked();

    // 2. If a valid directory is currently being shown, re-scan it immediately
    // with the updated configuration layout
    if (m_currentPath.IsOk()) {
        ListDir(m_currentPath);
    }
}

void FileBrowserTreePanel::OnItemActivated(wxDataViewEvent& event) {
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

void FileBrowserTreePanel::ReloadCurrentDir() {
    if (m_currentPath.IsOk() && m_currentPath.DirExists()) {
        ListDir(m_currentPath);
    }
}
