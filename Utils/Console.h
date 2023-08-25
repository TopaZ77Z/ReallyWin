#pragma once

#include <string>
#include "../SDK/Misc/Color.h"


class CGameConsole {
public:
	void Print(const std::string& msg);
	void ColorPrint(const std::string& msg, const Color& color);
	void Log(const std::string& msg);
};


extern CGameConsole* Console;