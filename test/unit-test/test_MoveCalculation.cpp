#include <IBoard.hpp>

#include "tests.hpp"
#include "BoardHelper.hpp"

using namespace tafl;
using namespace tafl::ut;

using namespace std::chrono_literals;

SCENARIO("a board can calculate the best moves")
{
    auto doPlay = [](IBoard &board)
    {
        auto f = board.calculateBestMove(100ms, [](){});
        f.wait();
        auto m = f.get();
        REQUIRE(m);

        board.move(*m);

        return board.getWinner();
    };

    THEN("a board without pieces immediately return a future without moves")
    {
        const std::string noWinner =
            "     "
            "     "
            "     "
            "     "
            "     ";
        auto b = parse(noWinner);

        auto f = b->board->calculateBestMove(100ms, [](){});
        REQUIRE(f.valid());
        REQUIRE(f.get() == std::nullopt);
    }

    WHEN("white can win in one move")
    {
        const std::string whiteInOne =
            " w b "
            " wb  "
            " k  b"
            "bb   "
            "   b ";
        auto b = parse(whiteInOne);
        b->board->setTurn(Color::White);

        REQUIRE(b->board->pieceAt({1,0}));

        REQUIRE_FALSE(b->board->getWinner());

        auto f = b->board->calculateBestMove(100ms, [](){});
        THEN("a calculation must be done")
        {
//            REQUIRE_FALSE(f.valid());
        }

        AND_THEN("the calculation returns such a move")
        {
            f.wait();
            auto move = f.get();
            REQUIRE(move != std::nullopt);

            b->board->move(*move);
            REQUIRE(b->board->getWinner() == Color::White);
        }
    }

    WHEN("black can win in two black moves")
    {
        const std::string blackInTwo =
            "bw b "
            "bk   "
            "b   w"
            " b   "
            "     ";
        auto b = parse(blackInTwo);
        b->board->setTurn(Color::Black);

        REQUIRE_FALSE(b->board->getWinner());

        auto w0 = doPlay(*b->board);
        auto w1 = doPlay(*b->board);
        auto w2 = doPlay(*b->board);

        AND_THEN("the calculation returns such a move")
        {
            REQUIRE_FALSE(w0);
            REQUIRE_FALSE(w1);
            REQUIRE(w2 == Color::Black);
        }
    }
}
