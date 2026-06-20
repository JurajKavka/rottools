#include "WebViewPanel.h"

#include <wx/sizer.h>
#include <wx/webview.h>

WebViewPanel::WebViewPanel(wxWindow* parent) : WebViewPanelWx(parent), m_webView(nullptr) {
    // 1. Create a sizer to manage the layout of this panel
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 2. Apply the sizer to the panel
    SetSizer(mainSizer);
    Layout();
};

WebViewPanel::~WebViewPanel() {};

void WebViewPanel::LoadHtml(const wxString& html) {
    if (!m_webView) {
        // 1. Instantiate the wxWebView
        m_webView = wxWebView::New(this, wxID_ANY);
        if (m_webView) {
            // 2. Add it to the sizer, expanding to fill the panel
            wxSizer* mainSizer = GetSizer();
            if (mainSizer) {
                mainSizer->Add(m_webView, 1, wxEXPAND | wxALL, 0);
                Layout();
            }
        }
    }

    if (m_webView) {
        m_webView->SetPage(html, ""); // Toto vyrenderuje HTML string
    }
}
