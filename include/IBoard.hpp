#pragma once

#include <string>
#include <memory>
#include <optional>

#include "IPiece.hpp"

namespace tafl
{

class IBoard
{
public:
    virtual ~IBoard()
    {
    }

    virtual unsigned getBoardDimension() const = 0;

    virtual std::optional<IPiece::Type> pieceAt(const Pos &pos) const = 0;

    static std::unique_ptr<IBoard> fromString(const std::string &s);
};

}