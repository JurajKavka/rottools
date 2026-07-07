#include <wx/wx.h>

#include "HtmlSourcePanel.h"

class HtmlSourcePanelApp : public wxApp {
   public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(HtmlSourcePanelApp);

bool HtmlSourcePanelApp::OnInit() {
    // 1. Create a Top-Level Window (Frame) to hold your panel
    wxFrame* mainFrame = new wxFrame(nullptr, wxID_ANY, "Test: HTML Source", wxDefaultPosition, wxSize(700, 500));

    // 2. Instantiate your custom panel, passing the mainFrame as its parent
    HtmlSourcePanel* sourcePanel = new HtmlSourcePanel(mainFrame);

    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(sourcePanel, 1, wxEXPAND | wxALL, 0);
    mainFrame->SetSizer(frameSizer);
    mainFrame->Layout();

    // 3. Show the frame (which automatically shows the child panel)
    mainFrame->Show(true);

    // Sample output resembling what the markdown parser produces
    sourcePanel->ShowHtml(
        "<!-- generated from sample.md -->\n"
        "<h1 id=\"welcome\">Welcome</h1>\n"
        "<p>Some <strong>bold</strong> text with an entity: &amp;copy; &#169;</p>\n"
        "<ul>\n"
        "  <li>First item</li>\n"
        "  <li>Second item with <a href='https://example.com'>a link</a></li>\n"
        "</ul>\n"
        "<table>\n"
        "  <tr><th>Name</th><th>Value</th></tr>\n"
        "  <tr><td>answer</td><td>42</td></tr>\n"
        "</table>\n");

    return true;
}
