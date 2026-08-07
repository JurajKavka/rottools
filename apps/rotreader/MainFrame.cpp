#include "MainFrame.h"

#include <wx/dnd.h>      // Required for wxFileDropTarget
#include <wx/filedlg.h>  // Required for wxFileDialog
#include <wx/msgdlg.h>   // Required for wxMessageBox

#include <string>
#include <vector>

#include "AppIcon.h"
#include "AppIconData.h"  // generated: the icon PNGs compiled into the binary
#include "FileDropTarget.h"
#include "HelperFunctions.h"
#include "HtmlSourcePanel.h"
#include "MarkdownSourcePanel.h"

MainFrame::MainFrame(wxWindow* parent) : MainFrameWx(parent) {
#ifndef __WXOSX__
    // macOS takes the window and Dock icon from AppIcon.icns in the .app bundle,
    // where SetIcons does nothing. Windows and X11 need it set explicitly.
    SetIcons(rottools::MakeIconBundle(kAppIconPngs, kAppIconPngCount));
#endif

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
                                                  std::bind_front(&MainFrame::HandleDirectoryChanged, this),
                                                  std::vector<std::string>{".md"});

    m_rightSplitter =
        new wxSplitterWindow(m_mainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_markdownPreviewPanel =
        new MarkdownPreviewPanel(m_rightSplitter, std::bind_front(&MainFrame::HandleMarkdownReady, this),
                                 std::bind_front(&MainFrame::HandleMarkdownError, this));
    m_sourceSplitter =
        new wxSplitterWindow(m_rightSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_htmlSourcePanel =
        new HtmlSourcePanel(m_sourceSplitter, std::bind_front(&MainFrame::HandleHtmlSourcePanelClose, this));
    m_markdownSourcePanel = new MarkdownSourcePanel(
        m_sourceSplitter, std::bind_front(&MainFrame::HandleMarkdownSourceSave, this),
        std::bind_front(&MainFrame::HandleMarkdownSourcePanelClose, this));
    m_rightSplitter->SetMinimumPaneSize(100);
    m_sourceSplitter->SetMinimumPaneSize(100);

    // Start with both source views hidden; the menu items split them in
    m_rightSplitter->Initialize(m_markdownPreviewPanel);
    m_sourceSplitter->Hide();
    m_htmlSourcePanel->Hide();
    m_markdownSourcePanel->Hide();

    m_mainSplitter->SplitVertically(m_fileBrowserPanel, m_rightSplitter, m_fileBrowserWidth);
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
        "Markdown files (*.md;*.markdown)|*.md;*.markdown",  // File extensions filter
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
    HideFileBrowser();
    m_htmlSourcePanel->Hide();
    m_markdownSourcePanel->Hide();
    ApplySourcePanelVisibility();
}

void MainFrame::HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event) {
    if (m_mainSplitter->IsSplit()) {
        HideFileBrowser();
    } else {
        // The right pane is the nested preview/source splitter, not the web
        // view itself, which is no longer a direct child of m_mainSplitter.
        m_mainSplitter->SplitVertically(m_fileBrowserPanel, m_rightSplitter, m_fileBrowserWidth);
    }
}

void MainFrame::HideFileBrowser() {
    if (!m_mainSplitter->IsSplit()) {
        return;
    }

    int sashPosition = m_mainSplitter->GetSashPosition();
    if (sashPosition > 0) {
        m_fileBrowserWidth = sashPosition;
    }
    m_mainSplitter->Unsplit(m_fileBrowserPanel);
}

void MainFrame::HandleToggleHtmlSourcePanelMenuItemClick(wxCommandEvent& event) {
    m_htmlSourcePanel->Show(!m_htmlSourcePanel->IsShown());
    ApplySourcePanelVisibility();
}

void MainFrame::HandleToggleMarkdownSourcePanelMenuItemClick(wxCommandEvent& event) {
    m_markdownSourcePanel->Show(!m_markdownSourcePanel->IsShown());
    ApplySourcePanelVisibility();
}

void MainFrame::HandleHtmlSourcePanelClose() {
    m_htmlSourcePanel->Hide();
    ApplySourcePanelVisibility();
}

void MainFrame::HandleMarkdownSourcePanelClose() {
    m_markdownSourcePanel->Hide();
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
        Layout();
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

    m_rightSplitter->UpdateSize();
    m_sourceSplitter->UpdateSize();

    Layout();
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
    // The document is already parsed; only the page around it changes, so keep
    // the reader's scroll position.
    m_markdownPreviewPanel->Render(GetPreviewOptions(ScrollBehavior::KeepPosition));
}

MarkdownPreviewOptions MainFrame::GetPreviewOptions(ScrollBehavior scrollBehavior) const {
    return {.injectStyle = cssThemes[m_themeId].css, .scrollBehavior = scrollBehavior};
}

