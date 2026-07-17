#pragma once

#include "HelperFunctions.h"
#include "WebViewPanelWx.h"

// Forward declaration prevents bloated compile times
class wxWebView;

/**
 * @brief What happens to the scroll position when a new page is shown.
 */
enum class ScrollBehavior {
    /// Start at the top, as a normal page load does
    ResetToTop,
    /// Keep the current vertical scroll offset across the update
    KeepPosition,
};

class WebViewPanel : public WebViewPanelWx {
   private:
    wxWebView* m_webView = nullptr;

   public:
    explicit WebViewPanel(wxWindow* parent);

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
