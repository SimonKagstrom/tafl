#include <IMoveTrait.hpp>

using namespace tafl;

namespace
{

class MoveTrait : public IMoveTrait
{
public:
    MoveTrait() = default;

    std::vector<Move> getMoves(const IBoard &board, const Piece &piece) override
    {
        auto pos = piece.getPosition();
        std::vector<Move> out;

        auto xLeft = pos;
        auto xRight = pos;
        auto yUp = pos;
        auto yDown = pos;

        while (auto p = nextX(board, xLeft, -1))
        {
            out.push_back({pos, *p});
        }
        while (auto p = nextX(board, xRight, 1))
        {
            out.push_back({pos, *p});
        }

        while (auto p = nextY(board, yUp, -1))
        {
            out.push_back({pos, *p});
        }
        while (auto p = nextY(board, yDown, 1))
        {
            out.push_back({pos, *p});
        }

        return out;
    }

private:
    std::optional<Pos> nextPos(const IBoard &board, Pos &pos, int dir, int isX) const
    {
        auto dim = board.getBoardDimension();

        auto nextX = pos.x + dir * isX;
        auto nextY = pos.y + dir * static_cast<int>(!isX);

        if (nextX >= dim)
        {
            return {};
        }
        if (nextY >= dim)
        {
            return {};
        }

        // Another piece?
        if (board.pieceAt({nextX, nextY}))
        {
            return {};
        }

        // The middle spot? Forbidden
        if (Pos{nextX, nextY} == Pos{dim/2, dim/2})
        {
            return {};
        }

        pos = {nextX, nextY};

        return pos;
    }

    std::optional<Pos> nextX(const IBoard &board, Pos &pos, int dir) const
    {
        return nextPos(board, pos, dir, 1);
    }

    std::optional<Pos> nextY(const IBoard &board, Pos &pos, int dir) const
    {
        return nextPos(board, pos, dir, 0);
    }

};

}

std::unique_ptr<IMoveTrait> IMoveTrait::create()
{
    return std::make_unique<MoveTrait>();
}
