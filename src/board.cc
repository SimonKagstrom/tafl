/*
 * Implementation of the Tafl board - movement, captures etc.
 *
 * The rules are taken from Joanna Zoryas article,
 *
 *   http://www.martialtaichi.co.uk/articles/Tafl_Rules.pdf
 *
 * With the following exception:
 *
 *   "if a pair or a group become surrounded, one of the pieces
 *    may be removed each turn"
 *
 * is not implemented.
 */
#include <iboard.hh>
#include <utils.hh>

using namespace tafl;

enum boardData
{
	B_EMPTY  = 'e',
	B_CORNER = 'c',
	B_BLACK  = 'b',
	B_WHITE  = 'w',
	B_KING   = 'k'
};

enum strOffsets
{
	OFF_IDENT     = 0,
	OFF_WIDTH     = 1,
	OFF_HEIGHT    = 3,
	OFF_TURN      = 5,
	OFF_PLAYFIELD = 7,
};

enum directions
{
	DIR_LEFT  = 0,
	DIR_RIGHT = 1,
	DIR_UP    = 2,
	DIR_DOWN  = 3,

	N_DIRS
};

typedef uint8_t boardPiece_t;

class Board : public IBoard
{
public:
	friend class IBoard;

	Board(std::string str) : m_currentTurn(BLACK)
	{
		unsigned int turn;

		// We've already checked this
		getIntFromString(str, OFF_WIDTH, &m_w);
		getIntFromString(str, OFF_HEIGHT, &m_h);
		getIntFromString(str, OFF_TURN, &turn);

		if (turn == 1)
			m_currentTurn = WHITE;

		m_board = new boardPiece_t[m_w * m_h];

		for (unsigned int i = 0; i < m_w * m_h; i++)
		{
			switch(str[i + OFF_PLAYFIELD])
			{
			case 'w':
				m_board[i] = B_WHITE;
				break;
			case 'k':
				m_board[i] = B_KING;
				break;
			case 'b':
				m_board[i] = B_BLACK;
				break;
			case 'e':
				m_board[i] = B_EMPTY;
				break;
			case 'x':
				m_board[i] = B_CORNER;
				break;

			default:
				break;
			}
		}
	}

	Board(Board *other) :
		m_currentTurn(other->m_currentTurn), m_h(other->m_h), m_w(other->m_w)
	{
		m_board = new boardPiece_t[m_h * m_w];

		memcpy(m_board, other->m_board, sizeof(boardPiece_t) * m_h * m_w);
	}

	~Board()
	{
		delete[] m_board;
	}

	Color_t getTurn()
	{
		return m_currentTurn;
	}

	unsigned getDimensions()
	{
		return m_w;
	}


	PieceList_t getPieces(Color_t color)
	{
		PieceList_t out;

		for (unsigned int i = 0; i < m_w * m_h; i++) {
			Piece piece;
			bool res;

			res = getPiece(Point(i % m_w, i / m_h), piece);
			if (res && color == BOTH)
				out.push_back(piece);
			else if (res && piece.m_color == color)
				out.push_back(piece);
		}

		return out;
	}

	bool getPiece(Point where, Piece &out)
	{
		if (where.m_x < 0 || where.m_y >= m_w ||
				where.m_y < 0 || where.m_y >= m_h)
			return false;

		boardPiece_t piece = m_board[where.m_y * m_w + where.m_x];

		if (piece != B_BLACK &&
				piece != B_WHITE &&
				piece != B_KING)
			return false;

		out.m_color = piece == B_BLACK ? BLACK : WHITE;
		out.m_location = where;

		return true;
	}


	MoveList_t getPossibleMoves(Piece &piece)
	{
		MoveList_t out;
		unsigned int sx, sy, x, y;

		sx = x = piece.m_location.m_x;
		sy = y = piece.m_location.m_y;

		// Horizontal
		for (x = sx + 1; x < m_w; x++) {
			if (m_board[sy * m_w + x] != B_EMPTY)
				break;
			out.push_back(Move(sx, sy, x, sy));
		}
		for (x = sx - 1; x >= 0 && x < m_w; x--) {
			if (m_board[sy * m_w + x] != B_EMPTY)
				break;
			out.push_back(Move(sx, sy, x, sy));
		}

		// Vertical
		for (y = sy + 1; y >= 0 && y < m_h; y++) {
			if (m_board[y * m_w + sx] != B_EMPTY)
				break;
			out.push_back(Move(sx, sy, sx, y));
		}
		for (y = sy - 1; y >= 0 && y < m_h; y--) {
			if (m_board[y * m_w + sx] != B_EMPTY)
				break;
			out.push_back(Move(sx, sy, sx, y));
		}


		return out;
	}

