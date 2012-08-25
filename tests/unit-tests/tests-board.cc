#include "test.hh"

#include <iboard.hh>

#include "../../src/board.cc"

#include "board-fixture.hh"

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

TESTSUITE(board)
{
	TEST(createBoard, BoardFixture)
	{
		Board *p, *other, *copy;
		std::string wrongFirst = BOARD_9X9_INIT_STRING;
		std::string wrongChar = BOARD_9X9_INIT_STRING;
		std::string twoKings = BOARD_9X9_INIT_STRING;

		wrongFirst[OFF_IDENT] = 'f';
		p = (Board *)IBoard::fromString(wrongFirst); // Wrong first character type
		ASSERT_TRUE(!p);

		p = (Board *)IBoard::fromString("Bbroken"); // Wrong count
		ASSERT_TRUE(!p);

		wrongChar[OFF_PLAYFIELD] = 'i'; // Not OK
		p = (Board *)IBoard::fromString(wrongChar); // Wrong character in the middle
		ASSERT_TRUE(!p);

		twoKings[OFF_PLAYFIELD] = 'k'; // Not OK
		p = (Board *)IBoard::fromString(twoKings);
		ASSERT_TRUE(!p);

		p = (Board *)IBoard::fromString(BOARD_9X9_INIT_STRING);
		other = (Board *)IBoard::fromString(toBoardString(initialBoard9x9));

		ASSERT_TRUE(p);
		ASSERT_TRUE(other);
		ASSERT_TRUE(p->toString() == other->toString());

		copy = (Board *)IBoard::fromBoard(other);
		ASSERT_TRUE(copy);
		ASSERT_EQ(copy->m_w, other->m_w);
		ASSERT_EQ(copy->m_h, other->m_h);
		ASSERT_EQ(copy->m_currentTurn, other->m_currentTurn);
		ASSERT_TRUE(copy->toString() == other->toString());
		delete copy;

		copy = (Board *)IBoard::fromString(other->toString());
		ASSERT_TRUE(copy);
		ASSERT_EQ(copy->m_w, other->m_w);
		ASSERT_EQ(copy->m_h, other->m_h);
		ASSERT_EQ(copy->m_currentTurn, other->m_currentTurn);
		ASSERT_TRUE(copy->toString() == other->toString());

		delete p;
		delete other;
		delete copy;
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

		// Move the wrong color
		move.m_from.m_x = 4;
		move.m_from.m_y = 2;
		move.m_to.m_x = 5;
		move.m_to.m_y = 2;
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
				"      .  "
				"         "
				"+   o   ."
				"..ookoo.."
				".   o   ."
				"         "
				"    .    "
				"   ...   ";
		const std::string end =
				"   ...   "
				"      .  "
				"         "
				"   +o   ."
				"..ookoo.."
				".   o   ."
				"         "
				"    .    "
				"   ...   ";

		IBoard *p, *other;
		IBoard::Move move;
		IBoard::Piece piece;
		IBoard::MoveList_t possibleMoves;
		bool res;

		p = IBoard::fromString(toBoardString(start));
		ASSERT_TRUE(p);

		other = IBoard::fromString(toBoardString(end, IBoard::WHITE));
		ASSERT_TRUE(other);

		// Valid move
		res = constructMove(start, end, move);
		ASSERT_TRUE(res);

		res = p->getPiece(move.m_from, piece);
		ASSERT_TRUE(res);


		possibleMoves = p->getPossibleMoves(piece);
		ASSERT_EQ(possibleMoves.size(), 3 + 3); // Vertical and horizontal

		res = p->canMove(move);
		ASSERT_TRUE(res);

		res = p->doMove(move);
		ASSERT_TRUE(res);

		ASSERT_TRUE(other->toString() == p->toString());
		ASSERT_TRUE(p->getTurn() == IBoard::WHITE);


		// The king can't move now
		res = p->getPiece(IBoard::Point(4,4), piece);
		ASSERT_TRUE(res);

		possibleMoves = p->getPossibleMoves(piece);
		ASSERT_EQ(possibleMoves.size(), 0);

		res = p->getPiece(IBoard::Point(6,1), piece);
		ASSERT_TRUE(res);

		possibleMoves = p->getPossibleMoves(piece);
		ASSERT_EQ(possibleMoves.size(), 3 + 8);


		// Check that getPices work
		ASSERT_EQ(p->getPieces(IBoard::WHITE).size(), 7);
		ASSERT_EQ(p->getPieces(IBoard::BLACK).size(), 16);
		ASSERT_EQ(p->getPieces(IBoard::BOTH).size(), 7 + 16);

		delete p;
		delete other;
	}

	TEST(testCaptures, BoardFixture)
	{
		// Custodian capture
		const std::string custodianStart =
				"         "
				"         "
				"         "
				"    o    "
				"    k    "
				"    o    "
				"  O      "
				"o.       "
				"         ";
		const std::string custodianEnd =
				"         "
				"         "
				"         "
				"    o    "
				"    k    "
				"    o    "
				"         "
				"o.O      "
				"         ";
		const std::string immobilizationStart =
				".        "
				"oO       "
				"         "
				"    o    "
				"    k    "
				"    o    "
				"  o      "
				"o.       "
				"         ";
		const std::string immobilizationEnd =
				".O       "
				"o        "
				"         "
				"    o    "
				"    k    "
				"    o    "
				"  o      "
				"o.       "
				"         ";
		// Double custodian
		const std::string doubleCustodianStart =
				"  o      "
				"  .      "
				"   O     "
				"  .      "
				"  o k    "
				"    o    "
				"         "
				"         "
				"         ";
		const std::string doubleCustodianEnd =
				"  o      "
				"  .      "
				"  O      "
				"  .      "
				"  o k    "
				"    o    "
				"         "
				"         "
				"         ";

		Board *p;
		IBoard::Move move;
		IBoard::PieceList_t pieces;
		bool res;

		p = (Board *)IBoard::fromString(toBoardString(custodianStart, IBoard::WHITE));
		ASSERT_TRUE(p);

		p->registerListener(*this);

		pieces = p->getPieces(IBoard::BLACK);
		ASSERT_EQ(pieces.size(), 1);


		res = constructMove(custodianStart, custodianEnd, move);
		ASSERT_TRUE(res);

		ASSERT_EQ(m_winner, IBoard::BOTH);

		res = p->doMove(move);
		ASSERT_TRUE(res);
		ASSERT_EQ(m_winner, IBoard::WHITE);

		pieces = p->getPieces(IBoard::BLACK);
		ASSERT_EQ(pieces.size(), 0);

		delete p;


		p = (Board *)IBoard::fromString(toBoardString(doubleCustodianStart, IBoard::WHITE));
		ASSERT_TRUE(p);

		pieces = p->getPieces(IBoard::BLACK);
		ASSERT_EQ(pieces.size(), 2);

		res = constructMove(doubleCustodianStart, doubleCustodianEnd, move);
		ASSERT_TRUE(res);

		res = p->doMove(move);
		ASSERT_TRUE(res);

		pieces = p->getPieces(IBoard::BLACK);
		ASSERT_EQ(pieces.size(), 0);

		delete p;




		p = (Board *)IBoard::fromString(toBoardString(immobilizationStart, IBoard::WHITE));
		ASSERT_TRUE(p);

		pieces = p->getPieces(IBoard::BLACK);
		ASSERT_EQ(pieces.size(), 2);

		res = constructMove(immobilizationStart, immobilizationEnd, move);
		ASSERT_TRUE(res);

		res = p->doMove(move);
		ASSERT_TRUE(res);

		pieces = p->getPieces(IBoard::BLACK);
		ASSERT_EQ(pieces.size(), 1);

		delete p;
	}

	TEST(testWins, BoardFixture)
	{
		const std::string whiteWinStart =
				". o      "
				"  .      "
				"   o     "
				"  .      "
				"  o K    "
				"    o    "
				"         "
				"         "
				"         ";
		const std::string whiteWinEnd =
				". o      "
				"  .      "
				"   o     "
				"  .      "
				"  o     K"
				"    o    "
				"         "
				"         "
				"         ";

		const std::string blackWinStart =
				"  o      "
				"  .      "
				"   o     "
				"     +   "
				"  o.k    "
				"    o    "
				"         "
				"         "
				"         ";
		const std::string blackWinEnd =
				"  o      "
				"  .      "
				"   o     "
				"         "
				"  o.k+   "
				"    o    "
				"         "
				"         "
				"         ";


		Board *p;
		IBoard::Move move;
		bool res;

		p = (Board *)IBoard::fromString(toBoardString(whiteWinStart, IBoard::WHITE));
		ASSERT_TRUE(p);

		p->registerListener(*this);

		res = constructMove(whiteWinStart, whiteWinEnd, move);
		ASSERT_TRUE(res);

		ASSERT_EQ(m_winner, IBoard::BOTH);
		ASSERT_EQ(m_winner, p->m_winner);

		res = p->doMove(move);
		ASSERT_TRUE(res);

		// Not whites turn
		ASSERT_EQ(m_winner, IBoard::BOTH);
		ASSERT_EQ(m_winner, p->m_winner);

		// Stupid move by black
		IBoard::Move bm(IBoard::Point(0,0), IBoard::Point(0, 1));
		res = p->doMove(bm);
		ASSERT_TRUE(res);

		ASSERT_EQ(m_winner, IBoard::WHITE);
		ASSERT_EQ(m_winner, p->m_winner);

		delete p;
		m_winner = IBoard::BOTH;

		p = (Board *)IBoard::fromString(toBoardString(blackWinStart, IBoard::BLACK));
		ASSERT_TRUE(p);
		p->registerListener(*this);

		res = constructMove(blackWinStart, blackWinEnd, move);
		ASSERT_TRUE(res);

		ASSERT_EQ(m_winner, IBoard::BOTH);
		ASSERT_EQ(m_winner, p->m_winner);

		res = p->doMove(move);
		ASSERT_TRUE(res);

		ASSERT_EQ(m_winner, IBoard::BLACK);
		ASSERT_EQ(m_winner, p->m_winner);
	}
}
