#pragma once

namespace tafl
{

enum class Color
{
    Black,
    White,
};

constexpr Color operator!(const Color which)
{
    if (which == Color::Black)
    {
        return Color::White;
    }

    return Color::Black;
}

}