void MainFrame::OpenMarkdownFile(const wxFileName& filePath) {
    wxFileName absolutePath(filePath);
    absolutePath.MakeAbsolute();

    if (!absolutePath.FileExists()) {
        wxMessageBox(wxString("File does not exist: ") + absolutePath.GetFullPath(), "Error", wxICON_ERROR);
        return;
    }
    if (!absolutePath.IsFileReadable()) {
        wxMessageBox(wxString("No permission to read file: ") + absolutePath.GetFullPath(), "Error", wxICON_ERROR);
        return;
    }
    m_currentFile = absolutePath;
    // Unknown until the load reports back in HandleMarkdownReady
    m_loadedText.clear();
    RefreshWatchedPaths();
    // Follow the browser to the opened file's directory (drag&drop, the open
    // dialog, or a file picked elsewhere), unless it is already shown. This is
    // deliberately here and not in HandleMarkdownReady: a live reload after an
    // external save re-parses through LoadFile directly, and must not drag the
    // browser back to the document's folder while the user browses elsewhere.
    if (!m_fileBrowserPanel->IsShowingDir(absolutePath.GetPath())) {
        m_fileBrowserPanel->ListDir(absolutePath.GetPath());
    }
    statusBar->SetStatusText(wxString("Loading ..."));
    // A different file: start at the top (the ScrollBehavior default).
    m_markdownPreviewPanel->LoadFile(absolutePath, GetPreviewOptions());
}

// Handles changes made by other programs (an external editor, git, a second
// window). Our own save does not come through here: it renders directly, and the
// file-system event it triggers is dropped by the content comparison below.
void MainFrame::ReloadOpenDocument() {
    if (!m_currentFile.IsOk() || !m_currentFile.FileExists()) {
        return;
    }

    wxString onDisk;
    if (!ReadFileUtf8(m_currentFile, onDisk)) {
        printError("[Watcher] Could not read changed file: {}", m_currentFile.GetFullPath());
        return;
    }

    // The echo of our own save, or a repeated event for a change already loaded
    if (onDisk == m_loadedText) {
        return;
    }

    // Never discard what the user typed. They keep their version; the file on
    // disk is left alone and picked up by the next save or reopen.
    if (m_markdownSourcePanel->HasUnsavedChanges()) {
        statusBar->SetStatusText(wxString("File changed on disk - your unsaved edits were kept: ") +
                                 m_currentFile.GetFullPath());
        return;
    }

    printLog("[Watcher] Reloading changed file: {}", m_currentFile.GetFullPath());
    m_loadedText = onDisk;
    // The text is already in hand, so render it directly instead of reading the
    // file a second time. That means HandleMarkdownReady will not fill the
    // editor, so do it here. Same file reloaded live: keep the scroll positions.
    m_markdownSourcePanel->ShowMarkdown(onDisk, ScrollBehavior::KeepPosition);
    m_markdownPreviewPanel->LoadMarkdown(onDisk, m_currentFile, GetPreviewOptions(ScrollBehavior::KeepPosition));
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
    // Only a render that read the file fills the editor. Text that came from the
    // editor itself must not be written back into it: the round trip takes long
    // enough to swallow anything typed in the meantime, and it would clear the
    // buffer's saved state.
    if (markdownPreviewData.origin == MarkdownOrigin::Disk) {
        m_loadedText = markdownPreviewData.markdown;
        // ResetToTop means a different document, which always replaces the
        // editor. KeepPosition means a repaint of the same one - a theme change
        // re-renders the last parse - and must not throw away edits in progress.
        if (markdownPreviewData.scrollBehavior == ScrollBehavior::ResetToTop ||
            !m_markdownSourcePanel->HasUnsavedChanges()) {
            m_markdownSourcePanel->ShowMarkdown(markdownPreviewData.markdown, markdownPreviewData.scrollBehavior);
        }
    }
    statusBar->SetStatusText(markdownPreviewData.fileName.GetAbsolutePath());
}

void MainFrame::HandleMarkdownError(const wxString& error) {
    wxString message = error.IsEmpty() ? wxString("Error parsing markdown!") : error;
    wxMessageBox(message, "Error", wxICON_ERROR);
    statusBar->SetStatusText(wxString(""));
}

void MainFrame::HandleMarkdownSourceSave(const wxString& markdown) {
    if (!m_currentFile.IsOk()) {
        return;
    }
    if (m_currentFile.FileExists() && !m_currentFile.IsFileWritable()) {
        wxMessageBox(wxString("No permission to write file: ") + m_currentFile.GetFullPath(), "Error", wxICON_ERROR);
        return;
    }

    if (!WriteFileUtf8(m_currentFile, markdown)) {
        wxMessageBox(wxString("Could not save file: ") + m_currentFile.GetFullPath(), "Error", wxICON_ERROR);
        return;
    }
    statusBar->SetStatusText(wxString("Saved ") + m_currentFile.GetFullPath());

    // Repaint straight from the text we just wrote. The document watcher will
    // also report this write, but ReloadOpenDocument drops it as its own echo:
    // waiting for that event would delay the repaint by the debounce, fail
    // whenever the operating system merges or loses the event, and overwrite
    // anything typed in the meantime.
    m_loadedText = markdown;
    m_markdownPreviewPanel->LoadMarkdown(markdown, m_currentFile, GetPreviewOptions(ScrollBehavior::KeepPosition));
}
