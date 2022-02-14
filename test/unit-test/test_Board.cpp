#include <IBoard.hpp>

#include "tests.hpp"

using namespace tafl;

namespace
{
/*
 * Legend:
 *   b : black piece
 *   w : white piece
 *   k : white king + castle
 *
 * For the unit tests, capitals are used to specify a specific piece.
 */

// 9x9
const std::string tablut =
    "   bbb   "
    "    b    "
    "    w    "
    "b   w   b"
    "bbwwkwwbb"
    "b   w   b"
    "    w    "
    "    b    "
    "  bbbb   "
;

}

SCENARIO("A board is created from a string")
{
    WHEN("an invalid string is used to create a board")
    {
        auto unaligned = tablut.substr(0, tablut.size() - 1);

        auto p0 = IBoard::fromString("");
        auto p1 = IBoard::fromString("s");
        auto p2 = IBoard::fromString(unaligned);

        THEN("it fails")
        {
            REQUIRE_FALSE(p0);
            REQUIRE_FALSE(p1);
            REQUIRE_FALSE(p2);
        }
    }

    WHEN("a tablut board is given")
    {
        auto p = IBoard::fromString(tablut);

        THEN("it can be created")
        {
            REQUIRE(p);
        }

        THEN("the board dimensions is correct")
        {
            // 9x9
            REQUIRE(p->getBoardDimension() == 9);
        }

        THEN("the pieces are placed correctly")
        {
            REQUIRE_FALSE(p->pieceAt({0,0}).has_value());
            REQUIRE_FALSE(p->pieceAt({0,1}).has_value());
            REQUIRE_FALSE(p->pieceAt({9,0}).has_value());
            REQUIRE_FALSE(p->pieceAt({0,9}).has_value());
            REQUIRE_FALSE(p->pieceAt({9,9}).has_value());

            REQUIRE(p->pieceAt({3,0}) == IPiece::Type::Black);
            REQUIRE(p->pieceAt({4,4}) == IPiece::Type::King);
            REQUIRE(p->pieceAt({4,5}) == IPiece::Type::White);
            REQUIRE(p->pieceAt({5,4}) == IPiece::Type::White);
        }
    }
}
