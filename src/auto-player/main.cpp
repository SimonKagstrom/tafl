#include <IBoard.hpp>
#include <fmt/format.h>

using namespace tafl;
using namespace std::chrono_literals;

int
main(int argc, const char* argv[])
{
    auto board = IBoard::fromString(kTablut);
    const auto dim = board->getBoardDimension();

    fmt::print("\033[H\033[2J");
    fmt::print("\n");
    auto winner = board->getWinner();
    do
    {
        IBoard::printBoard(*board);
        auto best = board->calculateBestMove(1s, []() {});
        best.wait();

        if (best.valid())
        {
            auto m = best.get().value();
            auto f = m.from;
            auto t = m.to;

            // Clear the screen
//            fmt::print("\033[H\033[2J");
            fmt::print("Best move for {}: {}:{} -> {}:{}\n", board->getTurn() == Color::Black ? "Black" : "White",
             f.x, f.y, t.x, t.y);

            board->move(m);
        }
        else
        {
            break;
        }
        winner = board->getWinner();
    } while (winner == std::nullopt);

    if (winner)
    {
        fmt::print("Winner: {}\n", *winner == Color::Black ? "Black" : "White");
        IBoard::printBoard(*board);
    }

    return 0;
}
