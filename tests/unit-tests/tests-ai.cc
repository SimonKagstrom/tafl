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
	}
}
