#include "Console.h"

#include "../SDK/Interfaces.h"


CGameConsole* Console = new CGameConsole;

void CGameConsole::Print(const std::string& msg) {
	CVar->ConsolePrintf(msg.c_str());
}

void CGameConsole::ColorPrint(const std::string& msg, const Color& color) {
	CVar->ConsoleColorPrintf(color, msg.c_str());
}

void CGameConsole::Log(const std::string& msg) {
	CVar->ConsoleColorPrintf(Color(176, 209, 245), "[reallywin] ");
	CVar->ConsoleColorPrintf(Color(255, 255, 255), msg.c_str());
	CVar->ConsolePrintf("\n");
}