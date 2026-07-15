#include "MarkdownPreviewPanel.h"

MarkdownPreviewPanel::MarkdownPreviewPanel(wxWindow* parent, OnMarkdownReadyCallback onMarkdownReadyCallback,
                                           OnMarkdownErrorCallback onMarkdownErrorCallback)
    : WebViewPanel(parent),
      m_markdownParser(this),
      m_onMarkdownReadyCallback(std::move(onMarkdownReadyCallback)),
      m_onMarkdownErrorCallback(std::move(onMarkdownErrorCallback)) {
    Bind(EVT_MARKDOWN_READY, &MarkdownPreviewPanel::HandleMarkdownReady, this);
    Bind(EVT_MARKDOWN_ERROR, &MarkdownPreviewPanel::HandleMarkdownError, this);
};

void MarkdownPreviewPanel::LoadFile(const wxFileName& fileName, MarkdownPreviewOptions options) {
    m_options = std::move(options);
    m_markdownParser.ParseFile(fileName);
}

void MarkdownPreviewPanel::Render(MarkdownPreviewOptions options) {
    m_options = std::move(options);
    if (m_parsedHtml.IsEmpty()) {
        return;
    }
    Paint();
}

wxString MarkdownPreviewPanel::GetHtmlPage(const wxString& parsedMarkdownToHtml,
                                           const MarkdownPreviewOptions& options) const {
    wxString styleTag =
        options.injectStyle.IsEmpty() ? wxString() : std::format("<style>{}</style>", options.injectStyle);

    wxString finalHtml = std::format(R"(
<!DOCTYPE html>
  <html>
  <head>
    {}
  </head>
  <body>
    {}
  </body>
</html>)",
                                     styleTag, parsedMarkdownToHtml);
    return finalHtml;
}

void MarkdownPreviewPanel::Paint() {
    m_htmlPage = this->GetHtmlPage(m_parsedHtml, m_options);
    this->LoadHtml(m_htmlPage);

    if (m_onMarkdownReadyCallback) {
        MarkdownPreviewData markdownPreviewData = {.html = m_htmlPage, .markdown = m_markdown, .fileName = m_fileName};
        m_onMarkdownReadyCallback(markdownPreviewData);
    }
}

void MarkdownPreviewPanel::HandleMarkdownReady(MarkdownToHtmlAsyncEvent& event) {
    m_parsedHtml = event.html;
    m_markdown = event.markdown;
    m_fileName = event.filePath;
    Paint();
}

void MarkdownPreviewPanel::HandleMarkdownError(MarkdownToHtmlAsyncEvent& event) {
    if (m_onMarkdownErrorCallback) {
        m_onMarkdownErrorCallback(event.error);
    }
}
