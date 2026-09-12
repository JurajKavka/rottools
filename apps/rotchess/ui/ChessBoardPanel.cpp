#include "ChessBoardPanel.h"

#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/settings.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

namespace {

[[nodiscard]] int Luminance(const wxColour& colour) {
    return 299 * colour.Red() + 587 * colour.Green() + 114 * colour.Blue();
}

[[nodiscard]] wxColour Mix(const wxColour& first, const wxColour& second, double secondWeight) {
    const auto channel = [secondWeight](unsigned char firstValue, unsigned char secondValue) {
        return static_cast<unsigned char>(std::lround(firstValue * (1.0 - secondWeight) + secondValue * secondWeight));
    };
    return {channel(first.Red(), second.Red()), channel(first.Green(), second.Green()),
            channel(first.Blue(), second.Blue())};
}

}  // namespace

ChessBoardPanel::ChessBoardPanel(wxWindow* parent, rottools::chess::ChessGame& game,
                                 std::function<void()> onMoveCompleted)
    : wxPanel(parent), m_game(game), m_onMoveCompleted(std::move(onMoveCompleted)) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    SetName(_("Chess board"));
    SetToolTip(_("Select a piece, then click a highlighted square."));

    Bind(wxEVT_PAINT, &ChessBoardPanel::HandlePaint, this);
    Bind(wxEVT_LEFT_DOWN, &ChessBoardPanel::HandleLeftDown, this);
    Bind(wxEVT_SIZE, &ChessBoardPanel::HandleSize, this);
    Bind(wxEVT_SYS_COLOUR_CHANGED, &ChessBoardPanel::HandleSystemColourChanged, this);
}

void ChessBoardPanel::ResetInteraction() {
    m_selected.reset();
    m_lastMove.reset();
    m_legalDestinations.clear();
    Refresh();
}

wxRect ChessBoardPanel::BoardRectangle() const {
    const wxSize clientSize = GetClientSize();
    const int margin = FromDIP(8);
    const int available = std::max(0, std::min(clientSize.x, clientSize.y) - 2 * margin);
    const int boardSize = (available / rottools::chess::kBoardSize) * rottools::chess::kBoardSize;
    return {(clientSize.x - boardSize) / 2, (clientSize.y - boardSize) / 2, boardSize, boardSize};
}

wxRect ChessBoardPanel::SquareRectangle(rottools::chess::Position position) const {
    const wxRect board = BoardRectangle();
    const int squareSize = board.width / rottools::chess::kBoardSize;
    const int screenRank = rottools::chess::kBoardSize - 1 - position.rank;
    return {board.x + position.file * squareSize, board.y + screenRank * squareSize, squareSize, squareSize};
}

std::optional<rottools::chess::Position> ChessBoardPanel::PositionAt(wxPoint point) const {
    const wxRect board = BoardRectangle();
    if (board.width == 0 || !board.Contains(point)) {
        return std::nullopt;
    }

    const int squareSize = board.width / rottools::chess::kBoardSize;
    const int file = (point.x - board.x) / squareSize;
    const int screenRank = (point.y - board.y) / squareSize;
    return rottools::chess::Position{file, rottools::chess::kBoardSize - 1 - screenRank};
}

bool ChessBoardPanel::IsLegalDestination(rottools::chess::Position position) const {
    return std::find(m_legalDestinations.begin(), m_legalDestinations.end(), position) != m_legalDestinations.end();
}

void ChessBoardPanel::Select(rottools::chess::Position position) {
    m_selected = position;
    m_legalDestinations = m_game.LegalDestinations(position);
}

void ChessBoardPanel::HandlePaint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();

    const wxRect board = BoardRectangle();
    if (board.width == 0) {
        return;
    }

    const wxColour windowColour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    const wxColour textColour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    const wxColour brighter = Luminance(windowColour) >= Luminance(textColour) ? windowColour : textColour;
    const wxColour darker = Luminance(windowColour) < Luminance(textColour) ? windowColour : textColour;
    const wxColour lightSquare = Mix(brighter, darker, 0.12);
    const wxColour darkSquare = Mix(brighter, darker, 0.38);
    const wxColour accent = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);

    dc.SetPen(*wxTRANSPARENT_PEN);
    for (int rank = 0; rank < rottools::chess::kBoardSize; ++rank) {
        for (int file = 0; file < rottools::chess::kBoardSize; ++file) {
            const rottools::chess::Position position{file, rank};
            const bool light = (file + rank) % 2 != 0;
            wxColour squareColour = light ? lightSquare : darkSquare;

            if (m_lastMove && (m_lastMove->from == position || m_lastMove->to == position)) {
                squareColour = Mix(squareColour, accent, 0.22);
            }
            if (m_selected == position) {
                squareColour = Mix(squareColour, accent, 0.48);
            }

            dc.SetBrush(wxBrush(squareColour));
            dc.DrawRectangle(SquareRectangle(position));
        }
    }

    const int squareSize = board.width / rottools::chess::kBoardSize;
    for (const auto destination : m_legalDestinations) {
        wxRect marker = SquareRectangle(destination);
        if (m_game.PieceAt(destination)) {
            const int inset = std::max(2, squareSize / 12);
            marker.Deflate(inset);
            dc.SetPen(wxPen(accent, std::max(2, FromDIP(2))));
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawEllipse(marker);
        } else {
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(accent));
            const wxPoint centre{marker.x + marker.width / 2, marker.y + marker.height / 2};
            dc.DrawCircle(centre, std::max(3, squareSize / 10));
        }
    }

    std::unique_ptr<wxGraphicsContext> graphics(wxGraphicsContext::Create(dc));
    if (graphics) {
        graphics->SetAntialiasMode(wxANTIALIAS_DEFAULT);
        for (int rank = 0; rank < rottools::chess::kBoardSize; ++rank) {
            for (int file = 0; file < rottools::chess::kBoardSize; ++file) {
                const rottools::chess::Position position{file, rank};
                const auto piece = m_game.PieceAt(position);
                if (piece) {
                    DrawPiece(*graphics, SquareRectangle(position), *piece);
                }
            }
        }
    }

    dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW), 1));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(board);
}

