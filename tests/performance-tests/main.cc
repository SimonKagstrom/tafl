#include <iai.hh>
#include <utils.hh>

using namespace tafl;

int main(int argc, const char *argv[])
{
	IBoard *board = IBoard::fromString(BOARD_9X9_INIT_STRING);
	IAi *ai = IAi::createAi();

	panic_if(!board,
			"Can't create board");
	panic_if(!ai,
			"Can't create AI");

	for (unsigned i = 0; i < 10; i++) {
		printf("Move %d: ", i + 1);
		fflush(stdout);
		IBoard::Move move = ai->getBestMove(*board);
		bool res;
		printf("%d,%d   ->   %d,%d  board now %.2f\n",
				move.m_from.m_x, move.m_from.m_y, move.m_to.m_x, move.m_to.m_y,
				ai->evaluate(*board)
				);

		res = board->doMove(move);
		panic_if(!res,
				"Can't do move");

		if (board->getWinner() != IBoard::BOTH)
			break;
	}

	delete ai;
	delete board;

	return 0;
}
