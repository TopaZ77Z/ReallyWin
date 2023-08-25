#pragma once
#include "../../SDK/Misc/Vector.h"

class IGameEvent;
class CBaseEntity;

class CBombInfo {
private:
	enum {
		BOMBSTATE_NONE = 0,
		BOMBSTATE_PLANTING,
		BOMBSTATE_PLANTED,
		BOMBSTATE_DEFUSING
	};

	float alpha = 0.f;
	int bombState = 0;
	float timeStamp = 0.f;
	float defuseTimeStamp = 0.f;
	bool defuseKits = false;
	int bombSite = 0;
	Vector bombPosition;
public:
	void EventHandler(IGameEvent* event);
	void Draw();
};

namespace UI {
	void KeyBinds();
	extern CBombInfo* BombInfo;
}