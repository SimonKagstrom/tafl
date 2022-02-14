#pragma once

#include "Move.hpp"
#include "IBoard.hpp"

#include <vector>

namespace tafl
{

class IMoveTrait
{
public:
    virtual ~IMoveTrait()
    {
    }

    /**
     * Calculate the possible moves for a particular piece.
     *
     * @param board the board to calculate for
     * @param piece the piece to move
     *
     * @return the set of possible moves
     */
    virtual std::vector<Move> getMoves(const IBoard &board, const IPiece &piece) = 0;


    static std::unique_ptr<IMoveTrait> create();
};

}
