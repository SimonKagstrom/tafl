#include <iai.hh>
#include <utils.hh>

#include <math.h>

using namespace tafl;

enum configuration
{
	PIECE,
	PIECE_CAN_REACH,
	VICTORY,
	N_ENTRIES,
};

class Ai : public IAi
{
public:
	Ai() : m_maxDepth(2)
	{
		// Guesses
		m_configuration[PIECE] = 10;
		m_configuration[PIECE_CAN_REACH] = 5;
		m_configuration[VICTORY] = 100000;
	}

	int getColorSign(IBoard::Color_t color)
	{
		int sign = 0;

		if (color == IBoard::WHITE)
			sign = -1;
		else if (color == IBoard::BLACK)
			sign = 1;

		return sign;
	}

	double evaluate(IBoard &board)
	{
		if (board.getWinner() != IBoard::BOTH)
			return m_configuration[VICTORY] * getColorSign(board.getWinner());

		IBoard::PieceList_t pieces = board.getPieces(IBoard::Color_t::BOTH);
		unsigned dimensions = board.getDimensions();

		double scoreBlack[dimensions * dimensions];
		double scoreWhite[dimensions * dimensions];
		double score[dimensions * dimensions];
		double out = 0;

		for (unsigned i = 0; i < dimensions * dimensions; i++) {
			scoreBlack[i] = 0.0;
			scoreWhite[i] = 0.0;
			score[i] = 0.0;
		}

		for (IBoard::PieceList_t::iterator it = pieces.begin();
				it != pieces.end();
				it++) {
			IBoard::Piece piece = *it;
			IBoard::MoveList_t moves = board.getPossibleMoves(piece);
			double *table = scoreBlack;
			unsigned piecePos = piece.m_location.m_y * dimensions + piece.m_location.m_x;

			if (piece.m_color == IBoard::WHITE)
				table = scoreWhite;

			table[piecePos] += m_configuration[PIECE];


			for (IBoard::MoveList_t::iterator moveIt = moves.begin();
					moveIt != moves.end();
					moveIt++) {
				IBoard::Move move = *moveIt;
				unsigned moveDest = move.m_to.m_y * dimensions + move.m_to.m_x;

				table[moveDest] += m_configuration[PIECE_CAN_REACH];
			}
		}

		// Will do some more stuff in the future
		for (unsigned i = 0; i < dimensions * dimensions; i++)
			score[i] = scoreBlack[i] - scoreWhite[i];

		for (unsigned i = 0; i < dimensions * dimensions; i++)
			out += score[i];

		return out;
	}

	double minimax(IBoard &board, unsigned depth)
	{
		if (board.getWinner() != IBoard::BOTH || depth == m_maxDepth)
			return evaluate(board);

		IBoard::Color_t turn = board.getTurn();
		IBoard::PieceList_t pieces = board.getPieces(turn);
		double out = INFINITY * getColorSign(turn);

		for (IBoard::PieceList_t::iterator it = pieces.begin();
				it != pieces.end();
				it++) {
			IBoard::Piece piece = *it;
			IBoard::MoveList_t moves = board.getPossibleMoves(piece);

			for (IBoard::MoveList_t::iterator moveIt = moves.begin();
					moveIt != moves.end();
					moveIt++) {
				IBoard::Move move = *moveIt;
				double cur;
				IBoard *p;
				bool res;

				p = IBoard::fromBoard(&board);
				res = p->doMove(move);
				panic_if(!res,
						"Can't make possible move!");

				cur = minimax(*p, depth + 1);
				if (abs(cur) > abs(out))
					out = cur;

				delete p;
			}
		}

		return out;
	}

	IBoard::Move getBestMove(IBoard &board)
	{
		IBoard::Move out;
		IBoard::Color_t turn = board.getTurn();
		IBoard::PieceList_t pieces = board.getPieces(turn);
		double best = INFINITY * getColorSign(turn);

		for (IBoard::PieceList_t::iterator it = pieces.begin();
				it != pieces.end();
				it++) {
			IBoard::Piece piece = *it;
			IBoard::MoveList_t moves = board.getPossibleMoves(piece);

			for (IBoard::MoveList_t::iterator moveIt = moves.begin();
					moveIt != moves.end();
					moveIt++) {
				IBoard::Move move = *moveIt;
				double cur;
				IBoard *p;
				bool res;

				p = IBoard::fromBoard(&board);
				res = p->doMove(move);
				panic_if(!res,
						"Can't make possible move!");

				cur = minimax(*p, 0);
				if (abs(cur) > abs(best)) {
					out = move;
					best = cur;
				}

				delete p;
			}
		}

		return out;
	}

	std::string toString()
	{
		return std::string("");
	}


// private:
	double m_configuration[N_ENTRIES];
	unsigned m_maxDepth;
};

IAi *IAi::createAi()
{
	return new Ai();
}

IAi *IAi::fromString(std::string &str)
{
	return NULL;
}