	bool canMove(Move &move)
	{
		unsigned int sx, sy, dx, dy;

		moveToXy(move, &sx, &sy, &dx, &dy);

		// Out of bounds
		if (sx >= m_w || sx < 0 ||
				sy >= m_h || sy < 0 ||
				dx >= m_w || dx < 0 ||
				dy >= m_h || dy < 0)
			return false;

		// Move like a rook from the current tile
		if ( !((sy-dy == 0 && abs(sx - dx) >= 1) ||
				(sx-dx == 0 && abs(sy - dy) >= 1) ))
			return false;


		boardPiece_t start = m_board[sy * m_w + sx];
		boardPiece_t end = m_board[dy * m_w + dx];

		// Not a piece
		if (start != B_BLACK &&
				start != B_WHITE &&
				start != B_KING)
			return false;

		if (end != B_EMPTY ||
				(end == B_CORNER && start != B_KING))
			return false;

		if (m_currentTurn == BLACK &&
				start != B_BLACK)
			return false;

		if (m_currentTurn == WHITE &&
				(start != B_WHITE &&
				start != B_KING))
			return false;

		int dirX = dir(dx - sx);
		int dirY = dir(dy - sy);

		// Check the path from sx,sy -> dx,dy
		while (sx != dx || sy != dy) {
			boardPiece_t next;

			sx = sx + dirX;
			sy = sy + dirY;
			next = m_board[sy * m_w + sx];

			if (next != B_EMPTY)
				return false;
		}

		return true;
	}

	bool doMove(Move &move)
	{
		if (!canMove(move))
			return false;

		unsigned int sx, sy, dx, dy;

		moveToXy(move, &sx, &sy, &dx, &dy);
		m_board[dy * m_w + dx] = m_board[sy * m_w + sx];
		m_board[sy * m_w + sx] = B_EMPTY;

		m_currentTurn = m_currentTurn == BLACK ? WHITE : BLACK;

		checkCaptures();

		return true;
	}


	void registerListener(IListener &listener)
	{
		m_listeners.push_back(&listener);
	}

	void unRegisterListener(IListener &listener)
	{
		for (ListenerList_t::iterator it = m_listeners.begin();
				it != m_listeners.end();
				it++) {
			if (*it == &listener)
				m_listeners.erase(it);
		}
	}

	std::string toString()
	{
		std::string out = "B" + fmt("%02x%02x%02x", m_w, m_h, m_currentTurn);

		for (unsigned int i = 0; i < m_w * m_h; i++) {
			switch (m_board[i])
			{
			case B_WHITE:
				out += 'w';
				break;
			case B_KING:
				out += 'k';
				break;
			case B_BLACK:
				out += 'b';
				break;
			case B_EMPTY:
				out += 'e';
				break;
			case B_CORNER:
				out += 'x';
				break;
			default:
				// Something's broken
				return std::string("");
			}
		}

		return out;
	}

// For the unit test...
//private:
	void checkCaptures()
	{
		bool captured[m_w * m_h];

		memset(captured, false, sizeof(bool) * m_w * m_h);

		for (unsigned int i = 0; i < m_w * m_h; i++) {
			boardPiece_t adjacent[N_DIRS];
			boardPiece_t piece = m_board[i];
			boardPiece_t opponentColor;
			unsigned int x = i % m_w;
			unsigned int y = i / m_w;

			if (piece != B_BLACK &&
					piece != B_WHITE &&
					piece != B_KING)
				continue;

			if (piece == B_BLACK) {
				opponentColor = B_WHITE;
			} else {
				opponentColor = B_BLACK;
				piece = B_WHITE;
			}

			getAdjacent(x, y, adjacent);

			// Check custodian capture, i.e., flanking a piece on opposite sides
			if (adjacent[DIR_LEFT] == opponentColor &&
					adjacent[DIR_RIGHT] == opponentColor)
				captured[i] = true;
			if (adjacent[DIR_UP] == opponentColor &&
					adjacent[DIR_DOWN] == opponentColor)
				captured[i] = true;

			// Check immobilization capture
			bool immobilized = true;
			for (unsigned int dir = 0; dir < N_DIRS; dir++) {
				if (adjacent[dir] == piece)
					immobilized = false;
				else if (adjacent[dir] == B_EMPTY)
					immobilized = false;
			}

			if (immobilized)
				captured[i] = true;
		}

		// Commit captures, count pieces
		Color_t winner = BOTH; // Well, none of them
		bool kingAlive = false;
		unsigned int nBlack = 0;

		for (unsigned int i = 0; i < m_w * m_h; i++) {
			unsigned int x = i % m_w;
			unsigned int y = i / m_w;

			if (captured[i])
				m_board[i] = B_EMPTY;

			if (m_board[i] == B_BLACK)
				nBlack++;

			if (m_board[i] == B_KING) {
				boardPiece_t adjacent[N_DIRS];

				kingAlive = true;
				if (m_currentTurn != IBoard::WHITE)
					continue;

				getAdjacent(x, y, adjacent);
				if ((x == 0 || x == m_w - 1) &&
						((adjacent[DIR_UP] == B_EMPTY) ||
						(adjacent[DIR_DOWN] == B_EMPTY)))
					winner = WHITE;
				if ((y == 0 || y == m_h - 1) &&
						((adjacent[DIR_LEFT] == B_EMPTY) ||
						(adjacent[DIR_RIGHT] == B_EMPTY)))
					winner = WHITE;
			}
		}

		if (!kingAlive)
			winner = BLACK;
		if (nBlack == 0)
			winner = WHITE;

		// We have a winner!
		if (winner != BOTH) {
			for (ListenerList_t::iterator it = m_listeners.begin();
					it != m_listeners.end();
					it++)
				(*it)->onGameEnd(winner);
		}
	}

