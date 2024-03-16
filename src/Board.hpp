#pragma once

#include <IBoard.hpp>
#include <IMoveTrait.hpp>
#include <span>

namespace tafl
{

class Board : public IBoard
{
public:
    Board(unsigned dimensions, std::vector<std::unique_ptr<Piece>>& pieces);

    unsigned getBoardDimension() const override;

    std::optional<Piece::Type> pieceAt(const Pos& pos) const override;

    std::vector<Piece> getPieces(const Color& which) const override;

    std::vector<Move> getPossibleMoves() const override;

    void move(Move move) override;

    Color getTurn() const override;

    void setTurn(Color which) override;

    std::optional<Color> getWinner() const override;

    std::future<std::optional<Move>>
    calculateBestMove(const std::chrono::milliseconds& quota,
                      std::function<void()> onFutureReady) override;

private:
    struct PlayResult
    {
        double whiteWins {0};
        double blackWins {0};
        unsigned samples {0};

        PlayResult() = default;

        PlayResult(double w, double b, unsigned s)
            : whiteWins {w}
            , blackWins {b}
            , samples {s}
        {
        }

        PlayResult(Color win, unsigned ply)
        {
            if (win == Color::White)
            {
                whiteWins = 1.0 / ply;
            }
            else
            {
                blackWins = 1.0 / ply;
            }
            samples = 1;
        }

        PlayResult operator+(const PlayResult& other) const
        {
            return PlayResult {
                whiteWins + other.whiteWins, blackWins + other.blackWins, samples + other.samples};
        }
    };

    struct MoveAndResults
    {
        Move move;
        PlayResult results;
    };

    Board(const Board&);

    void scanCaptures();

    std::optional<Color> pieceColorAt(const Pos& pos) const;

    std::future<std::vector<MoveAndResults>>
    runSimulationInThread(const std::chrono::milliseconds& quota,
                          std::span<const Move> movesToSimulate);

    /*
     * Run random moves until a winner is found.
     */
    PlayResult simulate(unsigned ply);

    const unsigned m_dimensions;
    Color m_turn {Color::White};
    std::unordered_map<Pos, Piece> m_pieces;

    std::unique_ptr<IMoveTrait> m_moveTrait;
};

} // namespace tafl