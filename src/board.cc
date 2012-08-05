#include <iboard.hh>
#include <utils.hh>
#include <string.h>

using namespace tafl;

enum boardData
{
	B_EMPTY  = 0,
	B_CORNER = 1,
	B_BLACK  = 2,
	B_WHITE  = 3,
	B_KING   = 4
};

enum strOffsets
{
	OFF_IDENT     = 0,
	OFF_WIDTH     = 1,
	OFF_HEIGHT    = 3,
	OFF_TURN      = 5,
	OFF_PLAYFIELD = 7,
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
		Board::getIntFromString(str, OFF_WIDTH, &m_w);
		Board::getIntFromString(str, OFF_HEIGHT, &m_h);
		Board::getIntFromString(str, OFF_TURN, &turn);

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

		checkCaptures();

		m_currentTurn = m_currentTurn == BLACK ? WHITE : BLACK;

		return true;
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
			unsigned int x = i % m_w;
			unsigned int y = i / m_w;

			if (m_board[i] != B_BLACK &&
					m_board[i] != B_WHITE &&
					m_board[i] != B_KING)
				continue;

			// Check custodian capture, i.e., flanking a piece on opposite sides
			if (x > 0 && x < m_w - 1)
				captured[i] |= isOppositePiece(x - 1, y, m_board[i]) &&
						isOppositePiece(x + 1, y, m_board[i]);
			if (y > 0 && y < m_h - 1)
				captured[i] |= isOppositePiece(x, y - 1, m_board[i]) &&
						isOppositePiece(x, y + 1, m_board[i]);
		}

		// Commit captures
		for (unsigned int i = 0; i < m_w * m_h; i++) {
			if (captured[i])
				m_board[i] = B_EMPTY;
		}
	}

	bool isOppositePiece(unsigned int x, unsigned int y, boardPiece_t color)
	{
		boardPiece_t piece = m_board[y * m_w + x];

		if (color == B_KING)
			color = B_WHITE;
		if (piece == B_KING)
			piece = B_WHITE;

		if ((color == B_BLACK && piece != B_WHITE) ||
				(color == B_WHITE && piece != B_BLACK))
			return false;

		return true;
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


	static bool getIntFromString(std::string str, unsigned int offset, unsigned int *out)
	{
		std::string in = str.substr(offset, 2);

		char *endp;
		const char *inc = in.c_str();

		*out = strtoul(inc, &endp, 16);
		if (endp == inc || (endp && *endp != '\0'))
			return false;

		return true;
	}

	Color_t m_currentTurn;
	boardPiece_t *m_board;
	unsigned int m_h;
	unsigned int m_w;
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
	res = Board::getIntFromString(str, OFF_WIDTH, &w);
	if (!res)
		return NULL;
	res = Board::getIntFromString(str, OFF_HEIGHT, &h);
	if (!res)
		return NULL;
	res = Board::getIntFromString(str, OFF_TURN, &turn);
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
