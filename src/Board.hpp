#pragma once

#include <IBoard.hpp>
#include <IMoveTrait.hpp>

namespace tafl
{

class Board : public IBoard
{
public:
    Board(unsigned dimensions, std::vector<std::unique_ptr<Piece>> &pieces);

    unsigned getBoardDimension() const override;

    std::optional<Piece::Type> pieceAt(const Pos &pos) const override;

    const std::vector<Piece> getPieces(const Color &which) const override;

    std::vector<Move> getPossibleMoves() const override;

    void move(Move move) override;

    Color getTurn() const override;

    void setTurn(Color which) override;

    std::optional<Color> getWinner() const override;

    std::future<std::optional<Move>> calculateBestMove(const std::chrono::milliseconds &quota,
        std::function<void()> onFutureReady) override;

private:
    struct PlayResult
    {
        unsigned whiteWins{0};
        unsigned blackWins{0};
        unsigned samples{0};

        PlayResult operator+(const std::optional<Color> &result)
        {
            return {whiteWins + (result.value() == Color::White),
                    blackWins + (result.value() == Color::Black),
                    samples + 1};
        }

        PlayResult operator+(const PlayResult &other)
        {
            return {whiteWins + other.whiteWins,
                    blackWins + other.blackWins,
                    samples + 1};
        }
    };

    struct MoveAndResults
    {
        Move move;
        PlayResult results;
    };
 
    Board(const Board &);

    void scanCaptures();

    std::optional<Color> pieceColorAt(const Pos &pos) const;

    std::future<std::vector<MoveAndResults>> runSimulationInThread(const std::chrono::milliseconds &quota,
        const std::vector<Move> &movesToSimulate);

    /*
     * Run random moves until a winner is found.
     */
    PlayResult simulate();

    const unsigned m_dimensions;
    Color m_turn{Color::White};
    std::unordered_map<Pos, Piece> m_pieces;

    std::unique_ptr<IMoveTrait> m_moveTrait;
};

}