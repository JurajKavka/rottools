#include <wx/wx.h>

#include "CsvFilePreviewDialog.h"

namespace {
struct PreviewSample {
    wxString title;
    wxString text;
};

const PreviewSample kSamples[] = {
    {"Quoted fields and uneven rows.csv",
     "ID,Description,Notes\r\n"
     "00123,\"quoted, value\",\"two\nlines\"\r\n"
     "00456,\"She said \"\"hello\"\"\"\r\n"
     "00789,,extra,field"},
    {"Header only.csv", "ID,Description,Notes"},
    {"Empty.csv", wxEmptyString},
    {"Malformed.csv", "ID,Description\n00123,\"unclosed"},
    {"Too many columns.csv", wxString(',', 256)},
};

void HandlePreviewError(wxWindow* parent, const CsvError& error) {
    if (error.code == CsvError::Code::InvalidSyntax) {
        wxMessageBox(wxString::Format("CSV parsing failed at record %zu.", error.recordNumber), "Cannot Preview Table",
                     wxOK | wxICON_INFORMATION, parent);
    } else {
        wxMessageBox("The CSV exceeds the table display limit.", "Table Too Large", wxOK | wxICON_INFORMATION, parent);
    }
}
}  // namespace

class CsvFilePreviewDialogApp final : public wxApp {
   public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(CsvFilePreviewDialogApp);

bool CsvFilePreviewDialogApp::OnInit() {
    auto* frame = new wxFrame(nullptr, wxID_ANY, "Test: CSV File Preview");
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    const int margin = frame->FromDIP(8);
    for (const auto& sample : kSamples) {
        auto* button = new wxButton(frame, wxID_ANY, sample.title);
        sizer->Add(button, 0, wxEXPAND | wxALL, margin);
        const auto HandleOpenSample = [frame, sample](wxCommandEvent&) {
            CsvFilePreviewDialog dialog(frame, sample.title);
            const auto HandleError = [frame](const CsvError& error) { HandlePreviewError(frame, error); };
            dialog.ShowCsvModal(sample.text, HandleError);
        };
        button->Bind(wxEVT_BUTTON, HandleOpenSample);
    }
    frame->SetSizerAndFit(sizer);
    frame->Show();
    return true;
}
