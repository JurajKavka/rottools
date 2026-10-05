#include <wx/wx.h>

#include "TextFilePreviewDialog.h"

namespace {
const wxString kSampleText =
    "-- Select, copy, and toggle Word Wrap to exercise the dialog.\n"
    "SELECT id, title, created_at FROM documents WHERE title LIKE '%preview%' ORDER BY created_at DESC;\n"
    "\n"
    "{\"name\": \"sample\", \"enabled\": true, \"items\": [1, 2, 3]}\n"
    "\n"
    "2026-09-17 12:00:00 INFO Preview opened\n"
    "2026-09-17 12:00:01 INFO Text remains selectable when wrapping changes.\n";
}

class TextFilePreviewDialogApp final : public wxApp {
   public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(TextFilePreviewDialogApp);

bool TextFilePreviewDialogApp::OnInit() {
    auto* frame = new wxFrame(nullptr, wxID_ANY, "Test: Text File Preview", wxDefaultPosition,
                              wxSize(480, 180));
    auto* button = new wxButton(frame, wxID_ANY, "Open Preview");
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->AddStretchSpacer();
    sizer->Add(button, 0, wxALIGN_CENTER | wxALL, frame->FromDIP(12));
    sizer->AddStretchSpacer();
    frame->SetSizer(sizer);

    button->Bind(wxEVT_BUTTON, [frame](wxCommandEvent&) {
        TextFilePreviewDialog dialog(frame, "Sample text file");
        dialog.ShowText(kSampleText);
        dialog.ShowModal();
    });

    frame->Show();
    return true;
}
