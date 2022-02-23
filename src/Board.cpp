
#include <IBoard.hpp>
#include <cmath>
#include <vector>
#include <map>
#include <set>
#include <future>

#include "Board.hpp"

using namespace tafl;

Board::Board(unsigned dimensions, std::vector<std::unique_ptr<Piece>> &pieces) :
    m_dimensions(dimensions),
    m_moveTrait(IMoveTrait::create())
{
    for (auto &p : pieces)
    {
        m_pieces.emplace(p->getPosition(), *p);
    }
}

Board::Board(const Board &other) :
    m_dimensions(other.m_dimensions),
    m_turn(other.m_turn),
    m_pieces(other.m_pieces),
    m_moveTrait(IMoveTrait::create())
{
}

unsigned Board::getBoardDimension() const
{
    return m_dimensions;
}


std::optional<Piece::Type> Board::pieceAt(const Pos &pos) const
{
    auto it = m_pieces.find(pos);

    if (it == m_pieces.end())
    {
        return {};
    }

    return it->second.getType();
}

const std::vector<Piece> Board::getPieces(const Color &which) const
{
    std::vector<Piece> out;

    for (auto &[k, v] : m_pieces)
    {
        if (v.getColor() == which)
        {
            out.push_back(v);
        }
    }

    return out;
}

void Board::move(Move move)
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

Color Board::getTurn() const
{
    return m_turn;
}

void Board::setTurn(Color which)
{
    m_turn = which;
}

std::optional<Color> Board::getWinner() const
{
    auto itKing = std::find_if(m_pieces.begin(), m_pieces.end(), [](const std::pair<Pos, Piece> &cur)
    {
        return cur.second.getType() == Piece::Type::King;
    });

    if (itKing == m_pieces.end())
    {
        // The king is gone
        return Color::Black;
    }
    const auto dim = getBoardDimension();
    auto kingPos = itKing->first;

    if (kingPos.x == 0 || kingPos.x == dim - 1 ||
        kingPos.y == 0 || kingPos.y == dim - 1)
    {
        return Color::White;
    }

    return std::nullopt;
}

std::future<std::optional<Move>> Board::calculateBestMove(const std::chrono::milliseconds &quota,
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

    auto nPerThread = possibleMoves.size() / nThreads;

    std::vector<std::future<std::vector<MoveAndResults>>> threadFutures;
    for (auto thr = 0u; thr < nThreads; thr++)
    {
        std::vector<Move> threadMoves(possibleMoves.begin() + thr * nPerThread,
            thr == nThreads - 1 ? possibleMoves.end() :
            possibleMoves.begin() + (thr + 1) * nPerThread);

        if (!threadMoves.empty())
        {
            threadFutures.push_back(runSimulationInThread(quota, threadMoves));
        }
    }
    auto black = m_turn == Color::Black;

    return std::async(std::launch::async, [black, threadFutures = std::move(threadFutures)]() mutable
    {
        std::optional<Move> out;
        std::vector<MoveAndResults> results;

        for (auto &f : threadFutures)
        {
            f.wait();
            auto r = f.get();

            for (auto &cur : r)
            {
                // Only care about valid values
                if (cur.results.samples)
                {
                    results.push_back(cur);
                }
            }
        }

        std::sort(results.begin(), results.end(), [black](const MoveAndResults &a, const MoveAndResults &b)
        {
            if (black)
            {
                return float(a.results.blackWins) / a.results.samples > float(b.results.blackWins) / b.results.samples;
            }
            return float(a.results.whiteWins) / a.results.samples > float(b.results.whiteWins) / b.results.samples;
        });

        out = results[0].move;

        return out;
    });
}

void Board::scanCaptures()
{
    std::set<Pos> captures;

    auto dim = getBoardDimension();

    for (auto &[pos, piece] : m_pieces)
    {
        if (piece.getColor() == m_turn)
        {
            continue;
        }

        auto above = pieceColorAt(pos.above());
        auto below = pieceColorAt(pos.below());
        auto right = pieceColorAt(pos.right());
        auto left  = pieceColorAt(pos.left());

        auto verticalEnemies = above && below && *above != piece.getColor() && *below != piece.getColor();
        auto horizontalEnemies = left && right && *left != piece.getColor() && *right != piece.getColor();

        if (piece.getType() == Piece::Type::King && pos == Pos{dim/2, dim/2})
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

    for (auto &toErase : captures)
    {
        m_pieces.erase(toErase);
    }
}

std::optional<Color> Board::pieceColorAt(const Pos &pos) const
{
    auto it = m_pieces.find(pos);

    if (it == m_pieces.end())
    {
        return {};
    }

    return it->second.getColor();
}

std::vector<Move> Board::getPossibleMoves() const
{
    std::vector<Move> possibleMoves;

    for (auto &[pos, piece] : m_pieces)
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

std::future<std::vector<Board::MoveAndResults>> Board::runSimulationInThread(const std::chrono::milliseconds &quota,
    const std::vector<Move> &movesToSimulate)
{
    auto bIn = Board(*this);
    return std::async(std::launch::async, [bIn, movesToSimulate, quota]
    {
        std::vector<Board::MoveAndResults> out;
        out.resize(movesToSimulate.size());

        for (auto i = 0u; i < movesToSimulate.size(); i++)
        {
            out[i].move = movesToSimulate[i];
        }

        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start < quota)
        {
            for (auto x = 0u; x < 50; x++)
            {
                for (auto i = 0u; i < movesToSimulate.size(); i++)
                {
                    auto b = bIn;
                    b.move(out[i].move);
                    out[i].results = out[i].results + b.simulate();
                }
            }
        }

        return out;
    });
}

Board::PlayResult Board::simulate()
{
    while (true)
    {
        auto winner = getWinner();

        if (winner)
        {
            return Board::PlayResult() + winner;
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
    }
}


std::unique_ptr<IBoard> IBoard::fromString(const std::string &s)
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
            p->place({x,y});
            pieces.push_back(std::move(p));
        }
    }

    return std::make_unique<Board>(dimension, pieces);
}
