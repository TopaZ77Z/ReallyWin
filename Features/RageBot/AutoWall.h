#pragma once
#include <vector>

#include "../../SDK/Misc/Vector.h"
#include "../../SDK/Interfaces/IEngineTrace.h"

struct FireBulletData_t
{
	CGameTrace enterTrace;
	float damage = 0.f;
	int hitbox = 0;
	std::vector<Vector> impacts;
};

class CBasePlayer;
class CBaseCombatWeapon;
class CCSWeaponData;
class surfacedata_t;

class CAutoWall
{
public:
	void ScaleDamage(CGameTrace& enterTrace, CCSWeaponData* weaponInfo, float& currentDamage);
	bool FireBullet(CBasePlayer* attacket, const Vector& start, const Vector& end, FireBulletData_t& data, CBasePlayer* target = nullptr);

private:
	void TraceLine(const Vector& absStart, const Vector& absEnd, unsigned int mask, void* ignore, CGameTrace* ptr);
	void ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, const unsigned int fMask, ITraceFilter* pFilter, CGameTrace* pTrace, CBasePlayer* target);
	bool TraceToExit(CGameTrace& enterTrace, CGameTrace& exitTrace, const Vector& vecPosition, const Vector& vecDirection);
	bool HandleBulletPenetration(CBasePlayer* attacker, CCSWeaponData* weaponData, CGameTrace& enterTrace, surfacedata_t* enterSurfaceData, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage);
};

extern CAutoWall* AutoWall;