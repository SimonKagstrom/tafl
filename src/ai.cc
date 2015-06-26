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

static const unsigned maxPly = 200;

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

// unit tests
namespace ai
{
	class gameStats;
}

class Ai : public IAi
{
public:
	friend class ai::gameStats;

	Ai()
	{
	}

	virtual double evaluate(IBoard &board) const
	{
		auto winner = board.getWinner();
		double out = 0;

		// Check winner
		if (winner == IBoard::BLACK)
			return 1000;
		else if (winner == IBoard::WHITE)
			return -1000;

		auto black = board.getPieces(IBoard::BLACK);
		auto white = board.getPieces(IBoard::WHITE);

		// Count pieces
		out += black.size() * 4;
		out -= white.size() * 4;

		// Being able to move is good
		// FIXME! This is inefficient
		for (const auto &it : black)
			out += board.getPossibleMoves(it).size();
		for (const auto &it : white)
			out -= board.getPossibleMoves(it).size();

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
		GameStats() : m_plys(maxPly), m_boardEval(0)
		{
		}

		GameStats(const GameStats &other) :
			m_plys(other.m_plys), m_boardEval(other.m_boardEval)
		{
		}

		GameStats(unsigned int plys, double eval) :
			m_plys(plys),
			m_boardEval(eval)
		{
		}

		// Positive for who?
		bool compareForColor(IBoard::Color_t who, const GameStats &other) const
		{
			auto diff = m_boardEval - other.m_boardEval;

			if (diff != 0) {
				if (who == IBoard::BLACK)
					return diff > 0;

				return diff < 0;
			}

			return m_plys < other.m_plys;
		}

		GameStats &operator=(const GameStats &other)
		{
			m_plys = other.m_plys;
			m_boardEval = other.m_boardEval;

			return *this;
		}

		int m_plys;
		double m_boardEval;
	};

	IBoard::Move runMonteCarlo(IBoard &board, unsigned int deadlineUs) const
	{
		std::map<double, IBoard::Move> bestMoves;

		/* Black evals to positive, but is negated here to
		 * make the first entry in the map be the best */
		auto turn = board.getTurn();
		int sign = turn == IBoard::BLACK ? -1 : 1;


		// Run games on all possible moves
		unsigned int nMoves = 0;
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
				// Take board eval and plys-for-value in account
				double order = best.m_boardEval * sign + (1 - maxPly / (double)best.m_plys) * sign;

				bestMoves[order] = move;
			}
		}

		// Should not really happen...
		if (bestMoves.empty()) {
			printf("WARNING: No valid moves???\n");
			return IBoard::Move(0,0,0,0);
		}

		// Return the minimum value (as per sign)
		auto it = bestMoves.begin();

		return it->second;
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

			if (res.compareForColor(who, bestGame))
				bestGame = res;

			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> seconds = std::chrono::duration_cast<std::chrono::duration<double>>(now - before);
			n++;
			if (seconds.count() * 1000 * 1000 > deadlineUs)
				break;
		}

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
			return GameStats(ply, evaluate(board));

		// Don't recurse too far
		if (ply >= maxPly - 1)
			return GameStats(ply, evaluate(board));

		auto pieces = board.getPieces(board.getTurn());

		// No pieces (can't really happen, but let's say it's a draw)
		if (pieces.size() == 0)
			return GameStats(ply, evaluate(board));

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

		// Perform the move on a copy
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
