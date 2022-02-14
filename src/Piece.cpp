#include <IPiece.hpp>

using namespace tafl;

namespace
{

class Piece : public IPiece
{
public:
    Piece(IPiece::Type which) :
        m_type(which)
    {
    }

    virtual void place(const Pos &where) override
    {
        m_pos = where;
    }

    virtual Pos getPosition() const override
    {
        return m_pos;
    }

    virtual Type getType() const override
    {
        return m_type;
    }

private:
    const IPiece::Type m_type;
    Pos m_pos;
};

}

std::unique_ptr<IPiece> IPiece::fromChar(char c)
{
    switch(c)
    {
    case 'w':
        return std::make_unique<Piece>(IPiece::Type::White);
    case 'k':
        return std::make_unique<Piece>(IPiece::Type::King);
    case 'b':
        return std::make_unique<Piece>(IPiece::Type::Black);
    default:
        break;
    }


    return nullptr;
}
