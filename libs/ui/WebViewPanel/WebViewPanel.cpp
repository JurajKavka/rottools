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

bool WebViewPanel::ContainsFocus() const {
    return ::ContainsFocus(this);
}

wxString WebViewPanel::GetSelectedText() const {
    return m_webView != nullptr ? m_webView->GetSelectedText() : wxString{};
}

void WebViewPanel::FocusContent() {
    if (m_webView != nullptr) {
        m_webView->SetFocus();
    } else {
        SetFocus();
    }
}

long WebViewPanel::FindText(const wxString& text, const TextSearchOptions& options) {
    if (m_webView == nullptr || text.IsEmpty()) {
        return wxNOT_FOUND;
    }

    int flags = wxWEBVIEW_FIND_HIGHLIGHT_RESULT;
    if (options.wrap) {
        flags |= wxWEBVIEW_FIND_WRAP;
    }
    if (options.wholeWord) {
        flags |= wxWEBVIEW_FIND_ENTIRE_WORD;
    }
    if (options.matchCase) {
        flags |= wxWEBVIEW_FIND_MATCH_CASE;
    }
    if (options.backwards) {
        flags |= wxWEBVIEW_FIND_BACKWARDS;
    }

    return m_webView->Find(text, flags);
}

void WebViewPanel::ClearSearch() {
    if (m_webView != nullptr) {
        m_webView->Find(wxString{});
    }
}

void WebViewPanel::HandleWebViewLeftDown(wxMouseEvent& event) {
    // A non-editable native web view can consume a click without becoming the
    // wxWidgets focus window. Make the same focus transition explicitly so
    // frame-level commands such as Find and Copy target the preview.
    m_webView->SetFocus();
    event.Skip();
}

void WebViewPanel::LoadHtml(const wxString& html, ScrollBehavior scrollBehavior) {
    if (!m_webView) {
        // 3. Instantiate the wxWebView
        m_webView = wxWebView::New(this, wxID_ANY);
        m_webView->Bind(wxEVT_LEFT_DOWN, &WebViewPanel::HandleWebViewLeftDown, this);
        wxSizer* mainSizer = GetSizer();
        // 4. Add it to the sizer, expanding to fill the panel
        mainSizer->Add(m_webView, 1, wxEXPAND | wxALL, 0);
        Layout();
        // First load: no page to update in place yet.
        m_webView->SetPage(html, "");
        return;
    }

    // A new page body invalidates the web engine's current match. Search
    // replay is deliberately left to a later feature; the next Find starts a
    // fresh search in the new document.
    ClearSearch();

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
