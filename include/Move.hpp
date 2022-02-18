#pragma once

#include "Pos.hpp"

namespace tafl
{

struct Move
{
    Pos from;
    Pos to;

    bool operator==(const Move &other) const
    {
        return from == other.from && to == other.to;
    }
};

}

