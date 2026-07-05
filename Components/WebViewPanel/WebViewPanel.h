#pragma once

#include "HelperFunctions.h"
#include "WebViewPanelWx.h"

// Forward declaration prevents bloated compile times
class wxWebView;

class WebViewPanel : public WebViewPanelWx {
   private:
    wxWebView* m_webView = nullptr;

   public:
    explicit WebViewPanel(wxWindow* parent);

    void LoadHtml(const wxString& html);
};
