#include <IPiece.hpp>

#include "tests.hpp"

using namespace tafl;

SCENARIO("a piece can be created from a char")
{
    WHEN("an invalid char is given")
    {
        auto p0 = IPiece::fromChar('c');
        auto p1 = IPiece::fromChar(' ');

        THEN("it fails")
        {
            REQUIRE_FALSE(p0);
            REQUIRE_FALSE(p1);
        }
    }

    WHEN("valid chars are given")
    {
        auto black = IPiece::fromChar('b');
        auto white = IPiece::fromChar('w');
        auto king = IPiece::fromChar('k');

        THEN("they can be created")
        {
            REQUIRE(black);
            REQUIRE(white);
            REQUIRE(king);
        }
    }
}
