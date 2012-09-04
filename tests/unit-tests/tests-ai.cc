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

	static void setupNodes()
	{
			// Tier 1
			g_root.addChild(Node());
			g_root.addChild(Node());
			g_root.addChild(Node());

			// Tier 2
			Node &c2_1 = g_root.getChild(0);
			c2_1.addChild(Node());
			c2_1.addChild(Node());
			Node &c2_2 = g_root.getChild(1);
			c2_2.addChild(Node());
			c2_2.addChild(Node());
			Node &c2_3 = g_root.getChild(2);
			c2_3.addChild(Node());
			c2_3.addChild(Node());

			// Tier 3
			Node &c3_1 = c2_1.getChild(0);
			c3_1.addChild(Node());
			c3_1.addChild(Node());

			Node &c3_2 = c2_1.getChild(1);
			c3_2.addChild(Node());

			Node &c3_3 = c2_2.getChild(0);
			c3_3.addChild(Node());
			c3_3.addChild(Node());
			Node &c3_4 = c2_2.getChild(1);
			c3_4.addChild(Node());

			Node &c3_5 = c2_3.getChild(0);
			c3_5.addChild(Node());
			Node &c3_6 = c2_3.getChild(1);
			c3_6.addChild(Node());


			// Tier 4
			Node &c4_1 = c3_1.getChild(0);
			c4_1.addChild(Node(5));
			c4_1.addChild(Node(6));
			Node &c4_2 = c3_1.getChild(1);
			c4_2.addChild(Node(7));
			c4_2.addChild(Node(4));
			c4_2.addChild(Node(5));

			Node &c4_3 = c3_2.getChild(0);
			c4_3.addChild(Node(3));

			Node &c4_4 = c3_3.getChild(0);
			c4_4.addChild(Node(6));
			Node &c4_5 = c3_3.getChild(1);
			c4_5.addChild(Node(6));
			c4_5.addChild(Node(9));

			Node &c4_6 = c3_4.getChild(0);
			c4_6.addChild(Node(7));

			Node &c4_7 = c3_5.getChild(0);
			c4_7.addChild(Node(5));

			Node &c4_8 = c3_6.getChild(0);
			c4_8.addChild(Node(9));
			c4_8.addChild(Node(8));
			Node &c4_9 = c3_6.getChild(1);
			c4_9.addChild(Node(6));
	}

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

		virtual Color_t getTurn()
		{
			return m_turn;
		}

		virtual unsigned getDimensions()
		{
			return 9;
		}


		virtual const PieceList_t getPieces(Color_t color)
		{
			PieceList_t out;

			out.push_back(Piece());

			return out;
		}

		virtual bool getPiece(Point where, Piece &out)
		{
			return false;
		}


		virtual MoveList_t getPossibleMoves(Piece &piece)
		{
			MoveList_t out;

			for (unsigned i = 0; i < m_curNode->m_children.size(); i++)
				out.push_back(IBoard::Move(0, 0, i, 0));

			if (m_curNode->m_children.size() == 0)
				m_curNode = m_curNode->m_parent;

			m_idx++;

			return out;
		}


		virtual bool canMove(Move &move)
		{
			return true;
		}

		virtual bool doMove(Move &move)
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

		virtual Color_t getWinner()
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

		virtual IBoard *copy()
		{
			return (IBoard *)new MockBoard(this);
		}

		MockBoard(MockBoard *copy)
		{
			m_turn = copy->m_turn;
			m_curNode = copy->m_curNode;
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
			// See http://en.wikipedia.org/wiki/File:AB_pruning.svg
			m_maxDepth = 4;
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
		ASSERT_TRUE(abs(eval) < ai->m_configuration[VICTORY]);

		res = p->doMove(move);
		ASSERT_TRUE(res);

		// Stupid move by black
		IBoard::Move bm(IBoard::Point(0,0), IBoard::Point(0, 1));
		res = p->doMove(bm);
		ASSERT_TRUE(res);


		// White wins
		ASSERT_EQ(p->getWinner(), IBoard::WHITE);

		eval = ai->evaluate(*p);
		ASSERT_TRUE(eval == -ai->m_configuration[VICTORY]);

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

	TEST(testAlphaBeta, BoardFixture)
	{
		setupNodes();

		MockBoard *board = new MockBoard();
		MockEvaluateAi *ai = new MockEvaluateAi();
		IBoard::Move move;

		// Test using example from Wikipedia
		ai->m_useAlphaBeta = false;
		move = ai->getBestMove(*board);
		ASSERT_EQ(move.m_to.m_x, 1);

		printf("\n");
		ai->reset();
		board->reset();
		ai->m_useAlphaBeta = true;
		move = ai->getBestMove(*board);
		ASSERT_EQ(move.m_to.m_x, 1);

		delete board;
		delete ai;

		IBoard *realBoard = IBoard::fromString(BOARD_9X9_INIT_STRING);
		Ai *realAi = (Ai *)IAi::createAi();
		IBoard::Move otherMove;

		// Will take too long otherwise
		realAi->m_maxDepth = 2;
		realAi->m_useAlphaBeta = false;

		move = realAi->getBestMove(*realBoard);

		realAi->m_useAlphaBeta = true;
		otherMove = realAi->getBestMove(*realBoard);

		printf("(%d,%d) -> (%d,%d)    vs    (%d,%d) -> (%d,%d)\n",
				move.m_from.m_x, move.m_from.m_y,
				move.m_to.m_x, move.m_to.m_y,
				otherMove.m_from.m_x, otherMove.m_from.m_y,
				otherMove.m_to.m_x, otherMove.m_to.m_y
				);
		ASSERT_EQ(move.m_from.m_x, otherMove.m_from.m_x);
		ASSERT_EQ(move.m_from.m_y, otherMove.m_from.m_y);
		ASSERT_EQ(move.m_to.m_x, otherMove.m_to.m_x);
		ASSERT_EQ(move.m_to.m_y, otherMove.m_to.m_y);
	}
}
