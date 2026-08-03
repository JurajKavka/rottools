#include <wx/wx.h>

#include <functional>

#include "MarkdownSourcePanel.h"

class MarkdownSourcePanelApp : public wxApp {
   public:
    bool OnInit() override;

   private:
    MarkdownSourcePanel* m_sourcePanel = nullptr;

    void HandleSave(const wxString& markdown);
};

wxIMPLEMENT_APP(MarkdownSourcePanelApp);

bool MarkdownSourcePanelApp::OnInit() {
    // 1. Create a Top-Level Window (Frame) to hold your panel
    wxFrame* mainFrame = new wxFrame(nullptr, wxID_ANY, "Test: Markdown Source", wxDefaultPosition, wxSize(700, 500));

    // 2. Instantiate your custom panel, passing the mainFrame as its parent
    m_sourcePanel = new MarkdownSourcePanel(mainFrame, std::bind_front(&MarkdownSourcePanelApp::HandleSave, this));

    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(m_sourcePanel, 1, wxEXPAND | wxALL, 0);
    mainFrame->SetSizer(frameSizer);
    mainFrame->Layout();

    // 3. Show the frame (which automatically shows the child panel)
    mainFrame->Show(true);

    // Sample markdown exercising the lexer styles
    m_sourcePanel->ShowMarkdown(
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

// Cmd+S in the test app stands in for a real save: re-show the edited text with
// KeepPosition, as the live reload after a save does. The caret and scroll
// should stay where they were instead of jumping to the top.
void MarkdownSourcePanelApp::HandleSave(const wxString& markdown) {
    m_sourcePanel->ShowMarkdown(markdown, ScrollBehavior::KeepPosition);
}
