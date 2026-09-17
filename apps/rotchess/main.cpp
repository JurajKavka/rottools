#include <wx/wx.h>

#include "MainFrame.h"

class RotChessApp final : public wxApp {
   public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(RotChessApp);

bool RotChessApp::OnInit() {
    SetAppName("rotchess-rottools");
    SetVendorName("Juraj Kavka");

#ifdef __WXOSX__
    OSXEnableAutomaticTabbing(false);
#endif

    auto* frame = new MainFrame(nullptr);
    frame->Show(true);
    return true;
}
