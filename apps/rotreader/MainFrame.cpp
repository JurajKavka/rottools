#include "MainFrame.h"

#include <wx/dnd.h>      // Required for wxFileDropTarget
#include <wx/filedlg.h>  // Required for wxFileDialog
#include <wx/msgdlg.h>   // Required for wxMessageBox

#include <fstream>  // For opening the file
#include <sstream>  // For reading the file content

#include "FileDropTarget.h"
#include "HelperFunctions.h"
#include "HtmlSourcePanel.h"
#include "MarkdownSourcePanel.h"
#include "WebViewPanel.h"

MainFrame::MainFrame(wxWindow* parent) : MainFrameWx(parent), m_markdownParser(this) {
    Bind(wxEVT_MENU, &MainFrame::HandleNewWindowMenuItemClick, this, wxID_NEW_WINDOW_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleOpenFileMenuItemClick, this, wxID_OPEN);
    Bind(wxEVT_MENU, &MainFrame::HandleSoloWebViewPanelMenuItemClick, this, wxID_SOLO_WEB_VIEW_PANEL_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleToggleFileBrowserMenuItemClick, this, wxID_TOGGLE_FILE_BROWSER_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleToggleHtmlSourcePanelMenuItemClick, this,
         wxID_TOGGLE_HTML_SOURCE_PANEL_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleToggleMarkdownSourcePanelMenuItemClick, this,
         wxID_TOGGLE_MARKDOWN_SOURCE_PANEL_MENU_ITEM);
    Bind(wxEVT_TOOL, &MainFrame::HandleOpenFileMenuItemClick, this, fileOpenTool->GetId());

    Bind(EVT_MARKDOWN_READY, &MainFrame::HandleMarkdownReady, this);
    Bind(EVT_MARKDOWN_ERROR, &MainFrame::HandleMarkdownError, this);

    // 1. Instantiate the panels: file browser on the left, and on the right a
    // nested splitter holding the rendered preview and the HTML source view
    m_mainSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_fileBrowserPanel = new FileBrowserTreePanel(m_mainSplitter, std::bind_front(&MainFrame::OpenMarkdownFile, this),
                                                  std::bind_front(&MainFrame::HandleDirectoryChanged, this));

    m_rightSplitter =
        new wxSplitterWindow(m_mainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_webViewPanel = new WebViewPanel(m_rightSplitter);
    m_sourceSplitter =
        new wxSplitterWindow(m_rightSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_htmlSourcePanel = new HtmlSourcePanel(m_sourceSplitter);
    m_markdownSourcePanel = new MarkdownSourcePanel(m_sourceSplitter);
    m_rightSplitter->SetMinimumPaneSize(100);
    m_sourceSplitter->SetMinimumPaneSize(100);

    // Start with both source views hidden; the menu items split them in
    m_rightSplitter->Initialize(m_webViewPanel);
    m_sourceSplitter->Hide();
    m_htmlSourcePanel->Hide();
    m_markdownSourcePanel->Hide();

    m_mainSplitter->SplitVertically(m_fileBrowserPanel, m_rightSplitter, 100);
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

    m_reloadDebounceTimer.SetOwner(this);
    Bind(wxEVT_TIMER, &MainFrame::HandleReloadDebounceTimer, this);

    Layout();

    // 4. Register drag and drop targets
    this->SetDropTarget(new FileDropTarget(std::bind_front(&MainFrame::OpenMarkdownFile, this)));
}

MainFrame::~MainFrame() {
    // Works around https://github.com/wxWidgets/wxWidgets/issues/26658:
    // on macOS, ~wxFsEventsFileSystemWatcher deletes its stream map without
    // FSEventStreamStop/Invalidate, so streams created by AddTree() stay
    // scheduled on the run loop with a context pointer to the destroyed
    // watcher -> SIGSEGV on the next fs event after this frame closes.
    // RemoveAll() on the still-alive watcher runs the FSEvents override,
    // which does invalidate the streams. Drop this once the fix ships.
    m_fileSystemWatcher.RemoveAll();
}

// Each window is a fully independent MainFrame (own parser, watcher, panels);
// wx keeps the app running until the last top-level window closes
void MainFrame::HandleNewWindowMenuItemClick(wxCommandEvent& event) {
    (new MainFrame(nullptr))->Show(true);
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

// Collapses every panel except the always-visible markdown preview so it fills
// the window. The individual toggle items then bring the others back one by one.
void MainFrame::HandleSoloWebViewPanelMenuItemClick(wxCommandEvent& event) {
    if (m_mainSplitter->IsSplit()) {
        m_mainSplitter->Unsplit(m_fileBrowserPanel);
    }
    m_htmlSourcePanel->Hide();
    m_markdownSourcePanel->Hide();
    ApplySourcePanelVisibility();
}

void MainFrame::HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event) {
    if (m_mainSplitter->IsSplit()) {
        // Hides the file browser panel and expands the web view
        m_mainSplitter->Unsplit(m_fileBrowserPanel);
    } else {
        // Restores the file browser on the left with a width of 200 pixels.
        // The right pane is the nested preview/source splitter, not the web
        // view itself, which is no longer a direct child of m_mainSplitter.
        m_mainSplitter->SplitVertically(m_fileBrowserPanel, m_rightSplitter, 200);
    }
}

void MainFrame::HandleMarkdownReady(MarkdownToHtmlAsyncEvent& event) {
    m_webViewPanel->LoadHtml(event.html);
    // Keep the source views current even while hidden, so toggling them in
    // always shows the source of the displayed document
    m_htmlSourcePanel->ShowHtml(event.html);
    m_markdownSourcePanel->ShowMarkdown(event.markdown);
    statusBar->SetStatusText(event.filePath.GetAbsolutePath());
    // Navigate the tree to the file's directory (e.g. after drag&drop or the open
    // dialog), but skip the rescan when that directory is already being shown.
    if (!m_fileBrowserPanel->IsShowingDir(event.filePath.GetPath())) {
        m_fileBrowserPanel->ListDir(event.filePath.GetPath());
    }
}

void MainFrame::HandleToggleHtmlSourcePanelMenuItemClick(wxCommandEvent& event) {
    m_htmlSourcePanel->Show(!m_htmlSourcePanel->IsShown());
    ApplySourcePanelVisibility();
}

void MainFrame::HandleToggleMarkdownSourcePanelMenuItemClick(wxCommandEvent& event) {
    m_markdownSourcePanel->Show(!m_markdownSourcePanel->IsShown());
    ApplySourcePanelVisibility();
}

// Lays the right-hand side out as columns: the always-visible preview, then
// the HTML source and markdown source columns, according to which source panels
// are currently shown. The panels' own IsShown() state is the source of truth.
// Rebuilds from a collapsed state so every combination takes the same single
// code path.
void MainFrame::ApplySourcePanelVisibility() {
    // Capture the desired state before the collapse below clears it: Unsplit()
    // hides the pane it removes.
    bool showHtml = m_htmlSourcePanel->IsShown();
    bool showMarkdown = m_markdownSourcePanel->IsShown();

    if (m_sourceSplitter->IsSplit()) {
        m_sourceSplitter->Unsplit(m_markdownSourcePanel);
    }
    if (m_rightSplitter->IsSplit()) {
        m_rightSplitter->Unsplit(m_sourceSplitter);
    }
    m_htmlSourcePanel->Hide();
    m_markdownSourcePanel->Hide();

    if (!showHtml && !showMarkdown) {
        return;
    }

    if (showHtml && showMarkdown) {
        m_htmlSourcePanel->Show();
        m_markdownSourcePanel->Show();
        m_sourceSplitter->SplitVertically(m_htmlSourcePanel, m_markdownSourcePanel);
    } else {
        wxWindow* sourceView =
            showHtml ? static_cast<wxWindow*>(m_htmlSourcePanel) : static_cast<wxWindow*>(m_markdownSourcePanel);
        sourceView->Show();
        m_sourceSplitter->Initialize(sourceView);
    }
    m_rightSplitter->SplitVertically(m_webViewPanel, m_sourceSplitter);
}

void MainFrame::HandleMarkdownError(MarkdownToHtmlAsyncEvent& event) {
    wxString message = event.error.IsEmpty() ? wxString("Error parsing markdown!") : event.error;
    wxMessageBox(message, "Error", wxICON_ERROR);
    statusBar->SetStatusText(wxString(""));
}

void MainFrame::OpenMarkdownFile(const wxFileName& filePath) {
    m_currentFile = filePath;
    RefreshWatchedPaths();
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

    // Live reload of the open document. MODIFY covers in-place writes, but the
    // wx docs warn it is not reliably delivered on macOS; editors that save
    // atomically (write temp file, rename over the original) surface as
    // CREATE/RENAME on the document's path instead, which macOS does deliver.
    if (!m_currentFile.IsOk()) {
        return;
    }
    if (changeType & (wxFSW_EVENT_MODIFY | wxFSW_EVENT_CREATE | wxFSW_EVENT_RENAME)) {
        bool touchesCurrentFile = event.GetPath().SameAs(m_currentFile) ||
                                  (changeType & wxFSW_EVENT_RENAME && event.GetNewPath().SameAs(m_currentFile));
        if (touchesCurrentFile) {
            // (Re)starting the timer collapses an event burst into one reload
            m_reloadDebounceTimer.StartOnce(250);
        }
    }
}

void MainFrame::HandleReloadDebounceTimer(wxTimerEvent& event) {
    if (m_currentFile.IsOk() && m_currentFile.FileExists()) {
        printLog("[Watcher] Reloading changed file: {}", m_currentFile.GetFullPath());
        // Parse directly (not OpenMarkdownFile) to avoid status-bar flicker on
        // every save; HandleMarkdownReady refreshes the status bar anyway.
        m_markdownParser.ParseFile(m_currentFile);
    }
}

void MainFrame::HandleDirectoryChanged(const wxFileName& filePath) {
    m_browsedDirectory = wxFileName::DirName(filePath.GetFullPath());
    RefreshWatchedPaths();
}

void MainFrame::RefreshWatchedPaths() {
    m_fileSystemWatcher.RemoveAll();

    // 💡 Using AddTree is required for reliable macOS FSEvents integration
    if (m_browsedDirectory.IsOk() && m_browsedDirectory.DirExists()) {
        if (!m_fileSystemWatcher.AddTree(m_browsedDirectory)) {
            printError("[ERROR] Watcher failed to add path: {}", m_browsedDirectory.GetFullPath());
        }
    }

    // Watch the open document's directory too, so live reload keeps working
    // when the file browser navigates somewhere else.
    if (m_currentFile.IsOk()) {
        wxFileName documentDir = wxFileName::DirName(m_currentFile.GetPath());
        bool alreadyWatched = m_browsedDirectory.IsOk() && documentDir.SameAs(m_browsedDirectory);
        if (!alreadyWatched && documentDir.DirExists()) {
            if (!m_fileSystemWatcher.AddTree(documentDir)) {
                printError("[ERROR] Watcher failed to add path: {}", documentDir.GetFullPath());
            }
        }
    }
}
