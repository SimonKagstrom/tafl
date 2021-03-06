#include "test.hh"

#include <iai.hh>
#include <utils.hh>

#include "../../src/ai.cc"
#include "board-fixture.hh"

TESTSUITE(ai)
{
	static void displayBoard(IBoard &board)
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

	class Node
	{
	public:
		Node(double value = 0)
		{
			m_value = value;
			m_n = 0;
		}

		void addChild(Node child)
		{
			child.m_parent = this;
			m_children[m_n] = child;
			m_n++;
		}

		Node &getChild(int which)
		{
			return m_children[which];
		}

		std::map<int, Node> m_children;
		Node *m_parent;
		double m_value;
		int m_n;
	};

	static Node g_root;
	static Node *g_curNode;

	class MockBoard : public IBoard
	{
	public:
		MockBoard()
		{
			m_curNode = &g_root;

			const unsigned nodeChildren[] =
			{
					3,
					2, 2, 2,
					3,
					1, 1,
					2, 2, 1,
					2,
					1, 1,
					2, 1, 1,
					2, 2,
					1, 1
			};

			memcpy(m_nodeChildren, nodeChildren, sizeof(m_nodeChildren));
			m_turn = IBoard::BLACK;
		}

		void reset()
		{
			MockBoard::m_idx = 0;
			m_turn = IBoard::BLACK;
			m_curNode = &g_root;
		}

		virtual Color_t getTurn() const
		{
			return m_turn;
		}

		virtual unsigned getDimensions() const
		{
			return 9;
		}


		virtual const PieceList_t getPieces(Color_t color) const
		{
			PieceList_t out;

			out.push_back(Piece());

			return out;
		}

		virtual bool getPiece(const Point &where, Piece &out) const
		{
			return false;
		}


		virtual MoveList_t getPossibleMoves(const Piece &piece)
		{
			MoveList_t out;

			for (unsigned i = 0; i < m_curNode->m_children.size(); i++)
				out.push_back(IBoard::Move(0, 0, i, 0));

			if (m_curNode->m_children.size() == 0)
				m_curNode = m_curNode->m_parent;

			m_idx++;

			return out;
		}


		virtual bool canMove(const Move &move) const
		{
			return true;
		}

		virtual bool doMove(const Move &move)
		{
			if (m_turn == IBoard::BLACK)
				m_turn = IBoard::WHITE;
			else
				m_turn = IBoard::BLACK;

			Node &child = m_curNode->getChild(move.m_to.m_x);
			m_curNode = &child;

			g_curNode = m_curNode;
			if (child.m_value != 0)
				m_curNode = m_curNode->m_parent;

			return true;
		}

		virtual bool undoMove(const Move &move)
		{
			return doMove(move);
		}

		virtual Color_t getWinner() const
		{
			return IBoard::BOTH;
		}

		virtual void registerListener(IListener &listener)
		{
		}

		virtual void unRegisterListener(IListener &listener)
		{
		}

		virtual std::string toString()
		{
			return "";
		}

		virtual std::unique_ptr<IBoard> copy() const
		{
			return std::unique_ptr<IBoard>(new MockBoard(this));
		}

		MockBoard(const MockBoard *copy) :
			m_turn(copy->m_turn),
			m_curNode(copy->m_curNode)
		{
			memcpy(m_nodeChildren, copy->m_nodeChildren, sizeof(m_nodeChildren));
		}

		IBoard::Color_t m_turn;
		static unsigned m_idx;
		unsigned m_nodeChildren[32];

		Node *m_curNode;
	};

	class MockEvaluateAi : public Ai
	{
	public:
		MockEvaluateAi() : Ai()
		{
		}

		void reset()
		{
		}

		virtual double evaluate(IBoard &board)
		{
			double out = g_curNode->m_value;
			printf("Evaluating to %.2f for %s\n",
					out, board.getTurn() == IBoard::WHITE ? "white" : "black");

			return out;
		}
	};



	unsigned MockBoard::m_idx;

	TEST(evaluate, BoardFixture)
	{
		const std::string boardOneStr =
				"   ...   "
				"    .    "
				"    o    "
				".   o   ."
				"..ookoo.."
				".   o   ."
				"    o    "
				"    .    "
				"   ...   ";

		const std::string boardTwoStr =
				"   ...   "
				"    .    "
				"         "
				".       ."
				"..  k  .."
				".   o   ."
				"    o    "
				"    .    "
				"   ...   ";

		const std::string boardThreeStr =
				"         "
				"         "
				"         "
				"o        "
				"..  k    "
				".   o    "
				".o       "
				".        "
				".        ";


		double one, two, three;
		IBoard *boardOne = IBoard::fromString(toBoardString(boardOneStr));
		IBoard *boardTwo = IBoard::fromString(toBoardString(boardTwoStr));
		IBoard *boardThree = IBoard::fromString(toBoardString(boardThreeStr));
		ASSERT_TRUE(boardOne);
		ASSERT_TRUE(boardTwo);
		ASSERT_TRUE(boardThree);

		IAi *ai = IAi::createAi();

		// Board two should be better (positive) for black
		one = ai->evaluate(*boardOne);
		two = ai->evaluate(*boardTwo);
		three = ai->evaluate(*boardThree);

		ASSERT_GT(two, one);

		// Better for white (white is more mobile)
		ASSERT_LT(three, 0);

		delete ai;
	}

	TEST(win, BoardFixture)
	{
		const std::string whiteWinStart =
				". o      "
				"  .      "
				"   o     "
				"  .      "
				"  o K    "
				"    o    "
				"         "
				"         "
				"         ";
		const std::string whiteWinEnd =
				". o      "
				"  .      "
				"   o     "
				"  .      "
				"  o     K"
				"    o    "
				"         "
				"         "
				"         ";

		IBoard *p;
		IBoard::Move move;
		bool res;
		double eval;
		Ai *ai = (Ai *)IAi::createAi();

		p = IBoard::fromString(toBoardString(whiteWinStart, IBoard::WHITE));
		ASSERT_TRUE(p);
		ASSERT_EQ(p->getWinner(), IBoard::BOTH);
		ASSERT_EQ(p->getTurn(), IBoard::WHITE);


		res = constructMove(whiteWinStart, whiteWinEnd, move);
		ASSERT_TRUE(res);


		// Board two should be better (positive) for black
		eval = ai->evaluate(*p);
		ASSERT_TRUE(abs(eval) < 1000);

		res = p->doMove(move);
		ASSERT_TRUE(res);

		// Stupid move by black
		IBoard::Move bm(IBoard::Point(0,0), IBoard::Point(0, 1));
		res = p->doMove(bm);
		ASSERT_TRUE(res);


		// White wins
		ASSERT_EQ(p->getWinner(), IBoard::WHITE);

		eval = ai->evaluate(*p);
		ASSERT_TRUE(eval == -1000);

		delete ai;
	}

	TEST(runAi, BoardFixture, DEADLINE_REALTIME_MS(60000))
	{
		const std::string boardStr =
				"         "
				"         "
				"         "
				"o        "
				"..  k    "
				".   o    "
				".o       "
				".        "
				".        ";


		IBoard *board = IBoard::fromString(toBoardString(boardStr, IBoard::WHITE));
		IAi *ai;
		bool res;

		printf("\n");

		ASSERT_TRUE(board);
		ASSERT_EQ(board->getWinner(), IBoard::BOTH);
		ASSERT_EQ(board->getTurn(), IBoard::WHITE);
		ai = IAi::createAi();

		// Should move the king
		IBoard::Move move = ai->getBestMove(*board);
		displayBoard(*board);
		printf("(%d,%d) -> (%d,%d)\n",
				move.m_from.m_x, move.m_from.m_y,
				move.m_to.m_x, move.m_to.m_y);

		res = board->doMove(move);
		displayBoard(*board);
		ASSERT_TRUE(res);

		// Black move (does not matter now)
		move = ai->getBestMove(*board);
		printf("(%d,%d) -> (%d,%d)\n",
				move.m_from.m_x, move.m_from.m_y,
				move.m_to.m_x, move.m_to.m_y);
		res = board->doMove(move);
		displayBoard(*board);
		ASSERT_TRUE(res);

		// White move, should be a win
		move = ai->getBestMove(*board);
		printf("(%d,%d) -> (%d,%d)\n",
				move.m_from.m_x, move.m_from.m_y,
				move.m_to.m_x, move.m_to.m_y);
		res = board->doMove(move);
		displayBoard(*board);
		ASSERT_TRUE(res);
		ASSERT_EQ(board->getWinner(), IBoard::WHITE);


		delete ai;
	}

	TEST(toString)
	{
		ASSERT_SCOPE_HEAP_LEAK_FREE {
			Ai *ai = (Ai *)IAi::createAi();
			ASSERT_TRUE(ai);

			std::string s = ai->toString();

			Ai *other = (Ai *)IAi::fromString(s);
			ASSERT_TRUE(other);

			delete other;
			delete ai;
		}
	}
}
