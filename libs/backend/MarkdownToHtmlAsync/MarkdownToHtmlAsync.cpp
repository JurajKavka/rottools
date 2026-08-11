#include "MarkdownToHtmlAsync.h"

#include <md4c-html.h>

#include <filesystem>
#include <fstream>  // For opening the file
#include <sstream>  // For reading the file content
#include <string>
#include <thread>
#include <utility>
#include <vector>

wxDEFINE_EVENT(EVT_MARKDOWN_READY, MarkdownToHtmlAsyncEvent);
wxDEFINE_EVENT(EVT_MARKDOWN_ERROR, MarkdownToHtmlAsyncEvent);

static void HandleHtmlChunkGenerated(const MD_CHAR* htmlChunk, MD_SIZE chunkSize, void* userData) {
    std::string* outHtmlString = static_cast<std::string*>(userData);
    outHtmlString->append(htmlChunk, chunkSize);
}

namespace {

// YAML frontmatter (the "---" metadata block used by Jekyll, Obsidian, Hugo, ...)
// is not part of CommonMark; fed to md4c it comes out as a thematic break plus a
// setext <h2>. We split it off before parsing and render it as a properties block.

struct FrontmatterSplit {
    std::string frontmatter;  // content between the delimiters (empty if none found)
    std::string body;         // the markdown to feed to the parser
};

// A frontmatter block only counts when the very first line of the document is
// exactly "---"; that position rule keeps horizontal rules and setext headings
// working everywhere else. The block ends with a "---" or "..." line; without
// a terminator the document is left untouched.
FrontmatterSplit SplitFrontmatter(const std::string& markdown) {
    const std::string utf8Bom = "\xEF\xBB\xBF";
    size_t documentStart = markdown.starts_with(utf8Bom) ? utf8Bom.size() : 0;

    auto readLine = [&markdown](size_t from, size_t& next) {
        size_t newline = markdown.find('\n', from);
        std::string line;
        if (newline == std::string::npos) {
            line = markdown.substr(from);
            next = markdown.size();
        } else {
            line = markdown.substr(from, newline - from);
            next = newline + 1;
        }
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        return line;
    };

    size_t next = 0;
    if (readLine(documentStart, next) != "---") {
        return FrontmatterSplit{.frontmatter = "", .body = markdown};
    }

    size_t contentStart = next;
    size_t cursor = next;
    while (cursor < markdown.size()) {
        size_t lineStart = cursor;
        std::string line = readLine(cursor, next);
        if (line == "---" || line == "...") {
            return FrontmatterSplit{.frontmatter = markdown.substr(contentStart, lineStart - contentStart),
                                    .body = markdown.substr(next)};
        }
        cursor = next;
    }
    return FrontmatterSplit{.frontmatter = "", .body = markdown};
}

std::string EscapeHtml(const std::string& text) {
    std::string escaped;
    escaped.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '&':
                escaped += "&amp;";
                break;
            case '<':
                escaped += "&lt;";
                break;
            case '>':
                escaped += "&gt;";
                break;
            case '"':
                escaped += "&quot;";
                break;
            default:
                escaped += c;
                break;
        }
    }
    return escaped;
}

std::string TrimWhitespace(const std::string& text) {
    size_t first = text.find_first_not_of(" \t");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = text.find_last_not_of(" \t");
    return text.substr(first, last - first + 1);
}

// Renders the frontmatter as an Obsidian-style properties table. Only shallow
// "key: value" lines are recognized; indented lines continue the previous
// value and anything else lands in the value column as-is. The gray tones are
// semi-transparent so the block fits both light and dark webview themes.
std::string RenderFrontmatterHtml(const std::string& frontmatter) {
    std::vector<std::pair<std::string, std::string>> rows;

    std::istringstream lines(frontmatter);
    std::string line;
    while (std::getline(lines, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (TrimWhitespace(line).empty()) {
            continue;
        }
        bool isContinuation = (line.starts_with(' ') || line.starts_with('\t')) && !rows.empty();
        if (isContinuation) {
            rows.back().second += " " + TrimWhitespace(line);
            continue;
        }
        size_t colon = line.find(':');
        if (colon == std::string::npos) {
            rows.emplace_back("", TrimWhitespace(line));
        } else {
            rows.emplace_back(TrimWhitespace(line.substr(0, colon)), TrimWhitespace(line.substr(colon + 1)));
        }
    }
    if (rows.empty()) {
        return "";
    }

    std::string html =
        "<table style=\"border-collapse:separate; border-spacing:0; margin:0 0 1.5em 0; "
        "border:1px solid rgba(128,128,128,0.4); border-radius:6px; "
        "background:rgba(128,128,128,0.12); font-size:0.9em;\">\n";
    for (const auto& [key, value] : rows) {
        html +=
            "<tr><td style=\"padding:4px 14px; font-weight:600; vertical-align:top; "
            "white-space:nowrap; opacity:0.75;\">" +
            EscapeHtml(key) + "</td><td style=\"padding:4px 14px 4px 0;\">" + EscapeHtml(value) + "</td></tr>\n";
    }
    html += "</table>\n";
    return html;
}

}  // namespace

