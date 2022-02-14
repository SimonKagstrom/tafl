#pragma once

#include <memory>

#include "Pos.hpp"

namespace tafl
{

class IPiece
{
public:
    enum class Type
    {
        Black,
        White,
        King,
    };


    virtual ~IPiece()
    {
    }

    virtual void place(const Pos &pos) = 0;

    virtual Pos getPosition() const = 0;


    virtual Type getType() const = 0;


    static std::unique_ptr<IPiece> fromChar(char c);
};

}
