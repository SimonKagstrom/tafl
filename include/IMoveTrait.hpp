#pragma once

#include "IBoard.hpp"
#include "Move.hpp"

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
    virtual std::vector<Move> getMoves(const IBoard& board, const Piece& piece) = 0;


    static std::unique_ptr<IMoveTrait> create();
};

} // namespace tafl
