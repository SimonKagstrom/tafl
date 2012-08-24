#include <iai.hh>
#include <utils.hh>

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
	Ai()
	{
		// Guesses
		m_configuration[PIECE] = 10;
		m_configuration[PIECE_CAN_REACH] = 5;
		m_configuration[VICTORY] = 100000;
	}

	double evaluate(IBoard &board)
	{
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

	IBoard::Move getBestMove(IBoard &board)
	{
		return IBoard::Move();
	}

	std::string toString()
	{
		return std::string("");
	}


// private:
	double m_configuration[N_ENTRIES];

};

IAi *IAi::createAi()
{
	return new Ai();
}

IAi *IAi::fromString(std::string &str)
{
	return NULL;
}
