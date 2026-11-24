#include <wx/notebook.h>
#include <wx/wx.h>

#include "ScintillaTextEditorPanel.h"

class ScintillaTextEditorPanelApp final : public wxApp {
   public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(ScintillaTextEditorPanelApp);

bool ScintillaTextEditorPanelApp::OnInit() {
    auto* mainFrame =
        new wxFrame(nullptr, wxID_ANY, "Test: Scintilla Text Editor", wxDefaultPosition, wxSize(800, 600));
    auto* notebook = new wxNotebook(mainFrame, wxID_ANY);
    auto* editorPanel = new ScintillaTextEditorPanel(notebook);
    editorPanel->LoadText("Editable mode: type here.\n");
    notebook->AddPage(editorPanel, "Editable");
    using Syntax = ScintillaTextEditorPanel::Syntax;
    const struct {
        const char* title;
        Syntax syntax;
        const char* text;
    } samples[] = {
        {"SQL", Syntax::Sql, "-- comment\nSELECT 'text' FROM documents WHERE id = 1;\n"},
        {"JavaScript", Syntax::JavaScript, "// comment\nconst message = `hello ${name}`;\n"},
        {"TypeScript", Syntax::TypeScript,
         "// comment\ninterface User { name: string }\nconst greeting: string = `hello`;\n"},
        {"Python", Syntax::Python, "# comment\ndef greet(name):\n    return 'hello ' + name\n"},
        {"C++", Syntax::Cpp, "// comment\nconst char* greeting = \"hello\";\n"},
        {"Shell", Syntax::Shell, "#!/bin/sh\n# comment\nif test -n \"$USER\"; then\n    printf 'Hello %s\\n' \"$USER\"\nfi\n"},
    };
    for (const auto& sample : samples) {
        auto* panel = new ScintillaTextEditorPanel(
            notebook, {.syntax = sample.syntax, .lineNumbers = true, .wordWrap = true, .readOnly = true});
        panel->LoadText(sample.text);
        notebook->AddPage(panel, sample.title);
    }

    auto* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(notebook, 1, wxEXPAND | wxALL, 0);
    mainFrame->SetSizer(frameSizer);
    mainFrame->Layout();
    mainFrame->Show(true);

    editorPanel->FocusEditor();
    return true;
}
