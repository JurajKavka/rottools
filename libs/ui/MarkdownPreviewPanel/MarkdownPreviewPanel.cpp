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
    m_origin = MarkdownOrigin::Disk;
    m_parsePending = true;
    m_hasCurrentParse = false;
    m_parseRequestId = m_markdownParser.ParseFile(fileName);
}

void MarkdownPreviewPanel::LoadMarkdown(const wxString& markdown, const wxFileName& fileName,
                                        MarkdownPreviewOptions options) {
    m_options = std::move(options);
    m_origin = MarkdownOrigin::Memory;
    m_parsePending = true;
    m_hasCurrentParse = false;
    m_parseRequestId = m_markdownParser.ParseText(markdown, fileName);
}

void MarkdownPreviewPanel::Render(MarkdownPreviewOptions options) {
    if (m_parsePending) {
        // A theme can change while a new document is parsing. Keep that style
        // for the pending result, but retain the load request's scroll behavior
        // and do not repaint the previous document as though it were current.
        m_options.injectStyle = std::move(options.injectStyle);
        return;
    }

    m_options = std::move(options);
    if (!m_hasCurrentParse) {
        return;
    }
    Paint();
}

wxString MarkdownPreviewPanel::GetHtmlPage(const wxString& parsedMarkdownToHtml,
                                           const MarkdownPreviewOptions& options) const {
    wxString styleTag = options.injectStyle.IsEmpty()
                            ? wxString()
                            : wxString::FromUTF8(std::format("<style>{}</style>", options.injectStyle));

    wxString finalHtml = wxString::FromUTF8(std::format(R"(
<!DOCTYPE html>
  <html>
  <head>
    {}
  </head>
  <body>
    {}
  </body>
</html>)",
                                                        styleTag, parsedMarkdownToHtml));
    return finalHtml;
}

void MarkdownPreviewPanel::Paint() {
    m_htmlPage = this->GetHtmlPage(m_parsedHtml, m_options);
    this->LoadHtml(m_htmlPage, m_options.scrollBehavior);

    if (m_onMarkdownReadyCallback) {
        MarkdownPreviewData markdownPreviewData = {.html = m_htmlPage,
                                                   .markdown = m_markdown,
                                                   .fileName = m_fileName,
                                                   .scrollBehavior = m_options.scrollBehavior,
                                                   .origin = m_origin};
        m_onMarkdownReadyCallback(markdownPreviewData);
    }
}

void MarkdownPreviewPanel::HandleMarkdownReady(MarkdownToHtmlAsyncEvent& event) {
    if (event.requestId != m_parseRequestId) {
        return;
    }

    m_parsePending = false;
    m_hasCurrentParse = true;
    m_parsedHtml = event.html;
    m_markdown = event.markdown;
    m_fileName = event.filePath;
    Paint();
}

void MarkdownPreviewPanel::HandleMarkdownError(MarkdownToHtmlAsyncEvent& event) {
    if (event.requestId != m_parseRequestId) {
        return;
    }

    m_parsePending = false;
    m_hasCurrentParse = false;
    if (m_onMarkdownErrorCallback) {
        m_onMarkdownErrorCallback(event.error);
    }
}
