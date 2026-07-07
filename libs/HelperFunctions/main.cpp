#include <iostream>

#include "HelperFunctions.h"

int main() {
    printCppVersion();
    printLog("Log as standard string.");
    printLog(wxString("Log as wxString string."));
    printLog("Test std::format - Hello {}", "World");
    printError("Printing to std error the error code {}", 123);
    return 0;
}
