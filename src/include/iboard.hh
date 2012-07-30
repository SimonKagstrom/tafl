#pragma once

#include <string>

class IBoard
{
public:
	std::string marshal() = 0;

	static IBoard &fromString(std::string str);
};
