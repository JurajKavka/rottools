#include "TextEditorPanel.h"

#include <wx/sizer.h>
#include <wx/textctrl.h>

namespace {
long TextEditorStyle(bool wordWrapEnabled) {
    return wxTE_MULTILINE | (wordWrapEnabled ? 0 : wxTE_DONTWRAP);
}
}  // namespace

TextEditorPanel::TextEditorPanel(wxWindow* parent) : TextEditorPanelWx(parent) {
    m_textEditor = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                  TextEditorStyle(m_wordWrapEnabled));
    m_textEditor->SetFont(wxFontInfo(12).Family(wxFONTFAMILY_TELETYPE));
    GetSizer()->Add(m_textEditor, 1, wxEXPAND | wxALL, 2);
    Layout();

    m_textEditor->EmptyUndoBuffer();
    m_textEditor->DiscardEdits();
}

void TextEditorPanel::LoadText(const wxString& text) {
    m_textEditor->ChangeValue(text);
    m_textEditor->EmptyUndoBuffer();
    m_textEditor->DiscardEdits();
    m_textEditor->SetInsertionPoint(0);
    m_textEditor->ShowPosition(0);
    m_savedText = text;
}

wxString TextEditorPanel::GetText() const {
    return m_textEditor->GetValue();
}

bool TextEditorPanel::HasUnsavedChanges() const {
    return m_textEditor->GetValue() != m_savedText;
}

void TextEditorPanel::MarkSaved() {
    m_savedText = m_textEditor->GetValue();
    m_textEditor->DiscardEdits();
}

void TextEditorPanel::Undo() {
    if (m_textEditor->CanUndo()) {
        m_textEditor->Undo();
    }
}

void TextEditorPanel::Redo() {
    if (m_textEditor->CanRedo()) {
        m_textEditor->Redo();
    }
}

void TextEditorPanel::Copy() {
    if (CanCopy()) {
        m_textEditor->Copy();
    }
}

void TextEditorPanel::Cut() {
    if (CanCut()) {
        m_textEditor->Cut();
    }
}

void TextEditorPanel::Paste() {
    if (CanPaste()) {
        m_textEditor->Paste();
    }
}

bool TextEditorPanel::CanUndo() const {
    return m_textEditor->CanUndo();
}

bool TextEditorPanel::CanRedo() const {
    return m_textEditor->CanRedo();
}

bool TextEditorPanel::CanCopy() const {
    long selectionStart = 0;
    long selectionEnd = 0;
    m_textEditor->GetSelection(&selectionStart, &selectionEnd);
    return selectionStart != selectionEnd;
}

bool TextEditorPanel::CanCut() const {
    return CanCopy() && m_textEditor->IsEditable();
}

bool TextEditorPanel::CanPaste() const {
    return m_textEditor->CanPaste();
}

void TextEditorPanel::SetWordWrap(bool enabled) {
    if (m_wordWrapEnabled == enabled) {
        return;
    }

    RecreateTextEditor(enabled);
}

bool TextEditorPanel::IsWordWrapEnabled() const {
    return m_wordWrapEnabled;
}

void TextEditorPanel::SetEditorFont(const wxFont& font) {
    m_textEditor->SetFont(font);
}

wxFont TextEditorPanel::GetEditorFont() const {
    return m_textEditor->GetFont();
}

void TextEditorPanel::FocusEditor() {
    m_textEditor->SetFocus();
}

void TextEditorPanel::RecreateTextEditor(bool wordWrapEnabled) {
    wxTextCtrl* previousEditor = m_textEditor;
    const wxString text = previousEditor->GetValue();
    const wxFont font = previousEditor->GetFont();
    const bool hadFocus = wxWindow::FindFocus() == previousEditor;

    long selectionStart = 0;
    long selectionEnd = 0;
    previousEditor->GetSelection(&selectionStart, &selectionEnd);
    const long insertionPoint = previousEditor->GetInsertionPoint();

    auto* replacementEditor = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                             TextEditorStyle(wordWrapEnabled));
    replacementEditor->SetFont(font);
    replacementEditor->ChangeValue(text);
    replacementEditor->EmptyUndoBuffer();
    if (text == m_savedText) {
        replacementEditor->DiscardEdits();
    } else {
        replacementEditor->MarkDirty();
    }

    if (selectionStart != selectionEnd) {
        replacementEditor->SetSelection(selectionStart, selectionEnd);
    } else {
        replacementEditor->SetInsertionPoint(insertionPoint);
    }

    GetSizer()->Replace(previousEditor, replacementEditor);
    m_textEditor = replacementEditor;
    m_wordWrapEnabled = wordWrapEnabled;
    previousEditor->Destroy();
    Layout();

    if (hadFocus) {
        replacementEditor->SetFocus();
    }
}
