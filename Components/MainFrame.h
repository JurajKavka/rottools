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

class MainFrame : public MainFrameWx {
   private:
    MarkdownToHtmlAsync m_markdownParser;
    // Store a pointer to your custom panel
    WebViewPanel* m_webViewPanel = nullptr;
    FileBrowserTreePanel* m_fileBrowserPanel = nullptr;
    wxSplitterWindow* m_mainSplitter = nullptr;
    wxFileSystemWatcher m_fileSystemWatcher;
    // One save in an editor produces a burst of fs events; the timer collapses
    // the burst into a single re-parse of the open file.
    wxTimer m_reloadDebounceTimer;
    wxFileName m_currentFile;
    wxFileName m_browsedDirectory;

    void HandleOpenFileMenuItemClick(wxCommandEvent& event);
    void HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event);
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
