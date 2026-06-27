#include <wx/wx.h>

#include <iostream>

#include "MarkdownToHtmlAsync.h"  // Your parser class

// Custom Event Definition

class ConsoleReceiver : public wxEvtHandler {
   public:
    ConsoleReceiver() {
        Bind(EVT_MARKDOWN_READY, &ConsoleReceiver::OnMarkdownReady, this);
        Bind(EVT_MARKDOWN_ERROR, &ConsoleReceiver::OnMarkdownError, this);
    }

    void OnMarkdownReady(wxThreadEvent& event) {
        std::cout << "[Main Thread] Event Received!" << std::endl;
        std::cout << "Content: " << event.GetString().ToStdString() << std::endl;

        // Stop the app after receiving the event
        wxTheApp->ExitMainLoop();
    }

    void OnMarkdownError(wxThreadEvent& event) {
        std::cout << "[Main Thread] Event Received ERROR!" << std::endl;
        std::cout << "Content: " << event.GetString().ToStdString() << std::endl;

        // Stop the app after receiving the event
        wxTheApp->ExitMainLoop();
    }
};

class MyApp : public wxApp {
   public:
    bool OnInit() override {
        m_receiver = new ConsoleReceiver();
        m_parser = std::make_shared<MarkdownToHtmlAsync>(m_receiver);
        std::cout << "[Main Thread] Launching Async Parse..." << std::endl;
        m_parser->ParseFile("test_file.md");
        return true;  // Start the main loop
    }

   private:
    ConsoleReceiver* m_receiver;
    std::shared_ptr<MarkdownToHtmlAsync> m_parser;
};

// This macro creates the main() entry point for you
wxIMPLEMENT_APP(MyApp);