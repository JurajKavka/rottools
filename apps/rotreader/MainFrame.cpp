#include "MainFrame.h"

#include <wx/dnd.h>     // Required for wxFileDropTarget
#include <wx/msgdlg.h>  // Required for wxMessageBox
#include <wx/stockitem.h>

#include <functional>
#include <string>
#include <vector>

#include "AppIcon.h"
#include "AppIconData.h"  // generated: the icon PNGs compiled into the binary
#include "FileDropTarget.h"
#include "HtmlSourcePanel.h"

namespace {
bool IsWindowOrDescendant(wxWindow* window, wxWindow* candidate) {
    return window != nullptr && candidate != nullptr && (candidate == window || window->IsDescendant(candidate));
}

bool ContainsFocus(wxWindow* window) {
    return IsWindowOrDescendant(window, wxWindow::FindFocus());
}
}  // namespace

MainFrame::MainFrame(wxWindow* parent) : MainFrameWx(parent) {
#ifndef __WXOSX__
    // macOS takes the window and Dock icon from AppIcon.icns in the .app bundle,
    // where SetIcons does nothing. Windows and X11 need it set explicitly.
    SetIcons(rottools::MakeIconBundle(kAppIconPngs, kAppIconPngCount));
#endif

    Bind(wxEVT_MENU, &MainFrame::HandleNewWindowMenuItemClick, this, wxID_NEW_WINDOW_MENU_ITEM);
    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::HandleCloseWindow, this);
    Bind(wxEVT_MENU, &MainFrame::HandleOpenFileMenuItemClick, this, wxID_OPEN);
    Bind(wxEVT_MENU, &MainFrame::HandleSaveMenuItemClick, this, wxID_SAVE);
    Bind(wxEVT_MENU, &MainFrame::HandleSaveAsMenuItemClick, this, wxID_SAVEAS);
    Bind(wxEVT_MENU, &MainFrame::HandleUndoMenuItemClick, this, wxID_UNDO);
    Bind(wxEVT_MENU, &MainFrame::HandleRedoMenuItemClick, this, wxID_REDO);
    Bind(wxEVT_MENU, &MainFrame::HandleCopyMenuItemClick, this, wxID_COPY);
    Bind(wxEVT_MENU, &MainFrame::HandleCutMenuItemClick, this, wxID_CUT);
    Bind(wxEVT_MENU, &MainFrame::HandlePasteMenuItemClick, this, wxID_PASTE);
    Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdateUndoMenuItem, this, wxID_UNDO);
    Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdateRedoMenuItem, this, wxID_REDO);
    Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdateCopyMenuItem, this, wxID_COPY);
    Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdateCutMenuItem, this, wxID_CUT);
    Bind(wxEVT_UPDATE_UI, &MainFrame::HandleUpdatePasteMenuItem, this, wxID_PASTE);
    Bind(wxEVT_MENU, &MainFrame::HandleSoloMarkdownPreviewPanelMenuItemClick, this, wxID_SOLO_WEB_VIEW_PANEL_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleToggleFileBrowserMenuItemClick, this, wxID_TOGGLE_FILE_BROWSER_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleToggleHtmlSourcePanelMenuItemClick, this,
         wxID_TOGGLE_HTML_SOURCE_PANEL_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleToggleMarkdownEditorPanelMenuItemClick, this,
         wxID_TOGGLE_MARKDOWN_EDITOR_PANEL_MENU_ITEM);
    Bind(wxEVT_MENU, &MainFrame::HandleWordWrapMenuItemClick, this, wxID_WORDWRAP);
    Bind(wxEVT_MENU, &MainFrame::HandleFontMenuItemClick, this, wxID_FONT);
    Bind(wxEVT_TOOL, &MainFrame::HandleOpenFileMenuItemClick, this, m_fileOpenTool->GetId());
    Bind(wxEVT_TOOL, &MainFrame::HandleSaveMenuItemClick, this, m_saveTool->GetId());
    Bind(wxEVT_TOOL, &MainFrame::HandleSaveAsMenuItemClick, this, m_saveAsTool->GetId());

    // Let the active wxWidgets port supply its standard labels and accelerators.
    const long stockLabelFlags = wxSTOCK_WITH_MNEMONIC | wxSTOCK_WITH_ACCELERATOR;
    m_editMenu->FindItem(wxID_UNDO)->SetItemLabel(wxGetStockLabel(wxID_UNDO, stockLabelFlags));
    m_editMenu->FindItem(wxID_REDO)->SetItemLabel(wxGetStockLabel(wxID_REDO, stockLabelFlags));
    m_editMenu->FindItem(wxID_COPY)->SetItemLabel(wxGetStockLabel(wxID_COPY, stockLabelFlags));
    m_editMenu->FindItem(wxID_CUT)->SetItemLabel(wxGetStockLabel(wxID_CUT, stockLabelFlags));
    m_editMenu->FindItem(wxID_PASTE)->SetItemLabel(wxGetStockLabel(wxID_PASTE, stockLabelFlags));

    PopulateThemeMenu();

    // 1. Instantiate the panels: file browser on the left, and on the right a
    // nested splitter holding the rendered preview and the source/editor area
    m_mainSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_fileBrowserPanel = new FileBrowserTreePanel(
        m_mainSplitter, std::bind_front(&MainFrame::HandleOpenMarkdownFile, this),
        std::bind_front(&MainFrame::HandleDirectoryChanged, this), std::vector<std::string>{".md", ".markdown"});

    m_rightSplitter =
        new wxSplitterWindow(m_mainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_markdownPreviewPanel =
        new MarkdownPreviewPanel(m_rightSplitter, std::bind_front(&MainFrame::HandleMarkdownReady, this),
                                 std::bind_front(&MainFrame::HandleMarkdownError, this));
    m_sourceSplitter =
        new wxSplitterWindow(m_rightSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE);
    m_htmlSourcePanel =
        new HtmlSourcePanel(m_sourceSplitter, std::bind_front(&MainFrame::HandleHtmlSourcePanelClose, this));
    m_markdownEditorPanel =
        new MarkdownEditorPanel(m_sourceSplitter, std::bind_front(&MainFrame::HandleMarkdownDocumentChanged, this),
                                std::bind_front(&MainFrame::HandleMarkdownEditorStatusChanged, this),
                                std::bind_front(&MainFrame::HandleMarkdownEditorError, this));
    m_viewMenu->Check(wxID_WORDWRAP, m_markdownEditorPanel->IsWordWrapEnabled());
    m_rightSplitter->SetMinimumPaneSize(100);
    m_sourceSplitter->SetMinimumPaneSize(100);

    // Start with the HTML source and Markdown editor hidden; their View menu
    // items split them in when requested.
    m_rightSplitter->Initialize(m_markdownPreviewPanel);
    m_sourceSplitter->Hide();
    m_htmlSourcePanel->Hide();
    m_markdownEditorPanel->Hide();

    m_mainSplitter->SplitVertically(m_fileBrowserPanel, m_rightSplitter, m_fileBrowserWidth);
    m_mainSplitter->SetMinimumPaneSize(100);
    m_viewMenu->Check(wxID_TOGGLE_FILE_BROWSER_MENU_ITEM, true);
    m_viewMenu->Check(wxID_TOGGLE_HTML_SOURCE_PANEL_MENU_ITEM, false);
    m_viewMenu->Check(wxID_TOGGLE_MARKDOWN_EDITOR_PANEL_MENU_ITEM, false);

    wxSizer* mainSizer = this->GetSizer();
    mainSizer->Add(m_mainSplitter, 1, wxEXPAND | wxALL, 0);

    // inital directory list
    wxFileName initialDirectory;
    initialDirectory.AssignHomeDir();
    m_fileBrowserPanel->ListDir(initialDirectory);

    // The browser watcher is created lazily by RefreshBrowserWatcher, which
    // runs once the initial ListDir scan completes (via HandleDirectoryChanged).

    Layout();

    // 4. Register drag and drop targets
    this->SetDropTarget(new FileDropTarget(std::bind_front(&MainFrame::HandleOpenMarkdownFile, this)));
}

