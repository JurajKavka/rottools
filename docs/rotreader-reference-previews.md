# ROT Reader reference previews

Implement read-only reference previews for source code and CSV in ROT Reader. Follow the repository's [AGENTS.md](../AGENTS.md) for C++ style, wxFormBuilder ownership, and verification approvals.

## Expected behaviour

| Extensions | Preview |
| --- | --- |
| `.md`, `.markdown` | Existing Markdown document workflow |
| `.sql`, `.js`, `.ts`, `.py`, `.cpp` | Read-only source with syntax highlighting, line numbers, selection, copy, and word wrap |
| `.csv` | Read-only table with numbered columns and values displayed as text |
| `.txt`, `.log`, and other UTF-8 files | Existing native plain-text preview with word wrap |

Match extensions case-insensitively. Reference previews remain modal and use the filename as the title. Opening and closing one must preserve the active Markdown document, including unsaved edits. Keep the existing UTF-8 validation, 2 MiB file limit, and read-error messages.

## Implementation

### Route previews through the existing open path

- Keep Markdown dispatch in `MainFrame::HandleOpenFile`. In `ShowReferenceText`, select the preview using the file extension after `ReadTextFileUtf8` succeeds. Use the same route for every existing open entry point. [ROT Reader main frame](../apps/rotreader/MainFrame.cpp) · [UTF-8 reader](../libs/HelperFunctions/HelperFunctions.cpp)
- Extend the hand-written `TextFilePreviewDialog` to host the native text control, the read-only Scintilla panel, or the CSV table. Reuse the existing Word Wrap checkbox for plain text and code. Hide the wrap toolbar in table mode. Keep the existing `ShowText` path usable as the plain-text fallback. Make layout changes in the hand-written subclass and leave wxFormBuilder project and generated files untouched. [Preview dialog](../libs/ui/TextFilePreviewDialog/TextFilePreviewDialog.cpp)

### Reuse the Scintilla panel for code

- Extend `ScintillaTextEditorPanel::Options` with opt-in read-only behaviour and syntax choices for the requested source formats. Reuse the panel's font, line number, wrap, and system-colour handling. Keep lexer configuration and keyword lists in the shared component. [Scintilla panel](../libs/ui/ScintillaTextEditorPanel/ScintillaTextEditorPanel.h)
- Configure the SQL, Python, and C++ lexers for their respective formats. Use separate JavaScript and TypeScript keyword lists and lexer settings with the C++ lexer. Check version-sensitive settings such as JavaScript template literals against the supported wxWidgets versions. Verify token contrast in light and dark appearance. [wxWidgets lexer definitions](https://github.com/wxWidgets/wxWidgets/blob/v3.3.3/include/wx/stc/stc.h#L465) · [Lexilla history](https://www.scintilla.org/LexillaHistory.html)
- Enforce read-only mode on the underlying `wxStyledTextCtrl`. Restore it after loading preview text programmatically. The preview uses buffer loading and viewing operations only. Preserve the panel's editable defaults and the existing Markdown consumer. [Read-only loading example](../libs/ui/HtmlSourcePanel/HtmlSourcePanel.cpp) · [Markdown consumer](../libs/ui/MarkdownEditorPanel/MarkdownEditorPanel.cpp)

### Add the CSV parser and table

- Add a UI-independent CSV parser under `libs/backend`. Accept the validated text and return rows of string fields or an error with the record number. Handle quoted commas, doubled quotes, embedded line breaks, LF and CRLF records, a final record without a newline, and trailing empty fields. Preserve field contents such as `00123`. [CSV format reference](https://www.rfc-editor.org/rfc/rfc4180)
- Add `CsvTablePanel` under `libs/ui`. Use `wxDataViewCtrl` with standard read-only text renderers and platform styling. Display the first record as data and use numbered column headings. For rows of different lengths, display missing cells as blank and keep any extra fields visible. [wxDataViewCtrl documentation](https://github.com/wxWidgets/wxWidgets/blob/v3.3.3/interface/wx/dataview.h#L820)
- On malformed CSV, tell the user where parsing failed and open the original text in the plain-text preview. Keep table population bounded for unusually wide or dense files. If a file exceeds the table's display capacity, explain the limit and show the original text so all records remain accessible.

## Smoke tests and acceptance

1. Extend `make run-scintillatexteditor` with a sample for each new language. Check keyword, string, and comment styles, line numbers, and wrapping. Verify that selection and copy work and that typing, paste, and cut cannot modify preview text. Exercise the existing editable mode as well.
2. Give the CSV parser and table separate C++20 library targets, `rottools::*` aliases, and standalone `main.cpp` smoke apps behind `ROTTOOLS_BUILD_LIB_APPS`. Register each in its parent CMake file and add root `make run-csvparser` and `make run-csvtable` targets using `_demos`, with `.PHONY` entries and help descriptions. Exercise quoted commas, multiline records, escaped quotes, leading zeroes, ragged rows, empty fields, and malformed input. Display representative data in the table smoke app.
3. Extend `make run-textfilepreviewdialog` to exercise plain text, highlighted code, and CSV. Verify that word wrap is offered on the two text views and that the table is read-only.
4. Verify ROT Reader through **File → Open** and the file browser. Open representative code, CSV, text, and log files while a Markdown document has unsaved edits. Check uppercase extensions, malformed CSV fallback, the existing binary and oversized-file errors, and preservation of the active Markdown document when each preview closes.

Follow the approval rules in `AGENTS.md` before running `make check`, `make rotreader-build`, or the smoke apps. Report the commands actually run and any remaining verification gaps.
