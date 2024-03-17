#include <Piece.hpp>

#include <cassert>

using namespace tafl;

Piece::Piece()
    : m_type(Piece::Type::Unset)
{
}

Piece::Piece(Piece::Type which)
    : m_type(which)
{
}

void
Piece::place(const Pos& where)
{
    m_pos = where;
}

Pos
Piece::getPosition() const
{
    return m_pos;
}

Piece::Type
Piece::getType() const
{
    return m_type;
}

Color
Piece::getColor() const
{
    if (m_type == Piece::Type::Black)
    {
        return Color::Black;
    }

    return Color::White;
}

std::unique_ptr<Piece>
Piece::fromChar(char c)
{
    switch (tolower(c))
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

char
Piece::toChar(Piece::Type t)
{
    switch (t)
    {
    case Piece::Type::Unset:
        assert(false && "Unset piece");
        return ' ';
    case Piece::Type::White:
        return 'w';
    case Piece::Type::King:
        return 'k';
    case Piece::Type::Black:
        return 'b';
    }

    return ' ';
}