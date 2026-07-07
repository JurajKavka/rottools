#include <wx/image.h>
#include <wx/wx.h>

#include "MainFrame.h"

class MyApp : public wxApp {
   public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    wxImage::AddHandler(new wxPNGHandler());

    MainFrame *frame = new MainFrame(nullptr);
    frame->Show(true);
    return true;
}
