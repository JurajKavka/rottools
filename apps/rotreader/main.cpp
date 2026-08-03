#include <wx/filename.h>
#include <wx/image.h>
#include <wx/wx.h>

#include "MainFrame.h"

class MyApp : public wxApp {
   public:
    bool OnInit() override;
#ifdef __WXOSX__
    void MacOpenFiles(const wxArrayString& fileNames) override;
#endif

   private:
    MainFrame* m_frame = nullptr;
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    wxImage::AddHandler(new wxPNGHandler());

    m_frame = new MainFrame(nullptr);
    m_frame->Show(true);

    if (argc > 1) {
        m_frame->OpenMarkdownFile(wxFileName(argv[1]));
    }

    return true;
}

#ifdef __WXOSX__
void MyApp::MacOpenFiles(const wxArrayString& fileNames) {
    if (fileNames.IsEmpty()) {
        return;
    }
    m_frame->OpenMarkdownFile(wxFileName(fileNames[0]));
}
#endif
