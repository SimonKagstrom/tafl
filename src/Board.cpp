
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
    unsigned index = 0;
    for (auto& p : pieces)
    {
        m_pieceStorage[index] = *p;
        m_pieces.push_back(&m_pieceStorage[index]);
        m_board[p->getPosition().flatten(m_dimensions)] = &m_pieceStorage[index];
        index++;
    }
}

Board::Board(const Board& other)
    : m_dimensions(other.m_dimensions)
    , m_turn(other.m_turn)
    , m_moveTrait(IMoveTrait::create())
{
    unsigned index = 0;
    for (auto& p : other.m_pieces)
    {
        m_pieceStorage[index] = *p;
        m_pieces.push_back(&m_pieceStorage[index]);
        m_board[p->getPosition().flatten(m_dimensions)] = &m_pieceStorage[index];
        index++;
    }
}

unsigned
Board::getBoardDimension() const
{
    return m_dimensions;
}


std::optional<Piece::Type>
Board::pieceAt(const Pos& pos) const
{
    auto piece = m_board[pos.flatten(m_dimensions)];

    if (piece)
    {
        return piece->getType();
    }

    return std::nullopt;
}

std::vector<Piece>
Board::getPieces(const Color& which) const
{
    std::vector<Piece> out;

    for (auto& piece : m_pieces)
    {
        if (piece->getColor() == which)
        {
            out.push_back(*piece);
        }
    }

    return out;
}

void
Board::move(Move move)
{
    auto src = move.from.flatten(m_dimensions);
    auto dst = move.to.flatten(m_dimensions);
    auto p = m_board[src];

    if (!p)
    {
        assert(false && "No piece at from");
    }

    if (getTurn() != p->getColor())
    {
        assert(false && "Turn wrong");
    }

    if (m_board[dst])
    {
        assert(false && "piece at destination");
    }

    p->place(move.to);
    m_board[dst] = p;
    m_board[src] = nullptr;

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
    auto itKing = std::find_if(m_pieces.begin(), m_pieces.end(), [](const auto& cur) {
        return cur->getType() == Piece::Type::King;
    });

    if (itKing == m_pieces.end())
    {
        // The king is gone
        return Color::Black;
    }
    const auto dim = getBoardDimension();
    auto kingPos = (*itKing)->getPosition();

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
    const auto nThreads = 4u;

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

            std::ranges::sort(results, [](const MoveAndResults& a, const MoveAndResults& b) {
                return a.results.blackWins / a.results.whiteWins >
                       b.results.blackWins / b.results.whiteWins;
            });

            for (auto x : results)
            {
                auto f = x.move.from;
                auto t = x.move.to;

                fmt::print("{}:{} -> {}:{}, leads to {} black wins ({:.3f}:{:.3f} of {})\n",
                           f.x,
                           f.y,
                           t.x,
                           t.y,
                           x.results.blackWins / x.results.whiteWins,
                           x.results.blackWins,
                           x.results.whiteWins,
                           x.results.samples);
            }

            if (black)
            {
                out = results.front().move;
            }
            else
            {
                out = results.back().move;
            }

            return out;
        });
}

void
Board::scanCaptures()
{
    etl::vector<unsigned, 18 * 18> capture_indices;

    auto dim = getBoardDimension();

    for (auto index = 0u; index < m_pieces.size(); index++)
    {
        auto piece = m_pieces[index];

        if (piece->getColor() == m_turn)
        {
            continue;
        }
        const auto pos = piece->getPosition();

        auto above = pieceColorAt(pos.above());
        auto below = pieceColorAt(pos.below());
        auto right = pieceColorAt(pos.right());
        auto left = pieceColorAt(pos.left());

        auto verticalEnemies =
            above && below && *above != piece->getColor() && *below != piece->getColor();
        auto horizontalEnemies =
            left && right && *left != piece->getColor() && *right != piece->getColor();

        if (piece->getType() == Piece::Type::King && pos == Pos {dim / 2, dim / 2})
        {
            // The king in the castle - all 4 sides must be occupied
            if (verticalEnemies && horizontalEnemies)
            {
                capture_indices.push_back(index);
            }
        }
        else
        {
            if (verticalEnemies || horizontalEnemies)
            {
                capture_indices.push_back(index);
            }
        }
    }

    for (auto& idx : capture_indices)
    {
        m_board[m_pieces[idx]->getPosition().flatten(dim)] = nullptr;
    }

    // Iterate in reverse order over the captured indicces
    for (auto i = capture_indices.size(); i > 0; i--)
    {
        auto toErase = m_pieces.begin() + capture_indices[i - 1];
        m_pieces.erase(toErase);
    }
}

std::optional<Color>
Board::pieceColorAt(const Pos& pos) const
{
    if (pos.x >= m_dimensions || pos.y >= m_dimensions)
    {
        return std::nullopt;
    }

    auto p = m_board[pos.flatten(m_dimensions)];
    if (p)
    {
        return p->getColor();
    }

    return std::nullopt;
}

void Board::fillPossibleMoves()
{
    m_possibleMoves.clear();

    for (auto& piece : m_pieces)
    {
        if (piece->getColor() != m_turn)
        {
            continue;
        }

        auto it = m_moveTrait->begin(*this, *piece);
        while (it)
        {
            m_possibleMoves.push_back(it->move);
            it = m_moveTrait->next(*it, *this, *piece);
        }
    }
}

std::vector<Move>
Board::getPossibleMoves() const
{
    std::vector<Move> possibleMoves;

    for (auto& piece : m_pieces)
    {
        if (piece->getColor() != m_turn)
        {
            continue;
        }

        auto it = m_moveTrait->begin(*this, *piece);
        while (it)
        {
            possibleMoves.push_back(it->move);
            it = m_moveTrait->next(*it, *this, *piece);
        }
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

        fillPossibleMoves();
        if (m_possibleMoves.empty())
        {
            // Impossible, but anyway
            return Board::PlayResult();
        }

        auto selected = rand() % m_possibleMoves.size();

        auto m = m_possibleMoves[selected];
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