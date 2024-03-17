#pragma once

#include "IBoard.hpp"
#include "Move.hpp"

#include <iterator>
#include <vector>

namespace tafl
{

class IMoveTrait
{
public:
    struct MoveIterator
    {
        MoveIterator(const Piece& piece)
        {
            auto pos = piece.getPosition();
            xLeft = pos;
            xRight = pos;
            yUp = pos;
            xDown = pos;
        }

        Move move;
        Pos xLeft;
        Pos xRight;
        Pos yUp;
        Pos xDown;
    };

    virtual ~IMoveTrait() = default;

    /**
     * Calculate the possible moves for a particular piece.
     *
     * @param board the board to calculate for
     * @param piece the piece to move
     *
     * @return the set of possible moves
     */
    virtual std::vector<Move> getMoves(const IBoard& board, const Piece& piece) = 0;

    virtual std::optional<MoveIterator> begin(const IBoard& board, const Piece& piece) = 0;
    virtual std::optional<MoveIterator>
    next(MoveIterator& iterator, const IBoard& board, const Piece& piece) = 0;

    static std::unique_ptr<IMoveTrait> create();
};

} // namespace tafl
