#pragma once

#include "HelperFunctions.h"
#include "WebViewPanelWx.h"

// Forward declaration prevents bloated compile times
class wxWebView;

class WebViewPanel : public WebViewPanelWx {
   private:
    wxWebView* m_webView;

   public:
    explicit WebViewPanel(wxWindow* parent);
    ~WebViewPanel();

    void LoadHtml(const wxString& html);
};
