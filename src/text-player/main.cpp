#include <IBoard.hpp>
#include <fmt/format.h>

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

using namespace tafl;

namespace
{

/*
 * Chess-style notation: files a.. from the left, ranks 1.. from the bottom.
 */
std::string
toSquare(const Pos& pos, unsigned dim)
{
    return fmt::format("{}{}", static_cast<char>('a' + pos.x), dim - pos.y);
}

// Parse all squares (e.g., "e4", "a10") in a line, ignoring separators
std::vector<Pos>
parseSquares(const std::string& line, unsigned dim)
{
    std::vector<Pos> out;

    for (auto i = 0u; i < line.size(); i++)
    {
        auto file = std::tolower(line[i]);
        if (file < 'a' || file >= static_cast<int>('a' + dim))
        {
            continue;
        }

        unsigned rank = 0;
        auto j = i + 1;
        while (j < line.size() && std::isdigit(line[j]))
        {
            rank = rank * 10 + (line[j] - '0');
            j++;
        }
        if (j == i + 1 || rank < 1 || rank > dim)
        {
            continue;
        }

        out.push_back({static_cast<unsigned>(file - 'a'), dim - rank});
        i = j - 1;
    }

    return out;
}

void
printBoard(const IBoard& board)
{
    const auto dim = board.getBoardDimension();

    for (auto y = 0u; y < dim; y++)
    {
        fmt::print("{:>2}  ", dim - y);
        for (auto x = 0u; x < dim; x++)
        {
            auto p = board.pieceAt({x, y});
            fmt::print("{} ", p ? Piece::toChar(*p) : '.');
        }
        fmt::print("\n");
    }

    fmt::print("\n    ");
    for (auto x = 0u; x < dim; x++)
    {
        fmt::print("{} ", static_cast<char>('a' + x));
    }
    fmt::print("\n\n");
}

const char*
colorName(Color c)
{
    return c == Color::Black ? "Black" : "White";
}

void
printHelp()
{
    fmt::print("Commands:\n"
               "  e5 h5   move the piece at e5 to h5 (also e5-h5, e5h5)\n"
               "  e5      list legal moves for the piece at e5\n"
               "  q       quit\n");
}

} // namespace

int
main(int argc, const char* argv[])
{
    auto human = (argc > 1 && std::string(argv[1]) == "black") ? Color::Black : Color::White;
    auto quota = std::chrono::milliseconds(argc > 2 ? std::stoi(argv[2]) : 1000);

    auto board = IBoard::fromString(kTablut);
    const auto dim = board->getBoardDimension();

    fmt::print("You play {}. Type 'h' for help.\n\n", colorName(human));

    while (!board->getWinner())
    {
        printBoard(*board);

        if (board->getTurn() != human)
        {
            auto best = board->calculateBestMove(quota, []() {}).get();
            if (!best)
            {
                fmt::print("{} has no moves\n", colorName(board->getTurn()));
                break;
            }

            fmt::print("{} plays {} {}\n\n",
                       colorName(board->getTurn()),
                       toSquare(best->from, dim),
                       toSquare(best->to, dim));
            board->move(*best);
            continue;
        }

        auto legal = board->getPossibleMoves();
        if (legal.empty())
        {
            fmt::print("{} has no moves\n", colorName(human));
            break;
        }

        // Read until a legal move has been entered
        while (true)
        {
            fmt::print("{}> ", colorName(human));

            std::string line;
            if (!std::getline(std::cin, line) || line == "q")
            {
                return 0;
            }

            auto squares = parseSquares(line, dim);
            if (squares.size() == 1)
            {
                fmt::print(" ");
                for (const auto& m : legal)
                {
                    if (m.from == squares[0])
                    {
                        fmt::print(" {}", toSquare(m.to, dim));
                    }
                }
                fmt::print("\n");
                continue;
            }
            if (squares.size() != 2)
            {
                printHelp();
                continue;
            }

            Move m {squares[0], squares[1]};
            if (std::ranges::find(legal, m) == legal.end())
            {
                fmt::print("Illegal move\n");
                continue;
            }

            board->move(m);
            fmt::print("\n");
            break;
        }
    }

    if (auto winner = board->getWinner())
    {
        printBoard(*board);
        fmt::print("Winner: {}\n", colorName(*winner));
    }

    return 0;
}
