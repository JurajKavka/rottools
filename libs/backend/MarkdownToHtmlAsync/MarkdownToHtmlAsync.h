#pragma once

#include <wx/wx.h>

#include <atomic>
#include <string>
#include <thread>

#include "MarkdownToHtmlAsyncEvent.h"

wxDECLARE_EVENT(EVT_MARKDOWN_READY, MarkdownToHtmlAsyncEvent);
wxDECLARE_EVENT(EVT_MARKDOWN_ERROR, MarkdownToHtmlAsyncEvent);

class MarkdownToHtmlAsync {
   public:
    explicit MarkdownToHtmlAsync(wxEvtHandler* parent);
    ~MarkdownToHtmlAsync();

    void ParseFile(const wxFileName& filePath);
    void AbortParseFile();  // New method to trigger cancellation

   private:
    wxEvtHandler* m_parent;
    // Plain std::thread + atomic flag instead of std::jthread/std::stop_token:
    // Apple's libc++ only ships jthread from Xcode 26.4, too new to require.
    std::thread m_workerThread;  // Store the thread as a member
    std::atomic<bool> m_stopRequested{false};
    void StopWorker();  // request stop + join (std::thread must be joined before reuse/destruction)
    [[nodiscard]] static wxString ConvertMarkdownToHtml(const std::string& markdownContent);
};
