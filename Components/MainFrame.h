#pragma once

#include "MainFrameWx.h"
#include "MarkdownToHtmlAsync/MarkdownToHtmlAsync.h"

// Forward declaration
class WebViewPanel;

class MainFrame : public MainFrameWx {
   private:
    std::shared_ptr<MarkdownToHtmlAsync> m_parserThread;
    // Store a pointer to your custom panel
    WebViewPanel* m_webViewPanel;
    void HandleOpenFileMenuItemClick(wxCommandEvent& event);
    void OnMarkdownReady(MarkdownToHtmlAsyncEvent& event);
    void OnMarkdownError(MarkdownToHtmlAsyncEvent& event);

   public:
    MainFrame(wxWindow* parent);
    ~MainFrame();
    void OpenFile(const wxString& filePath);
};
