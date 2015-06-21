#pragma once

#include "iboard.hh"

#include <string>
#include <list>

namespace tafl
{
	class IAi
	{
	public:
		typedef enum
		{
			MINIMAX,
			ALPHA_BETA
		} Algorithm_t;


		virtual ~IAi() {}

		/**
		 * Evaluate a board.
		 *
		 * @param board the board to evaluate
		 *
		 * @return positive if the board is better for black, ~0 if equal, negative
		 * if better for white
		 */
		virtual double evaluate(IBoard &board) const = 0;

		virtual IBoard::Move getBestMove(IBoard &board) const = 0;

		virtual std::string toString() = 0;


		virtual std::list<double> getRawConfiguration() = 0;

		virtual void setRawConfiguration(std::list<double> conf) = 0;


		static IAi *createAi();

		static IAi *fromAi(IAi *other);

		static IAi *fromString(std::string &str);
	};
}
