#include <iai.hh>
#include <utils.hh>

#include <math.h>
#include <string.h>

#include <algorithm>
#include <map>
#include <chrono>
#include <ctime>

using namespace tafl;

#define min(x, y) ( (x) < (y) ? (x) : (y) )
#define max(x, y) ( (x) > (y) ? (x) : (y) )

enum boardPieces
{
	P_EMPTY,
	P_CORNER,
	P_BLACK,
	P_WHITE,
	P_KING,
};


enum directions
{
	DIR_LEFT  = 0,
	DIR_RIGHT = 1,
	DIR_UP    = 2,
	DIR_DOWN  = 3,

	N_DIRS
};


class Ai : public IAi
{
public:
	Ai()
	{
	}

	virtual double evaluate(IBoard &board) const
	{
		auto winner = board.getWinner();
		double out = 0;

		// Check winner
		if (winner == IBoard::BLACK) {
			out += 1000;
		} else if (winner == IBoard::WHITE) {
			out -= 1000;
		} else {
			// Count pieces
			out += board.getPieces(IBoard::BLACK).size();
			out -= board.getPieces(IBoard::WHITE).size();
		}

		return out;
	}

	IBoard::Move getBestMove(IBoard &board) const
	{
		double seconds = 1.5;
		return runMonteCarlo(board, seconds * 1000 * 1000);
	}

	std::string toString()
	{
		return "A";
	}

	std::list<double> getRawConfiguration()
	{
		std::list<double> out;

		return out;
	}

	void setRawConfiguration(std::list<double> conf)
	{
	}

 private:
	// Holder for statistics about the game
	class GameStats
	{
	public:
		// Default for non-played games
		GameStats() :
			m_winner(IBoard::BOTH), m_plys(1000000), m_boardEval(0)
		{
		}

		GameStats(const GameStats &other) :
			m_winner(other.m_winner), m_plys(other.m_plys), m_boardEval(other.m_boardEval)
		{
		}

		GameStats(const IBoard::Color_t winner, unsigned int plys, double eval) :
			m_winner(winner),
			m_plys(plys),
			m_boardEval(eval)
		{
		}

		// Positive for who?
		bool compareForColor(IBoard::Color_t who, const GameStats &other) const
		{
			if (m_winner == who && other.m_winner != who)
				return true;
			else if (m_winner != who && other.m_winner == who)
				return false;

			auto diff = m_boardEval - other.m_boardEval;

			if (who == IBoard::BLACK)
				return diff >= 0;

			return diff < 0;
		}

		GameStats &operator=(const GameStats &other)
		{
			m_winner = other.m_winner;
			m_plys = other.m_plys;
			m_boardEval = other.m_boardEval;

			return *this;
		}

		// Positive for black, negative for white
		bool operator<(const GameStats &other) const
		{
			if (m_winner != other.m_winner) {
				if (m_winner == IBoard::WHITE)
					return false;
				else if (m_winner == IBoard::BLACK)
					return true;
			}

			return m_boardEval - other.m_boardEval < 0;
		}

		IBoard::Color_t m_winner;
		unsigned int m_plys;
		double m_boardEval;
	};


	void display(const IBoard &board) const
	{
		unsigned size = board.getDimensions() * board.getDimensions();
		char field[size];

		memset(field, ' ', size);

		const IBoard::PieceList_t pieces = board.getPieces(IBoard::BOTH);

		for (IBoard::PieceList_t::const_iterator it = pieces.begin();
				it != pieces.end();
				it++) {
			IBoard::Piece piece = *it;
			char chr = '.';

			if (piece.m_color != IBoard::BLACK) {
				if (piece.m_isKing)
					chr = 'k';
				else
					chr = 'o';
			}

			field[piece.m_location.m_y * board.getDimensions() + piece.m_location.m_x] = chr;
		}

		printf("###########\n");
		for (unsigned y = 0; y < board.getDimensions(); y++) {
			printf("#");
			for (unsigned x = 0; x < board.getDimensions(); x++) {
				printf("%c", field[y * board.getDimensions() + x]);
			}
			printf("#\n");
		}
		printf("###########\n");
	}

