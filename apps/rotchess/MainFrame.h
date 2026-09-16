#pragma once

#include <wx/frame.h>

#include <optional>
#include <vector>

#include "ChessGame.h"

class ChessBoardPanel;

class MainFrame final : public wxFrame {
   public:
    explicit MainFrame(wxWindow* parent);

   private:
    rottools::chess::ChessGame m_game;
    ChessBoardPanel* m_boardPanel = nullptr;
    std::optional<rottools::chess::Position> m_selected;
    std::optional<rottools::chess::Move> m_lastMove;
    std::vector<rottools::chess::Position> m_legalDestinations;

    void HandleNewGameMenuItemClick(wxCommandEvent& event);
    void HandleExitMenuItemClick(wxCommandEvent& event);
    void HandleAboutMenuItemClick(wxCommandEvent& event);
    void HandlePiecePressed(rottools::chess::Piece piece, rottools::chess::Position position);
    void HandleSquareReleased(std::optional<rottools::chess::Position> destination);
    void UpdateBoard();
    void UpdateStatus();
};
