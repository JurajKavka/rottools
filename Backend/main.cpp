#include <wx/init.h>
#include <wx/string.h>

#include <iostream>

#include "HelperFunctions.h"
#include "MarkdownToHtml.h"

int main() {
    // 1. Initialize the wxWidgets console subsystem (crucial for wxString internals)
    printLog("Ahoj.");
    if (!wxInitialize()) {
        std::cerr << "Failed to initialize wxWidgets\n";
        return -1;
    }
    wxString testMarkdown = R"(
# Testing Markdown Parser
This is a **bold** test and a [link](https://example.com).
- Item 1
- Item 2
    )";
    printLog("--- Input Markdown ---");
    printLog(testMarkdown);

    wxString htmlResult = ConvertMarkdownToHtml(testMarkdown);

    printLog("--- HTML Result --- ");
    printLog(htmlResult);

    return 0;
}
