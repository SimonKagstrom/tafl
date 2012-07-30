#pragma once

#include <string>

class IBoard
{
public:
	class Point
	{
	public:
	};

	class Move
	{
	public:
		Point m_from;
		Point m_to;
	};



	virtual bool canMove(Move &move) = 0;

	virtual bool doMove(Move &move) = 0;


	virtual std::string toString() = 0;

	static IBoard &fromString(std::string str);
};
