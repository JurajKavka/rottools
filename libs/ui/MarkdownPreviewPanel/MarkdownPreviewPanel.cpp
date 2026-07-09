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

void MarkdownPreviewPanel::LoadFile(const wxFileName& fileName) {
    m_markdownParser.ParseFile(fileName);
}

wxString MarkdownPreviewPanel::GetHtmlPage(wxString& parsedMarkdownToHtml) {
    wxString finalHtml = std::format(R"(
<!DOCTYPE html>
  <html>
  <head>
    <link rel="stylesheet" href="https://classless.de/classless.css">
  </head>
  <body>
    {}
  </body>
</html>)",
                                     parsedMarkdownToHtml);
    return finalHtml;
}

void MarkdownPreviewPanel::HandleMarkdownReady(MarkdownToHtmlAsyncEvent& event) {
    wxString htmlPage = this->GetHtmlPage(event.html);
    this->LoadHtml(htmlPage);
    if (m_onMarkdownReadyCallback) {
        MarkdownPreviewData markdownPreviewData = {
            .html = htmlPage, .markdown = event.markdown, .fileName = event.filePath};
        m_onMarkdownReadyCallback(markdownPreviewData);
    }
}

void MarkdownPreviewPanel::HandleMarkdownError(MarkdownToHtmlAsyncEvent& event) {
    if (m_onMarkdownErrorCallback) {
        m_onMarkdownErrorCallback(event.error);
    }
}