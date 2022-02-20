#pragma once

#include "Piece.hpp"
#include "Move.hpp"
#include "Color.hpp"

#include <string>
#include <memory>
#include <optional>
#include <vector>

namespace tafl
{

class IBoard
{
public:
    virtual ~IBoard()
    {
    }

    virtual unsigned getBoardDimension() const = 0;

    virtual std::optional<Piece::Type> pieceAt(const Pos &pos) const = 0;

    virtual const std::vector<Piece> getPieces(const Color &which) const = 0;

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

    static std::unique_ptr<IBoard> fromString(const std::string &s);
};

}