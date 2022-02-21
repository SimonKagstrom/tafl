#pragma once

#include <IBoard.hpp>
#include <Pos.hpp>

namespace tafl::ut
{

struct BoardHelper
{
    std::optional<Piece> selected;
    std::optional<Move> move;
    std::unique_ptr<IBoard> board;
};

static std::optional<BoardHelper> parse(const std::string &s)
{
    auto b = IBoard::fromString(s);
    if (!b)
    {
        return std::nullopt;
    }

    BoardHelper out;
    auto dim = b->getBoardDimension();
    for (unsigned x = 0; x < dim; x++)
    {
        for (unsigned y = 0; y < dim; y++)
        {
            auto i = y * dim + x;
            auto piece = b->pieceAt({x,y});

            if (piece && isupper(s[i]))
            {
                REQUIRE_FALSE(out.selected);
                auto p = Piece(*piece);
                p.place({x,y});

                out.selected = p;
            }

            if (s[i] == '.')
            {
                // Fixup below
                out.move = Move{{0,0}, Pos{x,y}};
            }
        }
    }

    if (out.move)
    {
        REQUIRE(out.selected);
        out.move->from = out.selected->getPosition();
    }

    out.board = std::move(b);

    return out;
}

}
