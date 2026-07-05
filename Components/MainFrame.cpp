#include "MainFrame.h"

#include <wx/dnd.h>      // Required for wxFileDropTarget
#include <wx/filedlg.h>  // Required for wxFileDialog
#include <wx/msgdlg.h>   // Required for wxMessageBox

#include <fstream>  // For opening the file
#include <sstream>  // For reading the file content

#include "FileDropTarget/FileDropTarget.h"
#include "HelperFunctions.h"
#include "WebViewPanel/WebViewPanel.h"

MainFrame::MainFrame(wxWindow* parent) : MainFrameWx(parent), m_markdownParser(this) {
    Bind(wxEVT_MENU, &MainFrame::HandleOpenFileMenuItemClick, this, wxID_OPEN);
    Bind(wxEVT_MENU, &MainFrame::HandleToggleFileBrowserMenuItemClick, this, wxID_TOGGLE_FILE_BROWSER_MENU_ITEM);
    Bind(wxEVT_TOOL, &MainFrame::HandleOpenFileMenuItemClick, this, fileOpenTool->GetId());

    Bind(EVT_MARKDOWN_READY, &MainFrame::OnMarkdownReady, this);
    Bind(EVT_MARKDOWN_ERROR, &MainFrame::OnMarkdownError, this);

    // 1. Instantiate the WebViewPanel, setting this frame as its parent
    m_mainSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_fileBrowserPanel = new FileBrowserTreePanel(m_mainSplitter, std::bind_front(&MainFrame::OpenMarkdownFile, this),
                                                  std::bind_front(&MainFrame::HandleDirectoryChanged, this));
    m_webViewPanel = new WebViewPanel(m_mainSplitter);

    m_mainSplitter->SplitVertically(m_fileBrowserPanel, m_webViewPanel, 100);
    m_mainSplitter->SetMinimumPaneSize(100);

    wxSizer* mainSizer = this->GetSizer();
    mainSizer->Add(m_mainSplitter, 1, wxEXPAND | wxALL, 0);

    // inital directory list
    wxFileName initialDirectory;
    initialDirectory.AssignHomeDir();
    m_fileBrowserPanel->ListDir(initialDirectory);

    // file system watcher
    m_fileSystemWatcher.SetOwner(this);
    Bind(wxEVT_FSWATCHER, &MainFrame::HandleFileSystemWatcherEvent, this);

    Layout();

    // 4. Register drag and drop targets
    this->SetDropTarget(new FileDropTarget(std::bind_front(&MainFrame::OpenMarkdownFile, this)));
}

void MainFrame::HandleOpenFileMenuItemClick(wxCommandEvent& event) {
    wxFileDialog openFileDialog(
        this,                  // Parent window
        "Open Markdown File",  // Dialog Title
        "",                    // Default directory (empty means current)
        "",                    // Default filename
        "Markdown files (*.md;*.markdown)|*.md;*.markdown|All files (*.*)|*.*",  // File extensions filter
        wxFD_OPEN | wxFD_FILE_MUST_EXIST  // Flags: Open mode & force file existence
    );

    if (openFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    OpenMarkdownFile(wxFileName(openFileDialog.GetPath()));
}

void MainFrame::HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event) {
    if (m_mainSplitter->IsSplit()) {
        // Hides the file browser panel and expands the web view
        m_mainSplitter->Unsplit(m_fileBrowserPanel);
    } else {
        // Restores the file browser on the left with a width of 200 pixels
        m_mainSplitter->SplitVertically(m_fileBrowserPanel, m_webViewPanel, 200);
    }
}

void MainFrame::OnMarkdownReady(MarkdownToHtmlAsyncEvent& event) {
    m_webViewPanel->LoadHtml(event.html);
    statusBar->SetStatusText(event.filePath.GetAbsolutePath());
    // Navigate the tree to the file's directory (e.g. after drag&drop or the open
    // dialog), but skip the rescan when that directory is already being shown.
    if (!m_fileBrowserPanel->IsShowingDir(event.filePath.GetPath())) {
        m_fileBrowserPanel->ListDir(event.filePath.GetPath());
    }
}

void MainFrame::OnMarkdownError(MarkdownToHtmlAsyncEvent& event) {
    wxString message = event.error.IsEmpty() ? wxString("Error parsing markdown!") : event.error;
    wxMessageBox(message, "Error", wxICON_ERROR);
    statusBar->SetStatusText(wxString(""));
}

void MainFrame::OpenMarkdownFile(const wxFileName& filePath) {
    statusBar->SetStatusText(wxString("Loading ..."));
    m_markdownParser.ParseFile(filePath);
}

void MainFrame::HandleFileSystemWatcherEvent(wxFileSystemWatcherEvent& event) {
    int changeType = event.GetChangeType();

    // 💡 If a file is added, removed, or renamed, re-trigger ListDir silently.
    // The change type is a bitmask, so test with & rather than equality.
    if (changeType & (wxFSW_EVENT_CREATE | wxFSW_EVENT_DELETE | wxFSW_EVENT_RENAME)) {
        m_fileBrowserPanel->ReloadCurrentDir();
    }
}

void MainFrame::HandleDirectoryChanged(const wxFileName& filePath) {
    m_fileSystemWatcher.RemoveAll();
    if (filePath.DirExists()) {
        // 💡 Using AddTree is required for reliable macOS FSEvents integration
        bool success = m_fileSystemWatcher.AddTree(filePath);
        if (!success) {
            printError("[ERROR] Watcher failed to add path: {}", filePath.GetFullPath());
        }
    }
}
