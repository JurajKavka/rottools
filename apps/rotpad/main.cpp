#include <wx/wx.h>

#include "MainFrame.h"

class RotpadApp final : public wxApp {
   public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(RotpadApp);

bool RotpadApp::OnInit() {
    auto* frame = new MainFrame(nullptr);
    frame->Show(true);
    return true;
}