MainFrame::~MainFrame() {
    // Tear the browser watcher down while the panel its callback references is
    // still alive. The editor owns and tears down its document watcher.
    m_browserWatcher.reset();
}

// Each window is a fully independent MainFrame (own parser, watcher, panels);
// wx keeps the app running until the last top-level window closes
void MainFrame::HandleNewWindowMenuItemClick(wxCommandEvent& event) {
    (new MainFrame(nullptr))->Show(true);
}

void MainFrame::HandleCloseWindow(wxCloseEvent& event) {
    if (event.CanVeto() && !m_markdownEditorPanel->ConfirmSaveBeforeDiscard()) {
        event.Veto();
        return;
    }

    event.Skip();
}

void MainFrame::HandleOpenFileMenuItemClick(wxCommandEvent& event) {
    (void)m_markdownEditorPanel->ShowOpenDialog();
}

void MainFrame::HandleSaveMenuItemClick(wxCommandEvent& event) {
    (void)m_markdownEditorPanel->Save();
}

void MainFrame::HandleSaveAsMenuItemClick(wxCommandEvent& event) {
    (void)m_markdownEditorPanel->SaveAs();
}

void MainFrame::HandleUndoMenuItemClick(wxCommandEvent& event) {
    if (IsMarkdownEditorFocused()) {
        m_markdownEditorPanel->Undo();
    }
}

