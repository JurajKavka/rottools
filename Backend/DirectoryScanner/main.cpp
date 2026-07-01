#include <wx/wx.h>
#include <iostream>
#include <filesystem>
#include "DirectoryScanner.h"

class ScanReceiver : public wxEvtHandler {
public:
    ScanReceiver() {
        // 4. Bind to the GLOBAL app object.
        // Because wxTheApp is broadcasting, ANY class can use this exact bind line to listen.
        Bind(wxEVT_DIRECTORY_SCAN_COMPLETE, &ScanReceiver::OnScanComplete, this);
    }

    ~ScanReceiver() {
        // Always unbind custom event handlers in destructors to prevent crashes
        Unbind(wxEVT_DIRECTORY_SCAN_COMPLETE, &ScanReceiver::OnScanComplete, this);
    }

    // 5. The Event Handler
    void OnScanComplete(wxThreadEvent& event) {
        std::cout << "\n[Main Thread] Scan Complete Event Received!\n";

        // Extract the custom data from the event payload
        auto files = event.GetPayload<std::vector<FileEntry>>();

        std::cout << "Found " << files.size() << " items.\n";
        for (const auto& file : files) {
            if (file.isDirectory) {
                std::cout << "[DIR]  " << file.name << "\n";
            } else {
                std::cout << "[FILE] " << file.name << " (" << file.size << " bytes)\n";
            }
        }

        wxTheApp->ExitMainLoop();
    }
};

class MyApp : public wxApp {
public:
    bool OnInit() override {
        m_receiver = new ScanReceiver();
        m_scanner = std::make_shared<DirectoryScanner>();

        std::filesystem::path currentDir = std::filesystem::current_path();
        std::vector<std::string> noFilters = {}; 

        std::cout << "[Main Thread] Launching Async Scan of: " << currentDir << "...\n";

        // 6. Just call start. The event loop handles the rest.
        m_scanner->StartScan(currentDir, noFilters, m_receiver);

        return true;  
    }

    int OnExit() override {
        delete m_receiver;
        return wxApp::OnExit();
    }

private:
    ScanReceiver* m_receiver;
    std::shared_ptr<DirectoryScanner> m_scanner;
};

wxIMPLEMENT_APP(MyApp);