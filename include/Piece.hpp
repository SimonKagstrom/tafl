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
        Black,
        White,
        King,
    };

    Piece(Piece::Type which);

    Piece(const Piece&) = default;

    virtual ~Piece()
    {
    }

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
