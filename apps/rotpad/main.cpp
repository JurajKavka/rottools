#include <wx/filename.h>
#include <wx/wx.h>

#include "MainFrame.h"

class RotpadApp final : public wxApp {
   public:
    bool OnInit() override;
#ifdef __WXOSX__
    void MacOpenFiles(const wxArrayString& fileNames) override;
#endif

   private:
    MainFrame* m_frame = nullptr;
};

wxIMPLEMENT_APP(RotpadApp);

bool RotpadApp::OnInit() {
    SetAppName("rotpad-rottools");
    SetVendorName("Juraj Kavka");

#ifdef __WXOSX__
    OSXEnableAutomaticTabbing(false);
#endif

    m_frame = new MainFrame(nullptr);
    m_frame->Show(true);

    // Linux and Windows desktop shells pass the selected document as a
    // command-line argument. This also makes `rotpad path/to/file` work.
    if (argc > 1) {
        m_frame->OpenTextFile(wxFileName(argv[1]));
    }

    return true;
}

#ifdef __WXOSX__
// Finder delivers document-open requests as application events instead of
// command-line arguments for an app bundle.
void RotpadApp::MacOpenFiles(const wxArrayString& fileNames) {
    if (fileNames.IsEmpty()) {
        return;
    }
    m_frame->OpenTextFile(wxFileName(fileNames[0]));
}
#endif
