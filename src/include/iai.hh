#pragma once

#include "iboard.hh"

namespace tafl
{
	class IAi
	{
	public:
		virtual ~IAi() {}

		/**
		 * Evaluate a board.
		 *
		 * @param board the board to evaluate
		 *
		 * @return positive if the board is better for black, 0 if equal, negative
		 * if better for white
		 */
		virtual int evaluate(IBoard &board) = 0;

		virtual IBoard::Move getBestMove(IBoard &board) = 0;

		static IAi *createAi();
	};
}
