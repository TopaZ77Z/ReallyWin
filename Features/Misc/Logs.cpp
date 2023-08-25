#include <__msvc_chrono.hpp>
#include "Logs.h"
#include "../../SDK/Render.h"
#include "../../Utils/Console.h"
#include "../../SDK/Config.h"

int epoch_time()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void CEventLogs::paint_traverse()
{
	if (logs.empty())
		return;

	while (logs.size() > 10)
		logs.pop_back();

	auto last_y = 146;

	for (size_t i = 0; i < logs.size(); i++)
	{
		auto& log = logs.at(i);

		if (epoch_time() - log.log_time > 4600)
		{
			auto factor = log.log_time + 5000.0f - (float)epoch_time();
			factor *= 0.001f;

			auto opacity = (int)(255.0f * factor);

			if (opacity < 2)
			{
				logs.erase(logs.begin() + i);
				continue;
			}

			log.color.alpha_modulate(opacity);
			log.y -= factor * 1.25f;
		}

		last_y -= 14;

		auto logs_size_inverted = 10 - logs.size();
		Render->Text(log.message.c_str(), log.x, last_y + log.y - logs_size_inverted * 14, log.color, Verdana, TEXT_DROPSHADOW);
	}
}

void CEventLogs::add(std::string text)
{
	logs.emplace_front(loginfo_t(epoch_time(), text, Color(255,255,255)));

	last_log = true;
	Console->Log(text.c_str());
}

CEventLogs* Logs = new CEventLogs();