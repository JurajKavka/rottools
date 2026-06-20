#pragma once

#include <md4c-html.h>
#include <string>
#include <wx/string.h>

inline void OnHtmlChunkGenerated(const MD_CHAR* htmlChunk, MD_SIZE chunkSize, void* userData) {
    std::string* outHtmlString = static_cast<std::string*>(userData);
    outHtmlString->append(htmlChunk, chunkSize);
}

inline wxString ConvertMarkdownToHtml(const wxString& markdownContent) {
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
}
