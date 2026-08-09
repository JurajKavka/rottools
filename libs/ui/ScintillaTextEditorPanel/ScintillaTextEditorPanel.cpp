#include "ScintillaTextEditorPanel.h"

#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stc/stc.h>

#include <algorithm>

ScintillaTextEditorPanel::ScintillaTextEditorPanel(wxWindow* parent) : ScintillaTextEditorPanel(parent, Options{}) {}

ScintillaTextEditorPanel::ScintillaTextEditorPanel(wxWindow* parent, Options options)
    : ScintillaTextEditorPanelWx(parent), m_options(options) {
    m_textEditor = new wxStyledTextCtrl(this, wxID_ANY);

    // Keep the plain-text defaults visually close to a native text editor.
    // Richer editor features are enabled only when requested by the consumer.
    m_textEditor->SetLexer(m_options.syntax == Syntax::Markdown ? wxSTC_LEX_MARKDOWN : wxSTC_LEX_NULL);
    m_textEditor->SetCodePage(wxSTC_CP_UTF8);
    m_textEditor->SetMarginType(0, wxSTC_MARGIN_NUMBER);
    m_textEditor->SetMarginWidth(0, 0);
    m_textEditor->SetMarginWidth(1, 0);
    m_textEditor->SetMarginWidth(2, 0);
    m_textEditor->SetIndentationGuides(false);
    m_textEditor->SetViewEOL(false);
    m_textEditor->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
    m_textEditor->SetWrapMode(wxSTC_WRAP_NONE);
    m_textEditor->SetScrollWidthTracking(true);
#ifdef __WXMSW__
    m_textEditor->SetEOLMode(wxSTC_EOL_CRLF);
#else
    m_textEditor->SetEOLMode(wxSTC_EOL_LF);
#endif

    ApplyEditorStyles(wxFontInfo(12).Family(wxFONTFAMILY_TELETYPE));
    ApplySelectionColours();

    wxSizer* mainSizer = GetSizer();
    mainSizer->Add(m_textEditor, 1, wxEXPAND | wxALL, 0);
    Layout();

    if (m_options.lineNumbers) {
        m_textEditor->Bind(wxEVT_STC_MODIFIED, &ScintillaTextEditorPanel::HandleEditorMetricsChanged, this);
        m_textEditor->Bind(wxEVT_STC_ZOOM, &ScintillaTextEditorPanel::HandleEditorMetricsChanged, this);
        UpdateLineNumberMarginWidth();
    }
    Bind(wxEVT_SYS_COLOUR_CHANGED, &ScintillaTextEditorPanel::HandleSystemColourChanged, this);

    m_textEditor->EmptyUndoBuffer();
    m_textEditor->SetSavePoint();
}

void ScintillaTextEditorPanel::LoadText(const wxString& text) {
    LoadText(text, LoadBehavior::ResetToTop);
}

void ScintillaTextEditorPanel::LoadText(const wxString& text, LoadBehavior loadBehavior) {
    UpdateEolModeForText(text);

    if (loadBehavior == LoadBehavior::KeepPosition) {
        const int caretPosition = m_textEditor->GetCurrentPos();
        const int anchorPosition = m_textEditor->GetAnchor();
        const int firstVisibleLine = m_textEditor->GetFirstVisibleLine();
        const int horizontalOffset = m_textEditor->GetXOffset();

        m_textEditor->SetText(text);
        m_textEditor->EmptyUndoBuffer();
        m_textEditor->SetSavePoint();
        m_textEditor->SetAnchor(std::min(anchorPosition, m_textEditor->GetLength()));
        m_textEditor->SetCurrentPos(std::min(caretPosition, m_textEditor->GetLength()));
        // FirstVisibleLine is a display-line index (wrapped sub-lines count),
        // so let Scintilla clamp it rather than comparing it with document lines.
        m_textEditor->SetFirstVisibleLine(firstVisibleLine);
        m_textEditor->SetXOffset(horizontalOffset);
        UpdateLineNumberMarginWidth();
        return;
    }

    m_textEditor->SetText(text);
    m_textEditor->EmptyUndoBuffer();
    m_textEditor->SetSavePoint();
    m_textEditor->GotoPos(0);
    m_textEditor->SetFirstVisibleLine(0);
    m_textEditor->SetXOffset(0);
    UpdateLineNumberMarginWidth();
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
    ApplyEditorStyles(font);
    UpdateLineNumberMarginWidth();
}

wxFont ScintillaTextEditorPanel::GetEditorFont() const {
    return m_textEditor->StyleGetFont(wxSTC_STYLE_DEFAULT);
}

