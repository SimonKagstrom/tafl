#include "BoardHelper.hpp"
#include "tests.hpp"

#include <IBoard.hpp>
#include <map>

using namespace tafl;
using namespace tafl::ut;

namespace
{

const std::string smallBoard = " w b "
                               " w   "
                               " k  b"
                               " b   "
                               "   b ";
} // namespace

SCENARIO("the helper can select moves and pieces")
{
    auto h1 = parse("   ");
    const std::string blackTakenBoard = "    w"
                                        "    b"
                                        " k W."
                                        "     "
                                        "     ";
    auto h2 = parse(blackTakenBoard);

    THEN("invalid strings can't be parsed")
    {
        REQUIRE_FALSE(h1);
    }
    THEN("valid boards are parsed correctly")
    {
        REQUIRE(h2);

        REQUIRE(h2->selected.has_value());
        REQUIRE(h2->move.has_value());
        REQUIRE(h2->selected->getPosition() == Pos {3, 2});
        REQUIRE(*h2->move == Move {h2->selected->getPosition(), {4, 2}});
    }
}

SCENARIO("A board is created from a string")
{
    WHEN("an invalid string is used to create a board")
    {
        auto unaligned = kTablut.substr(0, kTablut.size() - 1);

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

    WHEN("a kTablut board is given")
    {
        auto p = IBoard::fromString(kTablut);

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
            auto pieces = p->getPieces(Color::Black);

            REQUIRE(pieces.size() == 16);

            REQUIRE_FALSE(p->pieceAt({0, 0}).has_value());
            REQUIRE_FALSE(p->pieceAt({0, 1}).has_value());
            REQUIRE_FALSE(p->pieceAt({9, 0}).has_value());
            REQUIRE_FALSE(p->pieceAt({0, 9}).has_value());
            REQUIRE_FALSE(p->pieceAt({9, 9}).has_value());

            REQUIRE(p->pieceAt({3, 0}) == Piece::Type::Black);
            REQUIRE(p->pieceAt({4, 4}) == Piece::Type::King);
            REQUIRE(p->pieceAt({4, 5}) == Piece::Type::White);
            REQUIRE(p->pieceAt({5, 4}) == Piece::Type::White);
        }
    }

    WHEN("a small 5x5 board is given")
    {
        auto p = IBoard::fromString(smallBoard);
        REQUIRE(p);
    }
}

SCENARIO("boards can calculate all possible moves")
{
    auto MF = [](const std::vector<Move>& moves, const Move& needle) {
        return std::count(moves.begin(), moves.end(), needle) == 1;
    };

    GIVEN("a 3x3 board with")
    {
        const std::string blackTakenBoard = "w b"
                                            " k "
                                            " b ";
        auto b = parse(blackTakenBoard);
        REQUIRE(b->board->getTurn() == Color::White);

        THEN("the set of possible moves for white is correct")
        {
            auto p = b->board->getPossibleMoves();

            REQUIRE(p.size() == 6);

            REQUIRE(MF(p, {{0, 0}, {1, 0}}));
            REQUIRE(MF(p, {{0, 0}, {0, 1}}));
            REQUIRE(MF(p, {{0, 0}, {0, 2}}));

            REQUIRE(MF(p, {{1, 1}, {0, 1}}));
            REQUIRE(MF(p, {{1, 1}, {1, 0}}));
            REQUIRE(MF(p, {{1, 1}, {2, 1}}));

            AND_THEN("After a move, the possible moves is updated")
            {
                b->board->move({{0, 0}, {1, 0}});
                REQUIRE(b->board->getTurn() == Color::Black);

                auto p = b->board->getPossibleMoves();

                REQUIRE(p.size() == 4);

                REQUIRE(MF(p, {{2, 0}, {2, 1}}));
                REQUIRE(MF(p, {{2, 0}, {2, 2}}));

                REQUIRE(MF(p, {{1, 2}, {0, 2}}));
                REQUIRE(MF(p, {{1, 2}, {2, 2}}));
            }
        }

        THEN("the set of possible moves for black is correct")
        {
            b->board->setTurn(Color::Black);
            auto p = b->board->getPossibleMoves();

            REQUIRE(p.size() == 5);

            REQUIRE(MF(p, {{2, 0}, {1, 0}}));
            REQUIRE(MF(p, {{2, 0}, {2, 1}}));
            REQUIRE(MF(p, {{2, 0}, {2, 2}}));

            REQUIRE(MF(p, {{1, 2}, {0, 2}}));
            REQUIRE(MF(p, {{1, 2}, {2, 2}}));
        }
    }
}

