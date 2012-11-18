#include <iai.hh>
#include <iboard.hh>
#include <utils.hh>

#include <string>
#include <list>
#include <map>
#include <stdlib.h>

using namespace tafl;


class AiPlayer
{
public:
	friend class Match;

	AiPlayer(IAi *ai, std::string name) :
		m_ai(ai), m_score(0), m_name(name)
	{
	}

	void mutate(double mutationRate)
	{
		std::list<double> conf = m_ai->getRawConfiguration();
		std::list<double> mutatedConf;

		for (std::list<double>::iterator it = conf.begin();
				it != conf.end();
				it++) {
			double cur = *it;

			cur = cur * ((drand48()-0.5) * mutationRate);

			mutatedConf.push_back(cur);
		}

		m_ai->setRawConfiguration(mutatedConf);
	}

	AiPlayer breed(AiPlayer &other, std::string &name)
	{
		AiPlayer out(IAi::createAi(), name);

		std::list<double> myConf = m_ai->getRawConfiguration();
		std::list<double> otherConf = other.m_ai->getRawConfiguration();
		std::list<double> crossConf;

		panic_if(myConf.size() != otherConf.size(),
				"Configurations of different sizes");

		std::list<double>::iterator it = myConf.begin();
		std::list<double>::iterator itOther = otherConf.begin();
		for (;
				it != myConf.end() && itOther != otherConf.end();
				it++, itOther++) {
			double myCur = *it;
			double otherCur = *itOther;
			double cur = (myCur + otherCur) / 2;

			if ((rand() & 1) == 0)
				cur = cross(myCur, otherCur);
			else
				cur = select(myCur, otherCur);

			crossConf.push_back(cur);
		}

		m_ai->setRawConfiguration(crossConf);

		return out;
	}

	std::string &getName()
	{
		return m_name;
	}

	static std::string generateName(unsigned generation, AiPlayer *father, AiPlayer *mother)
	{
		return fmt("AI_%u_%s_%s", generation, "A", "B");
	}

private:
	double cross(double a, double b)
	{
		return (a + b) / 2;
	}

	double select(double a, double b)
	{
		if ((rand() & 1) == 0)
			return a;

		return b;
	}


	IAi *m_ai;
	unsigned m_score;
	std::string m_name;
};

class Match
{
public:
	Match(AiPlayer *player1, AiPlayer *player2) :
		m_board(IBoard::fromString(BOARD_9X9_INIT_STRING))
	{
		m_players[0] = player1;
		m_players[1] = player2;
	}

	~Match()
	{
		delete m_board;
	}

	bool play(int *whiteScore, int *blackScore)
	{
		unsigned cur = 0;

		*whiteScore = 0;
		*blackScore = 0;

		// Play for maximum 100 rounds
		for (unsigned i = 0; i < 100; i++) {
			AiPlayer *ai = m_players[cur];
			IBoard::Move move = ai->m_ai->getBestMove(*m_board);
			bool res;

			res = m_board->doMove(move);
			panic_if(!res,
					"Can't do move");

			// We have a winner
			IBoard::Color_t winner = m_board->getWinner();
			if (winner != IBoard::BOTH) {
				if (winner == IBoard::WHITE)
					*whiteScore = 3;
				else
					*blackScore = 3;

				return true;
			}

			cur = !cur;
		}

		// If we get here it's a draw, both get 1 point
		*whiteScore = 1;
		*blackScore = 1;

		return false;
	}

	IBoard *m_board;
	AiPlayer *m_players[2];
};

class League
{
public:
	typedef std::list<AiPlayer *> PlayerList_t;
	typedef std::map<AiPlayer *, unsigned> PlayerScore_t;

	class IDisplayListener
	{
	public:
		virtual void onResult(PlayerList_t &sortedPlayers, PlayerScore_t &scores) = 0;
	};

	League()
	{
	}

	void addPlayer(AiPlayer *player)
	{
		m_players.push_back(player);
	}

	void runChampionship()
	{
		for (PlayerList_t::iterator it = m_players.begin();
				it != m_players.end();
				it++) {
			for (PlayerList_t::iterator itOther = m_players.begin();
					itOther != m_players.end();
					itOther++) {
				AiPlayer *cur = *it;
				AiPlayer *other = *itOther;

				// No match against yourself
				if (cur == other)
					continue;

				doMatches(cur, other);
			}
		}
	}

	PlayerList_t getResults()
	{
		return m_playersByScore;
	}

