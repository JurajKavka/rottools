#pragma once

#include <wx/panel.h>

#include <functional>
#include <optional>
#include <vector>

#include "ChessGame.h"

class wxGraphicsContext;
class wxMouseEvent;
class wxPaintEvent;
class wxSizeEvent;
class wxSysColourChangedEvent;

class ChessBoardPanel final : public wxPanel {
   public:
    ChessBoardPanel(wxWindow* parent, rottools::chess::ChessGame& game, std::function<void()> onMoveCompleted);

    void ResetInteraction();

   private:
    rottools::chess::ChessGame& m_game;
    std::function<void()> m_onMoveCompleted;
    std::optional<rottools::chess::Position> m_selected;
    std::optional<rottools::chess::Move> m_lastMove;
    std::vector<rottools::chess::Position> m_legalDestinations;

    [[nodiscard]] wxRect BoardRectangle() const;
    [[nodiscard]] wxRect SquareRectangle(rottools::chess::Position position) const;
    [[nodiscard]] std::optional<rottools::chess::Position> PositionAt(wxPoint point) const;
    [[nodiscard]] bool IsLegalDestination(rottools::chess::Position position) const;
    void Select(rottools::chess::Position position);
    void DrawPiece(wxGraphicsContext& graphics, const wxRect& rectangle, rottools::chess::Piece piece) const;

    void HandlePaint(wxPaintEvent& event);
    void HandleLeftDown(wxMouseEvent& event);
    void HandleSize(wxSizeEvent& event);
    void HandleSystemColourChanged(wxSysColourChangedEvent& event);
};
