#include <wx/wx.h>

#include <filesystem>
#include <iostream>

#include "DirectoryScanner.h"
#include "HelperFunctions.h"

class MyApp : public wxApp {
   public:
    bool OnInit() override {
        Bind(wxEVT_DIRECTORY_SCAN_COMPLETE, &MyApp::OnScanComplete, this);

        m_directoryScanner = std::make_shared<DirectoryScanner>();

        std::filesystem::path currentDir = std::filesystem::current_path();
        ScanOptions scanOptions;
        scanOptions.extensions = {};
        scanOptions.showHiddenFiles = true;

        wxFileName rootDir("/Users/jurajkavka/Documents/crossuite");
       // rootDir.AssignHomeDir();

        printLog("[Main Thread] Launching Async Scan of: {}", rootDir.GetFullPath().ToStdString());

        // 6. Just call start. The event loop handles the rest.
        m_directoryScanner->StartScan(rootDir, scanOptions, this);

        return true;
    }

    int OnExit() override {
        Unbind(wxEVT_DIRECTORY_SCAN_COMPLETE, &MyApp::OnScanComplete, this);
        return wxApp::OnExit();
    }

   private:
    std::shared_ptr<DirectoryScanner> m_directoryScanner;
    void OnScanComplete(wxThreadEvent& event) {
        printLog("[Main Thread] Scan Complete Event Received!");

        // Extract the custom data from the event payload
        auto files = event.GetPayload<std::vector<FileEntry>>();

        auto sortedData = m_directoryScanner->SortEntries(files);

        printLog("Found {} items.", sortedData.size());
        for (const auto& file : sortedData) {
            if (file.isDirectory) {
                printLog("d {}", file.name);
            } else {
                printLog("f {} ({} bytes)", file.name, file.size);
            }
        }

        wxTheApp->ExitMainLoop();
    }
};

wxIMPLEMENT_APP(MyApp);