#include <wx/wx.h>

#include "CsvTablePanel.h"

class CsvTableApp : public wxApp {
   public:
    bool OnInit() override {
        auto* frame = new wxFrame(nullptr, wxID_ANY, "CSV table", wxDefaultPosition, wxSize(700, 400));
        new CsvTablePanel(frame, ParseCsv("00123,\"quoted, value\",\"two\nlines\"\nshort\nextra,,field,value"));
        frame->Show();
        return true;
    }
};
wxIMPLEMENT_APP(CsvTableApp);
