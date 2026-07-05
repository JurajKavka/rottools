#pragma once

#include <wx/wx.h>

#include <string>
#include <thread>

#include "MarkdownToHtmlAsyncEvent.h"

wxDECLARE_EVENT(EVT_MARKDOWN_READY, MarkdownToHtmlAsyncEvent);
wxDECLARE_EVENT(EVT_MARKDOWN_ERROR, MarkdownToHtmlAsyncEvent);

class MarkdownToHtmlAsync {
   public:
    explicit MarkdownToHtmlAsync(wxEvtHandler* parent);

    void ParseFile(const wxFileName& filePath);
    void AbortParseFile();  // New method to trigger cancellation

   private:
    wxEvtHandler* m_parent;
    std::jthread m_workerThread;  // Store the thread as a member
    [[nodiscard]] static wxString ConvertMarkdownToHtml(const std::string& markdownContent);
};
