#pragma once

#include <wx/wx.h>

#include <thread>  // 1. REQUIRED: Include memory for shared_ptr and enable_shared_from_this

#include "MarkdownToHtmlAsyncEvent.h"

wxDECLARE_EVENT(EVT_MARKDOWN_READY, MarkdownToHtmlAsyncEvent);
wxDECLARE_EVENT(EVT_MARKDOWN_ERROR, MarkdownToHtmlAsyncEvent);

class MarkdownToHtmlAsync : public std::enable_shared_from_this<MarkdownToHtmlAsync> {
   public:
    explicit MarkdownToHtmlAsync(wxEvtHandler* parent);

    void ParseFile(const wxFileName& filePath);
    void AbortParseFile();  // New method to trigger cancellation

   private:
    wxEvtHandler* m_parent;
    std::jthread m_workerThread;  // Store the thread as a member
    [[nodiscard]] wxString ConvertMarkdownToHtml(const wxString& markdownContent);
};