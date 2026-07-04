#pragma once

#include "MainFrameWx.h"
#include "MarkdownToHtmlAsync/MarkdownToHtmlAsync.h"
#include "FileBrowserTreePanel/FileBrowserTreePanel.h"
#include <wx/splitter.h>

// Forward declaration
class WebViewPanel;
class FileBrowserTreePanel;

class MainFrame : public MainFrameWx {
   private:
    std::shared_ptr<MarkdownToHtmlAsync> m_parserThread;
    // Store a pointer to your custom panel
    WebViewPanel* m_webViewPanel = nullptr;
    FileBrowserTreePanel* m_fileBrowserPanel = nullptr;
    wxSplitterWindow* m_mainSplitter = nullptr;
    wxFileSystemWatcher* m_fileSystemWatcher;

    void HandleOpenFileMenuItemClick(wxCommandEvent& event);
    void HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event);
    void OnMarkdownReady(MarkdownToHtmlAsyncEvent& event);
    void OnMarkdownError(MarkdownToHtmlAsyncEvent& event);
    void HandleFileOpened(const wxFileName& filePath);
    void HandleFileSystemWatcherEvent(wxFileSystemWatcherEvent& event);
    void HandleDirectoryChanged(const wxFileName& filePath);

   public:
    MainFrame(wxWindow* parent);
    ~MainFrame();
    void OpenFile(const wxString& filePath);
};
