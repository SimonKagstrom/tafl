#include <Piece.hpp>

using namespace tafl;

Piece::Piece(Piece::Type which) :
    m_type(which)
{
}

void Piece::place(const Pos &where)
{
    m_pos = where;
}

Pos Piece::getPosition() const
{
    return m_pos;
}

Piece::Type Piece::getType() const
{
    return m_type;
}

Color Piece::getColor() const
{
    if (m_type == Piece::Type::Black)
    {
        return Color::Black;
    }

    return Color::White;
}

std::unique_ptr<Piece> Piece::fromChar(char c)
{
    switch(c)
    {
    case 'w':
        return std::make_unique<Piece>(Piece::Type::White);
    case 'k':
        return std::make_unique<Piece>(Piece::Type::King);
    case 'b':
        return std::make_unique<Piece>(Piece::Type::Black);
    default:
        break;
    }

    return nullptr;
}
