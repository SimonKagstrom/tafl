#pragma once

namespace tafl
{

struct Pos
{
    unsigned x{0};
    unsigned y{0};

    unsigned flatten(unsigned dimension) const
    {
        return y * dimension + x;
    }

    bool operator<(const Pos &b) const
    {
        // For the largest games
        return flatten(18) < b.flatten(18);
    }

    bool operator==(const Pos &b) const
    {
        return x == b.x && y == b.y;
    }
};

}