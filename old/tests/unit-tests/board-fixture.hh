#pragma once

#include <iboard.hh>

using namespace tafl;

class BoardFixture : public IBoard::IListener
{
public:
	BoardFixture() : m_winner(IBoard::BOTH)
	{
	}

	virtual ~BoardFixture()
	{
	}

	std::string toBoardString(const std::string &gfx, IBoard::Color_t color = IBoard::BLACK)
	{
		// Invalid
		if (gfx.size() != 9 * 9 &&
				gfx.size() != 11 * 11)
			return std::string("");

		std::string out = fmt("B0909%02d", color);
		for (unsigned int i = 0; i < gfx.size(); i++) {
			switch (gfx[i])
			{
			case 'o':
			case 'O':
				out += "w";
				break;
			case '.':
			case '+':
				out += "b";
				break;
			case 'k':
			case 'K':
				out += "k";
				break;
			case 'x':
				out += "x";
				break;
			case ' ':
				out += "e";
				break;

			default:
				return std::string("");
			}
		}

		return out;
	}

	bool getXy(std::string gfx, unsigned int *x, unsigned int *y)
	{
		unsigned int w = gfx.size() == 9 * 9 ? 9 : 11;
		bool out = false;

		for (unsigned int i = 0; i < gfx.size(); i++) {
			switch (gfx[i])
			{
			case 'O':
			case '+':
			case 'K':
				*y = i / w;
				*x = i % w;
				out = true;
				break;
			default:
				break;
			}
		}

		return out;
	}

	bool constructMove(std::string start, std::string end, IBoard::Move &out)
	{
		bool s = getXy(start, &out.m_from.m_x, &out.m_from.m_y);
		bool e = getXy(end, &out.m_to.m_x, &out.m_to.m_y);

		return s == true && e == true;
	}


	void onGameEnd(IBoard::Color_t winner)
	{
		m_winner = winner;
	}


	IBoard::Color_t m_winner;
};
