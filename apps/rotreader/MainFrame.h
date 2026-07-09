#pragma once

#include <wx/fswatcher.h>
#include <wx/splitter.h>
#include <wx/timer.h>

#include "FileBrowserTreePanel.h"
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
    wxFileSystemWatcher m_fileSystemWatcher;
    // One save in an editor produces a burst of fs events; the timer collapses
    // the burst into a single re-parse of the open file.
    wxTimer m_reloadDebounceTimer;
    wxFileName m_currentFile;
    wxFileName m_browsedDirectory;

    void HandleNewWindowMenuItemClick(wxCommandEvent& event);
    void HandleOpenFileMenuItemClick(wxCommandEvent& event);
    void HandleSoloMarkdownPreviewPanelMenuItemClick(wxCommandEvent& event);
    void HandleToggleFileBrowserMenuItemClick(wxCommandEvent& event);
    void HandleToggleHtmlSourcePanelMenuItemClick(wxCommandEvent& event);
    void HandleToggleMarkdownSourcePanelMenuItemClick(wxCommandEvent& event);
    void ApplySourcePanelVisibility();
    void OpenMarkdownFile(const wxFileName& filePath);
    void HandleFileSystemWatcherEvent(wxFileSystemWatcherEvent& event);
    void HandleDirectoryChanged(const wxFileName& filePath);
    void HandleReloadDebounceTimer(wxTimerEvent& event);
    void RefreshWatchedPaths();

    void HandleMarkdownReady(const MarkdownPreviewData& markdownPreviewData);
    void HandleMarkdownError(const wxString& error);

   public:
    explicit MainFrame(wxWindow* parent);
    ~MainFrame();
};
