#include <Piece.hpp>

#include "tests.hpp"

using namespace tafl;

SCENARIO("a piece can be created from a char")
{
    WHEN("an invalid char is given")
    {
        auto p0 = Piece::fromChar('c');
        auto p1 = Piece::fromChar(' ');

        THEN("it fails")
        {
            REQUIRE_FALSE(p0);
            REQUIRE_FALSE(p1);
        }
    }

    WHEN("valid chars are given")
    {
        auto black = Piece::fromChar('b');
        auto white = Piece::fromChar('w');
        auto king = Piece::fromChar('k');

        THEN("they can be created")
        {
            REQUIRE(black);
            REQUIRE(white);
            REQUIRE(king);
        }

        AND_THEN("they have the correct color and type")
        {
            REQUIRE(white->getType() == Piece::Type::White);
            REQUIRE(black->getType() == Piece::Type::Black);
            REQUIRE(king->getType() == Piece::Type::King);

            REQUIRE(white->getColor() == Color::White);
            REQUIRE(black->getColor() == Color::Black);
            REQUIRE(king->getColor() ==Color::White);
        }
    }
}
