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
};


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
}