void MainFrame::HandleRedoMenuItemClick(wxCommandEvent& event) {
    if (IsMarkdownEditorFocused()) {
        m_markdownEditorPanel->Redo();
    }
}

void MainFrame::HandleCopyMenuItemClick(wxCommandEvent& event) {
    if (IsMarkdownEditorFocused()) {
        m_markdownEditorPanel->Copy();
    } else if (ContainsFocus(m_htmlSourcePanel)) {
        m_htmlSourcePanel->Copy();
    } else if (ContainsFocus(m_markdownPreviewPanel)) {
        m_markdownPreviewPanel->Copy();
    }
}

void MainFrame::HandleCutMenuItemClick(wxCommandEvent& event) {
    if (IsMarkdownEditorFocused()) {
        m_markdownEditorPanel->Cut();
    }
}

void MainFrame::HandlePasteMenuItemClick(wxCommandEvent& event) {
    if (IsMarkdownEditorFocused()) {
        m_markdownEditorPanel->Paste();
    }
}

bool MainFrame::IsMarkdownEditorFocused() const {
    return m_markdownEditorPanel->ContainsFocus();
}

void MainFrame::HandleUpdateUndoMenuItem(wxUpdateUIEvent& event) {
    event.Enable(IsMarkdownEditorFocused() && m_markdownEditorPanel->CanUndo());
}

void MainFrame::HandleUpdateRedoMenuItem(wxUpdateUIEvent& event) {
    event.Enable(IsMarkdownEditorFocused() && m_markdownEditorPanel->CanRedo());
}

void MainFrame::HandleUpdateCopyMenuItem(wxUpdateUIEvent& event) {
    const bool canCopy = (IsMarkdownEditorFocused() && m_markdownEditorPanel->CanCopy()) ||
                         (ContainsFocus(m_htmlSourcePanel) && m_htmlSourcePanel->CanCopy()) ||
                         (ContainsFocus(m_markdownPreviewPanel) && m_markdownPreviewPanel->CanCopy());
    event.Enable(canCopy);
}

void MainFrame::HandleUpdateCutMenuItem(wxUpdateUIEvent& event) {
    event.Enable(IsMarkdownEditorFocused() && m_markdownEditorPanel->CanCut());
}

void MainFrame::HandleUpdatePasteMenuItem(wxUpdateUIEvent& event) {
    event.Enable(IsMarkdownEditorFocused() && m_markdownEditorPanel->CanPaste());
}

// Collapses every panel except the always-visible markdown preview so it fills
// the window. The individual toggle items then bring the others back one by one.
void MainFrame::HandleSoloMarkdownPreviewPanelMenuItemClick(wxCommandEvent& event) {
    wxWindow* focusedWindow = wxWindow::FindFocus();
    HideFileBrowser();
    m_htmlSourcePanel->Hide();
    m_markdownEditorPanel->Hide();
    ApplySourcePanelVisibility(focusedWindow);
}

void MainFrame::HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event) {
    if (!event.IsChecked()) {
        HideFileBrowser();
    } else if (!m_mainSplitter->IsSplit()) {
        // The right pane is the nested preview/source splitter, not the web
        // view itself, which is no longer a direct child of m_mainSplitter.
        m_mainSplitter->SplitVertically(m_fileBrowserPanel, m_rightSplitter, m_fileBrowserWidth);
    }
}

void MainFrame::HideFileBrowser() {
    const bool browserHadFocus = ContainsFocus(m_fileBrowserPanel);
    m_viewMenu->Check(wxID_TOGGLE_FILE_BROWSER_MENU_ITEM, false);
    if (!m_mainSplitter->IsSplit()) {
        if (browserHadFocus) {
            m_markdownPreviewPanel->FocusContent();
        }
        return;
    }

    int sashPosition = m_mainSplitter->GetSashPosition();
    if (sashPosition > 0) {
        m_fileBrowserWidth = sashPosition;
    }
    m_mainSplitter->Unsplit(m_fileBrowserPanel);
    if (browserHadFocus) {
        m_markdownPreviewPanel->FocusContent();
    }
}