void ScintillaTextEditorPanel::FocusEditor() {
    m_textEditor->SetFocus();
}

void ScintillaTextEditorPanel::ApplyEditorStyles(const wxFont& font) {
    const wxColour foreground = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    const wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);

    m_textEditor->StyleSetForeground(wxSTC_STYLE_DEFAULT, foreground);
    m_textEditor->StyleSetBackground(wxSTC_STYLE_DEFAULT, background);
    m_textEditor->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
    m_textEditor->StyleClearAll();
    m_textEditor->SetCaretForeground(foreground);

    if (m_options.syntax == Syntax::Markdown) {
        ApplyMarkdownStyles();
    }

    if (m_options.lineNumbers) {
        m_textEditor->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }
}

void ScintillaTextEditorPanel::ApplyMarkdownStyles() {
    const wxColour mutedColour = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
    const wxColour headerColour(0, 0, 200);
    const wxColour codeColour(160, 30, 30);
    const wxColour linkColour(0, 100, 200);
    const wxColour listColour(200, 100, 0);

    for (int style = wxSTC_MARKDOWN_HEADER1; style <= wxSTC_MARKDOWN_HEADER6; ++style) {
        m_textEditor->StyleSetForeground(style, headerColour);
        m_textEditor->StyleSetBold(style, true);
    }

    m_textEditor->StyleSetBold(wxSTC_MARKDOWN_STRONG1, true);
    m_textEditor->StyleSetBold(wxSTC_MARKDOWN_STRONG2, true);
    m_textEditor->StyleSetItalic(wxSTC_MARKDOWN_EM1, true);
    m_textEditor->StyleSetItalic(wxSTC_MARKDOWN_EM2, true);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_CODE, codeColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_CODE2, codeColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_CODEBK, codeColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_BLOCKQUOTE, mutedColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_LINK, linkColour);
    m_textEditor->StyleSetUnderline(wxSTC_MARKDOWN_LINK, true);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_ULIST_ITEM, listColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_OLIST_ITEM, listColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_HRULE, mutedColour);
    m_textEditor->StyleSetForeground(wxSTC_MARKDOWN_STRIKEOUT, mutedColour);
}

void ScintillaTextEditorPanel::ApplySelectionColours() {
#ifndef __WXOSX__
    // Cocoa supplies Scintilla's native selection appearance. Overriding its
    // text colour splits selected text into separate shaped runs, which can
    // make glyphs appear to shift as the selection changes.
    m_textEditor->SetSelBackground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
    m_textEditor->SetSelForeground(true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
#endif
}

void ScintillaTextEditorPanel::UpdateEolModeForText(const wxString& text) {
    if (text.Find("\r\n") != wxNOT_FOUND) {
        m_textEditor->SetEOLMode(wxSTC_EOL_CRLF);
    } else if (text.Find('\n') != wxNOT_FOUND) {
        m_textEditor->SetEOLMode(wxSTC_EOL_LF);
    } else if (text.Find('\r') != wxNOT_FOUND) {
        m_textEditor->SetEOLMode(wxSTC_EOL_CR);
    } else {
#ifdef __WXMSW__
        m_textEditor->SetEOLMode(wxSTC_EOL_CRLF);
#else
        m_textEditor->SetEOLMode(wxSTC_EOL_LF);
#endif
    }
}

void ScintillaTextEditorPanel::UpdateLineNumberMarginWidth() {
    if (!m_options.lineNumbers) {
        return;
    }

    int digitCount = 1;
    for (int lineCount = std::max(1, m_textEditor->GetLineCount()); lineCount >= 10; lineCount /= 10) {
        ++digitCount;
    }

    // Three digits avoids constant resizing in small documents; larger files
    // grow and shrink the margin as their line-count digit count changes.
    digitCount = std::max(3, digitCount);
    wxString widthTemplate("_");
    for (int digit = 0; digit < digitCount; ++digit) {
        widthTemplate += '9';
    }

    const int requiredWidth = m_textEditor->TextWidth(wxSTC_STYLE_LINENUMBER, widthTemplate);
    if (m_textEditor->GetMarginWidth(0) != requiredWidth) {
        m_textEditor->SetMarginWidth(0, requiredWidth);
    }
}

void ScintillaTextEditorPanel::HandleEditorMetricsChanged(wxStyledTextEvent& event) {
    UpdateLineNumberMarginWidth();
    event.Skip();
}

void ScintillaTextEditorPanel::HandleSystemColourChanged(wxSysColourChangedEvent& event) {
    ApplyEditorStyles(GetEditorFont());
    ApplySelectionColours();
    UpdateLineNumberMarginWidth();
    event.Skip();
}