void ChessBoardPanel::DrawPiece(wxGraphicsContext& graphics, const wxRect& rectangle,
                                rottools::chess::Piece piece) const {
    const wxColour windowColour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    const wxColour textColour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    const wxColour lightPiece = Luminance(windowColour) >= Luminance(textColour) ? windowColour : textColour;
    const wxColour darkPiece = Luminance(windowColour) < Luminance(textColour) ? windowColour : textColour;
    const wxColour fill = piece.color == rottools::chess::Color::White ? lightPiece : darkPiece;
    const wxColour outline = piece.color == rottools::chess::Color::White ? darkPiece : lightPiece;
    const double size = rectangle.width;
    const auto x = [&rectangle, size](double value) { return rectangle.x + value * size; };
    const auto y = [&rectangle, size](double value) { return rectangle.y + value * size; };

    graphics.SetBrush(wxBrush(fill));
    graphics.SetPen(graphics.CreatePen(wxGraphicsPenInfo(outline).Width(std::max(1.0, size * 0.025))));

    const auto drawBase = [&] {
        wxGraphicsPath base = graphics.CreatePath();
        base.MoveToPoint(x(0.25), y(0.72));
        base.AddLineToPoint(x(0.75), y(0.72));
        base.AddLineToPoint(x(0.82), y(0.86));
        base.AddLineToPoint(x(0.18), y(0.86));
        base.CloseSubpath();
        graphics.DrawPath(base);
        graphics.StrokeLine(x(0.23), y(0.78), x(0.77), y(0.78));
    };

    switch (piece.type) {
        case rottools::chess::PieceType::Pawn: {
            graphics.DrawEllipse(x(0.37), y(0.18), size * 0.26, size * 0.26);
            wxGraphicsPath body = graphics.CreatePath();
            body.MoveToPoint(x(0.43), y(0.43));
            body.AddCurveToPoint(x(0.43), y(0.56), x(0.34), y(0.61), x(0.30), y(0.72));
            body.AddLineToPoint(x(0.70), y(0.72));
            body.AddCurveToPoint(x(0.66), y(0.61), x(0.57), y(0.56), x(0.57), y(0.43));
            body.CloseSubpath();
            graphics.DrawPath(body);
            drawBase();
            break;
        }
        case rottools::chess::PieceType::Rook: {
            wxGraphicsPath rook = graphics.CreatePath();
            rook.MoveToPoint(x(0.25), y(0.20));
            rook.AddLineToPoint(x(0.37), y(0.20));
            rook.AddLineToPoint(x(0.37), y(0.30));
            rook.AddLineToPoint(x(0.45), y(0.30));
            rook.AddLineToPoint(x(0.45), y(0.20));
            rook.AddLineToPoint(x(0.55), y(0.20));
            rook.AddLineToPoint(x(0.55), y(0.30));
            rook.AddLineToPoint(x(0.63), y(0.30));
            rook.AddLineToPoint(x(0.63), y(0.20));
            rook.AddLineToPoint(x(0.75), y(0.20));
            rook.AddLineToPoint(x(0.71), y(0.40));
            rook.AddLineToPoint(x(0.66), y(0.45));
            rook.AddLineToPoint(x(0.69), y(0.72));
            rook.AddLineToPoint(x(0.31), y(0.72));
            rook.AddLineToPoint(x(0.34), y(0.45));
            rook.AddLineToPoint(x(0.29), y(0.40));
            rook.CloseSubpath();
            graphics.DrawPath(rook);
            drawBase();
            break;
        }
        case rottools::chess::PieceType::Knight: {
            wxGraphicsPath knight = graphics.CreatePath();
            knight.MoveToPoint(x(0.29), y(0.72));
            knight.AddCurveToPoint(x(0.31), y(0.58), x(0.34), y(0.48), x(0.41), y(0.40));
            knight.AddLineToPoint(x(0.34), y(0.34));
            knight.AddLineToPoint(x(0.50), y(0.18));
            knight.AddCurveToPoint(x(0.68), y(0.22), x(0.75), y(0.38), x(0.76), y(0.51));
            knight.AddCurveToPoint(x(0.68), y(0.58), x(0.59), y(0.59), x(0.52), y(0.55));
            knight.AddCurveToPoint(x(0.55), y(0.62), x(0.60), y(0.67), x(0.64), y(0.72));
            knight.CloseSubpath();
            graphics.DrawPath(knight);
            graphics.SetBrush(wxBrush(outline));
            graphics.DrawEllipse(x(0.57), y(0.31), size * 0.055, size * 0.055);
            graphics.SetBrush(wxBrush(fill));
            drawBase();
            break;
        }
        case rottools::chess::PieceType::Bishop: {
            wxGraphicsPath bishop = graphics.CreatePath();
            bishop.MoveToPoint(x(0.50), y(0.17));
            bishop.AddCurveToPoint(x(0.37), y(0.26), x(0.34), y(0.38), x(0.43), y(0.48));
            bishop.AddCurveToPoint(x(0.39), y(0.56), x(0.34), y(0.63), x(0.31), y(0.72));
            bishop.AddLineToPoint(x(0.69), y(0.72));
            bishop.AddCurveToPoint(x(0.66), y(0.63), x(0.61), y(0.56), x(0.57), y(0.48));
            bishop.AddCurveToPoint(x(0.66), y(0.38), x(0.63), y(0.26), x(0.50), y(0.17));
            bishop.CloseSubpath();
            graphics.DrawPath(bishop);
            graphics.StrokeLine(x(0.55), y(0.25), x(0.45), y(0.40));
            drawBase();
            break;
        }
        case rottools::chess::PieceType::Queen: {
            wxGraphicsPath queen = graphics.CreatePath();
            queen.MoveToPoint(x(0.24), y(0.28));
            queen.AddLineToPoint(x(0.36), y(0.42));
            queen.AddLineToPoint(x(0.43), y(0.24));
            queen.AddLineToPoint(x(0.50), y(0.42));
            queen.AddLineToPoint(x(0.57), y(0.24));
            queen.AddLineToPoint(x(0.64), y(0.42));
            queen.AddLineToPoint(x(0.76), y(0.28));
            queen.AddLineToPoint(x(0.67), y(0.72));
            queen.AddLineToPoint(x(0.33), y(0.72));
            queen.CloseSubpath();
            graphics.DrawPath(queen);
            graphics.DrawEllipse(x(0.19), y(0.21), size * 0.10, size * 0.10);
            graphics.DrawEllipse(x(0.38), y(0.16), size * 0.10, size * 0.10);
            graphics.DrawEllipse(x(0.52), y(0.16), size * 0.10, size * 0.10);
            graphics.DrawEllipse(x(0.71), y(0.21), size * 0.10, size * 0.10);
            drawBase();
            break;
        }
        case rottools::chess::PieceType::King: {
            wxGraphicsPath king = graphics.CreatePath();
            king.MoveToPoint(x(0.39), y(0.40));
            king.AddCurveToPoint(x(0.29), y(0.49), x(0.33), y(0.61), x(0.37), y(0.72));
            king.AddLineToPoint(x(0.63), y(0.72));
            king.AddCurveToPoint(x(0.67), y(0.61), x(0.71), y(0.49), x(0.61), y(0.40));
            king.AddCurveToPoint(x(0.57), y(0.36), x(0.54), y(0.34), x(0.50), y(0.34));
            king.AddCurveToPoint(x(0.46), y(0.34), x(0.43), y(0.36), x(0.39), y(0.40));
            king.CloseSubpath();
            graphics.DrawPath(king);
            graphics.StrokeLine(x(0.50), y(0.12), x(0.50), y(0.35));
            graphics.StrokeLine(x(0.41), y(0.20), x(0.59), y(0.20));
            drawBase();
            break;
        }
    }
}

void ChessBoardPanel::HandleLeftDown(wxMouseEvent& event) {
    SetFocus();
    const auto clicked = PositionAt(event.GetPosition());
    if (!clicked || m_game.Status() != rottools::chess::GameStatus::Playing) {
        return;
    }

    if (m_selected && IsLegalDestination(*clicked)) {
        const rottools::chess::Move move{*m_selected, *clicked};
        if (m_game.TryMove(move.from, move.to)) {
            m_lastMove = move;
            m_selected.reset();
            m_legalDestinations.clear();
            Refresh();
            if (m_onMoveCompleted) {
                m_onMoveCompleted();
            }
            return;
        }
    }

    const auto piece = m_game.PieceAt(*clicked);
    if (piece && piece->color == m_game.SideToMove()) {
        Select(*clicked);
    } else {
        m_selected.reset();
        m_legalDestinations.clear();
    }
    Refresh();
}

void ChessBoardPanel::HandleSize(wxSizeEvent& event) {
    Refresh();
    event.Skip();
}

void ChessBoardPanel::HandleSystemColourChanged(wxSysColourChangedEvent& event) {
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    Refresh();
    event.Skip();
}
