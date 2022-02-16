#include <IMoveTrait.hpp>

#include "mocks/MockBoard.hpp"
#include "tests.hpp"

#include <set>

using namespace tafl;

namespace
{

std::set<Pos> toPosSet(const std::vector<Move> &moves)
{
    std::set<Pos> out;
    for (auto x : moves)
    {
        out.insert(x.to);
    }

    return out;
}

}

SCENARIO("Moves can be caluclated correctly")
{
    // A 3x3 board
    auto mockBoard = mocks::MockBoard::createMock(3);

    auto x = IMoveTrait::create();

    auto pMiddle = Piece::fromChar('w');
    auto pTopLeft = Piece::fromChar('b');
    auto pMiddleRight = Piece::fromChar('k');

    pMiddle->place({1,1});
    pTopLeft->place({0,0});
    pMiddleRight->place({2,1});

    GIVEN("an empty board")
    {
        // Created above
        THEN("a piece in the middle can move horizontally and vertically")
        {
            auto posSet = toPosSet(x->getMoves(*mockBoard, *pMiddle));

            REQUIRE(posSet.size() == 4);
            REQUIRE(posSet.find({1,0}) != posSet.end());
            REQUIRE(posSet.find({1,2}) != posSet.end());
            REQUIRE(posSet.find({0,1}) != posSet.end());
            REQUIRE(posSet.find({2,1}) != posSet.end());
        }

        THEN("the top left piece can move down and right")
        {
            auto posSet = toPosSet(x->getMoves(*mockBoard, *pTopLeft));

            REQUIRE(posSet.size() == 4);
            REQUIRE(posSet.find({1,0}) != posSet.end());
            REQUIRE(posSet.find({2,0}) != posSet.end());
            REQUIRE(posSet.find({0,1}) != posSet.end());
            REQUIRE(posSet.find({0,2}) != posSet.end());
        }

        THEN("the middle right piece can't move to the center")
        {
            auto posSet = toPosSet(x->getMoves(*mockBoard, *pMiddleRight));

            REQUIRE(posSet.size() == 2);
            REQUIRE(posSet.find({2,0}) != posSet.end());
            REQUIRE(posSet.find({2,2}) != posSet.end());
        }
    }

    GIVEN("other pieces in all directions")
    {
        REQUIRE_CALL(*mockBoard, pieceAt(Pos{1,0})).RETURN(Piece::Type::White).TIMES(AT_LEAST(1));
        REQUIRE_CALL(*mockBoard, pieceAt(Pos{1,2})).RETURN(Piece::Type::White).TIMES(AT_LEAST(1));
        REQUIRE_CALL(*mockBoard, pieceAt(Pos{0,1})).RETURN(Piece::Type::White).TIMES(AT_LEAST(1));
        REQUIRE_CALL(*mockBoard, pieceAt(Pos{2,1})).RETURN(Piece::Type::White).TIMES(AT_LEAST(1));

        THEN("The middle pos can't move")
        {
            auto posSet = toPosSet(x->getMoves(*mockBoard, *pMiddle));

            REQUIRE(posSet.size() == 0);
        }
    }

    GIVEN("other pieces up and right")
    {
        REQUIRE_CALL(*mockBoard, pieceAt(Pos{1,0})).RETURN(Piece::Type::White).TIMES(AT_LEAST(1));
        REQUIRE_CALL(*mockBoard, pieceAt(Pos{1,2})).RETURN(Piece::Type::White).TIMES(AT_LEAST(1));

        THEN("The middle pos can move left and down")
        {
            auto posSet = toPosSet(x->getMoves(*mockBoard, *pMiddle));

            REQUIRE(posSet.size() == 2);
            REQUIRE(posSet.find({0,1}) != posSet.end());
            REQUIRE(posSet.find({2,1}) != posSet.end());
        }
    }
}