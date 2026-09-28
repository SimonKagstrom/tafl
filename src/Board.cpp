
#include "Board.hpp"

#include <IBoard.hpp>
#include <cassert>
#include <cmath>
#include <fmt/format.h>
#include <future>
#include <map>
#include <random>
#include <ranges>
#include <set>
#include <vector>

using namespace tafl;

Board::Board(unsigned dimensions, std::vector<std::unique_ptr<Piece>>& pieces)
    : m_dimensions(dimensions)
    , m_moveTrait(IMoveTrait::create())
    , m_knownPlays(std::make_unique<TaflBoardHashTable>())
{
    unsigned index = 0;
    for (auto& p : pieces)
    {
        m_pieceStorage[index] = *p;
        m_pieces.push_back(&m_pieceStorage[index]);
        m_board[p->getPosition().flatten(m_dimensions)] = &m_pieceStorage[index];
        index++;
    }

    std::ifstream file("known_plays.dat");
    if (file)
    {
        m_knownPlays->fillFromFile(file);
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

    scanCaptures(move.to);

    setTurn(!m_turn);
}

uint64_t
Board::pieceChecksum(const Piece& piece) const
{
    auto pos = piece.getPosition();

    return (pos.y * 180 + pos.x) * 4 + static_cast<unsigned>(piece.getType());
}

uint64_t
Board::checksum() const
{
    uint64_t sum = 0;

    for (auto& p : m_pieces)
    {
        sum += pieceChecksum(*p);
    }

    return sum;
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
    const auto nThreads = 6u;

    auto possibleMoves = getPossibleMoves();

    std::promise<std::optional<Move>> p;

    if (possibleMoves.empty())
    {
        p.set_value(std::nullopt);
        return p.get_future();
    }

    // No need to search if the game can be won right away
    if (auto win = findWinningMove())
    {
        p.set_value(win);
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
        [this, black, number_of_moves, threadFutures = std::move(threadFutures)]() mutable {
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

            // The most visited move is the most robust choice in MCTS
            std::ranges::sort(results, [](const MoveAndResults& a, const MoveAndResults& b) {
                return a.results.samples > b.results.samples;
            });

            for (auto& x : results)
            {
                auto f = x.move.from;
                auto t = x.move.to;

                auto total = x.results.blackWins + x.results.whiteWins;
                fmt::print("{}:{} -> {}:{}, {:.3f} win rate for {} ({} visits)\n",
                           f.x,
                           f.y,
                           t.x,
                           t.y,
                           total > 0 ? (black ? x.results.blackWins : x.results.whiteWins) / total
                                     : 0.0f,
                           black ? "black" : "white",
                           x.results.samples);

                auto bIn = Board(*this);
                bIn.move(x.move);
                auto checksum = bIn.checksum();
                auto database = m_knownPlays->get(checksum);
                if (database)
                {
                    x.results.blackWins += database->blackWins;
                    x.results.whiteWins += database->whiteWins;
                }

                m_knownPlays->insert(checksum, x.results.whiteWins, x.results.blackWins);
            }

            out = results.front().move;

            std::ofstream file("known_plays.dat");
            if (file)
            {
                m_knownPlays->writeToFile(file);
            }

            return out;
        });
}

void
Board::scanCaptures(const Pos& moved)
{
    // Captures are active: only the piece that just moved can capture, and only
    // the enemy pieces directly next to it. A piece may safely move in between
    // two enemies.
    const auto dim = getBoardDimension();
    const auto castle = Pos {dim / 2, dim / 2};
    const auto isMover = [this](const Pos& pos) { return pieceColorAt(pos) == m_turn; };

    const std::array<std::pair<int, int>, 4> directions {{{0, -1}, {0, 1}, {-1, 0}, {1, 0}}};

    for (auto [dx, dy] : directions)
    {
        auto victimPos = Pos {moved.x + dx, moved.y + dy};
        if (victimPos.x >= dim || victimPos.y >= dim)
        {
            continue;
        }

        auto victim = m_board[victimPos.flatten(dim)];
        if (!victim || victim->getColor() == m_turn)
        {
            continue;
        }

        bool captured = false;
        if (victim->getType() == Piece::Type::King && victimPos == castle)
        {
            // The king in the castle - all 4 sides must be occupied
            captured = isMover(victimPos.above()) && isMover(victimPos.below()) &&
                       isMover(victimPos.left()) && isMover(victimPos.right());
        }
        else
        {
            captured = isMover(Pos {victimPos.x + dx, victimPos.y + dy});
        }

        if (captured)
        {
            m_board[victimPos.flatten(dim)] = nullptr;
            m_pieces.erase(std::find(m_pieces.begin(), m_pieces.end(), victim));
        }
    }
}

std::optional<Move>
Board::findWinningMove() const
{
    const auto dim = getBoardDimension();
    const auto castle = Pos {dim / 2, dim / 2};

    auto itKing = std::find_if(m_pieces.begin(), m_pieces.end(), [](const auto& cur) {
        return cur->getType() == Piece::Type::King;
    });
    if (itKing == m_pieces.end())
    {
        return std::nullopt;
    }
    const auto kingPos = (*itKing)->getPosition();

    const std::array<std::pair<int, int>, 4> directions {{{0, -1}, {0, 1}, {-1, 0}, {1, 0}}};

    if (m_turn == Color::White)
    {
        // Can the king slide to an edge?
        for (auto [dx, dy] : directions)
        {
            auto cur = kingPos;
            while (true)
            {
                cur = Pos {cur.x + dx, cur.y + dy};
                if (cur.x >= dim || cur.y >= dim || m_board[cur.flatten(dim)] || cur == castle)
                {
                    break;
                }
                if (cur.x == 0 || cur.x == dim - 1 || cur.y == 0 || cur.y == dim - 1)
                {
                    return Move {kingPos, cur};
                }
            }
        }

        return std::nullopt;
    }

    // Black: can a piece move next to the king and capture it?
    const auto isBlack = [this](const Pos& pos) { return pieceColorAt(pos) == Color::Black; };
    for (auto [dx, dy] : directions)
    {
        auto target = Pos {kingPos.x + dx, kingPos.y + dy};
        if (target.x >= dim || target.y >= dim || m_board[target.flatten(dim)] || target == castle)
        {
            continue;
        }

        bool captures = false;
        if (kingPos == castle)
        {
            captures = true;
            for (auto [ox, oy] : directions)
            {
                auto other = Pos {kingPos.x + ox, kingPos.y + oy};
                if (!(other == target) && !isBlack(other))
                {
                    captures = false;
                }
            }
        }
        else
        {
            captures = isBlack(Pos {kingPos.x - dx, kingPos.y - dy});
        }

        if (!captures)
        {
            continue;
        }

        // Is there a black piece which can reach the target square?
        for (auto [sx, sy] : directions)
        {
            auto cur = target;
            while (true)
            {
                cur = Pos {cur.x + sx, cur.y + sy};
                if (cur.x >= dim || cur.y >= dim || cur == castle)
                {
                    break;
                }
                auto p = m_board[cur.flatten(dim)];
                if (p)
                {
                    if (p->getColor() == Color::Black)
                    {
                        return Move {cur, target};
                    }
                    break;
                }
            }
        }
    }

    return std::nullopt;
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

void
Board::fillPossibleMoves()
{
    m_possibleMoves.uninitialized_resize(0);

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

namespace
{

unsigned
randomIndex(size_t n)
{
    thread_local std::minstd_rand rng {std::random_device {}()};

    return std::uniform_int_distribution<unsigned>(0, n - 1)(rng);
}

// A node in the Monte-Carlo search tree, i.e., the board after "move" was played
struct Node
{
    Move move;
    Color mover; // The color which played move
    unsigned parent {0};

    float wins {0}; // From the view of mover
    unsigned visits {0};

    std::vector<Move> untried;
    std::vector<unsigned> children;
};

} // namespace

std::future<std::vector<Board::MoveAndResults>>
Board::runSimulationInThread(const std::chrono::milliseconds& quota,
                             std::span<const Move> movesToSimulate)
{
    auto bIn = Board(*this);

    auto moves = std::vector<Move>(movesToSimulate.begin(), movesToSimulate.end());

    return std::async(std::launch::async, [bIn, moves, quota] {
        // UCT exploration constant, rewards are in [0, 1]
        constexpr auto kExploration = 0.7f;

        // The root node is the current board (where the opponent made the last move)
        std::vector<Node> tree(1);
        tree[0].mover = !bIn.getTurn();
        tree[0].untried = moves;

        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start < quota)
        {
            auto b = bIn;
            auto cur = 0u;
            auto depth = 0u;

            // Selection: walk down fully expanded nodes, picking the best by UCT
            while (tree[cur].untried.empty() && !tree[cur].children.empty())
            {
                const auto logParent = std::log(static_cast<float>(tree[cur].visits));
                auto best = tree[cur].children.front();
                auto bestValue = -1.0f;

                for (auto childIdx : tree[cur].children)
                {
                    const auto& child = tree[childIdx];
                    auto value = child.wins / child.visits +
                                 kExploration * std::sqrt(logParent / child.visits);
                    if (value > bestValue)
                    {
                        bestValue = value;
                        best = childIdx;
                    }
                }

                b.move(tree[best].move);
                cur = best;
                depth++;
            }

            // Expansion: try one unexplored move
            if (!tree[cur].untried.empty())
            {
                auto& untried = tree[cur].untried;
                auto idx = randomIndex(untried.size());
                auto m = untried[idx];
                untried[idx] = untried.back();
                untried.pop_back();

                auto mover = b.getTurn();
                b.move(m);

                Node child;
                child.move = m;
                child.mover = mover;
                child.parent = cur;
                if (!b.getWinner())
                {
                    // If there is an immediately winning move, there's no point in
                    // exploring anything else
                    if (auto win = b.findWinningMove())
                    {
                        child.untried.push_back(*win);
                    }
                    else
                    {
                        child.untried = b.getPossibleMoves();
                    }
                }

                tree.push_back(std::move(child));
                auto childIdx = static_cast<unsigned>(tree.size() - 1);
                tree[cur].children.push_back(childIdx);
                cur = childIdx;
                depth++;
            }

            // Simulation and backpropagation
            auto result = b.simulate(depth);
            while (true)
            {
                auto& node = tree[cur];

                node.visits++;
                if (result.samples == 0)
                {
                    // Draw
                    node.wins += 0.5f;
                }
                else if (node.mover == Color::White)
                {
                    node.wins += result.whiteWins;
                }
                else
                {
                    node.wins += result.blackWins;
                }

                if (cur == 0)
                {
                    break;
                }
                cur = node.parent;
            }
        }

        // Report the root children, in the same order as the moves we got
        auto out = std::vector<Board::MoveAndResults>();
        out.reserve(moves.size());
        for (auto& m : moves)
        {
            out.push_back({m, Board::PlayResult()});
        }

        for (auto childIdx : tree[0].children)
        {
            const auto& child = tree[childIdx];
            auto it = std::ranges::find_if(
                out, [&child](const auto& cur) { return cur.move == child.move; });
            auto losses = child.visits - child.wins;

            if (child.mover == Color::White)
            {
                it->results = Board::PlayResult(child.wins, losses, child.visits);
            }
            else
            {
                it->results = Board::PlayResult(losses, child.wins, child.visits);
            }
        }

        return out;
    });
}

Board::PlayResult
Board::simulate(unsigned ply)
{
    for (; ply < kMaxSimulationPly; ply++)
    {
        auto winner = getWinner();

        if (winner)
        {
            return Board::PlayResult(*winner, ply);
        }

        // Always take a win when there is one, otherwise play randomly
        if (auto win = findWinningMove())
        {
            move(*win);
            continue;
        }

        fillPossibleMoves();
        if (m_possibleMoves.empty())
        {
            // No moves available, which loses the game
            return Board::PlayResult(!m_turn, ply);
        }

        move(m_possibleMoves[randomIndex(m_possibleMoves.size())]);
    }

    return Board::PlayResult();
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