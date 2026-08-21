#pragma once

#include <wx/fdrepdlg.h>
#include <wx/font.h>
#include <wx/splitter.h>

#include <memory>
#include <optional>

#include "CssThemes.h"
#include "FileBrowserTreePanel.h"
#include "FsWatcher.h"
#include "MainFrameWx.h"
#include "MarkdownEditorPanel.h"
#include "MarkdownPreviewPanel.h"

// Forward declarations
class HtmlSourcePanel;

class MainFrame : public MainFrameWx {
   private:
    enum class SearchTarget {
        MarkdownPreview,
        MarkdownEditor,
    };

    // Store a pointer to your custom panel
    MarkdownPreviewPanel* m_markdownPreviewPanel = nullptr;
    HtmlSourcePanel* m_htmlSourcePanel = nullptr;
    MarkdownEditorPanel* m_markdownEditorPanel = nullptr;
    FileBrowserTreePanel* m_fileBrowserPanel = nullptr;
    wxSplitterWindow* m_mainSplitter = nullptr;
    int m_fileBrowserWidth = 100;
    // Right of the always-visible preview it holds the source-views area
    wxSplitterWindow* m_rightSplitter = nullptr;
    int m_previewWidth = 0;
    // Divides the source area into HTML source and Markdown editor columns
    wxSplitterWindow* m_sourceSplitter = nullptr;
    int m_htmlSourceWidth = 0;
    // Watches the browsed directory so its list follows external changes. The
    // MarkdownEditorPanel owns the separate current-document watcher.
    std::unique_ptr<FsWatcher> m_fileBrowserWatcher;
    wxFileName m_browsedDirectory;
    // First of the CssThemeCount consecutive ids given to the Theme menu items
    wxWindowID m_themeMenuBaseId = wxID_ANY;
    // Index into cssThemes; the frame owns the theme, the preview panel does not
    int m_themeId = RotdownNotesMonoLight;
    /**
     * Whether the status bar currently shows transient editor progress. A
     * matching preview completion may replace it with the document path, while
     * newer save confirmations and external-change warnings remain visible.
     */
    bool m_replaceEditorStatusOnPreviewReady = false;
    wxFindReplaceData m_findData{wxFR_DOWN};
    wxFindReplaceDialog* m_findDialog = nullptr;
    SearchTarget m_searchTarget = SearchTarget::MarkdownPreview;
    bool m_findDialogIsReplace = false;

    void HandleNewWindowMenuItemClick(wxCommandEvent& event);
    void HandleNewFileMenuItemClick(wxCommandEvent& event);
    void HandleCloseWindow(wxCloseEvent& event);
    void HandleOpenFileMenuItemClick(wxCommandEvent& event);
    void HandleSaveMenuItemClick(wxCommandEvent& event);
    void HandleSaveAsMenuItemClick(wxCommandEvent& event);
    void HandleUndoMenuItemClick(wxCommandEvent& event);
    void HandleRedoMenuItemClick(wxCommandEvent& event);
    void HandleCopyMenuItemClick(wxCommandEvent& event);
    void HandleCutMenuItemClick(wxCommandEvent& event);
    void HandlePasteMenuItemClick(wxCommandEvent& event);
    void HandleFindMenuItemClick(wxCommandEvent& event);
    void HandleReplaceMenuItemClick(wxCommandEvent& event);
    void HandleFindDialogFind(wxFindDialogEvent& event);
    void HandleFindDialogReplace(wxFindDialogEvent& event);
    void HandleFindDialogReplaceAll(wxFindDialogEvent& event);
    void HandleFindDialogClose(wxFindDialogEvent& event);
    [[nodiscard]] std::optional<SearchTarget> GetFocusedSearchTarget() const;
    void ShowFindDialog(bool replace);
    void HandleUpdateUndoMenuItem(wxUpdateUIEvent& event);
    void HandleUpdateRedoMenuItem(wxUpdateUIEvent& event);
    void HandleUpdateCopyMenuItem(wxUpdateUIEvent& event);
    void HandleUpdateCutMenuItem(wxUpdateUIEvent& event);
    void HandleUpdatePasteMenuItem(wxUpdateUIEvent& event);
    void HandleUpdateFindMenuItem(wxUpdateUIEvent& event);
    void HandleUpdateReplaceMenuItem(wxUpdateUIEvent& event);
    void HandleSoloMarkdownPreviewPanelMenuItemClick(wxCommandEvent& event);
    void HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event);
    void HandleToggleHtmlSourcePanelMenuItemClick(wxCommandEvent& event);
    void HandleToggleMarkdownEditorPanelMenuItemClick(wxCommandEvent& event);
    void HandleWordWrapMenuItemClick(wxCommandEvent& event);
    void HandleFontMenuItemClick(wxCommandEvent& event);
    void HandleHtmlSourcePanelClose();
    void HideFileBrowser();
    void ApplySourcePanelVisibility(wxWindow* focusedWindowBeforeChange = nullptr);
    void PopulateThemeMenu();
    void HandleThemeMenuItemClick(wxCommandEvent& event);
    MarkdownPreviewOptions GetPreviewOptions(ScrollBehavior scrollBehavior = ScrollBehavior::ResetToTop) const;
    void HandleDirectoryChanged(const wxFileName& filePath);
    void HandleBrowserWatcherChange();
    void RefreshBrowserWatcher();

    void HandleMarkdownDocumentChanged(const MarkdownEditorPanel::DocumentChange& change);
    void HandleMarkdownEditorStatusChanged(const MarkdownEditorPanel::StatusMessage& message);
    MarkdownEditorPanel::SavePromptDecision HandleConfirmSaveBeforeDiscard(
        const MarkdownEditorPanel::SavePrompt& prompt);
    MarkdownEditorPanel::OverwritePromptDecision HandleConfirmOverwriteExternalChanges(
        const MarkdownEditorPanel::OverwritePromptMessage& prompt);
    void HandleMarkdownEditorError(const MarkdownEditorPanel::ErrorMessage& message);
    std::optional<wxFileName> HandleSelectOpenFile();
    std::optional<wxFileName> HandleSelectSaveFile(const wxFileName& currentFile);
    void HandleMarkdownReady(const MarkdownPreviewData& markdownPreviewData);
    void HandleMarkdownError(const wxString& error);

   public:
    explicit MainFrame(wxWindow* parent);
    ~MainFrame();

    void HandleOpenMarkdownFile(const wxFileName& filePath);
};
