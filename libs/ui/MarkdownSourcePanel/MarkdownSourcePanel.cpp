#include "MarkdownSourcePanel.h"

#include <algorithm>
#include <utility>

MarkdownSourcePanel::MarkdownSourcePanel(wxWindow* parent, OnSaveCallback onSaveCallback,
                                         OnCloseCallback onCloseCallback)
    : MarkdownSourcePanelWx(parent),
      m_onSaveCallback(std::move(onSaveCallback)),
      m_onCloseCallback(std::move(onCloseCallback)) {
    m_styledTextCtrl->SetLexer(wxSTC_LEX_MARKDOWN);

    // Monospace font for every style; StyleClearAll propagates the default
    wxFont monoFont(wxFontInfo(12).Family(wxFONTFAMILY_TELETYPE));
    m_styledTextCtrl->StyleSetFont(wxSTC_STYLE_DEFAULT, monoFont);
    m_styledTextCtrl->StyleClearAll();

    // Colors for the markdown lexer styles
    const wxColour headerColour(0, 0, 200);
    for (int style = wxSTC_MARKDOWN_HEADER1; style <= wxSTC_MARKDOWN_HEADER6; ++style) {
        m_styledTextCtrl->StyleSetForeground(style, headerColour);
        m_styledTextCtrl->StyleSetBold(style, true);
    }
    m_styledTextCtrl->StyleSetBold(wxSTC_MARKDOWN_STRONG1, true);
    m_styledTextCtrl->StyleSetBold(wxSTC_MARKDOWN_STRONG2, true);
    m_styledTextCtrl->StyleSetItalic(wxSTC_MARKDOWN_EM1, true);
    m_styledTextCtrl->StyleSetItalic(wxSTC_MARKDOWN_EM2, true);
    m_styledTextCtrl->StyleSetForeground(wxSTC_MARKDOWN_CODE, wxColour(160, 30, 30));
    m_styledTextCtrl->StyleSetForeground(wxSTC_MARKDOWN_CODE2, wxColour(160, 30, 30));
    m_styledTextCtrl->StyleSetForeground(wxSTC_MARKDOWN_CODEBK, wxColour(160, 30, 30));
    m_styledTextCtrl->StyleSetForeground(wxSTC_MARKDOWN_BLOCKQUOTE, wxColour(128, 128, 128));
    m_styledTextCtrl->StyleSetForeground(wxSTC_MARKDOWN_LINK, wxColour(0, 100, 200));
    m_styledTextCtrl->StyleSetUnderline(wxSTC_MARKDOWN_LINK, true);
    m_styledTextCtrl->StyleSetForeground(wxSTC_MARKDOWN_ULIST_ITEM, wxColour(200, 100, 0));
    m_styledTextCtrl->StyleSetForeground(wxSTC_MARKDOWN_OLIST_ITEM, wxColour(200, 100, 0));
    m_styledTextCtrl->StyleSetForeground(wxSTC_MARKDOWN_HRULE, wxColour(128, 128, 128));
    m_styledTextCtrl->StyleSetForeground(wxSTC_MARKDOWN_STRIKEOUT, wxColour(128, 128, 128));
    m_styledTextCtrl->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(128, 128, 128));

    // The markdown lexer does not compute fold levels, so hide the fold
    // margin the base panel enables
    m_styledTextCtrl->SetMarginWidth(1, 0);

    m_styledTextCtrl->SetEOLMode(wxSTC_EOL_LF);

    m_styledTextCtrl->Bind(wxEVT_KEY_DOWN, &MarkdownSourcePanel::HandleKeyDown, this);
    m_closeButton->Bind(wxEVT_BUTTON, &MarkdownSourcePanel::HandleCloseButtonClick, this);
}

void MarkdownSourcePanel::ShowMarkdown(const wxString& markdown, ScrollBehavior scrollBehavior) {
    if (scrollBehavior == ScrollBehavior::ResetToTop) {
        m_styledTextCtrl->SetText(markdown);
        return;
    }

    const int caret = m_styledTextCtrl->GetCurrentPos();
    const int firstVisibleLine = m_styledTextCtrl->GetFirstVisibleLine();

    m_styledTextCtrl->SetText(markdown);

    m_styledTextCtrl->GotoPos(std::min(caret, m_styledTextCtrl->GetLength()));
    m_styledTextCtrl->SetFirstVisibleLine(firstVisibleLine);
}

void MarkdownSourcePanel::HandleKeyDown(wxKeyEvent& event) {
    if (event.GetKeyCode() == 'S' && event.CmdDown() && !event.ShiftDown() && !event.AltDown()) {
        if (m_onSaveCallback) {
            m_onSaveCallback(m_styledTextCtrl->GetText());
        }
        m_styledTextCtrl->SetSavePoint();
        return;
    }
    event.Skip();
}

void MarkdownSourcePanel::HandleCloseButtonClick(wxCommandEvent& event) {
    if (m_onCloseCallback) {
        m_onCloseCallback();
    }
}
