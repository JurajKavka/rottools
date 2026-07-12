#pragma once

#include "MarkdownToHtmlAsync.h"
#include "WebViewPanel.h"

struct MarkdownPreviewOptions {
    // Bare CSS, no <style> tag: the panel wraps it in one. Empty means no styling.
    wxString injectStyle;
};

struct MarkdownPreviewData {
    const wxString& html;
    const wxString& markdown;
    const wxFileName& fileName;
};

class MarkdownPreviewPanel : public WebViewPanel {
   public:
    using OnMarkdownReadyCallback = std::function<void(const MarkdownPreviewData& markdownPreviewData)>;
    using OnMarkdownErrorCallback = std::function<void(const wxString& error)>;
    explicit MarkdownPreviewPanel(wxWindow* parent, OnMarkdownReadyCallback onMarkdownReadyCallback = nullptr,
                                  OnMarkdownErrorCallback onMarkdownErrorCallback = nullptr);

    void LoadFile(const wxFileName& fileName, MarkdownPreviewOptions options = {});
    void Render(MarkdownPreviewOptions options);

   private:
    MarkdownToHtmlAsync m_markdownParser;
    OnMarkdownReadyCallback m_onMarkdownReadyCallback;
    OnMarkdownErrorCallback m_onMarkdownErrorCallback;
    MarkdownPreviewOptions m_options;
    wxString m_parsedHtml;
    wxString m_markdown;
    wxFileName m_fileName;
    void HandleMarkdownReady(MarkdownToHtmlAsyncEvent& event);
    void HandleMarkdownError(MarkdownToHtmlAsyncEvent& event);
    wxString GetHtmlPage(const wxString& parsedMarkdownToHtml, const MarkdownPreviewOptions& options) const;
    void Paint();
};
