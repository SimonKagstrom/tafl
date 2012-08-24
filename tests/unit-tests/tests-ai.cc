#include "test.hh"

#include <iai.hh>
#include <utils.hh>

#include "../../src/ai.cc"
#include "board-fixture.hh"

TESTSUITE(ai)
{
	TEST(evaluate, BoardFixture)
	{
		const std::string boardOneStr =
				"   ...   "
				"    .    "
				"    o    "
				".   o   ."
				"..ookoo.."
				".   o   ."
				"    o    "
				"    .    "
				"   ...   ";

		const std::string boardTwoStr =
				"   ...   "
				"    .    "
				"         "
				".       ."
				"..  k  .."
				".   o   ."
				"    o    "
				"    .    "
				"   ...   ";

		const std::string boardThreeStr =
				"         "
				"         "
				"         "
				"o        "
				"..  k    "
				".   o    "
				".o       "
				".        "
				".        ";


		double one, two, three;
		IBoard *boardOne = IBoard::fromString(toBoardString(boardOneStr));
		IBoard *boardTwo = IBoard::fromString(toBoardString(boardTwoStr));
		IBoard *boardThree = IBoard::fromString(toBoardString(boardThreeStr));
		ASSERT_TRUE(boardOne);
		ASSERT_TRUE(boardTwo);
		ASSERT_TRUE(boardThree);

		IAi *ai = IAi::createAi();

		// Board two should be better (positive) for black
		one = ai->evaluate(*boardOne);
		two = ai->evaluate(*boardTwo);
		three = ai->evaluate(*boardThree);

		ASSERT_GT(two, one);

		// Better for white (white is more mobile)
		ASSERT_LT(three, 0);

		delete ai;
	}

	TEST(win, BoardFixture)
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

		IBoard *p;
		IBoard::Move move;
		bool res;
		double eval;
		Ai *ai = (Ai *)IAi::createAi();

		p = IBoard::fromString(toBoardString(whiteWinStart, IBoard::WHITE));
		ASSERT_TRUE(p);
		ASSERT_EQ(p->getWinner(), IBoard::BOTH);
		ASSERT_EQ(p->getTurn(), IBoard::WHITE);


		res = constructMove(whiteWinStart, whiteWinEnd, move);
		ASSERT_TRUE(res);


		// Board two should be better (positive) for black
		eval = ai->evaluate(*p);
		ASSERT_TRUE(abs(eval) < ai->m_configuration[VICTORY]);

		res = p->doMove(move);
		ASSERT_TRUE(res);

		// Stupid move by black
		IBoard::Move bm(IBoard::Point(0,0), IBoard::Point(0, 1));
		res = p->doMove(bm);
		ASSERT_TRUE(res);


		// White wins
		ASSERT_EQ(p->getWinner(), IBoard::WHITE);

		eval = ai->evaluate(*p);
		ASSERT_TRUE(eval == -ai->m_configuration[VICTORY]);

		delete ai;
	}
}
