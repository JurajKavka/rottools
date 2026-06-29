#include "FileBrowserTreePanel.h"

#include <wx/artprov.h>
#include <wx/imaglist.h>

#include <algorithm>

FileBrowserTreePanel::FileBrowserTreePanel(wxWindow* parent) : FileBrowserTreePanelWx(parent) {
    wxTheApp->Bind(wxEVT_DIRECTORY_SCAN_COMPLETE, &FileBrowserTreePanel::OnDirectoryScanComplete, this);

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
    wxTheApp->Unbind(wxEVT_DIRECTORY_SCAN_COMPLETE, &FileBrowserTreePanel::OnDirectoryScanComplete, this);
};

std::vector<FileEntry> FileBrowserTreePanel::SortEntries(const std::vector<FileEntry>& entries) const {
    // 1. Create a copy so we don't destroy the original data
    // mozem pouzit std::move a zahodit povodne data
    std::vector<FileEntry> sorted = entries;

    std::sort(sorted.begin(), sorted.end(), [](const FileEntry& a, const FileEntry& b) {
        // Tier 1: Group Directories at the top
        if (a.isDirectory != b.isDirectory) {
            return a.isDirectory > b.isDirectory;
        }
        // Tier 2: Alphabetical sort
        return a.name < b.name;
    });

    // 3. Return the sorted vector by value
    // C++11 and later use "Move Semantics," so this is very efficient!
    return sorted;
}

void FileBrowserTreePanel::UpdateTree(const std::vector<FileEntry>& entries) {
    // 2. Call your private helper
    auto sortedData = SortEntries(entries);
    // 3. Populate the UI
    m_dataViewTreeCtrl1->DeleteAllItems();

    wxDataViewItem root;

    for (const auto& entry : sortedData) {
        m_dataViewTreeCtrl1->AppendItem(root, entry.name, entry.isDirectory ? 0 : 1);
    }
}

void FileBrowserTreePanel::ListDir(const fs::path& filePath) {
    std::vector<std::string> noFilters = {};

    m_directoryScanner->StartScan(filePath, noFilters);
}

void FileBrowserTreePanel::OnDirectoryScanComplete(wxThreadEvent& event) {
    auto files = event.GetPayload<std::vector<FileEntry>>();

    UpdateTree(files);
}
