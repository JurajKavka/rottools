#include <wx/image.h>
#include <wx/wx.h>

#include "Components/MainFrame.h"

class MyApp : public wxApp {
   public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    wxImage::AddHandler(new wxPNGHandler());

    MainFrame *frame = new MainFrame(nullptr);
    frame->Show(true);
    return true;
}