SCENARIO("pieces can be taken on boards")
{
    THEN("black pieces can be taken")
    {
        const std::string blackTakenBoard = "    w"
                                            "    b"
                                            " k W."
                                            "     "
                                            "     ";
        auto b = parse(blackTakenBoard);

        REQUIRE(b->board->pieceAt({4, 1}));
        b->board->move(*b->move);
        REQUIRE_FALSE(b->board->pieceAt({4, 1}));

        REQUIRE(b->board->getTurn() == Color::Black);
        REQUIRE_FALSE(b->board->getWinner());
    }

    THEN("two pieces can be taken in one round")
    {
        const std::string twoBlackTakenBoard = "    w"
                                               "    b"
                                               " k W."
                                               "    b"
                                               "    w";
        auto b = parse(twoBlackTakenBoard);

        REQUIRE(b->board->pieceAt({4, 1}));
        REQUIRE(b->board->pieceAt({4, 3}));
        b->board->move(*b->move);
        REQUIRE_FALSE(b->board->pieceAt({4, 1}));
        REQUIRE_FALSE(b->board->pieceAt({4, 3}));

        REQUIRE(b->board->getTurn() == Color::Black);
        REQUIRE_FALSE(b->board->getWinner());
    }

    THEN("white pieces can be taken")
    {
        const std::string whiteTakenBoard = "bw. B"
                                            "     "
                                            "  k  "
                                            "     "
                                            "     ";

        auto b = parse(whiteTakenBoard);
        b->board->setTurn(Color::Black);

        REQUIRE(b->board->pieceAt({1, 0}));
        b->board->move(*b->move);
        REQUIRE_FALSE(b->board->pieceAt({1, 0}));

        REQUIRE(b->board->getTurn() == Color::White);
        REQUIRE_FALSE(b->board->getWinner());
    }

    THEN("the king pieces can be taken outside of the castle")
    {
        const std::string kingTakenBoard = "     "
                                           "bk. B"
                                           "     "
                                           "     "
                                           "     ";

        auto b = parse(kingTakenBoard);
        b->board->setTurn(Color::Black);

        REQUIRE(b->board->pieceAt({1, 1}));
        b->board->move(*b->move);
        REQUIRE_FALSE(b->board->pieceAt({1, 1}));
        REQUIRE(b->board->getTurn() == Color::White);
        REQUIRE(b->board->getWinner() == Color::Black);
    }

    THEN("the king pieces can be taken in the castle, with 4 enemies")
    {
        const std::string kingInCastleBoard = "b    "
                                              "  b  "
                                              " bk.B"
                                              "  b  "
                                              "w    ";

        auto b = parse(kingInCastleBoard);
        b->board->setTurn(Color::Black);

        // First move non-affecting pieces to trigger a scan so that the king isn't taken
        // with only 3 neighbors
        b->board->move({{0, 0}, {1, 0}});
        REQUIRE(b->board->getTurn() == Color::White);
        b->board->move({{0, 4}, {1, 4}});
        REQUIRE(b->board->getTurn() == Color::Black);

        REQUIRE(b->board->pieceAt({2, 2}));
        b->board->move(*b->move);
        REQUIRE_FALSE(b->board->pieceAt({2, 2}));
        REQUIRE(b->board->getWinner() == Color::Black);
    }
}

SCENARIO("the winner of a board can be evaluated")
{
    THEN("a board without a winner has no winner")
    {
        const std::string noWinner = "bw  b"
                                     "     "
                                     "  k  "
                                     "     "
                                     "     ";
        auto b = parse(noWinner);

        REQUIRE(b->board->getWinner() == std::nullopt);
    }

    THEN("a board without a king has black as the winner")
    {
        const std::string blackWinner = "bw  b"
                                        "     "
                                        "     "
                                        "     "
                                        "     ";
        auto b = parse(blackWinner);

        REQUIRE(b->board->getWinner() == Color::Black);
    }

    THEN("white wins if the king is at the edge of the board")
    {
        const std::string kingEdge1 = "b   b"
                                      "     "
                                      "k    "
                                      "     "
                                      "     ";
        const std::string kingEdge2 = "b   b"
                                      "    k"
                                      "     "
                                      "     "
                                      "     ";
        const std::string kingEdge3 = "bk  b"
                                      "     "
                                      "     "
                                      "     "
                                      "     ";
        const std::string kingEdge4 = "b   b"
                                      "     "
                                      "     "
                                      "     "
                                      "   k ";
        auto b1 = parse(kingEdge1);
        auto b2 = parse(kingEdge2);
        auto b3 = parse(kingEdge3);
        auto b4 = parse(kingEdge4);

        REQUIRE(b1->board->getWinner() == Color::White);
        REQUIRE(b2->board->getWinner() == Color::White);
        REQUIRE(b3->board->getWinner() == Color::White);
        REQUIRE(b4->board->getWinner() == Color::White);
    }
}
