#pragma once

#include <string>
#include <vector>

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
			Piece() : m_color(BLACK), m_location(Point(0,0)), m_isKing(false)
			{
			}

			Color_t m_color;
			Point m_location;
			bool m_isKing;
		};

		typedef std::vector<Piece> PieceList_t;
		typedef std::vector<Move> MoveList_t;



		virtual ~IBoard()
		{
		};


		virtual Color_t getTurn() const = 0;

		virtual unsigned getDimensions() const = 0;


		virtual const PieceList_t getPieces(Color_t color) const = 0;

		virtual bool getPiece(const Point &where, Piece &out) const = 0;


		virtual MoveList_t getPossibleMoves(const Piece &piece) = 0;


		virtual bool canMove(const Move &move) const = 0;

		virtual bool doMove(const Move &move) = 0;


		/**
		 * Get the winner
		 *
		 * @return BOTH if there is no winner, the color otherwise
		 */
		virtual Color_t getWinner() const = 0;

		virtual void registerListener(IListener &listener) = 0;

		virtual void unRegisterListener(IListener &listener) = 0;


		virtual std::string toString() = 0;

		virtual IBoard *copy() const = 0;


		static IBoard *fromString(std::string str);
	};
}
