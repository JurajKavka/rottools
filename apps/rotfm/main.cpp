#include <wx/filename.h>
#include <wx/wx.h>

#include "MainFrame.h"

class RotfmApp final : public wxApp {
   public:
    bool OnInit() override;
#ifdef __WXOSX__
    void MacOpenFiles(const wxArrayString& fileNames) override;
#endif

   private:
    MainFrame* m_frame = nullptr;
};

wxIMPLEMENT_APP(RotfmApp);

bool RotfmApp::OnInit() {
    SetAppName("rotfm-rottools");
    SetVendorName("Juraj Kavka");

#ifdef __WXOSX__
    OSXEnableAutomaticTabbing(false);
#endif

    m_frame = new MainFrame(nullptr);
    m_frame->Show(true);

    if (argc > 1) {
        m_frame->OpenDirectory(wxFileName::DirName(argv[1]));
    }
    return true;
}

#ifdef __WXOSX__
void RotfmApp::MacOpenFiles(const wxArrayString& fileNames) {
    if (!fileNames.IsEmpty()) {
        m_frame->OpenDirectory(wxFileName::DirName(fileNames[0]));
    }
}
#endif
