#include "HtmlSourcePanel.h"

HtmlSourcePanel::HtmlSourcePanel(wxWindow* parent, OnCloseCallback onCloseCallback)
    : HtmlSourcePanelWx(parent), m_onCloseCallback(std::move(onCloseCallback)) {
    m_styledTextCtrl->SetLexer(wxSTC_LEX_HTML);

    // Monospace font for every style; StyleClearAll propagates the default
    wxFont monoFont(wxFontInfo(12).Family(wxFONTFAMILY_TELETYPE));
    m_styledTextCtrl->StyleSetFont(wxSTC_STYLE_DEFAULT, monoFont);
    m_styledTextCtrl->StyleClearAll();

    // Colors for the HTML lexer styles
    m_styledTextCtrl->StyleSetForeground(wxSTC_H_TAG, wxColour(0, 0, 200));
    m_styledTextCtrl->StyleSetForeground(wxSTC_H_TAGEND, wxColour(0, 0, 200));
    m_styledTextCtrl->StyleSetForeground(wxSTC_H_TAGUNKNOWN, wxColour(0, 0, 200));
    m_styledTextCtrl->StyleSetForeground(wxSTC_H_ATTRIBUTE, wxColour(125, 0, 125));
    m_styledTextCtrl->StyleSetForeground(wxSTC_H_ATTRIBUTEUNKNOWN, wxColour(125, 0, 125));
    m_styledTextCtrl->StyleSetForeground(wxSTC_H_DOUBLESTRING, wxColour(0, 128, 0));
    m_styledTextCtrl->StyleSetForeground(wxSTC_H_SINGLESTRING, wxColour(0, 128, 0));
    m_styledTextCtrl->StyleSetForeground(wxSTC_H_COMMENT, wxColour(128, 128, 128));
    m_styledTextCtrl->StyleSetForeground(wxSTC_H_ENTITY, wxColour(200, 100, 0));
    m_styledTextCtrl->StyleSetForeground(wxSTC_H_NUMBER, wxColour(200, 100, 0));
    m_styledTextCtrl->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(128, 128, 128));

    // The base panel enables the fold margin; the HTML lexer additionally
    // needs this property to compute fold levels for tags
    m_styledTextCtrl->SetProperty("fold.html", "1");

    // The panel is a viewer; ShowHtml() lifts read-only around updates
    m_styledTextCtrl->SetReadOnly(true);

    m_styledTextCtrl->Bind(wxEVT_STC_MARGINCLICK, &HtmlSourcePanel::HandleMarginClick, this);
    m_closeButton->Bind(wxEVT_BUTTON, &HtmlSourcePanel::HandleCloseButtonClick, this);
}

void HtmlSourcePanel::ShowHtml(const wxString& html) {
    m_styledTextCtrl->SetReadOnly(false);
    m_styledTextCtrl->SetText(html);
    m_styledTextCtrl->SetReadOnly(true);
}

void HtmlSourcePanel::Copy() {
    if (CanCopy()) {
        m_styledTextCtrl->Copy();
    }
}

bool HtmlSourcePanel::CanCopy() const {
    return m_styledTextCtrl->GetSelectionStart() != m_styledTextCtrl->GetSelectionEnd();
}

void HtmlSourcePanel::HandleMarginClick(wxStyledTextEvent& event) {
    int line = m_styledTextCtrl->LineFromPosition(event.GetPosition());
    if (m_styledTextCtrl->GetFoldLevel(line) & wxSTC_FOLDLEVELHEADERFLAG) {
        m_styledTextCtrl->ToggleFold(line);
    }
}

void HtmlSourcePanel::HandleCloseButtonClick(wxCommandEvent& event) {
    if (m_onCloseCallback) {
        m_onCloseCallback();
    }
}