	IBoard::Move runMonteCarlo(IBoard &board, unsigned int deadlineUs) const
	{
		std::map<GameStats, IBoard::Move> bestMoves;

		auto turn = board.getTurn();

		unsigned int nMoves = 0;
		// Run games on all possible moves
		auto pieces = board.getPieces(board.getTurn());
		for (const auto &it : pieces) {
			auto moves = board.getPossibleMoves(it);

			nMoves += moves.size();
		}
		auto deadlinePerMove = deadlineUs / nMoves;

		pieces = board.getPieces(board.getTurn());
		for (const auto &it : pieces) {

			auto moves = board.getPossibleMoves(it);

			for (const auto &move : moves) {
				auto cpy = board.copy();
				cpy->doMove(move);

				auto best = runRandomGames(*cpy, deadlinePerMove);
				bestMoves[best] = move;
			}
		}

		// Should not really happen...
		if (bestMoves.empty()) {
			printf("WARNING: No valid moves???\n");
			return IBoard::Move(0,0,0,0);
		}

		// Return the maximum or minimum move
		if (turn == IBoard::BLACK) {
			auto it = bestMoves.end();
			--it;
			return it->second;
		}

		return bestMoves.begin()->second;
 	}

	GameStats runRandomGames(IBoard &board, unsigned int deadlineUs) const
	{
		GameStats bestGame;

		/*
		 * Actually maximise for !who - the board is after the move of the
		 * current player.
		 */
		auto who = board.getTurn();
		if (who == IBoard::WHITE)
			who = IBoard::BLACK;
		else
			who = IBoard::WHITE;


		unsigned n = 0;
		auto before = std::chrono::high_resolution_clock::now();
		while (1) {
			auto res = runRandomGame(board, 0);

//				printf("WINNER %s for %s\n",
//						res.m_winner == IBoard::BLACK ? "black" : "white",
//								who == IBoard::BLACK ? "black" : "white");
			if (bestGame.m_winner == IBoard::BOTH)
				bestGame = res;
			else if (res.compareForColor(who, bestGame))
				bestGame = res;

			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> seconds = std::chrono::duration_cast<std::chrono::duration<double>>(now - before);
			n++;
			if (seconds.count() * 1000 * 1000 > deadlineUs)
				break;
		}

//		printf("RAN %u games\n", n);
		return bestGame;
	}

	/*
	 * Run random games for deadlineUs microseconds
	 */
	GameStats runRandomGame(IBoard &board, unsigned int ply) const
	{
		auto winner = board.getWinner();

		// We have a winner!
		if (winner != IBoard::BOTH)
			return GameStats(winner, ply, evaluate(board));

		// Don't recurse too far
		if (ply >= 200)
			return GameStats(winner, ply, evaluate(board));

		auto pieces = board.getPieces(board.getTurn());

		// No pieces (can't really happen, but let's say it's a draw)
		if (pieces.size() == 0)
			return GameStats(IBoard::BOTH, ply, evaluate(board));

		// Find a piece which has valid moves
		IBoard::MoveList_t moves;
		while (1)
		{
			moves = board.getPossibleMoves(pieces[rand() % pieces.size()]);
			if (moves.size() != 0)
				break;
		}

		// Select a move at random and try that
		auto move = moves[rand() % moves.size()];

		// Perform the move, try next ply and then restore the board
		auto cpy = board.copy();
		cpy->doMove(move);

		auto out = runRandomGame(*cpy, ply + 1);

		return out;
	}
};

IAi *IAi::createAi()
{
	return new Ai();
}

IAi *IAi::fromAi(IAi *other)
{
	std::string str = other->toString();

	return IAi::fromString(str);
}

IAi *IAi::fromString(std::string &str)
{
	if (str.size() < 1)
		return NULL;

	if (str[0] != 'A')
		return NULL;

	Ai *out = new Ai();

	return out;
}
