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


    // Doesn't care about borders, so those must be checked
    Pos above() const
    {
        return {x, y - 1};
    }

    Pos below() const
    {
        return {x, y + 1};
    }

    Pos right() const
    {
        return {x + 1, y};
    }

    Pos left() const
    {
        return {x - 1, y};
    }
};

}