	void registerDisplayListener(IDisplayListener *listener)
	{
		m_displayListeners.push_back(listener);
	}

private:
	void processResults()
	{
		std::map<unsigned, std::list<AiPlayer *>> byScore;

		for (PlayerScore_t::iterator it = m_playerScore.begin();
				it != m_playerScore.end();
				it++) {
			AiPlayer *player = it->first;
			unsigned score = it->second;

			byScore[score].push_back(player);
		}

		m_playersByScore.clear();
		for (std::map<unsigned, std::list<AiPlayer *>>::iterator it = byScore.begin();
				it != byScore.end();
				it++) {
			std::list<AiPlayer *> cur = it->second;

			for (std::list<AiPlayer *>::iterator plIt = cur.begin();
					plIt != cur.end();
					plIt++) {
				AiPlayer *player = *plIt;

				m_playersByScore.push_front(player);
			}
		}

		// Display the results
		for (DisplayListenerList_t::iterator it = m_displayListeners.begin();
				it != m_displayListeners.end();
				it++) {
			(*it)->onResult(m_playersByScore, m_playerScore);
		}
	}

	void doMatch(AiPlayer *first, AiPlayer *second)
	{
		int whiteScore, blackScore;

		// Play a game (score will be updated)
		Match match(first, second);
		match.play(&whiteScore, &blackScore);

		processResults();

		m_playerScore[first] += whiteScore;
		m_playerScore[second] += blackScore;
	}

	void doMatches(AiPlayer *first, AiPlayer *second)
	{
		// Play both home and away
		doMatch(first, second);
		doMatch(second, first);
	}

	typedef std::list<IDisplayListener *> DisplayListenerList_t;

	PlayerList_t m_players;
	PlayerScore_t m_playerScore;
	PlayerList_t m_playersByScore;
	DisplayListenerList_t m_displayListeners;
};

class HtmlGenerator : public League::IDisplayListener
{
public:
	HtmlGenerator()
	{
	}

	virtual ~HtmlGenerator()
	{
	}

	void onResult(League::PlayerList_t &sortedPlayers, League::PlayerScore_t &scores)
	{
		std::string s = header();

		s = s + "<PRE>\n";
		for (League::PlayerList_t::iterator it = sortedPlayers.begin();
				it != sortedPlayers.end();
				it++) {
			AiPlayer *cur = *it;

			s = s + fmt("%s: %u\n", cur->getName().c_str(), scores[cur]);;
		}

		s = s + "</PRE>\n";

		s = s + footer();

		write_file(s.c_str(), s.size(), "/tmp/kalle.html");
	}

private:
	std::string header()
	{
		return "<HTML>\n"
				"<BODY>\n";
	}

	std::string footer()
	{
		return "</BODY>\n"
				"</HTML>\n";
	}
};

class Controller
{
public:
	Controller() :
		m_generation(0), m_nPlayers(8)
	{
	}

	void run()
	{
		League::PlayerList_t players = generateInitialPlayers(m_seed);

		while (1) {
			League league;

			league.registerDisplayListener(&m_output);

			for (League::PlayerList_t::iterator it = players.begin();
					it != players.end();
					it++) {
				league.addPlayer(*it);
			}

			league.runChampionship();
			// Sorted by result
			players = league.getResults();

			// Mutate and cross
			players = survivalOfTheFittest(players);

			m_generation++;
		}
	}

private:
	League::PlayerList_t survivalOfTheFittest(League::PlayerList_t &players)
	{
		unsigned toPop = 4;

		panic_if (players.size() < 4,
				"Need more players than %u", players.size());

		if (players.size() < 6)
			toPop = players.size() - 4;

		for (unsigned i = 0; i < toPop; i++) {
			AiPlayer *p = players.back();

			players.pop_back();

			delete p;
		}

		return players;
	}

	League::PlayerList_t generateInitialPlayers(std::string &seed)
	{
		League::PlayerList_t out;
		IAi *firstAi = NULL;

		if (seed == "")
			firstAi = IAi::fromString(seed);

		if (!firstAi)
			firstAi = IAi::createAi();
		out.push_back(new AiPlayer(firstAi, AiPlayer::generateName(m_generation, NULL, NULL)));

		for (unsigned i = 0; i < m_nPlayers - 1; i++) {
			AiPlayer *cur;

			cur = new AiPlayer(firstAi, AiPlayer::generateName(m_generation, NULL, NULL));
			cur->mutate(0.1);

			out.push_back(cur);
		}

		return out;
	}

	unsigned int m_generation;
	unsigned int m_nPlayers;
	std::string m_seed;
	HtmlGenerator m_output;
};


int main(int argc, const char *argv[])
{
	Controller ctrl;

	ctrl.run();

	return 0;
}
