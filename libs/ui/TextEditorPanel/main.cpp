#include <wx/wx.h>

#include <iostream>

#include "TextEditorPanel.h"

namespace {
bool Check(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
    }
    return condition;
}

bool CheckEditing(TextEditorPanel& editor) {
    editor.LoadText("one\r\ntwo\r\none");
    bool passed = Check(!editor.HasUnsavedChanges(), "CRLF documents start clean");
    passed = Check(editor.FindText("two"), "find across a native newline") && passed;
    passed = Check(editor.ReplaceText("two", "three"), "replace the selected match") && passed;
    passed = Check(editor.GetText() == "one\nthree\none", "replace keeps surrounding lines") && passed;
    passed = Check(editor.HasUnsavedChanges() && editor.CanUndo(), "replacement is dirty and undoable") && passed;
    editor.Undo();
    passed =
        Check(editor.GetText() == "one\ntwo\none" && !editor.HasUnsavedChanges(), "undo returns to saved contents") &&
        passed;
    editor.Redo();
    passed = Check(editor.GetText() == "one\nthree\none", "redo restores replacement") && passed;

    editor.LoadText("cat scatter CAT");
    passed =
        Check(editor.ReplaceAllText("cat", "dog", {.wholeWord = true}) == 2, "case-insensitive whole words") && passed;
    passed = Check(editor.GetText() == "dog scatter dog", "whole-word replacement skips substrings") && passed;
    editor.Undo();
    passed = Check(editor.GetText() == "cat scatter CAT", "one undo restores Replace All") && passed;

    editor.LoadText(wxString::FromUTF8("Žltý 😀\nŽltý"));
    passed =
        Check(editor.FindText(wxString::FromUTF8("Žltý"), {.backwards = true}), "backwards search wraps to the end") &&
        passed;
    passed =
        Check(editor.ReplaceText(wxString::FromUTF8("Žltý"), "last"), "replace after a non-BMP character") && passed;
    passed =
        Check(editor.GetText() == wxString::FromUTF8("Žltý 😀\nlast"), "Unicode and native position mapping") && passed;

    editor.LoadText("a a");
    passed = Check(editor.ReplaceAllText("a", "aa") == 2 && editor.GetText() == "aa aa",
                   "Replace All does not search its own replacements") &&
             passed;
    passed = Check(!editor.FindText("") && editor.ReplaceAllText("", "x") == 0, "empty search is a no-op") && passed;
    editor.LoadText("a a");
    passed = Check(editor.FindText("a", {.backwards = true, .wrap = false}) == false,
                   "backwards search at the start respects no-wrap") &&
             passed;
    passed = Check(editor.FindText("a", {.matchCase = true}), "case-sensitive match") && passed;
    editor.LoadText("a b", true);
    passed = Check(editor.GetSelectedText() == "a", "reload can retain the selection") && passed;
    editor.SelectAll();
    passed = Check(editor.GetSelectedText() == "a b", "Select All uses the native selection") && passed;
    editor.LoadText(wxString{});
    passed =
        Check(!editor.HasUnsavedChanges() && !editor.CanUndo(), "new contents reset dirty state and undo") && passed;
    return passed;
}
}  // namespace

class TextEditorPanelApp final : public wxApp {
   public:
    bool OnInit() override;
    int OnRun() override;
    void OnAssertFailure(const wxChar* file, int line, const wxChar* function, const wxChar* condition,
                         const wxChar* message) override;

   private:
    void HandleRunSelfTests();
    TextEditorPanel* m_editor = nullptr;
    bool m_selfTest = false;
    int m_testExitCode = 0;
    bool m_assertionFailed = false;
};

wxIMPLEMENT_APP(TextEditorPanelApp);

bool TextEditorPanelApp::OnInit() {
    m_selfTest = argc > 1 && wxString(argv[1]) == "--self-test";
    if (m_selfTest) {
        std::cerr << "Running native text editor checks\n";
    }
    auto* mainFrame = new wxFrame(nullptr, wxID_ANY, "Test: Text Editor", wxDefaultPosition, wxSize(800, 600));
    auto* editorPanel = new TextEditorPanel(mainFrame);
    m_editor = editorPanel;

    auto* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(editorPanel, 1, wxEXPAND | wxALL, 0);
    mainFrame->SetSizer(frameSizer);
    mainFrame->Layout();
    if (!m_selfTest) {
        mainFrame->Show(true);
        editorPanel->FocusEditor();
    }
    return true;
}

int TextEditorPanelApp::OnRun() {
    if (m_selfTest) {
        // Exercise native controls directly without waiting for a visible
        // window or a desktop activation event to start the checks.
        HandleRunSelfTests();
        return m_testExitCode;
    }
    return wxApp::OnRun();
}

void TextEditorPanelApp::HandleRunSelfTests() {
    const bool passed = CheckEditing(*m_editor);
    m_testExitCode = passed && !m_assertionFailed ? 0 : 1;
    std::cout << (m_testExitCode == 0 ? "Text editor checks passed\n" : "Text editor checks failed\n");
}

void TextEditorPanelApp::OnAssertFailure(const wxChar* file, int line, const wxChar* function, const wxChar* condition,
                                         const wxChar* message) {
    if (!m_selfTest) {
        wxApp::OnAssertFailure(file, line, function, condition, message);
        return;
    }
    m_assertionFailed = true;
    std::cerr << "wxWidgets assertion at " << wxString(file).ToStdString() << ':' << line << ": "
              << wxString(condition).ToStdString() << " " << wxString(message).ToStdString() << '\n';
}
