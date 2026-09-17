#pragma once

#include <wx/panel.h>

#include <array>
#include <functional>
#include <optional>
#include <vector>

#include "ChessTypes.h"

class wxGraphicsContext;
class wxMouseCaptureLostEvent;
class wxMouseEvent;
class wxPaintEvent;
class wxSizeEvent;
class wxSysColourChangedEvent;

class ChessBoardPanel final : public wxPanel {
   public:
    struct State {
        std::array<std::optional<rottools::chess::Piece>, rottools::chess::kBoardSize * rottools::chess::kBoardSize>
            pieces{};
        std::optional<rottools::chess::Position> selected;
        std::optional<rottools::chess::Move> lastMove;
        std::vector<rottools::chess::Position> legalDestinations;
    };

    using PiecePressedHandler = std::function<void(rottools::chess::Piece, rottools::chess::Position)>;
    using SquareReleasedHandler = std::function<void(std::optional<rottools::chess::Position>)>;

    ChessBoardPanel(wxWindow* parent, PiecePressedHandler onPiecePressed, SquareReleasedHandler onSquareReleased);

    void SetState(State state);

   private:
    State m_state;
    PiecePressedHandler m_onPiecePressed;
    SquareReleasedHandler m_onSquareReleased;
    bool m_mouseDown = false;

    [[nodiscard]] wxRect BoardRectangle() const;
    [[nodiscard]] wxRect SquareRectangle(rottools::chess::Position position) const;
    [[nodiscard]] std::optional<rottools::chess::Position> PositionAt(wxPoint point) const;
    [[nodiscard]] std::optional<rottools::chess::Piece> PieceAt(rottools::chess::Position position) const;
    void DrawPiece(wxGraphicsContext& graphics, const wxRect& rectangle, rottools::chess::Piece piece) const;

    void HandlePaint(wxPaintEvent& event);
    void HandleLeftDown(wxMouseEvent& event);
    void HandleLeftUp(wxMouseEvent& event);
    void HandleMouseCaptureLost(wxMouseCaptureLostEvent& event);
    void HandleSize(wxSizeEvent& event);
    void HandleSystemColourChanged(wxSysColourChangedEvent& event);
};
