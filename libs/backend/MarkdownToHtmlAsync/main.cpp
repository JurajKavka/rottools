#include <wx/wx.h>

#include <iostream>
#include <memory>

#include "HelperFunctions.h"
#include "MarkdownToHtmlAsync.h"  // Your parser class

// Custom Event Definition

class ConsoleReceiver : public wxEvtHandler {
   public:
    ConsoleReceiver() {
        Bind(EVT_MARKDOWN_READY, &ConsoleReceiver::HandleMarkdownReady, this);
        Bind(EVT_MARKDOWN_ERROR, &ConsoleReceiver::HandleMarkdownError, this);
    }

    void HandleMarkdownReady(MarkdownToHtmlAsyncEvent& event) {
        printLog("[Main Thread] Event Received!");
        printLog("Filename dir: {}", event.filePath.GetAbsolutePath());
        std::cout << "Content: " << event.html.ToStdString() << std::endl;

        // Stop the app after receiving the event
        wxTheApp->ExitMainLoop();
    }

    void HandleMarkdownError(MarkdownToHtmlAsyncEvent& event) {
        std::cout << "[Main Thread] Event Received ERROR!" << std::endl;
        std::cout << "Content: " << event.error.ToStdString() << std::endl;

        // Stop the app after receiving the event
        wxTheApp->ExitMainLoop();
    }
};

class ParserDemoApp : public wxApp {
   public:
    bool OnInit() override {
        m_receiver = std::make_unique<ConsoleReceiver>();
        m_parser = std::make_unique<MarkdownToHtmlAsync>(m_receiver.get());
        std::cout << "[Main Thread] Launching Async Parse..." << std::endl;
        (void)m_parser->ParseFile(wxFileName("AGENTS.md"));
        return true;  // Start the main loop
    }

   private:
    // Declared receiver-before-parser so the parser (and its worker thread) is
    // destroyed first and never posts an event to a dead receiver.
    std::unique_ptr<ConsoleReceiver> m_receiver;
    std::unique_ptr<MarkdownToHtmlAsync> m_parser;
};

// This macro creates the main() entry point for you
wxIMPLEMENT_APP(ParserDemoApp);
