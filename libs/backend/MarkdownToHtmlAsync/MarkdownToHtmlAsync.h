#pragma once

#include <wx/wx.h>

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>

#include "MarkdownToHtmlAsyncEvent.h"

wxDECLARE_EVENT(EVT_MARKDOWN_READY, MarkdownToHtmlAsyncEvent);
wxDECLARE_EVENT(EVT_MARKDOWN_ERROR, MarkdownToHtmlAsyncEvent);

class MarkdownToHtmlAsync {
   public:
    using RequestId = std::uint64_t;

    explicit MarkdownToHtmlAsync(wxEvtHandler* parent);
    ~MarkdownToHtmlAsync();

    [[nodiscard]] RequestId ParseFile(const wxFileName& filePath);

    /**
     * @brief Parses markdown already held in memory, without touching the disk.
     *
     * Used after the editor saves: the text is already there, so re-reading the
     * file we just wrote would only add latency and a chance of reading a
     * half-written file.
     *
     * @param markdown The markdown to parse
     * @param filePath The file the markdown belongs to; travels on the result event
     */
    [[nodiscard]] RequestId ParseText(const wxString& markdown, const wxFileName& filePath);

    void AbortParseFile();  // New method to trigger cancellation

   private:
    wxEvtHandler* m_parent;
    // Plain std::thread + atomic flag instead of std::jthread/std::stop_token:
    // Apple's libc++ only ships jthread from Xcode 26.4, too new to require.
    std::thread m_workerThread;  // Store the thread as a member
    std::atomic<bool> m_stopRequested{false};
    RequestId m_nextRequestId = 0;
    void StopWorker();  // request stop + join (std::thread must be joined before reuse/destruction)

    /**
     * @brief Starts the parse worker, the one code path behind ParseFile and ParseText.
     *
     * @param filePath The file the markdown belongs to; travels on the result event
     * @param readFromDisk true reads filePath on the worker thread, false parses markdown as given
     * @param markdown The UTF-8 markdown to parse; ignored when readFromDisk is true
     */
    [[nodiscard]] RequestId StartWorker(const wxFileName& filePath, bool readFromDisk, std::string markdown);

    [[nodiscard]] static wxString ConvertMarkdownToHtml(const std::string& markdownContent);
};
