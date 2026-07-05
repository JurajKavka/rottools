#include <wx/wx.h>

#include <iostream>

#include "DirectoryScanner.h"
#include "HelperFunctions.h"

class ScannerDemoApp : public wxApp {
   public:
    bool OnInit() override {
        Bind(wxEVT_DIRECTORY_SCAN_COMPLETE, &ScannerDemoApp::HandleScanComplete, this);

        ScanOptions scanOptions;
        scanOptions.extensions = {};
        scanOptions.showHiddenFiles = true;

        wxFileName rootDir;
        rootDir.AssignHomeDir();

        printLog("[Main Thread] Launching Async Scan of: {}", rootDir.GetFullPath().ToStdString());

        // 6. Just call start. The event loop handles the rest.
        m_directoryScanner.StartScan(rootDir, scanOptions, this);

        return true;
    }

    int OnExit() override {
        Unbind(wxEVT_DIRECTORY_SCAN_COMPLETE, &ScannerDemoApp::HandleScanComplete, this);
        return wxApp::OnExit();
    }

   private:
    DirectoryScanner m_directoryScanner;
    void HandleScanComplete(DirectoryScannerEvent& event) {
        printLog("[Main Thread] Scan Complete Event Received!");

        // Extract the custom data from the event payload

        auto sortedData = DirectoryScanner::SortEntries(event.files);

        printLog("Found {} items.", sortedData.size());
        for (const auto& file : sortedData) {
            if (file.isDirectory) {
                printLog("d {}", file.name);
            } else {
                printLog("f {} ({} bytes)", file.name, file.size);
            }
        }

        printLog("Current directory is: {}", event.currentDirectory.GetFullPath());

        wxTheApp->ExitMainLoop();
    }
};

wxIMPLEMENT_APP(ScannerDemoApp);
