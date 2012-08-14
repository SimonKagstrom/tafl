#pragma once

#include "iboard.hh"

namespace tafl
{
	class IAi
	{
	public:
		virtual ~IAi() {}

		virtual int evaluate(IBoard &board) = 0;

		virtual IBoard::Move getBestMove(IBoard &board) = 0;
	};
}
