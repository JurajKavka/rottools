#include "MarkdownToHtmlAsync.h"

#include <md4c-html.h>

#include <fstream>  // For opening the file
#include <sstream>  // For reading the file content
#include <string>
#include <thread>

wxDEFINE_EVENT(EVT_MARKDOWN_READY, MarkdownToHtmlAsyncEvent);
wxDEFINE_EVENT(EVT_MARKDOWN_ERROR, MarkdownToHtmlAsyncEvent);

static void OnHtmlChunkGenerated(const MD_CHAR* htmlChunk, MD_SIZE chunkSize, void* userData) {
    std::string* outHtmlString = static_cast<std::string*>(userData);
    outHtmlString->append(htmlChunk, chunkSize);
};

MarkdownToHtmlAsync::MarkdownToHtmlAsync(wxEvtHandler* parent) : m_parent(parent) {};

wxString MarkdownToHtmlAsync::ConvertMarkdownToHtml(const wxString& markdownContent) {
    std::string rawMarkdownInput = markdownContent.ToStdString(wxConvUTF8);
    std::string htmlOutputBuffer;

    // Enable Pro-level Markdown features (Tables, Checkboxes, Strikethrough)
    unsigned int parserFlags = MD_FLAG_TABLES | MD_FLAG_TASKLISTS | MD_FLAG_STRIKETHROUGH | MD_FLAG_PERMISSIVEAUTOLINKS;

    unsigned int renderFlags = MD_HTML_FLAG_SKIP_UTF8_BOM;

    int parseResult = md_html(rawMarkdownInput.c_str(), (MD_SIZE)rawMarkdownInput.size(), OnHtmlChunkGenerated,
                              &htmlOutputBuffer, parserFlags, renderFlags);

    if (parseResult != 0) {
        return wxString("<strong>Error: The internal parser failed to evaluate this document layout.</strong>");
    }

    return wxString::FromUTF8(htmlOutputBuffer);
};

void MarkdownToHtmlAsync::ParseFile(const wxString& filePath) {
    std::weak_ptr<MarkdownToHtmlAsync> weakSelf = shared_from_this();
    m_workerThread = std::jthread([weakSelf, filePath]() {
        auto self = weakSelf.lock();
        if (!self) {
            return;
        }

        // TODO: You can check st.stop_requested() here if you implement
        // a way to cancel the thread!

        std::ifstream file(filePath.ToStdString());
        if (!file.is_open()) {
            wxThreadEvent* event = new wxThreadEvent(EVT_MARKDOWN_ERROR);
            event->SetString(wxString("File open error!"));
            wxQueueEvent(self->m_parent, event);
            return;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string markdownStr = buffer.str();

        wxString rawMarkdown = wxString::FromUTF8(markdownStr);

        wxString htmlContent = self->ConvertMarkdownToHtml(rawMarkdown);
        MarkdownToHtmlAsyncEvent* event = new MarkdownToHtmlAsyncEvent(EVT_MARKDOWN_READY, wxID_ANY);
        event->html = htmlContent;
        event->filePath = filePath;
        wxQueueEvent(self->m_parent, event);
    });
}

void MarkdownToHtmlAsync::AbortParseFile() {
    m_workerThread.request_stop();
}