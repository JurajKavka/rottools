#include "ScintillaTextEditorPanel.h"

#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stc/stc.h>

ScintillaTextEditorPanel::ScintillaTextEditorPanel(wxWindow* parent) : ScintillaTextEditorPanelWx(parent) {
    m_textEditor = new wxStyledTextCtrl(this, wxID_ANY);

    // Keep Scintilla visually close to a plain native text editor. Its richer
    // code-editor features stay disabled until rotpad explicitly needs them.
    m_textEditor->SetLexer(wxSTC_LEX_NULL);
    m_textEditor->SetCodePage(wxSTC_CP_UTF8);
    m_textEditor->SetMarginWidth(0, 0);
    m_textEditor->SetMarginWidth(1, 0);
    m_textEditor->SetMarginWidth(2, 0);
    m_textEditor->SetIndentationGuides(false);
    m_textEditor->SetViewEOL(false);
    m_textEditor->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
    m_textEditor->SetWrapMode(wxSTC_WRAP_NONE);
    m_textEditor->SetScrollWidthTracking(true);

    const wxColour foreground = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    const wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    m_textEditor->StyleSetForeground(wxSTC_STYLE_DEFAULT, foreground);
    m_textEditor->StyleSetBackground(wxSTC_STYLE_DEFAULT, background);
    m_textEditor->StyleSetFont(wxSTC_STYLE_DEFAULT, wxFontInfo(12).Family(wxFONTFAMILY_TELETYPE));
    m_textEditor->StyleClearAll();
    m_textEditor->SetCaretForeground(foreground);
#ifndef __WXOSX__
    // Cocoa supplies Scintilla's native selection appearance. Overriding its
    // text colour splits selected text into separate shaped runs, which can
    // make glyphs appear to shift as the selection changes.
    m_textEditor->SetSelBackground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
    m_textEditor->SetSelForeground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
#endif

    wxSizer* mainSizer = GetSizer();
    mainSizer->Add(m_textEditor, 1, wxEXPAND | wxALL, 0);
    Layout();

    m_textEditor->EmptyUndoBuffer();
    m_textEditor->SetSavePoint();
}

void ScintillaTextEditorPanel::LoadText(const wxString& text) {
    m_textEditor->SetText(text);
    m_textEditor->EmptyUndoBuffer();
    m_textEditor->SetSavePoint();
    m_textEditor->GotoPos(0);
}

wxString ScintillaTextEditorPanel::GetText() const {
    return m_textEditor->GetText();
}

bool ScintillaTextEditorPanel::HasUnsavedChanges() const {
    return m_textEditor->GetModify();
}

void ScintillaTextEditorPanel::MarkSaved() {
    m_textEditor->SetSavePoint();
}

void ScintillaTextEditorPanel::Undo() {
    if (m_textEditor->CanUndo()) {
        m_textEditor->Undo();
    }
}

void ScintillaTextEditorPanel::Redo() {
    if (m_textEditor->CanRedo()) {
        m_textEditor->Redo();
    }
}

void ScintillaTextEditorPanel::Copy() {
    if (CanCopy()) {
        m_textEditor->Copy();
    }
}

void ScintillaTextEditorPanel::Cut() {
    if (CanCut()) {
        m_textEditor->Cut();
    }
}

void ScintillaTextEditorPanel::Paste() {
    if (CanPaste()) {
        m_textEditor->Paste();
    }
}

bool ScintillaTextEditorPanel::CanUndo() const {
    return m_textEditor->CanUndo();
}

bool ScintillaTextEditorPanel::CanRedo() const {
    return m_textEditor->CanRedo();
}

bool ScintillaTextEditorPanel::CanCopy() const {
    return m_textEditor->GetSelectionStart() != m_textEditor->GetSelectionEnd();
}

bool ScintillaTextEditorPanel::CanCut() const {
    return CanCopy() && !m_textEditor->GetReadOnly();
}

bool ScintillaTextEditorPanel::CanPaste() const {
    return m_textEditor->CanPaste();
}

void ScintillaTextEditorPanel::SetWordWrap(bool enabled) {
    m_textEditor->SetWrapMode(enabled ? wxSTC_WRAP_WORD : wxSTC_WRAP_NONE);
}

bool ScintillaTextEditorPanel::IsWordWrapEnabled() const {
    return m_textEditor->GetWrapMode() != wxSTC_WRAP_NONE;
}

void ScintillaTextEditorPanel::SetEditorFont(const wxFont& font) {
    m_textEditor->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
    m_textEditor->StyleClearAll();
}

wxFont ScintillaTextEditorPanel::GetEditorFont() const {
    return m_textEditor->StyleGetFont(wxSTC_STYLE_DEFAULT);
}

void ScintillaTextEditorPanel::FocusEditor() {
    m_textEditor->SetFocus();
}
