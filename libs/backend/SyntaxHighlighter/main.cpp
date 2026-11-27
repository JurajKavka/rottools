#include <wx/stc/stc.h>

#include <algorithm>
#include <iostream>
#include <string_view>

#include "SyntaxHighlighter.h"

int main() {
    using namespace SyntaxHighlighter;

    const auto pythonFormat = GetReferenceFormat(wxFileName("example.PY"));
    const auto python = GetReferenceSyntax(pythonFormat);
    const auto pythonLexer = GetLexer(python);
    const auto csvFormat = GetReferenceFormat(wxFileName("data.csv"));
    const auto plainFormat = GetReferenceFormat(wxFileName("README"));
    if (pythonFormat != ReferenceFormat::Python || python != Syntax::Python || pythonLexer.id != wxSTC_LEX_PYTHON ||
        !std::string_view(pythonLexer.keywords).starts_with("False None True") || csvFormat != ReferenceFormat::Csv ||
        GetReferenceSyntax(csvFormat) != Syntax::None || plainFormat != ReferenceFormat::PlainText ||
        GetLexer(GetReferenceSyntax(plainFormat)).id != wxSTC_LEX_NULL) {
        std::cerr << "Reference format or lexer check failed\n";
        return 1;
    }

    const wxColour lightBackground(255, 255, 255);
    const wxColour darkBackground(0, 0, 0);
    const wxColour mutedColour(96, 96, 96);
    const auto lightStyles = GetStyles(python, lightBackground, mutedColour);
    const auto darkStyles = GetStyles(python, darkBackground, mutedColour);
    const auto lightKeyword = std::ranges::find(lightStyles, wxSTC_P_WORD, &Style::id);
    const auto darkKeyword = std::ranges::find(darkStyles, wxSTC_P_WORD, &Style::id);
    const auto markdownStyles = GetStyles(Syntax::Markdown, lightBackground, mutedColour);
    const auto markdownLink = std::ranges::find(markdownStyles, wxSTC_MARKDOWN_LINK, &Style::id);
    if (lightKeyword == lightStyles.end() || lightKeyword->foreground != wxColour(0, 60, 170) ||
        darkKeyword == darkStyles.end() || darkKeyword->foreground != wxColour(130, 190, 255) ||
        markdownLink == markdownStyles.end() || !markdownLink->underline ||
        !GetStyles(Syntax::None, lightBackground, mutedColour).empty()) {
        std::cerr << "Syntax style check failed\n";
        return 1;
    }

    std::cout << "SyntaxHighlighter format, lexer, and style checks passed\n";
}
