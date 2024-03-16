
#include "Board.hpp"

#include <IBoard.hpp>
#include <cassert>
#include <cmath>
#include <fmt/format.h>
#include <future>
#include <map>
#include <ranges>
#include <set>
#include <vector>

using namespace tafl;

Board::Board(unsigned dimensions, std::vector<std::unique_ptr<Piece>>& pieces)
    : m_dimensions(dimensions)
    , m_moveTrait(IMoveTrait::create())
{
    for (auto& p : pieces)
    {
        m_pieces.emplace(p->getPosition(), *p);
    }
}

Board::Board(const Board& other)
    : m_dimensions(other.m_dimensions)
    , m_turn(other.m_turn)
    , m_pieces(other.m_pieces)
    , m_moveTrait(IMoveTrait::create())
{
}

unsigned
Board::getBoardDimension() const
{
    return m_dimensions;
}


std::optional<Piece::Type>
Board::pieceAt(const Pos& pos) const
{
    auto it = m_pieces.find(pos);

    if (it == m_pieces.end())
    {
        return {};
    }

    return it->second.getType();
}

std::vector<Piece>
Board::getPieces(const Color& which) const
{
    std::vector<Piece> out;
    std::ranges::copy(m_pieces | std::views::values | std::views::filter([which](const Piece& cur) {
                          return cur.getColor() == which;
                      }),
                      std::back_inserter(out));

    return out;
}

void
Board::move(Move move)
{
    if (!pieceAt(move.from))
    {
        assert(false && "No piece at from");
    }
    auto p = m_pieces.find(move.from)->second;

    if (getTurn() != p.getColor())
    {
        assert(false && "Turn wrong");
    }

    if (m_pieces.find(move.to) != m_pieces.end())
    {
        assert(false && "piece at destination");
    }

    p.place(move.to);
    m_pieces.erase(move.from);
    m_pieces.emplace(move.to, p);

    scanCaptures();

    setTurn(!m_turn);
}

Color
Board::getTurn() const
{
    return m_turn;
}

void
Board::setTurn(Color which)
{
    m_turn = which;
}

std::optional<Color>
Board::getWinner() const
{
    auto itKing =
        std::find_if(m_pieces.begin(), m_pieces.end(), [](const std::pair<Pos, Piece>& cur) {
            return cur.second.getType() == Piece::Type::King;
        });

    if (itKing == m_pieces.end())
    {
        // The king is gone
        return Color::Black;
    }
    const auto dim = getBoardDimension();
    auto kingPos = itKing->first;

    if (kingPos.x == 0 || kingPos.x == dim - 1 || kingPos.y == 0 || kingPos.y == dim - 1)
    {
        return Color::White;
    }

    return std::nullopt;
}

std::future<std::optional<Move>>
Board::calculateBestMove(const std::chrono::milliseconds& quota,
                         std::function<void()> onFutureReady)
{
    const auto nThreads = 8u;

    auto possibleMoves = getPossibleMoves();

    std::promise<std::optional<Move>> p;

    if (possibleMoves.empty())
    {
        p.set_value(std::nullopt);
        return p.get_future();
    }

    std::vector<std::future<std::vector<MoveAndResults>>> threadFutures;
    for (auto thr = 0u; thr < nThreads; thr++)
    {
        threadFutures.push_back(runSimulationInThread(quota, possibleMoves));
    }
    auto black = m_turn == Color::Black;

    const auto number_of_moves = possibleMoves.size();
    return std::async(
        std::launch::async,
        [black, number_of_moves, threadFutures = std::move(threadFutures)]() mutable {
            std::optional<Move> out;
            std::vector<MoveAndResults> results;
            results.resize(number_of_moves);

            for (auto& f : threadFutures)
            {
                f.wait();
                auto r = f.get();

                for (auto i = 0; i < number_of_moves; i++)
                {
                    results[i].move = r[i].move;
                    // Only care about valid values
                    if (r[i].results.samples)
                    {
                        results[i].results = results[i].results + r[i].results;
                    }
                }
            }

            std::ranges::sort(results, [black](const MoveAndResults& a, const MoveAndResults& b) {
                return black ? a.results.blackWins < b.results.blackWins
                             : a.results.whiteWins < b.results.whiteWins;
            });

            for (auto x : results)
            {
                auto f = x.move.from;
                auto t = x.move.to;

                fmt::print("{}:{} -> {}:{}, leads to {:.1f}% black wins ({:.3f}:{:.3f} of {})\n",
                           f.x,
                           f.y,
                           t.x,
                           t.y,
                           x.results.blackWins / x.results.samples * 100.0f,
                           x.results.blackWins,
                           x.results.whiteWins,
                           x.results.samples);
            }

            out = results.back().move;

            return out;
        });
}

