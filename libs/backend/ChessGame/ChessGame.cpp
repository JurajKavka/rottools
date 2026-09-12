#include "ChessGame.h"

#include <array>
#include <cmath>

namespace rottools::chess {
namespace {

constexpr std::array<PieceType, kBoardSize> kBackRank = {
    PieceType::Rook, PieceType::Knight, PieceType::Bishop, PieceType::Queen,
    PieceType::King, PieceType::Bishop, PieceType::Knight, PieceType::Rook,
};

[[nodiscard]] int Step(int value) {
    return (value > 0) - (value < 0);
}

}  // namespace

bool Position::IsValid() const {
    return file >= 0 && file < kBoardSize && rank >= 0 && rank < kBoardSize;
}

Color Opposite(Color color) {
    return color == Color::White ? Color::Black : Color::White;
}

ChessGame::ChessGame() {
    Reset();
}

void ChessGame::Reset() {
    m_board.fill(std::nullopt);

    for (int file = 0; file < kBoardSize; ++file) {
        m_board[Index({file, 0})] = Piece{kBackRank[static_cast<std::size_t>(file)], Color::White};
        m_board[Index({file, 1})] = Piece{PieceType::Pawn, Color::White};
        m_board[Index({file, 6})] = Piece{PieceType::Pawn, Color::Black};
        m_board[Index({file, 7})] = Piece{kBackRank[static_cast<std::size_t>(file)], Color::Black};
    }

    m_sideToMove = Color::White;
    m_status = GameStatus::Playing;
    m_moveCount = 0;
}

std::optional<Piece> ChessGame::PieceAt(Position position) const {
    if (!position.IsValid()) {
        return std::nullopt;
    }
    return m_board[Index(position)];
}

Color ChessGame::SideToMove() const {
    return m_sideToMove;
}

GameStatus ChessGame::Status() const {
    return m_status;
}

std::optional<Color> ChessGame::Winner() const {
    if (m_status != GameStatus::Checkmate) {
        return std::nullopt;
    }
    return Opposite(m_sideToMove);
}

bool ChessGame::IsInCheck(Color color) const {
    for (int rank = 0; rank < kBoardSize; ++rank) {
        for (int file = 0; file < kBoardSize; ++file) {
            const Position position{file, rank};
            const auto piece = PieceAt(position);
            if (piece && piece->color == color && piece->type == PieceType::King) {
                return IsSquareAttacked(position, Opposite(color));
            }
        }
    }

    // A normal game always has both kings. Treat a malformed board as unsafe.
    return true;
}

std::size_t ChessGame::MoveCount() const {
    return m_moveCount;
}

std::vector<Position> ChessGame::LegalDestinations(Position from) const {
    std::vector<Position> destinations;
    if (m_status != GameStatus::Playing) {
        return destinations;
    }

    const auto piece = PieceAt(from);
    if (!piece || piece->color != m_sideToMove) {
        return destinations;
    }

    for (int rank = 0; rank < kBoardSize; ++rank) {
        for (int file = 0; file < kBoardSize; ++file) {
            const Position to{file, rank};
            if (IsLegalMoveFor(from, to, m_sideToMove)) {
                destinations.push_back(to);
            }
        }
    }
    return destinations;
}

bool ChessGame::TryMove(Position from, Position to) {
    if (m_status != GameStatus::Playing || !IsLegalMoveFor(from, to, m_sideToMove)) {
        return false;
    }

    ApplyUnchecked(from, to);
    ++m_moveCount;
    m_sideToMove = Opposite(m_sideToMove);

    if (!HasAnyLegalMove(m_sideToMove)) {
        m_status = IsInCheck(m_sideToMove) ? GameStatus::Checkmate : GameStatus::Stalemate;
    }
    return true;
}

std::size_t ChessGame::Index(Position position) {
    return static_cast<std::size_t>(position.rank * kBoardSize + position.file);
}

bool ChessGame::IsPathClear(Position from, Position to) const {
    const int fileStep = Step(to.file - from.file);
    const int rankStep = Step(to.rank - from.rank);
    Position current{from.file + fileStep, from.rank + rankStep};

    while (current != to) {
        if (PieceAt(current)) {
            return false;
        }
        current.file += fileStep;
        current.rank += rankStep;
    }
    return true;
}

bool ChessGame::IsPseudoLegalMove(Position from, Position to) const {
    if (!from.IsValid() || !to.IsValid() || from == to) {
        return false;
    }

    const auto piece = PieceAt(from);
    if (!piece) {
        return false;
    }

    const auto target = PieceAt(to);
    if (target && (target->color == piece->color || target->type == PieceType::King)) {
        return false;
    }

    const int fileDelta = to.file - from.file;
    const int rankDelta = to.rank - from.rank;
    const int absoluteFileDelta = std::abs(fileDelta);
    const int absoluteRankDelta = std::abs(rankDelta);

    switch (piece->type) {
        case PieceType::Pawn: {
            const int direction = piece->color == Color::White ? 1 : -1;
            const int startRank = piece->color == Color::White ? 1 : 6;
            if (fileDelta == 0 && rankDelta == direction && !target) {
                return true;
            }
            if (fileDelta == 0 && rankDelta == 2 * direction && from.rank == startRank && !target) {
                return !PieceAt({from.file, from.rank + direction});
            }
            return absoluteFileDelta == 1 && rankDelta == direction && target.has_value();
        }
        case PieceType::Knight:
            return (absoluteFileDelta == 1 && absoluteRankDelta == 2) ||
                   (absoluteFileDelta == 2 && absoluteRankDelta == 1);
        case PieceType::Bishop:
            return absoluteFileDelta == absoluteRankDelta && IsPathClear(from, to);
        case PieceType::Rook:
            return (fileDelta == 0 || rankDelta == 0) && IsPathClear(from, to);
        case PieceType::Queen:
            return (fileDelta == 0 || rankDelta == 0 || absoluteFileDelta == absoluteRankDelta) &&
                   IsPathClear(from, to);
        case PieceType::King:
            return absoluteFileDelta <= 1 && absoluteRankDelta <= 1;
    }
    return false;
}

bool ChessGame::IsLegalMoveFor(Position from, Position to, Color color) const {
    const auto piece = PieceAt(from);
    if (!piece || piece->color != color || !IsPseudoLegalMove(from, to)) {
        return false;
    }

    ChessGame simulated = *this;
    simulated.ApplyUnchecked(from, to);
    return !simulated.IsInCheck(color);
}

bool ChessGame::IsSquareAttacked(Position position, Color byColor) const {
    for (int rank = 0; rank < kBoardSize; ++rank) {
        for (int file = 0; file < kBoardSize; ++file) {
            const Position from{file, rank};
            const auto piece = PieceAt(from);
            if (!piece || piece->color != byColor) {
                continue;
            }

            const int fileDelta = position.file - from.file;
            const int rankDelta = position.rank - from.rank;
            const int absoluteFileDelta = std::abs(fileDelta);
            const int absoluteRankDelta = std::abs(rankDelta);

            switch (piece->type) {
                case PieceType::Pawn: {
                    const int direction = byColor == Color::White ? 1 : -1;
                    if (absoluteFileDelta == 1 && rankDelta == direction) {
                        return true;
                    }
                    break;
                }
                case PieceType::Knight:
                    if ((absoluteFileDelta == 1 && absoluteRankDelta == 2) ||
                        (absoluteFileDelta == 2 && absoluteRankDelta == 1)) {
                        return true;
                    }
                    break;
                case PieceType::Bishop:
                    if (absoluteFileDelta == absoluteRankDelta && IsPathClear(from, position)) {
                        return true;
                    }
                    break;
                case PieceType::Rook:
                    if ((fileDelta == 0 || rankDelta == 0) && IsPathClear(from, position)) {
                        return true;
                    }
                    break;
                case PieceType::Queen:
                    if ((fileDelta == 0 || rankDelta == 0 || absoluteFileDelta == absoluteRankDelta) &&
                        IsPathClear(from, position)) {
                        return true;
                    }
                    break;
                case PieceType::King:
                    if (absoluteFileDelta <= 1 && absoluteRankDelta <= 1) {
                        return true;
                    }
                    break;
            }
        }
    }
    return false;
}

bool ChessGame::HasAnyLegalMove(Color color) const {
    for (int fromRank = 0; fromRank < kBoardSize; ++fromRank) {
        for (int fromFile = 0; fromFile < kBoardSize; ++fromFile) {
            const Position from{fromFile, fromRank};
            const auto piece = PieceAt(from);
            if (!piece || piece->color != color) {
                continue;
            }

            for (int toRank = 0; toRank < kBoardSize; ++toRank) {
                for (int toFile = 0; toFile < kBoardSize; ++toFile) {
                    if (IsLegalMoveFor(from, {toFile, toRank}, color)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void ChessGame::ApplyUnchecked(Position from, Position to) {
    Piece movingPiece = *m_board[Index(from)];
    if (movingPiece.type == PieceType::Pawn && (to.rank == 0 || to.rank == kBoardSize - 1)) {
        movingPiece.type = PieceType::Queen;
    }

    m_board[Index(to)] = movingPiece;
    m_board[Index(from)] = std::nullopt;
}

}  // namespace rottools::chess
