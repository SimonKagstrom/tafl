#pragma once

#include <BoardHashTable.hpp>
#include <IBoard.hpp>
#include <IMoveTrait.hpp>
#include <array>
#include <etl/vector.h>
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
    // Random games longer than this are considered a draw
    static constexpr unsigned kMaxSimulationPly = 400;

    using TaflBoardHashTable = BoardHashTable<1024*1024>;

    struct PlayResult
    {
        float whiteWins {0};
        float blackWins {0};
        unsigned samples {0};

        PlayResult() = default;

        PlayResult(auto w, auto b, unsigned s)
            : whiteWins {w}
            , blackWins {b}
            , samples {s}
        {
        }

        PlayResult(Color win, unsigned ply)
        {
            // Quick wins (and slow losses) are worth a bit more, so that the winning
            // side goes for the kill instead of shuffling around
            const auto margin = 0.3f * std::min(ply, kMaxSimulationPly) / kMaxSimulationPly;

            if (win == Color::White)
            {
                whiteWins = 1.0f - margin;
                blackWins = margin;
            }
            else
            {
                blackWins = 1.0f - margin;
                whiteWins = margin;
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

    void scanCaptures(const Pos& moved);

    /*
     * Return a move which immediately wins the game for the current color, if any.
     */
    std::optional<Move> findWinningMove() const;

    void fillPossibleMoves();

    std::optional<Color> pieceColorAt(const Pos& pos) const;

    std::future<std::vector<MoveAndResults>>
    runSimulationInThread(const std::chrono::milliseconds& quota,
                          std::span<const Move> movesToSimulate);

    /*
     * Run random moves until a winner is found.
     *
     * @param ply the number of moves already played from the searched position
     */
    PlayResult simulate(unsigned ply);

    uint64_t checksum() const;

    uint64_t pieceChecksum(const Piece& piece) const;

    const unsigned m_dimensions;
    Color m_turn {Color::White};

    std::array<Piece, 18 * 18> m_pieceStorage;
    etl::vector<Piece*, 18 * 18> m_pieces;
    std::array<Piece*, 18 * 18> m_board {nullptr};

    etl::vector<Move, 18 * 18 * 18> m_possibleMoves;

    std::unique_ptr<IMoveTrait> m_moveTrait;

    std::unique_ptr<TaflBoardHashTable> m_knownPlays;
};

} // namespace tafl