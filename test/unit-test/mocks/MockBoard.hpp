#include <IBoard.hpp>
#include <trompeloeil.hpp>

namespace tafl::mocks
{

class MockBoard : public IBoard
{
public:
    static std::unique_ptr<MockBoard> createMock(unsigned dimension)
    {
        auto out = std::make_unique<MockBoard>();

        out->setupModel(dimension);

        return out;
    }

    void setupModel(unsigned dimension)
    {
        using trompeloeil::_;

        m_expectations.push_back(NAMED_ALLOW_CALL(*this, getBoardDimension()).RETURN(dimension));
        m_expectations.push_back(NAMED_ALLOW_CALL(*this, pieceAt(_)).RETURN(std::nullopt));
    }

    MAKE_CONST_MOCK0(getBoardDimension, unsigned(), override);
    MAKE_CONST_MOCK1(pieceAt, std::optional<Piece::Type>(const Pos& pos), override);
    MAKE_CONST_MOCK1(getPieces, std::vector<Piece>(const Color& which), override);
    MAKE_CONST_MOCK0(getPossibleMoves, std::vector<Move>(), override);
    MAKE_MOCK1(move, void(Move move), override);
    MAKE_CONST_MOCK0(getTurn, Color(), override);
    MAKE_MOCK1(setTurn, void(Color which), override);
    MAKE_CONST_MOCK0(getWinner, std::optional<Color>(), override);
    MAKE_MOCK2(calculateBestMove,
               std::future<std::optional<Move>>(const std::chrono::milliseconds& quota,
                                                std::function<void()> onFutureReady),
               override);


private:
    std::vector<std::unique_ptr<trompeloeil::expectation>> m_expectations;
};

} // namespace tafl::mocks