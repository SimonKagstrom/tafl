
#include <IBoard.hpp>
#include <cmath>
#include <vector>
#include <map>
#include <set>

using namespace tafl;

namespace
{

class Board : public IBoard
{
public:
    Board(unsigned dimensions, std::vector<std::unique_ptr<Piece>> &pieces) :
        m_dimensions(dimensions)
    {
        for (auto &p : pieces)
        {
            m_pieces.emplace(p->getPosition(), *p);
        }
    }

    unsigned getBoardDimension() const override
    {
        return m_dimensions;
    }


    std::optional<Piece::Type> pieceAt(const Pos &pos) const override
    {
        auto it = m_pieces.find(pos);

        if (it == m_pieces.end())
        {
            return {};
        }

        return it->second.getType();
    }

    const std::vector<Piece> getPieces(const Color &which) const override
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

    virtual void move(Move move) override
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

    virtual Color getTurn() const override
    {
        return m_turn;
    }

    virtual void setTurn(Color which) override
    {
        m_turn = which;
    }

private:
    void scanCaptures()
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

            if (piece.getType() == Piece::Type::King && pos == Pos{dim/2, dim/2})
            {
                // The king in the castle - all 4 sides must be occupied
                if (above && below && left && right)
                {
                    if (*above != piece.getColor() && *below != piece.getColor() &&
                        *right != piece.getColor() && *right != piece.getColor())
                    {
                        captures.insert(pos);
                    }
                }

            }
            else
            {
                if (above && below)
                {
                    if (*above != piece.getColor() && *below != piece.getColor())
                    {
                        captures.insert(pos);
                    }
                }
                if (right && left)
                {
                    if (*right != piece.getColor() && *right != piece.getColor())
                    {
                        captures.insert(pos);
                    }
                }
            }
        }

        for (auto &toErase : captures)
        {
            m_pieces.erase(toErase);
        }
    }

    std::optional<Color> pieceColorAt(const Pos &pos) const
    {
        auto it = m_pieces.find(pos);

        if (it == m_pieces.end())
        {
            return {};
        }

        return it->second.getColor();
    }

    const unsigned m_dimensions;
    Color m_turn{Color::White};
    std::map<Pos, Piece> m_pieces;
};

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
