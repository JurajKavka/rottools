#pragma once

#include <wx/fswatcher.h>
#include <wx/splitter.h>

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

    void HandleOpenFileMenuItemClick(wxCommandEvent& event);
    void HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event);
    void OnMarkdownReady(MarkdownToHtmlAsyncEvent& event);
    void OnMarkdownError(MarkdownToHtmlAsyncEvent& event);
    void OpenMarkdownFile(const wxFileName& filePath);
    void HandleFileSystemWatcherEvent(wxFileSystemWatcherEvent& event);
    void HandleDirectoryChanged(const wxFileName& filePath);

   public:
    explicit MainFrame(wxWindow* parent);
};
