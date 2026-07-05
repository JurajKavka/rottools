#pragma once

#include <wx/fswatcher.h>
#include <wx/splitter.h>
#include <wx/timer.h>

#include "FileBrowserTreePanel/FileBrowserTreePanel.h"
#include "MainFrameWx.h"
#include "MarkdownToHtmlAsync/MarkdownToHtmlAsync.h"

// Forward declaration
class WebViewPanel;
class FileBrowserTreePanel;
class HtmlSourcePanel;
class MarkdownSourcePanel;

class MainFrame : public MainFrameWx {
   private:
    MarkdownToHtmlAsync m_markdownParser;
    // Store a pointer to your custom panel
    WebViewPanel* m_webViewPanel = nullptr;
    HtmlSourcePanel* m_htmlSourcePanel = nullptr;
    MarkdownSourcePanel* m_markdownSourcePanel = nullptr;
    FileBrowserTreePanel* m_fileBrowserPanel = nullptr;
    wxSplitterWindow* m_mainSplitter = nullptr;
    // Right of the always-visible preview it holds the source-views area
    wxSplitterWindow* m_rightSplitter = nullptr;
    // Divides the source area into HTML source and markdown source columns
    wxSplitterWindow* m_sourceSplitter = nullptr;
    bool m_showHtmlSource = false;
    bool m_showMarkdownSource = false;
    wxFileSystemWatcher m_fileSystemWatcher;
    // One save in an editor produces a burst of fs events; the timer collapses
    // the burst into a single re-parse of the open file.
    wxTimer m_reloadDebounceTimer;
    wxFileName m_currentFile;
    wxFileName m_browsedDirectory;

    void HandleOpenFileMenuItemClick(wxCommandEvent& event);
    void HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event);
    void HandleToggleHtmlSourcePanelMenuItemClick(wxCommandEvent& event);
    void HandleToggleMarkdownSourcePanelMenuItemClick(wxCommandEvent& event);
    void ApplySourcePanelVisibility();
    void OnMarkdownReady(MarkdownToHtmlAsyncEvent& event);
    void OnMarkdownError(MarkdownToHtmlAsyncEvent& event);
    void OpenMarkdownFile(const wxFileName& filePath);
    void HandleFileSystemWatcherEvent(wxFileSystemWatcherEvent& event);
    void HandleDirectoryChanged(const wxFileName& filePath);
    void OnReloadDebounceTimer(wxTimerEvent& event);
    void RefreshWatchedPaths();

   public:
    explicit MainFrame(wxWindow* parent);
    ~MainFrame();
};
