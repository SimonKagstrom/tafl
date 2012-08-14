#include <iai.hh>
#include <utils.hh>

using namespace tafl;

class Ai : public IAi
{
public:
	Ai()
	{
	}

	int evaluate(IBoard &board)
	{
		return 0;
	}

	IBoard::Move getBestMove(IBoard &board)
	{
		return IBoard::Move();
	}

};

IAi *IAi::createAi()
{
	return new Ai();
}