MarkdownToHtmlAsync::MarkdownToHtmlAsync(wxEvtHandler* parent) : m_parent(parent) {}

MarkdownToHtmlAsync::~MarkdownToHtmlAsync() {
    StopWorker();
}

void MarkdownToHtmlAsync::StopWorker() {
    m_stopRequested = true;
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

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

MarkdownToHtmlAsync::RequestId MarkdownToHtmlAsync::ParseFile(const wxFileName& filePath) {
    return StartWorker(filePath, /*readFromDisk=*/true, {});
}

MarkdownToHtmlAsync::RequestId MarkdownToHtmlAsync::ParseText(const wxString& markdown, const wxFileName& filePath) {
    const wxScopedCharBuffer utf8 = markdown.utf8_str();
    return StartWorker(filePath, /*readFromDisk=*/false, std::string(utf8.data(), utf8.length()));
}

MarkdownToHtmlAsync::RequestId MarkdownToHtmlAsync::StartWorker(const wxFileName& filePath, bool readFromDisk,
                                                                std::string markdown) {
    std::filesystem::path path = filePath.GetFullPath().ToStdWstring();

    // Retire any in-flight parse first: assigning over a joinable std::thread
    // calls std::terminate, so stop + join before reusing the member.
    StopWorker();
    m_stopRequested = false;
    const RequestId requestId = ++m_nextRequestId;

    // Capture a raw `this` rather than extending our own lifetime via a shared_ptr.
    // The parser is owned by the MainFrame; ~MarkdownToHtmlAsync (running on the
    // owner's thread) requests stop and joins this thread before we are destroyed,
    // so the worker can never end up destroying/joining itself.
    m_workerThread =
        std::thread([this, path, filePath, readFromDisk, markdown = std::move(markdown), requestId]() mutable {
            // An exception escaping the worker thread calls std::terminate, so guard the body
            try {
                std::string markdownStr = std::move(markdown);
                if (readFromDisk) {
                    std::ifstream file(path);
                    if (!file.is_open()) {
                        MarkdownToHtmlAsyncEvent* event = new MarkdownToHtmlAsyncEvent(EVT_MARKDOWN_ERROR, wxID_ANY);
                        event->error = wxString("Could not open file: ") + filePath.GetFullPath();
                        event->filePath = filePath;
                        event->requestId = requestId;
                        wxQueueEvent(m_parent, event);
                        return;
                    }
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    markdownStr = buffer.str();
                }

                // Bail out cheaply if a newer request or shutdown superseded this one,
                // so the join/reassign doesn't block on a stale parse
                if (m_stopRequested) {
                    return;
                }

                FrontmatterSplit split = SplitFrontmatter(markdownStr);
                wxString htmlContent = ConvertMarkdownToHtml(split.body);
                if (!split.frontmatter.empty()) {
                    htmlContent = wxString::FromUTF8(RenderFrontmatterHtml(split.frontmatter)) + htmlContent;
                }

                if (m_stopRequested) {
                    return;
                }

                MarkdownToHtmlAsyncEvent* event = new MarkdownToHtmlAsyncEvent(EVT_MARKDOWN_READY, wxID_ANY);
                event->html = htmlContent;
                event->markdown = wxString::FromUTF8(markdownStr);
                event->filePath = filePath;
                event->requestId = requestId;
                wxQueueEvent(m_parent, event);
            } catch (const std::exception& e) {
                MarkdownToHtmlAsyncEvent* event = new MarkdownToHtmlAsyncEvent(EVT_MARKDOWN_ERROR, wxID_ANY);
                event->error = wxString::FromUTF8(e.what());
                event->filePath = filePath;
                event->requestId = requestId;
                wxQueueEvent(m_parent, event);
            }
        });

    return requestId;
}

void MarkdownToHtmlAsync::AbortParseFile() {
    m_stopRequested = true;
}