void
Board::scanCaptures()
{
    std::set<Pos> captures;

    auto dim = getBoardDimension();

    for (auto& [pos, piece] : m_pieces)
    {
        if (piece.getColor() == m_turn)
        {
            continue;
        }

        auto above = pieceColorAt(pos.above());
        auto below = pieceColorAt(pos.below());
        auto right = pieceColorAt(pos.right());
        auto left = pieceColorAt(pos.left());

        auto verticalEnemies =
            above && below && *above != piece.getColor() && *below != piece.getColor();
        auto horizontalEnemies =
            left && right && *left != piece.getColor() && *right != piece.getColor();

        if (piece.getType() == Piece::Type::King && pos == Pos {dim / 2, dim / 2})
        {
            // The king in the castle - all 4 sides must be occupied
            if (verticalEnemies && horizontalEnemies)
            {
                captures.insert(pos);
            }
        }
        else
        {
            if (verticalEnemies || horizontalEnemies)
            {
                captures.insert(pos);
            }
        }
    }

    for (auto& toErase : captures)
    {
        m_pieces.erase(toErase);
    }
}

std::optional<Color>
Board::pieceColorAt(const Pos& pos) const
{
    auto it = m_pieces.find(pos);

    if (it == m_pieces.end())
    {
        return {};
    }

    return it->second.getColor();
}

std::vector<Move>
Board::getPossibleMoves() const
{
    std::vector<Move> possibleMoves;

    for (auto& [pos, piece] : m_pieces)
    {
        if (piece.getColor() != m_turn)
        {
            continue;
        }

        auto cur = m_moveTrait->getMoves(*this, piece);
        std::copy(cur.begin(), cur.end(), std::back_inserter(possibleMoves));
    }

    return possibleMoves;
}

std::future<std::vector<Board::MoveAndResults>>
Board::runSimulationInThread(const std::chrono::milliseconds& quota,
                             std::span<const Move> movesToSimulate)
{
    auto bIn = Board(*this);

    auto moves = std::vector<Move>(movesToSimulate.begin(), movesToSimulate.end());

    return std::async(std::launch::async, [bIn, moves, quota] {
        // Create an output vector
        auto out = std::vector<Board::MoveAndResults>();
        out.reserve(moves.size());

        auto v = std::views::transform(
            moves, [](auto&& move) { return Board::MoveAndResults {move, Board::PlayResult()}; });
        std::ranges::copy(v, std::back_inserter(out));

        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start < quota)
        {
            for (auto& cur : out)
            {
                auto b = bIn;
                b.move(cur.move);
                cur.results = cur.results + b.simulate(1);
            }
        }

        return out;
    });
}

Board::PlayResult
Board::simulate(unsigned ply)
{
    while (true)
    {
        auto winner = getWinner();

        if (winner)
        {
            return Board::PlayResult(*winner, ply);
        }

        auto possibleMoves = getPossibleMoves();
        if (possibleMoves.empty())
        {
            // Impossible, but anyway
            return Board::PlayResult();
        }

        auto selected = rand() % possibleMoves.size();

        auto m = possibleMoves[selected];
        move(m);
        ply++;
    }
}


std::unique_ptr<IBoard>
IBoard::fromString(const std::string_view& s)
{
    if (s.size() < 2)
    {
        // Too small to play on!
        return nullptr;
    }


    auto d = ::sqrt(s.size());
    auto f = ::floor(d);

    if (f != d)
    {
        // Must be square, e.g., 9 * 9, 11 * 11 etc
        return nullptr;
    }

    auto dimension = static_cast<unsigned>(f);

    std::vector<std::unique_ptr<Piece>> pieces;

    for (auto i = 0u; i < dimension * dimension; i++)
    {
        auto x = i % dimension;
        auto y = i / dimension;
        auto c = s[i];

        auto p = Piece::fromChar(c);
        if (p)
        {
            p->place({x, y});
            pieces.push_back(std::move(p));
        }
    }

    return std::make_unique<Board>(dimension, pieces);
}

void
IBoard::printBoard(const IBoard& board)
{
    const auto dim = board.getBoardDimension();
    fmt::print("   ");
    for (auto x = 0u; x < dim; x++)
    {
        fmt::print("{} ", x);
    }
    fmt::print("\n");

    for (auto y = 0u; y < dim; y++)
    {
        fmt::print("{}  ", y);
        for (auto x = 0u; x < dim; x++)
        {
            auto p = board.pieceAt({x, y});
            if (p)
            {
                fmt::print("{} ", Piece::toChar(*p));
            }
            else
            {
                fmt::print("  ");
            }
        }
        fmt::print("\n");
    }

    constexpr auto black_at_start = 16;
    constexpr auto white_at_start = 9;

    auto black = board.getPieces(Color::Black);
    auto white = board.getPieces(Color::White);

    fmt::print("\n\nTaken pieces: ");
    for (auto i = 0u; i < black_at_start - black.size(); i++)
    {
        fmt::print("b");
    }
    fmt::print(" ");
    for (auto i = 0u; i < white_at_start - white.size(); i++)
    {
        fmt::print("w");
    }
    fmt::print("\n");
}