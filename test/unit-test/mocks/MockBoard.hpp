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
    MAKE_CONST_MOCK1(pieceAt, std::optional<IPiece::Type>(const Pos &pos), override);

private:
    std::vector<std::unique_ptr<trompeloeil::expectation>> m_expectations;
};

}