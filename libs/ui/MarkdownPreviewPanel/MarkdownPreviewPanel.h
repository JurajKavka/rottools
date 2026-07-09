#pragma once

#include "MarkdownToHtmlAsync.h"
#include "WebViewPanel.h"

struct MarkdownPreviewData {
    wxString html;
    wxString markdown;
    wxFileName fileName;
};

class MarkdownPreviewPanel : public WebViewPanel {
   public:
    using OnMarkdownReadyCallback = std::function<void(const MarkdownPreviewData& markdownPreviewData)>;
    using OnMarkdownErrorCallback = std::function<void(const wxString& error)>;
    explicit MarkdownPreviewPanel(wxWindow* parent, OnMarkdownReadyCallback onMarkdownReadyCallback = nullptr,
                                  OnMarkdownErrorCallback onMarkdownErrorCallback = nullptr);

    void LoadFile(const wxFileName& fileName);

   private:
    MarkdownToHtmlAsync m_markdownParser;
    OnMarkdownReadyCallback m_onMarkdownReadyCallback;
    OnMarkdownErrorCallback m_onMarkdownErrorCallback;
    void HandleMarkdownReady(MarkdownToHtmlAsyncEvent& event);
    void HandleMarkdownError(MarkdownToHtmlAsyncEvent& event);
    wxString GetHtmlPage(wxString& parsedMarkdownToHtml);
};