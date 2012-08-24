#pragma once

#include "iboard.hh"

#include <string>

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
		 * @return positive if the board is better for black, ~0 if equal, negative
		 * if better for white
		 */
		virtual double evaluate(IBoard &board) = 0;

		virtual IBoard::Move getBestMove(IBoard &board) = 0;

		virtual std::string toString() = 0;


		static IAi *createAi();

		static IAi *fromString(std::string &str);
	};
}
