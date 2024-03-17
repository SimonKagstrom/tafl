#pragma once

#include "Pos.hpp"

#include <Color.hpp>
#include <memory>

namespace tafl
{

class Piece
{
public:
    enum class Type
    {
        Unset,
        Black,
        White,
        King,
    };

    Piece();

    Piece(Piece::Type which);

    Piece(const Piece&) = default;

    virtual ~Piece() = default;

    virtual void place(const Pos& pos);

    virtual Pos getPosition() const;

    virtual Type getType() const;

    virtual Color getColor() const;


    static std::unique_ptr<Piece> fromChar(char c);
    static char toChar(Type t);

protected:
    Type m_type;
    Pos m_pos;
};

} // namespace tafl
