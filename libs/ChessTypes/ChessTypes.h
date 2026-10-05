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

struct Position {
    int file = 0;
    int rank = 0;

    [[nodiscard]] constexpr bool IsValid() const {
        return file >= 0 && file < kBoardSize && rank >= 0 && rank < kBoardSize;
    }
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

[[nodiscard]] constexpr Color Opposite(Color color) {
    return color == Color::White ? Color::Black : Color::White;
}

}  // namespace rottools::chess
