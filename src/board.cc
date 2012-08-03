#include <iboard.hh>

using namespace tafl;

enum boardData
{
	B_EMPTY  = 0,
	B_CORNER = 1,
	B_BLACK  = 2,
	B_WHITE  = 3,
	B_KING   = 4
};

class Board : public IBoard
{
public:
	Board(std::string str) : m_currentTurn(BLACK), m_h(9), m_w(9)
	{
		if (str.size() == 11 * 11 + 1) {
			m_w = 11;
			m_h = 11;
		}

		m_board = new uint8_t[m_w * m_h];

		for (unsigned int i = 0; i < m_w * m_h; i++)
		{
			switch(str[i + 1])
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

	~Board()
	{
		delete[] m_board;
	}

	Color_t getTurn()
	{
		return m_currentTurn;
	}

	PieceList_t getPieces(Color_t color)
	{
		PieceList_t out;

		return out;
	}

	bool getPiece(Point where, Piece &out)
	{
		if (where.m_x < 0 || where.m_y >= m_w ||
				where.m_y < 0 || where.m_y >= m_h)
			return false;

		uint8_t piece = m_board[where.m_y * m_w + where.m_x];

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
		int sx, sy, x, y;

		sx = x = piece.m_location.m_x;
		sy = y = piece.m_location.m_y;

		// Horizontal
		for (x = sx + 1; x < m_w; x++) {
			if (m_board[sy * m_w + x] != B_EMPTY)
				break;
			out.push_back(Move(sx, sy, x, sy));
		}
		for (x = sx - 1; x >= 0; x--) {
			if (m_board[sy * m_w + x] != B_EMPTY)
				break;
			out.push_back(Move(sx, sy, x, sy));
		}

		// Vertical
		for (y = sy + 1; y < m_w; y++) {
			if (m_board[y * m_w + sx] != B_EMPTY)
				break;
			out.push_back(Move(sx, sy, sx, y));
		}
		for (y = sy - 1; y >= 0; y--) {
			if (m_board[y * m_w + sx] != B_EMPTY)
				break;
			out.push_back(Move(sx, sy, sx, y));
		}


		return out;
	}

	bool canMove(Move &move)
	{
		int sx, sy, dx, dy;

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


		uint8_t start = m_board[sy * m_w + sx];
		uint8_t end = m_board[dx * m_w + dx];

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
				(start != B_WHITE ||
				start != B_KING))
			return false;

		int dirX = dir(dx - sx);
		int dirY = dir(dy - sy);

		// Check the path from sx,sy -> dx,dy
		while (sx != dx || sy != dy) {
			uint8_t next;

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

		int sx, sy, dx, dy;

		moveToXy(move, &sx, &sy, &dx, &dy);
		m_board[dy * m_w + dx] = m_board[sy * m_w + sx];
		m_board[sy * m_w + sx] = B_EMPTY;
		// FIXME! Handle takes

		m_currentTurn = m_currentTurn == BLACK ? WHITE : BLACK;

		return true;
	}


	std::string toString()
	{
		std::string out = "B";

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

private:
	int dir(int v)
	{
		if (v == 0)
			return 0;

		if (v < 0)
			return -1;

		return 1;
	}

	void pointToXy(Point &pt, int *x, int *y)
	{
		*x = pt.m_x;
		*y = pt.m_y;
	}

	void moveToXy(Move &move, int *sx, int *sy, int *dx, int *dy)
	{
		pointToXy(move.m_from, sx, sy);
		pointToXy(move.m_to, dx, dy);
	}


	Color_t m_currentTurn;
	uint8_t *m_board;
	uint8_t m_h;
	uint8_t m_w;
};

IBoard *IBoard::fromString(std::string str)
{
	// Wrong size?
	if (str.size() != 9 * 9 + 1 &&
			str.size() != 11 * 11 + 1)
		return NULL;

	if (str[0] != 'B')
		return NULL;

	// Count kings, check for invalid characters
	unsigned kings = 0;

	for (unsigned i = 1; i < str.size(); i++) {
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

