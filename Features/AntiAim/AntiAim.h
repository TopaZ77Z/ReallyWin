#pragma once
#include <Windows.h>

class CUserCmd;
class CBasePlayer;


class CAntiAim {
	int manualAngleState = 0;
	CBasePlayer* target;
	float notModifiedYaw = 0.f;

public:
	bool jitter;
	float realAngle;
	bool desyncing = false;
	bool freezetime = false;

	void LegMovement();
	void FakeLag();
	void Angles();
	void Desync();
	void SlowWalk();
	void FakeDuck();

	bool IsPeeking();

	int DesyncFreestand();
	void OnKeyPressed(WPARAM key);
	CBasePlayer* GetNearestTarget();
	float AtTargets();
};

extern CAntiAim* AntiAim;