#pragma once

#include "HelperFunctions.h"
#include "WebViewPanelWx.h"

// Forward declaration prevents bloated compile times
class wxMouseEvent;
class wxWebView;

// ScrollBehavior comes from HelperFunctions.h; it is shared with the other
// content panels.

class WebViewPanel : public WebViewPanelWx {
   private:
    wxWebView* m_webView = nullptr;

    void HandleWebViewLeftDown(wxMouseEvent& event);

   public:
    explicit WebViewPanel(wxWindow* parent);

    void Copy();
    [[nodiscard]] bool CanCopy() const;
    [[nodiscard]] wxString GetSelectedText() const;
    void FocusContent();

    /**
     * Finds visible text in the rendered page and selects the next match.
     * Returns wxNOT_FOUND when the page is unavailable or there is no match.
     */
    long FindText(const wxString& text, const TextSearchOptions& options = {});

    /** Clears the web engine's current match and search highlighting. */
    void ClearSearch();

    /**
     * @brief Shows an HTML page in the webview.
     *
     * @param html The full HTML page to show
     * @param scrollBehavior With KeepPosition the page is updated in place (its
     *        head and body are swapped) rather than reloaded, so there is no
     *        flash back to the top. Ignored on the first load, when there is no
     *        page to update yet.
     */
    void LoadHtml(const wxString& html, ScrollBehavior scrollBehavior = ScrollBehavior::ResetToTop);
};
