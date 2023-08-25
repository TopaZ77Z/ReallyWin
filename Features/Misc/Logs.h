#pragma once
#include <string>
#include "../../SDK/Misc/Color.h"
#include <deque>

class CEventLogs
{
public:
	void paint_traverse();
	void add(std::string text);

	bool last_log = false;
private:
	struct loginfo_t
	{
		loginfo_t(float log_time, std::string message, const Color& color)  //-V818
		{
			this->log_time = log_time;
			this->message = message; //-V820
			this->color = color;

			x = 6.0f;
			y = 0.0f;
		}

		float log_time;
		std::string message;
		Color color;
		float x, y;
		int alpha;
	};

	std::deque <loginfo_t> logs;
};

extern CEventLogs* Logs;