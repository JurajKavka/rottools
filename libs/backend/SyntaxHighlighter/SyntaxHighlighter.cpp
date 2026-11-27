#include "SyntaxHighlighter.h"

#include <wx/stc/stc.h>
#include <wx/version.h>

#include <algorithm>
#include <array>

namespace SyntaxHighlighter {
namespace {
#if wxMAJOR_VERSION > 3 || (wxMAJOR_VERSION == 3 && wxMINOR_VERSION > 3) || \
    (wxMAJOR_VERSION == 3 && wxMINOR_VERSION == 3 && wxRELEASE_NUMBER >= 3)
constexpr auto kBackquotedStringMode = "2";
#else
constexpr auto kBackquotedStringMode = "1";
#endif
constexpr LexerProperty kBackquotedStrings{"lexer.cpp.backquoted.strings", kBackquotedStringMode};

std::vector<Style> GetMarkdownStyles(const wxColour& mutedColour) {
    const wxColour headerColour(0, 0, 200);
    const wxColour codeColour(160, 30, 30);
    const wxColour linkColour(0, 100, 200);
    const wxColour listColour(200, 100, 0);

    return {
        {.id = wxSTC_MARKDOWN_HEADER1, .foreground = headerColour, .bold = true},
        {.id = wxSTC_MARKDOWN_HEADER2, .foreground = headerColour, .bold = true},
        {.id = wxSTC_MARKDOWN_HEADER3, .foreground = headerColour, .bold = true},
        {.id = wxSTC_MARKDOWN_HEADER4, .foreground = headerColour, .bold = true},
        {.id = wxSTC_MARKDOWN_HEADER5, .foreground = headerColour, .bold = true},
        {.id = wxSTC_MARKDOWN_HEADER6, .foreground = headerColour, .bold = true},
        {.id = wxSTC_MARKDOWN_STRONG1, .bold = true},
        {.id = wxSTC_MARKDOWN_STRONG2, .bold = true},
        {.id = wxSTC_MARKDOWN_EM1, .italic = true},
        {.id = wxSTC_MARKDOWN_EM2, .italic = true},
        {wxSTC_MARKDOWN_CODE, codeColour},
        {wxSTC_MARKDOWN_CODE2, codeColour},
        {wxSTC_MARKDOWN_CODEBK, codeColour},
        {wxSTC_MARKDOWN_BLOCKQUOTE, mutedColour},
        {.id = wxSTC_MARKDOWN_LINK, .foreground = linkColour, .underline = true},
        {wxSTC_MARKDOWN_ULIST_ITEM, listColour},
        {wxSTC_MARKDOWN_OLIST_ITEM, listColour},
        {wxSTC_MARKDOWN_HRULE, mutedColour},
        {wxSTC_MARKDOWN_STRIKEOUT, mutedColour},
    };
}
}  // namespace

ReferenceFormat GetReferenceFormat(const wxFileName& filePath) {
    struct ExtensionMapping {
        const char* extension;
        ReferenceFormat format;
    };
    static constexpr std::array mappings = {
        ExtensionMapping{"csv", ReferenceFormat::Csv},       ExtensionMapping{"sql", ReferenceFormat::Sql},
        ExtensionMapping{"js", ReferenceFormat::JavaScript}, ExtensionMapping{"ts", ReferenceFormat::TypeScript},
        ExtensionMapping{"py", ReferenceFormat::Python},     ExtensionMapping{"cpp", ReferenceFormat::Cpp},
        ExtensionMapping{"sh", ReferenceFormat::Shell},      ExtensionMapping{"zsh", ReferenceFormat::Shell},
        ExtensionMapping{"java", ReferenceFormat::Java},     ExtensionMapping{"xml", ReferenceFormat::Xml},
    };
    const wxString extension = filePath.GetExt().Lower();
    const auto mapping = std::find_if(mappings.begin(), mappings.end(),
                                      [&extension](const auto& entry) { return extension == entry.extension; });
    if (mapping != mappings.end()) {
        return mapping->format;
    }
    return ReferenceFormat::PlainText;
}

Syntax GetReferenceSyntax(ReferenceFormat format) {
    switch (format) {
        case ReferenceFormat::Sql:
            return Syntax::Sql;
        case ReferenceFormat::JavaScript:
            return Syntax::JavaScript;
        case ReferenceFormat::TypeScript:
            return Syntax::TypeScript;
        case ReferenceFormat::Python:
            return Syntax::Python;
        case ReferenceFormat::Cpp:
            return Syntax::Cpp;
        case ReferenceFormat::Shell:
            return Syntax::Shell;
        case ReferenceFormat::Java:
            return Syntax::Java;
        case ReferenceFormat::Xml:
            return Syntax::Xml;
        case ReferenceFormat::PlainText:
        case ReferenceFormat::Csv:
            return Syntax::None;
    }
    return Syntax::None;
}

Lexer GetLexer(Syntax syntax) {
    switch (syntax) {
        case Syntax::Markdown:
            return {wxSTC_LEX_MARKDOWN};
        case Syntax::Sql:
            return {wxSTC_LEX_SQL,
                    "select from where insert into values update delete create table drop alter join left right inner "
                    "outer on as and or not null is order by group having limit distinct union all case when then else "
                    "end primary key references index database default"};
        case Syntax::Python:
            return {
                wxSTC_LEX_PYTHON,
                "False None True and as assert async await break class continue def del elif else except finally "
                "for from global if import in is lambda nonlocal not or pass raise return try while with yield match "
                "case"};
        case Syntax::Shell:
            return {wxSTC_LEX_BASH,
                    "if then else elif fi for while until do done case esac in function select time coproc break "
                    "continue return exit export local readonly unset shift source eval exec trap set printf echo read "
                    "test true false"};
        case Syntax::Cpp:
            return {wxSTC_LEX_CPP,
                    "alignas alignof auto bool break case catch char class const constexpr continue default delete do "
                    "double else enum explicit extern false float for friend if inline int long namespace new nullptr "
                    "operator private protected public return short signed sizeof static struct switch template this "
                    "throw true try typedef typename union unsigned using virtual void volatile while"};
        case Syntax::Java:
            return {wxSTC_LEX_CPP,
                    "abstract assert boolean break byte case catch char class const continue default do double else "
                    "enum extends final finally float for if implements import instanceof int interface long native "
                    "new package private protected public return short static strictfp super switch synchronized this "
                    "throw throws transient try void volatile while true false null var record sealed permits yield"};
        case Syntax::JavaScript:
            return {wxSTC_LEX_CPP,
                    "async await break case catch class const continue debugger default delete do else export extends "
                    "false finally for function if import in instanceof let new null return static super switch this "
                    "throw true try typeof var void while with yield",
                    kBackquotedStrings};
        case Syntax::TypeScript:
            return {wxSTC_LEX_CPP,
                    "abstract any as asserts async await boolean break case catch class const constructor continue "
                    "declare default delete do else enum export extends false finally for from function get if "
                    "implements import in infer instanceof interface is keyof let module namespace never new null "
                    "number object private protected public readonly return set static string super switch symbol "
                    "this throw true try type typeof undefined unique unknown var void while yield",
                    kBackquotedStrings};
        case Syntax::Xml:
            return {wxSTC_LEX_XML};
        case Syntax::None:
            return {wxSTC_LEX_NULL};
    }
    return {wxSTC_LEX_NULL};
}

std::vector<Style> GetStyles(Syntax syntax, const wxColour& background, const wxColour& mutedColour) {
    if (syntax == Syntax::None) {
        return {};
    }
    if (syntax == Syntax::Markdown) {
        return GetMarkdownStyles(mutedColour);
    }

    const bool dark = background.Red() * 299 + background.Green() * 587 + background.Blue() * 114 < 128000;
    const wxColour keyword = dark ? wxColour(130, 190, 255) : wxColour(0, 60, 170);
    const wxColour string = dark ? wxColour(255, 180, 140) : wxColour(145, 35, 15);
    const wxColour comment = dark ? wxColour(145, 205, 145) : wxColour(0, 110, 35);

    if (syntax == Syntax::Python) {
        return {
            {wxSTC_P_WORD, keyword},  {wxSTC_P_STRING, string},       {wxSTC_P_CHARACTER, string},
            {wxSTC_P_TRIPLE, string}, {wxSTC_P_TRIPLEDOUBLE, string}, {wxSTC_P_COMMENTLINE, comment},
        };
    } else if (syntax == Syntax::Xml) {
        return {
            {wxSTC_H_TAG, keyword},
            {wxSTC_H_TAGUNKNOWN, keyword},
            {wxSTC_H_TAGEND, keyword},
            {wxSTC_H_XMLSTART, keyword},
            {wxSTC_H_XMLEND, keyword},
            {wxSTC_H_ATTRIBUTE, keyword},
            {wxSTC_H_ATTRIBUTEUNKNOWN, keyword},
            {wxSTC_H_DOUBLESTRING, string},
            {wxSTC_H_SINGLESTRING, string},
            {wxSTC_H_COMMENT, comment},
            {wxSTC_H_ENTITY, keyword},
        };
    } else if (syntax == Syntax::Shell) {
        return {
            {wxSTC_SH_WORD, keyword},        {wxSTC_SH_STRING, string},     {wxSTC_SH_CHARACTER, string},
            {wxSTC_SH_BACKTICKS, string},    {wxSTC_SH_HERE_DELIM, string}, {wxSTC_SH_HERE_Q, string},
            {wxSTC_SH_COMMENTLINE, comment},
        };
    } else if (syntax == Syntax::Sql) {
        return {
            {wxSTC_SQL_WORD, keyword},    {wxSTC_SQL_STRING, string},       {wxSTC_SQL_CHARACTER, string},
            {wxSTC_SQL_COMMENT, comment}, {wxSTC_SQL_COMMENTLINE, comment},
        };
    }
    return {
        {wxSTC_C_WORD, keyword},     {wxSTC_C_STRING, string},       {wxSTC_C_CHARACTER, string},
        {wxSTC_C_COMMENT, comment},  {wxSTC_C_COMMENTLINE, comment}, {wxSTC_C_COMMENTDOC, comment},
#ifdef wxSTC_C_BACKTICKS
        {wxSTC_C_BACKTICKS, string},
#endif
#ifdef wxSTC_C_STRINGRAW
        {wxSTC_C_STRINGRAW, string},
#endif
    };
}

}  // namespace SyntaxHighlighter
