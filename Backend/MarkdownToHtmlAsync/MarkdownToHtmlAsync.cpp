#include "MarkdownToHtmlAsync.h"

#include <md4c-html.h>

#include <filesystem>
#include <fstream>  // For opening the file
#include <sstream>  // For reading the file content
#include <string>
#include <thread>

wxDEFINE_EVENT(EVT_MARKDOWN_READY, MarkdownToHtmlAsyncEvent);
wxDEFINE_EVENT(EVT_MARKDOWN_ERROR, MarkdownToHtmlAsyncEvent);

static void HandleHtmlChunkGenerated(const MD_CHAR* htmlChunk, MD_SIZE chunkSize, void* userData) {
    std::string* outHtmlString = static_cast<std::string*>(userData);
    outHtmlString->append(htmlChunk, chunkSize);
}

MarkdownToHtmlAsync::MarkdownToHtmlAsync(wxEvtHandler* parent) : m_parent(parent) {}

wxString MarkdownToHtmlAsync::ConvertMarkdownToHtml(const std::string& markdownContent) {
    std::string htmlOutputBuffer;

    // Enable Pro-level Markdown features (Tables, Checkboxes, Strikethrough)
    unsigned int parserFlags = MD_FLAG_TABLES | MD_FLAG_TASKLISTS | MD_FLAG_STRIKETHROUGH | MD_FLAG_PERMISSIVEAUTOLINKS;

    unsigned int renderFlags = MD_HTML_FLAG_SKIP_UTF8_BOM;

    int parseResult = md_html(markdownContent.c_str(), static_cast<MD_SIZE>(markdownContent.size()),
                              HandleHtmlChunkGenerated, &htmlOutputBuffer, parserFlags, renderFlags);

    if (parseResult != 0) {
        return wxString("<strong>Error: The internal parser failed to evaluate this document layout.</strong>");
    }

    return wxString::FromUTF8(htmlOutputBuffer);
}

void MarkdownToHtmlAsync::ParseFile(const wxFileName& filePath) {
    std::filesystem::path path = filePath.GetFullPath().ToStdWstring();

    // Capture a raw `this` rather than extending our own lifetime via a shared_ptr.
    // The parser is owned by the MainFrame; ~MarkdownToHtmlAsync (running on the
    // owner's thread) requests stop and joins this thread before we are destroyed,
    // so the worker can never end up destroying/joining itself.
    m_workerThread = std::jthread([this, path, filePath](std::stop_token stoken) {
        // An exception escaping a jthread calls std::terminate, so guard the body
        try {
            std::ifstream file(path);
            if (!file.is_open()) {
                MarkdownToHtmlAsyncEvent* event = new MarkdownToHtmlAsyncEvent(EVT_MARKDOWN_ERROR, wxID_ANY);
                event->error = wxString("Could not open file: ") + filePath.GetFullPath();
                event->filePath = filePath;
                wxQueueEvent(m_parent, event);
                return;
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string markdownStr = buffer.str();

            // Bail out cheaply if a newer request or shutdown superseded this one,
            // so the jthread join/reassign doesn't block on a stale parse
            if (stoken.stop_requested()) {
                return;
            }

            wxString htmlContent = ConvertMarkdownToHtml(markdownStr);

            if (stoken.stop_requested()) {
                return;
            }

            MarkdownToHtmlAsyncEvent* event = new MarkdownToHtmlAsyncEvent(EVT_MARKDOWN_READY, wxID_ANY);
            event->html = htmlContent;
            event->markdown = wxString::FromUTF8(markdownStr);
            event->filePath = filePath;
            wxQueueEvent(m_parent, event);
        } catch (const std::exception& e) {
            MarkdownToHtmlAsyncEvent* event = new MarkdownToHtmlAsyncEvent(EVT_MARKDOWN_ERROR, wxID_ANY);
            event->error = wxString::FromUTF8(e.what());
            event->filePath = filePath;
            wxQueueEvent(m_parent, event);
        }
    });
}

void MarkdownToHtmlAsync::AbortParseFile() {
    m_workerThread.request_stop();
}
