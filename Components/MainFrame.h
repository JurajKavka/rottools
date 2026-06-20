#ifndef __MainFrame__
#define __MainFrame__

#include "MainFrameWx.h"

// Forward declaration
class WebViewPanel;

class MainFrame : public MainFrameWx {
   private:
    // Store a pointer to your custom panel
    WebViewPanel* m_webViewPanel;
    void HandleOpenFileMenuItemClick(wxCommandEvent& event);

   public:
    MainFrame(wxWindow* parent);
    ~MainFrame();
};

#endif  // __MainFrame__
