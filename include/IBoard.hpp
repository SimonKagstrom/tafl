#pragma once

#include "Color.hpp"
#include "Move.hpp"
#include "Piece.hpp"

#include <chrono>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace tafl
{

class IBoard
{
public:
    virtual ~IBoard() = default;

    virtual unsigned getBoardDimension() const = 0;

    virtual std::optional<Piece::Type> pieceAt(const Pos& pos) const = 0;

    virtual std::vector<Piece> getPieces(const Color& which) const = 0;

    virtual std::vector<Move> getPossibleMoves() const = 0;

    // Perform a move on the board
    virtual void move(Move move) = 0;

    virtual Color getTurn() const = 0;

    virtual void setTurn(Color which) = 0;


    /**
     * Return the winner of the current board.
     *
     * @return std::nullopt if there is no winner, otherwise the winning color
     */
    virtual std::optional<Color> getWinner() const = 0;

    /**
     * @brief Calculate the best move for the current color
     *
     * @param quota the time allowed for the calculation
     * @param onFutureReady called when the future is readable
     *
     * @return a future which notifies of the best move available for this board (or std::nullopt, if there are none)
     */
    virtual std::future<std::optional<Move>>
    calculateBestMove(const std::chrono::milliseconds& quota,
                      std::function<void()> onFutureReady) = 0;


    static std::unique_ptr<IBoard> fromString(const std::string_view& s);
};

} // namespace tafl