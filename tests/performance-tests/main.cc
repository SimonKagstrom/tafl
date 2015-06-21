#include <iai.hh>
#include <utils.hh>

#include <string.h>

using namespace tafl;

static void display(IBoard &board)
{
	unsigned size = board.getDimensions() * board.getDimensions();
	char field[size];

	memset(field, ' ', size);

	const IBoard::PieceList_t pieces = board.getPieces(IBoard::BOTH);

	for (IBoard::PieceList_t::const_iterator it = pieces.begin();
			it != pieces.end();
			it++) {
		IBoard::Piece piece = *it;
		char chr = '.';

		if (piece.m_color != IBoard::BLACK) {
			if (piece.m_isKing)
				chr = 'k';
			else
				chr = 'o';
		}

		field[piece.m_location.m_y * board.getDimensions() + piece.m_location.m_x] = chr;
	}

	printf("###########\n");
	for (unsigned y = 0; y < board.getDimensions(); y++) {
		printf("#");
		for (unsigned x = 0; x < board.getDimensions(); x++) {
			printf("%c", field[y * board.getDimensions() + x]);
		}
		printf("#\n");
	}
	printf("###########\n");
}

int main(int argc, const char *argv[])
{
	IBoard *board = IBoard::fromString(BOARD_9X9_INIT_STRING);
	IAi *ai = IAi::createAi();

	panic_if(!board,
			"Can't create board");
	panic_if(!ai,
			"Can't create AI");

	for (unsigned i = 0; i < 100; i++) {
		display(*board);
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
	display(*board);

	delete ai;
	delete board;

	return 0;
}
