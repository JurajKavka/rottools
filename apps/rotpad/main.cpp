#include <wx/filename.h>
#include <wx/wx.h>

#include "MainFrame.h"

class RotpadApp final : public wxApp {
   public:
    bool OnInit() override;
#ifdef __WXOSX__
    void MacOpenFiles(const wxArrayString& fileNames) override;
#endif
};

wxIMPLEMENT_APP(RotpadApp);

bool RotpadApp::OnInit() {
    SetAppName("rotpad-rottools");
    SetVendorName("Juraj Kavka");

#ifdef __WXOSX__
    OSXEnableAutomaticTabbing(false);
#endif

    auto* frame = new MainFrame(nullptr);
    frame->Show(true);

    // Linux and Windows desktop shells pass the selected document as a
    // command-line argument. This also makes `rotpad path/to/file` work.
    if (argc > 1) {
        frame->HandleOpenTextFile(wxFileName(argv[1]));
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
    // New Window allows the original frame to close while the app stays open.
    // Resolve a live frame for each Finder request instead of keeping that
    // original frame's pointer.
    MainFrame* frame = nullptr;
    for (auto* window : wxTopLevelWindows) {
        auto* candidate = dynamic_cast<MainFrame*>(window);
        if (candidate != nullptr && (frame == nullptr || candidate->IsActive())) {
            frame = candidate;
        }
    }
    if (frame == nullptr) {
        frame = new MainFrame(nullptr);
        frame->Show(true);
    }
    frame->HandleOpenTextFile(wxFileName(fileNames[0]));
}
#endif
