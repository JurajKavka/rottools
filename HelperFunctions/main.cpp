#include <iostream>

#include "HelperFunctions.h"

int main() {
    printCppVersion();
    printLog("Log as standard string.");
    printLog(wxString("Log as wxString string."));
    return 0;
}
