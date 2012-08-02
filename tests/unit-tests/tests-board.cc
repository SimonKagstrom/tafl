#include "test.hh"

#include <iboard.hh>

using namespace tafl;

const std::string initialBoard9x9 =
		"   ...   "
		"    .    "
		"    o    "
		".   o   ."
		"..ookoo.."
		".   o   ."
		"    o    "
		"    .    "
		"   ...   ";

const std::string initialBoard11x11 =
		"x  .....  x"
		"     .     "
		"           "
		".    o    ."
		".   ooo   ."
		".. ookoo .."
		".   ooo   ."
		".    o    ."
		"           "
		"     .     "
		"x  .....  x";

class BoardFixture
{
public:
	BoardFixture()
	{
	}

	std::string toBoardString(const std::string &gfx)
	{
		// Invalid
		if (gfx.size() != 9 * 9 &&
				gfx.size() != 11 * 11)
			return std::string("");

		std::string out = "B";
		for (unsigned int i = 0; i < gfx.size(); i++) {
			switch (gfx[i])
			{
			case 'o':
			case 'O':
				out += "w";
				break;
			case '.':
			case '+':
				out += "b";
				break;
			case 'k':
			case 'K':
				out += "k";
				break;
			case 'x':
				out += "x";
				break;
			case ' ':
				out += "e";
				break;

			default:
				return std::string("");
			}
		}

		return out;
	}

	bool getXy(std::string gfx, int *x, int *y)
	{
		unsigned int w = gfx.size() == 9 * 9 ? 9 : 11;
		bool out = false;

		for (unsigned int i = 0; i < gfx.size(); i++) {
			switch (gfx[i])
			{
			case 'O':
			case '+':
			case 'K':
				*y = i / w;
				*x = i % w;
				out = true;
				break;
			default:
				break;
			}
		}

		return out;
	}

	bool constructMove(std::string start, std::string end, IBoard::Move &out)
	{
		bool s = getXy(start, &out.m_from.m_x, &out.m_from.m_y);
		bool e = getXy(end, &out.m_to.m_x, &out.m_to.m_y);

		return s == true && e == true;
	}
};

TESTSUITE(board)
{
	TEST(createBoard, BoardFixture)
	{
		IBoard *p, *other;
		std::string wrongFirst = BOARD_9X9_INIT_STRING;
		std::string wrongChar = BOARD_9X9_INIT_STRING;
		std::string twoKings = BOARD_9X9_INIT_STRING;

		wrongFirst[0] = 'f';
		p = IBoard::fromString(wrongFirst); // Wrong first character type
		ASSERT_TRUE(!p);

		p = IBoard::fromString("Bbroken"); // Wrong count
		ASSERT_TRUE(!p);
		wrongFirst[0] = 'f';

		wrongChar[1] = 'i'; // Not OK
		p = IBoard::fromString(wrongChar); // Wrong character in the middle
		ASSERT_TRUE(!p);

		twoKings[1] = 'k'; // Not OK
		p = IBoard::fromString(twoKings);
		ASSERT_TRUE(!p);

		p = IBoard::fromString(BOARD_9X9_INIT_STRING);
		other = IBoard::fromString(toBoardString(initialBoard9x9));

		ASSERT_TRUE(p);
		ASSERT_TRUE(other);
		ASSERT_TRUE(p->toString() == other->toString());

		delete p;
		delete other;
	}


	TEST(testInvalidMoves, BoardFixture)
	{
		const std::string start =
				"   ...   "
				"    .    "
				"    o    "
				"+   o   ."
				"..ookoo.."
				".   o   ."
				"    o    "
				"    .    "
				"   ...   ";
		const std::string end =
				"   ...   "
				"    .    "
				"    o    "
				"    o+  ."
				"..ookoo.."
				".   o   ."
				"    o    "
				"    .    "
				"   ...   ";

		IBoard *p;
		bool res;

		p = IBoard::fromString(toBoardString(start));
		ASSERT_TRUE(p);

		// Should not be the same
		ASSERT_TRUE(p->getTurn() == IBoard::BLACK);

		// Invalid moves
		IBoard::Move move;
		res = p->canMove(move);
		ASSERT_TRUE(!res);

		move.m_from.m_x = -1;
		res = p->canMove(move);
		ASSERT_TRUE(!res);

		// Not a piece
		move.m_from.m_x = 1;
		res = p->canMove(move);
		ASSERT_TRUE(!res);

		// Move to non-empty
		move.m_from.m_x = 3;
		move.m_to.m_x = 4;
		res = p->canMove(move);
		ASSERT_TRUE(!res);

		// Diagonal movement
		move.m_from.m_x = 3;
		move.m_from.m_y = 0;
		move.m_to.m_x = 2;
		move.m_to.m_y = 1;
		res = p->canMove(move);
		ASSERT_TRUE(!res);

		// Crosses other piece
		res = constructMove(start, end, move);
		ASSERT_TRUE(res);

		res = p->canMove(move);
		ASSERT_TRUE(!res);

		delete p;
	}

	TEST(testValidMoves, BoardFixture)
	{
		const std::string start =
				"   ...   "
				"    .    "
				"    o    "
				"+   o   ."
				"..ookoo.."
				".   o   ."
				"    o    "
				"    .    "
				"   ...   ";
		const std::string end =
				"   ...   "
				"    .    "
				"    o    "
				"   +o   ."
				"..ookoo.."
				".   o   ."
				"    o    "
				"    .    "
				"   ...   ";

		IBoard *p, *other;
		bool res;

		p = IBoard::fromString(toBoardString(start));
		ASSERT_TRUE(p);

		other = IBoard::fromString(toBoardString(end));
		ASSERT_TRUE(other);

		// Valid move
		IBoard::Move move;
		res = constructMove(start, end, move);
		ASSERT_TRUE(res);

		IBoard::Piece piece;
		res = p->getPiece(move.m_from, piece);
		ASSERT_TRUE(res);

		IBoard::MoveList_t possibleMoves = p->getPossibleMoves(piece);
		ASSERT_EQ(possibleMoves.size(), 3 + 3); // Vertical and horizontal

		res = p->canMove(move);
		ASSERT_TRUE(res);

		res = p->doMove(move);
		ASSERT_TRUE(res);

		ASSERT_TRUE(other->toString() == p->toString());
		ASSERT_TRUE(p->getTurn() == IBoard::WHITE);

		delete p;
		delete other;
	}
}
