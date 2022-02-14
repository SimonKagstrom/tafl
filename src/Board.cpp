#include <IBoard.hpp>
#include <IPiece.hpp>
#include <cmath>
#include <vector>
#include <map>

using namespace tafl;

namespace
{

class Board : public IBoard
{
public:
    Board(unsigned dimensions, std::vector<std::unique_ptr<IPiece>> &pieces) :
        m_dimensions(dimensions)
    {
        for (auto &p : pieces)
        {
            m_pieces[p->getPosition()] = std::move(p);
        }
    }

    unsigned getBoardDimension() const override
    {
        return m_dimensions;
    }


    std::optional<IPiece::Type> pieceAt(const Pos &pos) const override
    {
        auto it = m_pieces.find(pos);

        if (it == m_pieces.end())
        {
            return {};
        }

        return it->second->getType();
    }

private:
    const unsigned m_dimensions;
    std::map<Pos, std::unique_ptr<IPiece>> m_pieces;
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

    std::vector<std::unique_ptr<IPiece>> pieces;

    for (auto i = 0u; i < dimension * dimension; i++)
    {
        auto x = i % dimension;
        auto y = i / dimension;
        auto c = s[i];

        auto p = IPiece::fromChar(c);
        if (p)
        {
            p->place({x,y});
            pieces.push_back(std::move(p));
        }
    }

    return std::make_unique<Board>(dimension, pieces);
}
