#pragma once

#include <string>
#include <list>

// To create the initial board
#define BOARD_9X9_INIT_STRING "B090902eeebbbeeeeeeebeeeeeeeeweeeebeeeweeebbbwwkwwbbbeeeweeebeeeeweeeeeeeebeeeeeeebbbeee"

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

		class IListener
		{
		public:
			virtual void onGameEnd(Color_t winner) = 0;
		};

		class Point
		{
		public:
			Point(unsigned int x, unsigned int y) : m_x(x), m_y(y)
			{
			}

			unsigned int m_x;
			unsigned int m_y;
		};

		class Move
		{
		public:
			Move() :
				m_from(Point(0, 0)), m_to(Point(0, 0))
			{
			}

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
			Piece() : m_color(BLACK), m_location(Point(0,0))
			{
			}

			Color_t m_color;
			Point m_location;
		};

		typedef std::list<Piece> PieceList_t;
		typedef std::list<Move> MoveList_t;



		virtual ~IBoard()
		{
		};


		virtual Color_t getTurn() = 0;

		virtual int getDimensions() = 0;


		virtual PieceList_t getPieces(Color_t color) = 0;

		virtual bool getPiece(Point where, Piece &out) = 0;


		virtual MoveList_t getPossibleMoves(Piece &piece) = 0;


		virtual bool canMove(Move &move) = 0;

		virtual bool doMove(Move &move) = 0;


		virtual void registerListener(IListener &listener) = 0;

		virtual void unRegisterListener(IListener &listener) = 0;


		virtual std::string toString() = 0;


		static IBoard *fromString(std::string str);

		static IBoard *fromBoard(IBoard *other);
	};
}
