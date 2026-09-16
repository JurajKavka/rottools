#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

#include "ChessTypes.h"

namespace rottools::chess {

enum class GameStatus {
    Playing,
    Checkmate,
    Stalemate,
};

class ChessGame final {
   public:
    ChessGame();

    void Reset();

    [[nodiscard]] std::optional<Piece> PieceAt(Position position) const;
    [[nodiscard]] Color SideToMove() const;
    [[nodiscard]] GameStatus Status() const;
    [[nodiscard]] std::optional<Color> Winner() const;
    [[nodiscard]] bool IsInCheck(Color color) const;
    [[nodiscard]] std::size_t MoveCount() const;
    [[nodiscard]] std::vector<Position> LegalDestinations(Position from) const;

    // Moves are rejected unless they obey piece movement, turn order, blocking,
    // and king safety. Pawns reaching the last rank promote to a queen.
    [[nodiscard]] bool TryMove(Position from, Position to);

   private:
    using Square = std::optional<Piece>;
    using Board = std::array<Square, kBoardSize * kBoardSize>;

    Board m_board{};
    Color m_sideToMove = Color::White;
    GameStatus m_status = GameStatus::Playing;
    std::size_t m_moveCount = 0;

    [[nodiscard]] static std::size_t Index(Position position);
    [[nodiscard]] bool IsPathClear(Position from, Position to) const;
    [[nodiscard]] bool IsPseudoLegalMove(Position from, Position to) const;
    [[nodiscard]] bool IsLegalMoveFor(Position from, Position to, Color color) const;
    [[nodiscard]] bool IsSquareAttacked(Position position, Color byColor) const;
    [[nodiscard]] bool HasAnyLegalMove(Color color) const;
    void ApplyUnchecked(Position from, Position to);
};

}  // namespace rottools::chess
