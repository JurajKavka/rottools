#pragma once

#include "MarkdownToHtmlAsync.h"
#include "WebViewPanel.h"

/**
 * @brief Styling for the rendered page. The panel has no styling of its own.
 */
struct MarkdownPreviewOptions {
    /// Bare CSS, without a <style> tag: the panel wraps it in one. Empty renders the page unstyled.
    wxString injectStyle;
    /// KeepPosition holds the scroll when re-rendering the same document (a
    /// live reload or a theme change). Ignored on the first load of a document,
    /// which always starts at the top.
    ScrollBehavior scrollBehavior = ScrollBehavior::ResetToTop;
};

/**
 * @brief Where the markdown behind a repaint came from.
 *
 * The owner needs this to decide whether to push the text into an editor: text
 * that came out of that editor must not be written back into it.
 */
enum class MarkdownOrigin {
    /// Read from the file (LoadFile)
    Disk,
    /// Handed in already in memory (LoadMarkdown)
    Memory,
};

/**
 * @brief The rendered document, reported to the owner on every repaint.
 *
 * The members are references to storage owned by the panel and are only valid
 * for the duration of the callback; copy what you need to keep.
 */
struct MarkdownPreviewData {
    /// The full HTML page, including the injected style
    // cppcheck-suppress uninitMemberVarNoCtor -- initialized by every aggregate construction
    const wxString& html;
    /// The markdown source the page was rendered from
    // cppcheck-suppress uninitMemberVarNoCtor -- initialized by every aggregate construction
    const wxString& markdown;
    /// The file the markdown was read from
    // cppcheck-suppress uninitMemberVarNoCtor -- initialized by every aggregate construction
    const wxFileName& fileName;
    ScrollBehavior scrollBehavior;
    MarkdownOrigin origin = MarkdownOrigin::Disk;
};

/**
 * @brief Web view that renders a markdown file as a styled HTML page.
 *
 * Parsing is asynchronous: LoadFile() returns immediately and the panel repaints
 * once the parse finishes. The caller owns the styling and passes it in; the
 * panel keeps the parse, so restyling via Render() costs no re-parse.
 */
class MarkdownPreviewPanel : public WebViewPanel {
   public:
    using OnMarkdownReadyCallback = std::function<void(const MarkdownPreviewData& markdownPreviewData)>;
    using OnMarkdownErrorCallback = std::function<void(const wxString& error)>;

    /**
     * @param parent Parent window
     * @param onMarkdownReadyCallback Called after every repaint with the page just rendered
     * @param onMarkdownErrorCallback Called with the error message when a parse fails
     */
    explicit MarkdownPreviewPanel(wxWindow* parent, OnMarkdownReadyCallback onMarkdownReadyCallback = nullptr,
                                  OnMarkdownErrorCallback onMarkdownErrorCallback = nullptr);

    /**
     * @brief Reads and parses a markdown file, then renders it.
     *
     * Returns before the file is rendered; the ready callback reports completion.
     *
     * @param fileName Markdown file to render
     * @param options Styling to apply to this and any later render, until changed
     */
    void LoadFile(const wxFileName& fileName, MarkdownPreviewOptions options = {});

    /**
     * @brief Parses and renders markdown the caller already holds, without reading the file.
     *
     * Same asynchronous contract as LoadFile. The repaint it produces is reported
     * with MarkdownOrigin::Memory, so an owner that also drives an editor knows
     * not to write the text back into it.
     *
     * @param markdown Markdown to render
     * @param fileName The file this markdown belongs to, reported back in MarkdownPreviewData
     * @param options Styling to apply to this and any later render, until changed
     */
    void LoadMarkdown(const wxString& markdown, const wxFileName& fileName, MarkdownPreviewOptions options = {});

    /**
     * @brief Re-renders the file already parsed with new options, e.g. a theme change.
     *
     * The markdown is not read or parsed again, only wrapped in a new page.
     * Before the first result it keeps the requested style for that result. If
     * another document is currently parsing, it also keeps that load request's
     * scroll behavior instead of repainting the previous document.
     *
     * @param options Styling that replaces the options in force
     */
    void Render(MarkdownPreviewOptions options);

   private:
    MarkdownToHtmlAsync m_markdownParser;
    MarkdownToHtmlAsync::RequestId m_parseRequestId = 0;
    bool m_parsePending = false;
    bool m_hasCurrentParse = false;
    OnMarkdownReadyCallback m_onMarkdownReadyCallback;
    OnMarkdownErrorCallback m_onMarkdownErrorCallback;
    /// Options in force, from the last LoadFile() or Render()
    MarkdownPreviewOptions m_options;
    /// The last parse, kept so a repaint can rebuild the page without re-parsing
    wxString m_parsedHtml;
    wxString m_markdown;
    wxFileName m_fileName;
    /// Where the last parse request took its markdown from; a restyle via
    /// Render() keeps it, since the document itself did not change.
    MarkdownOrigin m_origin = MarkdownOrigin::Disk;
    /// The page currently shown; MarkdownPreviewData::html refers to it
    wxString m_htmlPage;

    /**
     * @brief Stores the finished parse and renders it. Handler for EVT_MARKDOWN_READY.
     */
    void HandleMarkdownReady(MarkdownToHtmlAsyncEvent& event);

    /**
     * @brief Reports a failed parse to the owner. Handler for EVT_MARKDOWN_ERROR.
     */
    void HandleMarkdownError(MarkdownToHtmlAsyncEvent& event);

    /**
     * @brief Wraps parsed markdown output in a full HTML page.
     *
     * @param parsedMarkdownToHtml The parser's HTML, which becomes the page body
     * @param options Styling to inject; an empty style leaves the head empty
     * @return The complete HTML page
     */
    wxString GetHtmlPage(const wxString& parsedMarkdownToHtml, const MarkdownPreviewOptions& options) const;

    /**
     * @brief Builds the page from the last parse plus the options in force, shows
     *        it, and reports it to the owner.
     *
     * The ready callback fires here rather than after a parse because what it
     * carries is the rendered page: a restyle makes it stale just as a re-parse does.
     */
    void Paint();
};
