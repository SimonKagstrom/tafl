#include <iboard.hh>

using namespace tafl;

class Board : public IBoard
{
public:
	Board(std::string str)
	{
	}


	Color_t getTurn()
	{
		return BLACK;
	}

	PieceList_t getPieces(Color_t color)
	{
		PieceList_t out;

		return out;
	}

	bool getPiece(Point where, Piece &out)
	{
		return false;
	}


	MoveList_t getPossibleMoves(Piece piece)
	{
		MoveList_t out;

		return out;
	}


	bool canMove(Move &move)
	{
		return false;
	}

	bool doMove(Move &move)
	{
		return false;
	}


	std::string toString()
	{
		return std::string("");
	}
};

IBoard *IBoard::fromString(std::string str)
{
	// Wrong size?
	if (str.size() != 9 * 9 + 1 &&
			str.size() != 11 * 11 + 1)
		return NULL;

	if (str[0] != 'B')
		return NULL;

	// Count kings, check for invalid characters
	unsigned kings = 0;

	for (unsigned i = 1; i < str.size(); i++) {
		switch (str[i])
		{
		case 'k':
			kings++;
			break;
		case 'w':
		case 'e':
		case 'b':
		case 'x':
			break;
		default:

			// Invalid character
			return NULL;
		}
	}

	// One king to rule them all, not more
	if (kings != 1)
		return NULL;

	return new Board(str);
}

