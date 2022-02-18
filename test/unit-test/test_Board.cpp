#include <IBoard.hpp>

#include "tests.hpp"

#include <map>

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
    "   bbb   "
;

const std::string smallBoard =
    " w b "
    " w   "
    " k  b"
    " b   "
    "   b "
;

const std::string kingTakenBoard =
    "     "
    "bk. B"
    "     "
    "     "
    "     "
;

const std::string kingInCastleBoard =
    "b    "
    "  b  "
    " bk.B"
    "  b  "
    "w    "
;

const std::string blackTakenBoard =
    "    w"
    "    b"
    " k W."
    "     "
    "     "
;

const std::string whiteTakenBoard =
    "bw. B"
    "     "
    "  k  "
    "     "
    "     "
;

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

SCENARIO("the helper can select moves and pieces")
{
    auto h1 = parse("   ");
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
        REQUIRE(h2->selected->getPosition() == Pos{3,2});
        REQUIRE(*h2->move == Move{h2->selected->getPosition(), {4,2}});
    }
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
            auto pieces = p->getPieces(Color::Black);

            REQUIRE(pieces.size() == 16);

            REQUIRE_FALSE(p->pieceAt({0,0}).has_value());
            REQUIRE_FALSE(p->pieceAt({0,1}).has_value());
            REQUIRE_FALSE(p->pieceAt({9,0}).has_value());
            REQUIRE_FALSE(p->pieceAt({0,9}).has_value());
            REQUIRE_FALSE(p->pieceAt({9,9}).has_value());

            REQUIRE(p->pieceAt({3,0}) == Piece::Type::Black);
            REQUIRE(p->pieceAt({4,4}) == Piece::Type::King);
            REQUIRE(p->pieceAt({4,5}) == Piece::Type::White);
            REQUIRE(p->pieceAt({5,4}) == Piece::Type::White);
        }
    }

    WHEN("a small 5x5 board is given")
    {
        auto p = IBoard::fromString(smallBoard);
        REQUIRE(p);
    }
}

SCENARIO("pieces can be taken on boards")
{
    THEN("black pieces can be taken")
    {
        auto b = parse(blackTakenBoard);

        REQUIRE(b->board->pieceAt({4,1}));
        b->board->move(*b->move);
        REQUIRE_FALSE(b->board->pieceAt({4,1}));

        REQUIRE(b->board->getTurn() == Color::Black);
    }

    THEN("the king pieces can be taken outside of the castle")
    {
        auto b = parse(kingTakenBoard);
        b->board->setTurn(Color::Black);

        REQUIRE(b->board->pieceAt({1,1}));
        b->board->move(*b->move);
        REQUIRE_FALSE(b->board->pieceAt({1,1}));
        REQUIRE(b->board->getTurn() == Color::White);
    }

    THEN("the king pieces can be taken in the castle, with 4 enemies")
    {
        auto b = parse(kingInCastleBoard);
        b->board->setTurn(Color::Black);

        // First move non-affecting pieces to trigger a scan so that the king isn't taken
        // with only 3 neighbors
        b->board->move({{0,0}, {1,0}});
        REQUIRE(b->board->getTurn() == Color::White);
        b->board->move({{0,4}, {1,4}});
        REQUIRE(b->board->getTurn() == Color::Black);

        REQUIRE(b->board->pieceAt({2,2}));
        b->board->move(*b->move);
        REQUIRE_FALSE(b->board->pieceAt({2,2}));
    }
}
