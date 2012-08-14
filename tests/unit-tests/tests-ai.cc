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


		int one, two;
		IBoard *boardOne = IBoard::fromString(toBoardString(boardOneStr));
		IBoard *boardTwo = IBoard::fromString(toBoardString(boardTwoStr));
		ASSERT_TRUE(boardOne);
		ASSERT_TRUE(boardTwo);

		IAi *ai = IAi::createAi();

		// Board two should be better (positive) for black
		one = ai->evaluate(*boardOne);
		two = ai->evaluate(*boardTwo);

		ASSERT_GT(two, one);
	}
}
