
#include <IBoard.hpp>
#include <cmath>
#include <vector>
#include <map>
#include <set>

#include "Board.hpp"

using namespace tafl;


Board::Board(unsigned dimensions, std::vector<std::unique_ptr<Piece>> &pieces) :
    m_dimensions(dimensions)
{
    for (auto &p : pieces)
    {
        m_pieces.emplace(p->getPosition(), *p);
    }
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
        return;
    }
    auto p = m_pieces.find(move.from)->second;

    if (getTurn() != p.getColor())
    {
        return;
    }

    if (m_pieces.find(move.to) != m_pieces.end())
    {
        return;
    }

    m_pieces.emplace(move.to, p);
    m_pieces.erase(move.from);

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