void MainFrame::HandleToggleHtmlSourcePanelMenuItemClick(wxCommandEvent& event) {
    wxWindow* focusedWindow = wxWindow::FindFocus();
    m_htmlSourcePanel->Show(event.IsChecked());
    ApplySourcePanelVisibility(focusedWindow);
}

void MainFrame::HandleToggleMarkdownEditorPanelMenuItemClick(wxCommandEvent& event) {
    wxWindow* focusedWindow = wxWindow::FindFocus();
    const bool showEditor = event.IsChecked();
    m_markdownEditorPanel->Show(showEditor);
    ApplySourcePanelVisibility(focusedWindow);
    if (showEditor) {
        m_markdownEditorPanel->FocusEditor();
    }
}

void MainFrame::HandleHtmlSourcePanelClose() {
    wxWindow* focusedWindow = wxWindow::FindFocus();
    m_htmlSourcePanel->Hide();
    m_viewMenu->Check(wxID_TOGGLE_HTML_SOURCE_PANEL_MENU_ITEM, false);
    ApplySourcePanelVisibility(focusedWindow);
}

void MainFrame::HandleWordWrapMenuItemClick(wxCommandEvent& event) {
    m_markdownEditorPanel->SetWordWrap(event.IsChecked());
}

void MainFrame::HandleFontMenuItemClick(wxCommandEvent& event) {
    m_markdownEditorPanel->ShowFontDialog();
}

