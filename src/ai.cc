#include <iai.hh>
#include <utils.hh>

#include <math.h>

using namespace tafl;

#define min(x, y) ( (x) < (y) ? (x) : (y) )
#define max(x, y) ( (x) > (y) ? (x) : (y) )

enum configuration
{
	PIECE,
	PIECE_CAN_REACH,
	KING_ON_BORDER,
	VICTORY,
	N_ENTRIES,
};

class Ai : public IAi
{
public:
	Ai() : m_maxDepth(3)
	{
		// Guesses
		m_configuration[PIECE] = 10;
		m_configuration[PIECE_CAN_REACH] = 5;
		m_configuration[KING_ON_BORDER] = 40;
		m_configuration[VICTORY] = 100000;

		m_useAlphaBeta = true;
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

	virtual double evaluate(IBoard &board)
	{
		if (board.getWinner() != IBoard::BOTH)
			return m_configuration[VICTORY] * getColorSign(board.getWinner());

		const IBoard::PieceList_t pieces = board.getPieces(IBoard::Color_t::BOTH);
		unsigned dimensions = board.getDimensions();

		double scoreBlack[dimensions * dimensions];
		double scoreWhite[dimensions * dimensions];
		double score[dimensions * dimensions];
		double out = 0;
		IBoard::Piece king;

		for (unsigned i = 0; i < dimensions * dimensions; i++) {
			scoreBlack[i] = 0.0;
			scoreWhite[i] = 0.0;
			score[i] = 0.0;
		}

		for (IBoard::PieceList_t::const_iterator it = pieces.begin();
				it != pieces.end();
				it++) {
			IBoard::Piece piece = *it;
			IBoard::MoveList_t moves = board.getPossibleMoves(piece);
			double *scoreTable = scoreBlack;
			unsigned piecePos = piece.m_location.m_y * dimensions + piece.m_location.m_x;

			if (piece.m_color == IBoard::WHITE)
				scoreTable = scoreWhite;
			if (piece.m_isKing)
				king = piece;

			scoreTable[piecePos] += m_configuration[PIECE];


			for (IBoard::MoveList_t::iterator moveIt = moves.begin();
					moveIt != moves.end();
					moveIt++) {
				IBoard::Move move = *moveIt;
				unsigned moveDest = move.m_to.m_y * dimensions + move.m_to.m_x;

				scoreTable[moveDest] += m_configuration[PIECE_CAN_REACH];
			}
		}

		if (isOnBorder(board, king.m_location))
			out += getColorSign(IBoard::WHITE) * m_configuration[KING_ON_BORDER];

		// Will do some more stuff in the future
		for (unsigned i = 0; i < dimensions * dimensions; i++)
			score[i] = scoreBlack[i] - scoreWhite[i];

		for (unsigned i = 0; i < dimensions * dimensions; i++)
			out += score[i];

		return out;
	}

	double minimax(IBoard &board, IBoard::Move *bestMove, unsigned depth)
	{
		if (board.getWinner() != IBoard::BOTH || depth == m_maxDepth)
			return evaluate(board);

		IBoard::Color_t turn = board.getTurn();
		const IBoard::PieceList_t pieces = board.getPieces(turn);
		int sign = getColorSign(turn);
		double out = INFINITY * -sign;

		for (IBoard::PieceList_t::const_iterator it = pieces.begin();
				it != pieces.end();
				it++) {
			IBoard::Piece piece = *it;
			IBoard::MoveList_t moves = board.getPossibleMoves(piece);

			for (IBoard::MoveList_t::iterator moveIt = moves.begin();
					moveIt != moves.end();
					moveIt++) {
				IBoard::Move move = *moveIt;
				double cur, oldBest;
				IBoard *p;
				bool res;

				p = board.copy();
				res = p->doMove(move);
				panic_if(!res,
						"Can't make possible move!");

				IBoard::Move nextMove;
				oldBest = out;
				cur = minimax(*p, &nextMove, depth + 1);

				if (turn == IBoard::BLACK)
					out = max(cur, out);
				else
					out = min(cur, out);

				if (out != oldBest) {
					out = cur;
					*bestMove = move;
				}

				delete p;
			}
		}

		return out;
	}

	double alphaBeta(IBoard &board, IBoard::Move *bestMove, unsigned depth, double alpha, double beta)
	{
		if (board.getWinner() != IBoard::BOTH || depth == m_maxDepth)
			return evaluate(board);

		IBoard::Color_t turn = board.getTurn();
		const IBoard::PieceList_t pieces = board.getPieces(turn);

		for (IBoard::PieceList_t::const_iterator it = pieces.begin();
				it != pieces.end();
				it++) {
			IBoard::Piece piece = *it;
			IBoard::MoveList_t moves = board.getPossibleMoves(piece);
			double cur;

			for (IBoard::MoveList_t::iterator moveIt = moves.begin();
					moveIt != moves.end();
					moveIt++) {
				IBoard::Move move = *moveIt;
				IBoard *p;
				bool res;

				p = board.copy();
				res = p->doMove(move);
				panic_if(!res,
						"Can't make possible move!");

				IBoard::Move nextBest;
				cur = alphaBeta(*p, &nextBest, depth + 1, alpha, beta);

				delete p;

				if (turn == IBoard::BLACK) {
					if (cur > alpha) {
						*bestMove = move;
						alpha = cur;
					}
					if (alpha >= beta)
						goto out;
				}
				else {
					if (cur < beta) {
						*bestMove = move;
						beta = cur;
					}
					// Cut off white
					if (alpha >= beta)
						goto out;
				}
			}
		}

out:
		if (turn == IBoard::BLACK)
			return alpha;

		return beta;
	}

	IBoard::Move getBestMove(IBoard &board)
	{
		IBoard *p = board.copy();
		IBoard::Move out;

		if (m_useAlphaBeta)
			alphaBeta(*p, &out, 0, -INFINITY, INFINITY);
		else
			minimax(*p, &out, 0);

		delete p;

		return out;
	}

	std::string toString()
	{
		return std::string("");
	}

	void setAlgoritm(Algorithm_t algo)
	{
		m_useAlphaBeta = (algo == ALPHA_BETA);
	}

	virtual void setSearchDepth(unsigned depth)
	{
		m_maxDepth = depth;
	}


// private:
	bool isOnBorder(IBoard &board, IBoard::Point &point)
	{
		const unsigned dimensions = board.getDimensions();

		return point.m_x == dimensions - 1 ||
				point.m_x == 0 ||
				point.m_y == dimensions - 1 ||
				point.m_y == 0;
	}


	double m_configuration[N_ENTRIES];
	unsigned m_maxDepth;
	bool m_useAlphaBeta;
};

IAi *IAi::createAi()
{
	return new Ai();
}

IAi *IAi::fromString(std::string &str)
{
	return NULL;
}
