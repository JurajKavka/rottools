#pragma once

#include <compare>

namespace rottools::chess {

constexpr int kBoardSize = 8;

enum class Color {
    White,
    Black,
};

enum class PieceType {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
};

enum class GameStatus {
    Playing,
    Checkmate,
    Stalemate,
};

struct Position {
    int file = 0;
    int rank = 0;

    [[nodiscard]] bool IsValid() const;
    auto operator<=>(const Position&) const = default;
};

struct Piece {
    PieceType type = PieceType::Pawn;
    Color color = Color::White;

    auto operator<=>(const Piece&) const = default;
};

struct Move {
    Position from;
    Position to;

    auto operator<=>(const Move&) const = default;
};

[[nodiscard]] Color Opposite(Color color);

}  // namespace rottools::chess
