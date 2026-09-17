#include <wx/sizer.h>
#include <wx/wx.h>

#include <array>
#include <functional>
#include <optional>

#include "ChessBoardPanel.h"
#include "HelperFunctions.h"

namespace {

using rottools::chess::Color;
using rottools::chess::kBoardSize;
using rottools::chess::Piece;
using rottools::chess::PieceType;
using rottools::chess::Position;

[[nodiscard]] const char* ColorName(Color color) {
    return color == Color::White ? "White" : "Black";
}

[[nodiscard]] const char* PieceName(PieceType type) {
    switch (type) {
        case PieceType::Pawn:
            return "pawn";
        case PieceType::Knight:
            return "knight";
        case PieceType::Bishop:
            return "bishop";
        case PieceType::Rook:
            return "rook";
        case PieceType::Queen:
            return "queen";
        case PieceType::King:
            return "king";
    }
    return "piece";
}

[[nodiscard]] ChessBoardPanel::State StartingBoard() {
    constexpr std::array<PieceType, kBoardSize> backRank = {
        PieceType::Rook, PieceType::Knight, PieceType::Bishop, PieceType::Queen,
        PieceType::King, PieceType::Bishop, PieceType::Knight, PieceType::Rook,
    };

    ChessBoardPanel::State state;
    for (int file = 0; file < kBoardSize; ++file) {
        state.pieces[file] = Piece{backRank[file], Color::White};
        state.pieces[kBoardSize + file] = Piece{PieceType::Pawn, Color::White};
        state.pieces[6 * kBoardSize + file] = Piece{PieceType::Pawn, Color::Black};
        state.pieces[7 * kBoardSize + file] = Piece{backRank[file], Color::Black};
    }
    return state;
}

class ChessBoardSmokeFrame final : public wxFrame {
   public:
    ChessBoardSmokeFrame() : wxFrame(nullptr, wxID_ANY, "Test: Chess Board", wxDefaultPosition, wxSize(400, 420)) {
        m_board = new ChessBoardPanel(this, std::bind_front(&ChessBoardSmokeFrame::HandlePiecePressed, this),
                                      std::bind_front(&ChessBoardSmokeFrame::HandleSquareReleased, this));

        auto* sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(m_board, 1, wxEXPAND);
        SetSizer(sizer);

        m_state = StartingBoard();
        m_board->SetState(m_state);
    }

   private:
    ChessBoardPanel* m_board = nullptr;
    ChessBoardPanel::State m_state;

    void HandlePiecePressed(Piece piece, Position position) {
        printLog("Piece pressed: {} {} at {}{}", ColorName(piece.color), PieceName(piece.type),
                 static_cast<char>('a' + position.file), position.rank + 1);
        m_state.selected = position;
        m_board->SetState(m_state);
    }

    void HandleSquareReleased(std::optional<Position> destination) {
        if (destination) {
            printLog("Mouse released at {}{}", static_cast<char>('a' + destination->file), destination->rank + 1);
        } else {
            printLog(wxString("Mouse released outside the board"));
        }
        m_state.selected.reset();
        m_board->SetState(m_state);
    }
};

}  // namespace

class ChessBoardPanelApp final : public wxApp {
   public:
    bool OnInit() override {
        auto* frame = new ChessBoardSmokeFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(ChessBoardPanelApp);