	void getAdjacent(unsigned int x, unsigned int y, boardPiece_t *out)
	{
		if (x < 1)
			out[DIR_LEFT] = B_CORNER;
		else
			out[DIR_LEFT] = m_board[y * m_w + (x - 1)];

		if (x > m_w - 1)
			out[DIR_RIGHT] = B_CORNER;
		else
			out[DIR_RIGHT] = m_board[y * m_w + (x + 1)];


		if (y < 1)
			out[DIR_UP] = B_CORNER;
		else
			out[DIR_UP] = m_board[(y - 1) * m_w + x];

		if (x > m_w - 1)
			out[DIR_DOWN] = B_CORNER;
		else
			out[DIR_DOWN] = m_board[(y + 1) * m_w + x];

		for (unsigned int i = 0; i < N_DIRS; i++) {
			if (out[i] == B_KING)
				out[i] = B_WHITE;
		}
	}

	int dir(int v)
	{
		if (v == 0)
			return 0;

		if (v < 0)
			return -1;

		return 1;
	}

	void pointToXy(Point &pt, unsigned int *x, unsigned int *y)
	{
		*x = pt.m_x;
		*y = pt.m_y;
	}

	void moveToXy(Move &move, unsigned int *sx, unsigned int *sy, unsigned int *dx, unsigned int *dy)
	{
		pointToXy(move.m_from, sx, sy);
		pointToXy(move.m_to, dx, dy);
	}



	typedef std::list<IListener *> ListenerList_t;

	Color_t m_currentTurn;
	boardPiece_t *m_board;
	unsigned int m_h;
	unsigned int m_w;

	ListenerList_t m_listeners;
};

IBoard *IBoard::fromString(std::string str)
{
	// Too small?
	if (str.size() < 7 * 7 + OFF_PLAYFIELD)
		return NULL;

	if (str[OFF_IDENT] != 'B')
		return NULL;

	unsigned int w, h, turn;
	bool res;
	res = getIntFromString(str, OFF_WIDTH, &w);
	if (!res)
		return NULL;
	res = getIntFromString(str, OFF_HEIGHT, &h);
	if (!res)
		return NULL;
	res = getIntFromString(str, OFF_TURN, &turn);
	if (!res)
		return NULL;

	// Too short string
	if (str.size() != w * h + OFF_PLAYFIELD)
		return NULL;

	if (turn != IBoard::WHITE && turn != IBoard::BLACK)
		return NULL;

	// Count kings, check for invalid characters
	unsigned kings = 0;

	for (unsigned i = OFF_PLAYFIELD; i < str.size(); i++) {
		switch (str[i])
		{
		case 'k':
			kings++;
			break;
		case 'w':
		case 'e':
		case 'b':
		case 'x':
			break;
		default:

			// Invalid character
			return NULL;
		}
	}

	// One king to rule them all, not more
	if (kings != 1)
		return NULL;

	return new Board(str);
}

IBoard *IBoard::fromBoard(IBoard *other)
{
	return new Board((Board *)other);
}
