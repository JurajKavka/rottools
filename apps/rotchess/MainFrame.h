#pragma once

#include <wx/frame.h>

#include "ChessGame.h"

class ChessBoardPanel;

class MainFrame final : public wxFrame {
   public:
    explicit MainFrame(wxWindow* parent);

   private:
    rottools::chess::ChessGame m_game;
    ChessBoardPanel* m_boardPanel = nullptr;

    void HandleNewGameMenuItemClick(wxCommandEvent& event);
    void HandleExitMenuItemClick(wxCommandEvent& event);
    void HandleAboutMenuItemClick(wxCommandEvent& event);
    void HandleGameChanged();
    void UpdateStatus();
};
