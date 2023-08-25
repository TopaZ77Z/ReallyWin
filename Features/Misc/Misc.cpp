#include "Misc.h"
#include "../../Utils/Utils.h"
#include "../../SDK/Config.h"
#include "../../SDK/Globals.h"

void Miscelleaneus::Clantag()
{
	auto tag = ("");

	if (!config.misc.miscellaneous.clantag->get())
	{
		tag = ("");
		return;
	}

	if (config.misc.miscellaneous.clantag->get())
	{
		auto nci = EngineClient->GetNetChannelInfo();

		if (!nci)
			return;

		static auto time = -1;

		auto ticks = TIME_TO_TICKS(nci->GetAvgLatency(FLOW_OUTGOING)) + (float)GlobalVars->tickcount;
		auto intervals = 0.3f / GlobalVars->interval_per_tick;

		auto main_time = (int)(ticks / intervals) % 33;

		if (main_time != time && !ClientState->m_nChokedCommands)
		{
			switch (main_time)
			{
			case 0:  tag = ("r"); break;
			case 1:  tag = ("re"); break;
			case 2:  tag = ("rea"); break;
			case 3:  tag = ("real"); break;
			case 4:  tag = ("reall"); break;
			case 5:  tag = ("really"); break;
			case 6:  tag = ("reallyw"); break;
			case 7:  tag = ("reallywi"); break;
			case 8:  tag = ("reallywin"); break;
			case 9:  tag = ("reallywin"); break;
			case 10: tag = ("reallywin"); break;
			case 11: tag = ("reallywin"); break;
			case 12: tag = ("reallywin"); break;
			case 13: tag = ("reallywin"); break;
			case 14: tag = ("reallywin"); break;
			case 15: tag = ("reallywin"); break;
			case 16: tag = ("reallywi"); break;
			case 17: tag = ("reallyw"); break;
			case 18: tag = ("really"); break;
			case 19: tag = ("reall"); break;
			case 20: tag = ("real"); break;
			case 21: tag = ("rea"); break;
			case 22: tag = ("re"); break;
			case 23: tag = ("r"); break;
			}

			time = main_time;
			Utils::SetClantag(tag);
		}
	}
}

void Miscelleaneus::QuickSwitch() {

	auto pWeapon = Cheat.LocalPlayer->GetActiveWeapon();

	if (!pWeapon && !Cheat.LocalPlayer && !Cheat.LocalPlayer->IsAlive())
		return;


}