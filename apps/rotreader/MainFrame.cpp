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

MainFrame::MainFrame(wxWindow* parent) : MainFrameWx(parent) {
    Bind(wxEVT_MENU, &MainFrame::HandleNewWindowMenuItemClick, this, wxID_NEW_WINDOW_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleOpenFileMenuItemClick, this, wxID_OPEN);
    Bind(wxEVT_MENU, &MainFrame::HandleSoloMarkdownPreviewPanelMenuItemClick, this, wxID_SOLO_WEB_VIEW_PANEL_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleToggleFileBrowserMenuItemClick, this, wxID_TOGGLE_FILE_BROWSER_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleToggleHtmlSourcePanelMenuItemClick, this,
         wxID_TOGGLE_HTML_SOURCE_PANEL_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleToggleMarkdownSourcePanelMenuItemClick, this,
         wxID_TOGGLE_MARKDOWN_SOURCE_PANEL_MENU_ITEM);
    Bind(wxEVT_TOOL, &MainFrame::HandleOpenFileMenuItemClick, this, fileOpenTool->GetId());

    PopulateThemeMenu();

    // 1. Instantiate the panels: file browser on the left, and on the right a
    // nested splitter holding the rendered preview and the HTML source view
    m_mainSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_fileBrowserPanel = new FileBrowserTreePanel(m_mainSplitter, std::bind_front(&MainFrame::OpenMarkdownFile, this),
                                                  std::bind_front(&MainFrame::HandleDirectoryChanged, this));

    m_rightSplitter =
        new wxSplitterWindow(m_mainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_markdownPreviewPanel =
        new MarkdownPreviewPanel(m_rightSplitter, std::bind_front(&MainFrame::HandleMarkdownReady, this),
                                 std::bind_front(&MainFrame::HandleMarkdownError, this));
    m_sourceSplitter =
        new wxSplitterWindow(m_rightSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_htmlSourcePanel = new HtmlSourcePanel(m_sourceSplitter);
    m_markdownSourcePanel = new MarkdownSourcePanel(m_sourceSplitter);
    m_rightSplitter->SetMinimumPaneSize(100);
    m_sourceSplitter->SetMinimumPaneSize(100);

    // Start with both source views hidden; the menu items split them in
    m_rightSplitter->Initialize(m_markdownPreviewPanel);
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

    // The file system watchers are created lazily by RefreshWatchedPaths, which
    // runs once the initial ListDir scan completes (via HandleDirectoryChanged).

    Layout();

    // 4. Register drag and drop targets
    this->SetDropTarget(new FileDropTarget(std::bind_front(&MainFrame::OpenMarkdownFile, this)));
}

MainFrame::~MainFrame() {
    // Tear the watchers down first (FsWatcher's destructor invalidates its
    // FSEvents streams) while the panels their callbacks reference are alive.
    m_browserWatcher.reset();
    m_documentWatcher.reset();
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
void MainFrame::HandleSoloMarkdownPreviewPanelMenuItemClick(wxCommandEvent& event) {
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
    m_rightSplitter->SplitVertically(m_markdownPreviewPanel, m_sourceSplitter);
}

// Fills the (empty) Theme submenu from the CssThemes.h table, one radio item per
// theme. The ids run from m_themeMenuBaseId in CssThemeId order, so an item's id
// minus the base is the theme's index in cssThemes.
void MainFrame::PopulateThemeMenu() {
    m_themeMenuBaseId = wxWindow::NewControlId(CssThemeCount);

    for (int themeIndex = 0; themeIndex < CssThemeCount; ++themeIndex) {
        m_themeSubmenu->AppendRadioItem(m_themeMenuBaseId + themeIndex, cssThemes[themeIndex].name);
    }

    m_themeSubmenu->Check(m_themeMenuBaseId + m_themeId, true);
    Bind(wxEVT_MENU, &MainFrame::HandleThemeMenuItemClick, this, m_themeMenuBaseId,
         m_themeMenuBaseId + CssThemeCount - 1);
}

void MainFrame::HandleThemeMenuItemClick(wxCommandEvent& event) {
    m_themeId = event.GetId() - m_themeMenuBaseId;
    // The document is already parsed; only the page around it changes
    m_markdownPreviewPanel->Render(GetPreviewOptions());
}

MarkdownPreviewOptions MainFrame::GetPreviewOptions() const {
    return {.injectStyle = cssThemes[m_themeId].css};
}

void MainFrame::OpenMarkdownFile(const wxFileName& filePath) {
    m_currentFile = filePath;
    RefreshWatchedPaths();
    // Follow the browser to the opened file's directory (drag&drop, the open
    // dialog, or a file picked elsewhere), unless it is already shown. This is
    // deliberately here and not in HandleMarkdownReady: a live reload after an
    // external save re-parses through LoadFile directly, and must not drag the
    // browser back to the document's folder while the user browses elsewhere.
    if (!m_fileBrowserPanel->IsShowingDir(filePath.GetPath())) {
        m_fileBrowserPanel->ListDir(filePath.GetPath());
    }
    statusBar->SetStatusText(wxString("Loading ..."));
    m_markdownPreviewPanel->LoadFile(filePath, GetPreviewOptions());
}

void MainFrame::ReloadOpenDocument() {
    if (m_currentFile.IsOk() && m_currentFile.FileExists()) {
        printLog("[Watcher] Reloading changed file: {}", m_currentFile.GetFullPath());
        // Load directly (not OpenMarkdownFile) to avoid status-bar flicker on
        // every save; HandleMarkdownReady refreshes the status bar anyway.
        m_markdownPreviewPanel->LoadFile(m_currentFile, GetPreviewOptions());
    }
}

void MainFrame::HandleDirectoryChanged(const wxFileName& filePath) {
    m_browsedDirectory = wxFileName::DirName(filePath.GetFullPath());
    RefreshWatchedPaths();
}

void MainFrame::RefreshWatchedPaths() {
    m_browserWatcher.reset();
    if (m_browsedDirectory.IsOk() && m_browsedDirectory.DirExists()) {
        m_browserWatcher =
            std::make_unique<FsWatcher>(m_browsedDirectory, [this] { m_fileBrowserPanel->ReloadCurrentDir(); });
    }

    m_documentWatcher.reset();
    if (m_currentFile.IsOk()) {
        m_documentWatcher = std::make_unique<FsWatcher>(m_currentFile, [this] { ReloadOpenDocument(); });
    }
}

void MainFrame::HandleMarkdownReady(const MarkdownPreviewData& markdownPreviewData) {
    m_htmlSourcePanel->ShowHtml(markdownPreviewData.html);
    m_markdownSourcePanel->ShowMarkdown(markdownPreviewData.markdown);
    statusBar->SetStatusText(markdownPreviewData.fileName.GetAbsolutePath());
}

void MainFrame::HandleMarkdownError(const wxString& error) {
    wxString message = error.IsEmpty() ? wxString("Error parsing markdown!") : error;
    wxMessageBox(message, "Error", wxICON_ERROR);
    statusBar->SetStatusText(wxString(""));
}
