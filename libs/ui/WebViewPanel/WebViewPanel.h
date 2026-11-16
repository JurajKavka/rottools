#pragma once

#include "HelperFunctions.h"
#include "WebViewPanelWx.h"

#include <functional>

// Forward declaration prevents bloated compile times
class wxMouseEvent;
class wxWebView;
class wxWebViewEvent;

// ScrollBehavior comes from HelperFunctions.h; it is shared with the other
// content panels.

class WebViewPanel : public WebViewPanelWx {
   private:
    wxWebView* m_webView = nullptr;
    wxString m_embeddedUrl;
    bool m_scriptMessagingAvailable = false;
    std::function<void()> m_onEscapePressed;

    void EnsureWebView();
    void HandleWebViewLeftDown(wxMouseEvent& event);
    void HandleWebViewNavigation(wxWebViewEvent& event);
    void HandleWebViewLoaded(wxWebViewEvent& event);
    void HandleWebViewScriptMessage(wxWebViewEvent& event);

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

    /** Sets the callback invoked when Escape is pressed in an embedded page. */
    void SetEscapePressedCallback(std::function<void()> callback);

    /**
     * @brief Shows an HTML page in the webview.
     * HTTP and HTTPS links open in the default browser, including links that
     * request a new window. In-page anchors retain native webview navigation.
     *
     * @param html The full HTML page to show
     * @param scrollBehavior With KeepPosition the page is updated in place (its
     *        head and body are swapped) rather than reloaded, so there is no
     *        flash back to the top. Ignored on the first load, when there is no
     *        page to update yet.
     */
    void LoadHtml(const wxString& html, ScrollBehavior scrollBehavior = ScrollBehavior::ResetToTop);

    /**
     * Loads an HTTP or HTTPS page inside the panel. Navigation away from the
     * requested page continues to open in the system browser.
     */
    void LoadUrl(const wxString& url);
};