// Lays the right-hand side out as columns: the always-visible preview, then
// the HTML source and Markdown editor columns, according to which panels
// are currently shown. The panels' own IsShown() state is the source of truth.
// Rebuilds from a collapsed state so every combination takes the same single
// code path.
void MainFrame::ApplySourcePanelVisibility(wxWindow* focusedWindowBeforeChange) {
    // Capture the desired state before the collapse below clears it: Unsplit()
    // hides the pane it removes.
    const bool showHtml = m_htmlSourcePanel->IsShown();
    const bool showMarkdown = m_markdownEditorPanel->IsShown();
    wxWindow* focusedWindow = focusedWindowBeforeChange != nullptr ? focusedWindowBeforeChange : wxWindow::FindFocus();
    const bool htmlHadFocus = IsWindowOrDescendant(m_htmlSourcePanel, focusedWindow);
    const bool editorHadFocus = IsWindowOrDescendant(m_markdownEditorPanel, focusedWindow);
    m_viewMenu->Check(wxID_TOGGLE_HTML_SOURCE_PANEL_MENU_ITEM, showHtml);
    m_viewMenu->Check(wxID_TOGGLE_MARKDOWN_EDITOR_PANEL_MENU_ITEM, showMarkdown);

    if (m_sourceSplitter->IsSplit()) {
        const int sashPosition = m_sourceSplitter->GetSashPosition();
        if (sashPosition > 0) {
            m_htmlSourceWidth = sashPosition;
        }
        m_sourceSplitter->Unsplit(m_markdownEditorPanel);
    }
    if (m_rightSplitter->IsSplit()) {
        const int sashPosition = m_rightSplitter->GetSashPosition();
        if (sashPosition > 0) {
            m_previewWidth = sashPosition;
        }
        m_rightSplitter->Unsplit(m_sourceSplitter);
    }
    m_htmlSourcePanel->Hide();
    m_markdownEditorPanel->Hide();

    if (!showHtml && !showMarkdown) {
        Layout();
        if (htmlHadFocus || editorHadFocus) {
            m_markdownPreviewPanel->FocusContent();
        }
        return;
    }

    if (showHtml && showMarkdown) {
        m_htmlSourcePanel->Show();
        m_markdownEditorPanel->Show();
        m_sourceSplitter->SplitVertically(m_htmlSourcePanel, m_markdownEditorPanel, m_htmlSourceWidth);
    } else {
        wxWindow* sourceView =
            showHtml ? static_cast<wxWindow*>(m_htmlSourcePanel) : static_cast<wxWindow*>(m_markdownEditorPanel);
        sourceView->Show();
        m_sourceSplitter->Initialize(sourceView);
    }
    m_rightSplitter->SplitVertically(m_markdownPreviewPanel, m_sourceSplitter, m_previewWidth);

    m_rightSplitter->UpdateSize();
    m_sourceSplitter->UpdateSize();

    Layout();

    if ((htmlHadFocus && showHtml) || (editorHadFocus && showMarkdown)) {
        focusedWindow->SetFocus();
    } else if (htmlHadFocus || editorHadFocus) {
        m_markdownPreviewPanel->FocusContent();
    }
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

void MainFrame::HandleOpenMarkdownFile(const wxFileName& filePath) {
    (void)m_markdownEditorPanel->OpenFile(filePath);
}

void MainFrame::HandleDirectoryChanged(const wxFileName& filePath) {
    m_browsedDirectory = wxFileName::DirName(filePath.GetFullPath());
    RefreshBrowserWatcher();
}

void MainFrame::HandleBrowserWatcherChange() {
    m_fileBrowserPanel->ReloadCurrentDir();
}

void MainFrame::RefreshBrowserWatcher() {
    m_browserWatcher.reset();
    if (m_browsedDirectory.IsOk() && m_browsedDirectory.DirExists()) {
        m_browserWatcher = std::make_unique<FsWatcher>(m_browsedDirectory,
                                                       std::bind_front(&MainFrame::HandleBrowserWatcherChange, this));
    }
}

void MainFrame::HandleMarkdownDocumentChanged(const MarkdownEditorPanel::DocumentChange& change) {
    if (change.reason == MarkdownEditorPanel::ChangeReason::Opened || change.diskEntryChanged) {
        // Explicit navigation and Save As follow/select the active file. Live
        // reloads deliberately leave the user's browsed directory alone.
        m_fileBrowserPanel->ShowFile(change.filePath);
    }

    const ScrollBehavior scrollBehavior = change.reason == MarkdownEditorPanel::ChangeReason::Opened
                                              ? ScrollBehavior::ResetToTop
                                              : ScrollBehavior::KeepPosition;
    m_markdownPreviewPanel->LoadMarkdown(change.text, change.filePath, GetPreviewOptions(scrollBehavior));
}

void MainFrame::HandleMarkdownEditorStatusChanged(const MarkdownEditorPanel::StatusMessage& message) {
    statusBar->SetStatusText(message.label);
    m_replaceEditorStatusOnPreviewReady = message.replaceOnPreviewReady;
}

void MainFrame::HandleMarkdownEditorError(MarkdownEditorPanel::ErrorCode errorCode, const wxFileName& filePath) {
    wxString message;
    switch (errorCode) {
        case MarkdownEditorPanel::ErrorCode::FileDoesNotExist:
            message = wxString("File does not exist: ") + filePath.GetFullPath();
            break;
        case MarkdownEditorPanel::ErrorCode::FileNotReadable:
            message = wxString("No permission to read file: ") + filePath.GetFullPath();
            break;
        case MarkdownEditorPanel::ErrorCode::FileReadFailed:
            message = wxString("Could not read file: ") + filePath.GetFullPath();
            break;
        case MarkdownEditorPanel::ErrorCode::FileNotWritable:
            message = wxString("No permission to write file: ") + filePath.GetFullPath();
            break;
        case MarkdownEditorPanel::ErrorCode::FileWriteFailed:
            message = wxString("Could not save file: ") + filePath.GetFullPath();
            break;
        case MarkdownEditorPanel::ErrorCode::ExternalChangeCheckFailed:
            message = wxString("Could not check the current file before saving: ") + filePath.GetFullPath();
            break;
    }

    wxMessageBox(message, "Error", wxOK | wxICON_ERROR, this);
}

void MainFrame::HandleMarkdownReady(const MarkdownPreviewData& markdownPreviewData) {
    m_htmlSourcePanel->ShowHtml(markdownPreviewData.html);
    // Resolve the ordinary open-progress message, but do not erase a newer
    // save confirmation or external-change warning emitted by the editor.
    if (m_replaceEditorStatusOnPreviewReady) {
        statusBar->SetStatusText(markdownPreviewData.fileName.GetFullPath());
        m_replaceEditorStatusOnPreviewReady = false;
    }
}

void MainFrame::HandleMarkdownError(const wxString& error) {
    wxString message = error.IsEmpty() ? wxString("Error parsing markdown!") : error;
    wxMessageBox(message, "Error", wxICON_ERROR);
    const bool clearProgressStatus = m_replaceEditorStatusOnPreviewReady;
    m_replaceEditorStatusOnPreviewReady = false;
    if (clearProgressStatus) {
        statusBar->SetStatusText(wxString(""));
    }
}
