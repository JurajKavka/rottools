#include "WebViewPanel.h"

#include <wx/sizer.h>
#include <wx/webview.h>

WebViewPanel::WebViewPanel(wxWindow* parent) : WebViewPanelWx(parent) {
    // 1. Create a sizer to manage the layout of this panel
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 2. Instantiate the wxWebView
    // wxWebView::New() automatically picks the best native engine:
    // Windows: Edge (WebView2) or IE
    // macOS: WebKit (Safari)
    // Linux: WebKitGTK
    m_webView = wxWebView::New(this, wxID_ANY);

    // 3. Safety check and layout attachment
    if (m_webView) {
        // Add the webview to the sizer, letting it expand in all directions (proportion 1)
        mainSizer->Add(m_webView, 1, wxEXPAND | wxALL, 0);
    } else {
        // Fallback: If wxWebView fails to initialize, you could add an error label here.
        // wxLogError("Failed to initialize wxWebView.");
    }

    // 4. Apply the sizer to the panel
    SetSizer(mainSizer);
    Layout();
};

WebViewPanel::~WebViewPanel() {};

void WebViewPanel::LoadHtml(const wxString& html) {
    if (m_webView) {
        m_webView->SetPage(html, ""); // Toto vyrenderuje HTML string
    }
}
