#include <IMoveTrait.hpp>

using namespace tafl;

namespace
{

class MoveTrait : public IMoveTrait
{
public:
    MoveTrait()
    {
    }

    std::vector<Move> getMoves(const IBoard &board, const IPiece &piece) override
    {
        auto dim = board.getBoardDimension();
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
        auto nextY = pos.y + dir * !isX;

        if (nextX >= dim)
        {
            return {};
        }
        if (nextY >= dim)
        {
            return {};
        }
        if (board.pieceAt({nextX, nextY}))
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
