#pragma once

#include <wx/colour.h>
#include <wx/filename.h>

#include <optional>
#include <vector>

enum class Syntax {
    None,
    Markdown,
    Sql,
    JavaScript,
    TypeScript,
    Python,
    Cpp,
    Shell,
    Java,
    Xml,
};

enum class ReferenceFormat { PlainText, Csv, Sql, JavaScript, TypeScript, Python, Cpp, Shell, Java, Xml };

namespace SyntaxHighlighter {

struct LexerProperty {
    const char* name;
    const char* value;
};

struct Lexer {
    int id = 0;
    const char* keywords = "";
    std::optional<LexerProperty> property = std::nullopt;
};

struct Style {
    int id = 0;
    std::optional<wxColour> foreground = std::nullopt;
    bool bold = false;
    bool italic = false;
    bool underline = false;
};

[[nodiscard]] ReferenceFormat GetReferenceFormat(const wxFileName& filePath);
[[nodiscard]] Syntax GetReferenceSyntax(ReferenceFormat format);

/** Return the Scintilla lexer settings and style overrides for the requested syntax. */
[[nodiscard]] Lexer GetLexer(Syntax syntax);
[[nodiscard]] std::vector<Style> GetStyles(Syntax syntax, const wxColour& background, const wxColour& mutedColour);

}  // namespace SyntaxHighlighter
