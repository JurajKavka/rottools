#pragma once

#include <wx/splitter.h>

#include <memory>

#include "CssThemes.h"
#include "FileBrowserTreePanel.h"
#include "FsWatcher.h"
#include "MainFrameWx.h"
#include "MarkdownPreviewPanel.h"

// Forward declarations
class HtmlSourcePanel;
class MarkdownSourcePanel;

class MainFrame : public MainFrameWx {
   private:
    // Store a pointer to your custom panel
    MarkdownPreviewPanel* m_markdownPreviewPanel = nullptr;
    HtmlSourcePanel* m_htmlSourcePanel = nullptr;
    MarkdownSourcePanel* m_markdownSourcePanel = nullptr;
    FileBrowserTreePanel* m_fileBrowserPanel = nullptr;
    wxSplitterWindow* m_mainSplitter = nullptr;
    // Right of the always-visible preview it holds the source-views area
    wxSplitterWindow* m_rightSplitter = nullptr;
    // Divides the source area into HTML source and markdown source columns
    wxSplitterWindow* m_sourceSplitter = nullptr;
    // Watches the browsed directory (list reloads) and the open document (live
    // reload); recreated by RefreshWatchedPaths when either target changes.
    std::unique_ptr<FsWatcher> m_browserWatcher;
    std::unique_ptr<FsWatcher> m_documentWatcher;
    wxFileName m_currentFile;
    wxFileName m_browsedDirectory;
    // The document text already loaded and rendered, i.e. what we believe is on
    // disk. ReloadOpenDocument compares against it to ignore the file-system
    // event our own save triggers, which has already been rendered directly.
    wxString m_loadedText;
    // First of the CssThemeCount consecutive ids given to the Theme menu items
    wxWindowID m_themeMenuBaseId = wxID_ANY;
    // Index into cssThemes; the frame owns the theme, the preview panel does not
    int m_themeId = RotdownMonoLight;

    void HandleNewWindowMenuItemClick(wxCommandEvent& event);
    void HandleOpenFileMenuItemClick(wxCommandEvent& event);
    void HandleSoloMarkdownPreviewPanelMenuItemClick(wxCommandEvent& event);
    void HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event);
    void HandleToggleHtmlSourcePanelMenuItemClick(wxCommandEvent& event);
    void HandleToggleMarkdownSourcePanelMenuItemClick(wxCommandEvent& event);
    void HandleHtmlSourcePanelClose();
    void HandleMarkdownSourcePanelClose();
    void ApplySourcePanelVisibility();
    void PopulateThemeMenu();
    void HandleThemeMenuItemClick(wxCommandEvent& event);
    MarkdownPreviewOptions GetPreviewOptions(ScrollBehavior scrollBehavior = ScrollBehavior::ResetToTop) const;
    void ReloadOpenDocument();
    void HandleDirectoryChanged(const wxFileName& filePath);
    void RefreshWatchedPaths();

    void HandleMarkdownReady(const MarkdownPreviewData& markdownPreviewData);
    void HandleMarkdownError(const wxString& error);
    void HandleMarkdownSourceSave(const wxString& markdown);

   public:
    explicit MainFrame(wxWindow* parent);
    ~MainFrame();

    void OpenMarkdownFile(const wxFileName& filePath);
};
