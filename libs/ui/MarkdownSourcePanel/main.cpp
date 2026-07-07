#include <wx/wx.h>

#include "MarkdownSourcePanel.h"

class MarkdownSourcePanelApp : public wxApp {
   public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(MarkdownSourcePanelApp);

bool MarkdownSourcePanelApp::OnInit() {
    // 1. Create a Top-Level Window (Frame) to hold your panel
    wxFrame* mainFrame = new wxFrame(nullptr, wxID_ANY, "Test: Markdown Source", wxDefaultPosition, wxSize(700, 500));

    // 2. Instantiate your custom panel, passing the mainFrame as its parent
    MarkdownSourcePanel* sourcePanel = new MarkdownSourcePanel(mainFrame);

    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(sourcePanel, 1, wxEXPAND | wxALL, 0);
    mainFrame->SetSizer(frameSizer);
    mainFrame->Layout();

    // 3. Show the frame (which automatically shows the child panel)
    mainFrame->Show(true);

    // Sample markdown exercising the lexer styles
    sourcePanel->ShowMarkdown(
        "# Welcome\n"
        "\n"
        "Some **bold** text, some *emphasized* text and `inline code`.\n"
        "\n"
        "## Features\n"
        "\n"
        "- First item\n"
        "- Second item with a [link](https://example.com)\n"
        "\n"
        "1. Ordered item\n"
        "2. ~~Struck through~~\n"
        "\n"
        "> A blockquote with wisdom.\n"
        "\n"
        "```\n"
        "code block\n"
        "```\n"
        "\n"
        "---\n");

    return true;
}
