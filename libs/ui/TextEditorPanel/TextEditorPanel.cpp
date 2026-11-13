#include "TextEditorPanel.h"

#include <wx/sizer.h>
#include <wx/textctrl.h>

#include <algorithm>

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

void TextEditorPanel::LoadText(const wxString& text, bool keepPosition) {
    long selectionStart = 0;
    long selectionEnd = 0;
    m_textEditor->GetSelection(&selectionStart, &selectionEnd);
    const int scrollPosition = m_textEditor->GetScrollPos(wxVERTICAL);
    wxString normalized = text;
    normalized.Replace("\r\n", "\n");
    normalized.Replace("\r", "\n");
    m_textEditor->ChangeValue(normalized);
    m_textEditor->EmptyUndoBuffer();
    m_textEditor->DiscardEdits();
    if (keepPosition) {
        const long length = m_textEditor->GetLastPosition();
        m_textEditor->SetSelection(std::min(selectionStart, length), std::min(selectionEnd, length));
        m_textEditor->ScrollLines(scrollPosition - m_textEditor->GetScrollPos(wxVERTICAL));
    } else {
        m_textEditor->SetInsertionPoint(0);
        m_textEditor->ShowPosition(0);
    }
    // Compare against the control's representation (notably native newlines).
    m_savedText = m_textEditor->GetValue();
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

void TextEditorPanel::SelectAll() {
    m_textEditor->SelectAll();
}

wxString TextEditorPanel::GetSelectedText() const {
    return m_textEditor->GetStringSelection();
}

std::optional<std::pair<long, long>> TextEditorPanel::FindMatch(const wxString& text, const TextSearchOptions& options,
                                                                long start) const {
    if (text.IsEmpty()) {
        return std::nullopt;
    }
#ifdef __WXOSX__
    // Cocoa search returns UTF-16 control positions, rather than wxString
    // character indices. Its end-of-document sentinel is -1.
    if (options.backwards && start == 0) {
        return std::nullopt;
    }
    wxTextSearch search(text);
    search.Start(options.backwards && start == m_textEditor->GetLastPosition() ? -1 : start)
        .MatchCase(options.matchCase)
        .MatchWholeWord(options.wholeWord)
        .SearchDirection(options.backwards ? wxTextSearch::Direction::Up : wxTextSearch::Direction::Down);
    const wxTextSearchResult result = m_textEditor->SearchText(search);
    if (result) {
        return std::pair{result.m_start, result.m_end};
    }
#else
    // wxGTK 3.2 has no SearchText API, and Windows only implements it for
    // RichEdit controls. Retain the existing native plain-text control.
    const wxString contents = m_textEditor->GetValue();
    const long length = static_cast<long>(contents.length());
    const long needleLength = static_cast<long>(text.length());
    const long offset = static_cast<long>(m_textEditor->GetRange(0, start).length());
    const auto isWordCharacter = [](wxUniChar character) { return character == '_' || wxIsalnum(character); };
    for (long position = options.backwards ? std::min(offset - needleLength, length - needleLength) : offset;
         position >= 0 && position + needleLength <= length; position += options.backwards ? -1 : 1) {
        if (!contents.Mid(position, needleLength).IsSameAs(text, options.matchCase)) {
            continue;
        }
        if (options.wholeWord &&
            ((position > 0 && isWordCharacter(contents[position - 1])) ||
             (position + needleLength < length && isWordCharacter(contents[position + needleLength])))) {
            continue;
        }
#ifdef __WXMSW__
        // The plain Windows EDIT control counts UTF-16 units and CRLF pairs;
        // GetValue/GetRange normalize those pairs to LF in wxString.
        const auto nativePosition = [&contents](long index) {
            const wxString prefix = contents.Left(index);
            return static_cast<long>(prefix.ToStdWstring().length() + prefix.Freq('\n'));
        };
        return std::pair{nativePosition(position), nativePosition(position + needleLength)};
#else
        return std::pair{position, position + needleLength};
#endif
    }
#endif
    return std::nullopt;
}

bool TextEditorPanel::FindText(const wxString& text, const TextSearchOptions& options) {
    long start = 0;
    long end = 0;
    m_textEditor->GetSelection(&start, &end);
    auto match = FindMatch(text, options, options.backwards ? start : end);
    if (!match && options.wrap) {
        match = FindMatch(text, options, options.backwards ? m_textEditor->GetLastPosition() : 0);
    }
    if (!match) {
        return false;
    }
    m_textEditor->SetSelection(match->first, match->second);
    m_textEditor->ShowPosition(match->first);
    return true;
}

bool TextEditorPanel::ReplaceText(const wxString& text, const wxString& replacement, const TextSearchOptions& options) {
    long start = 0;
    long end = 0;
    m_textEditor->GetSelection(&start, &end);
    TextSearchOptions forward = options;
    forward.backwards = false;
    const auto match = FindMatch(text, forward, start);
    if (!match || *match != std::pair{start, end}) {
        return FindText(text, options);
    }
    m_textEditor->Replace(start, end, replacement);
    // Replace() leaves the insertion point at the end of the replacement.
    // For backwards search, resume before it instead.
    if (options.backwards) {
        m_textEditor->SetInsertionPoint(start);
    }
    (void)FindText(text, options);
    return true;
}

int TextEditorPanel::ReplaceAllText(const wxString& text, const wxString& replacement,
                                    const TextSearchOptions& options) {
    TextSearchOptions forward = options;
    forward.backwards = false;
    wxString updated;
    long position = 0;
    int count = 0;
    while (const auto match = FindMatch(text, forward, position)) {
        updated += m_textEditor->GetRange(position, match->first) + replacement;
        position = match->second;
        ++count;
    }
    if (count > 0) {
        updated += m_textEditor->GetRange(position, m_textEditor->GetLastPosition());
        // One native replacement preserves Undo instead of loading new text
        // (which would reset the document's save point and undo history).
        m_textEditor->Replace(0, m_textEditor->GetLastPosition(), updated);
        m_textEditor->SetInsertionPoint(0);
    }
    return count;
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
