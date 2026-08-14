#include "WebViewPanel.h"

#include <wx/sizer.h>
#include <wx/webview.h>

namespace {

/**
 * @brief Escapes text for embedding in a double-quoted JavaScript string literal.
 */
wxString EscapeForJsString(const wxString& text) {
    wxString escaped;
    escaped.reserve(text.length());
    for (wxUniChar ch : text) {
        switch (ch.GetValue()) {
            case '\\':
                escaped << "\\\\";
                break;
            case '"':
                escaped << "\\\"";
                break;
            case '\n':
                escaped << "\\n";
                break;
            case '\r':
                escaped << "\\r";
                break;
            default:
                escaped << ch;
        }
    }
    return escaped;
}

}  // namespace

WebViewPanel::WebViewPanel(wxWindow* parent) : WebViewPanelWx(parent) {
    // 1. Create a sizer to manage the layout of this panel
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 2. Apply the sizer to the panel
    SetSizer(mainSizer);

    Layout();
}

void WebViewPanel::Copy() {
    if (CanCopy()) {
        m_webView->Copy();
    }
}

bool WebViewPanel::CanCopy() const {
    return m_webView != nullptr && m_webView->CanCopy();
}

void WebViewPanel::FocusContent() {
    if (m_webView != nullptr) {
        m_webView->SetFocus();
    } else {
        SetFocus();
    }
}

void WebViewPanel::LoadHtml(const wxString& html, ScrollBehavior scrollBehavior) {
    if (!m_webView) {
        // 3. Instantiate the wxWebView
        m_webView = wxWebView::New(this, wxID_ANY);
        wxSizer* mainSizer = GetSizer();
        // 4. Add it to the sizer, expanding to fill the panel
        mainSizer->Add(m_webView, 1, wxEXPAND | wxALL, 0);
        Layout();
        // First load: no page to update in place yet.
        m_webView->SetPage(html, "");
        return;
    }

    if (scrollBehavior == ScrollBehavior::ResetToTop) {
        m_webView->SetPage(html, "");
        return;
    }

    // Update the live page in place: swapping head and body and restoring the
    // scroll offset in one synchronous script leaves no intermediate state to
    // paint, so the view neither flashes nor jumps to the top. SetPage() would
    // instead navigate to a fresh document, which always starts at the top.
    wxString script = wxString::FromUTF8(std::format(R"((function () {{
    var doc = new DOMParser().parseFromString("{}", "text/html");
    var scrollY = window.scrollY;
    document.head.replaceWith(doc.head);
    document.body.replaceWith(doc.body);
    // A script parsed by DOMParser is inert. Recreate only the generated theme
    // script; scripts originating in Markdown remain inactive during restyling.
    var oldThemeScript = document.getElementById("rotdown-theme-script");
    if (oldThemeScript) {{
        var newThemeScript = document.createElement("script");
        newThemeScript.id = oldThemeScript.id;
        newThemeScript.textContent = oldThemeScript.textContent;
        oldThemeScript.replaceWith(newThemeScript);
    }}
    window.scrollTo(0, scrollY);
}})();)",
                                                     EscapeForJsString(html)));
    m_webView->RunScriptAsync(script);
}
