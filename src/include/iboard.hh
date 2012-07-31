#pragma once

#include <string>
#include <list>

// To create the initial board
#define BOARD_9X9_INIT_STRING "Binit9x9"

namespace tafl
{
	class IBoard
	{
	public:
		typedef enum
		{
			WHITE = 1,
			BLACK = 2,

			BOTH =  3
		} Color_t;

		class Point
		{
		public:
			Point(int x, int y) : m_x(x), m_y(y)
			{
			}

			int m_x;
			int m_y;
		};

		class Move
		{
		public:
			Move(int sx, int sy, int dx, int dy) :
				m_from(Point(sx, sy)), m_to(Point(dx, dy))
			{
			}

			Move(Point from, Point to) : m_from(from), m_to(to)
			{
			}

			Point m_from;
			Point m_to;
		};


		class Piece
		{
		public:
			Color_t m_color;
			Point m_location;
		};

		typedef std::list<Piece> PieceList_t;
		typedef std::list<Move> MoveList_t;



		virtual ~IBoard()
		{
		};


		virtual Color_t getTurn() = 0;

		virtual PieceList_t getPieces(Color_t color) = 0;

		virtual bool getPiece(Point where, Piece &out) = 0;


		virtual MoveList_t getPossibleMoves(Piece piece) = 0;


		virtual bool canMove(Move &move) = 0;

		virtual bool doMove(Move &move) = 0;


		virtual std::string toString() = 0;

		static IBoard *fromString(std::string str);
	};
}
