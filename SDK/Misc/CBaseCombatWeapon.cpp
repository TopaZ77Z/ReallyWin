#include "CBaseCombatWeapon.h"
#include "CBasePlayer.h"
#include "../../SDK/Globals.h"
#include "../../Utils/Utils.h"
#include <string>
#include "../../Features/Misc/Prediction.h"


CCSWeaponData* CBaseCombatWeapon::GetWeaponInfo() {
	if (!this)
		return nullptr;

	return WeaponSystem->GetWeaponData(m_iItemDefinitionIndex());
}

const char* CBaseCombatWeapon::GetName() {
	CCSWeaponData* data = GetWeaponInfo();

	if (!data)
		return "";

	const wchar_t* name = Localize->FindSafe(data->szHudName);
	char buffer[64];
	size_t len = wcstombs(buffer, name, wcslen(name));
	char* result = new char[len + 1];
	memset(result, 0, len + 1);
	memcpy(result, buffer, len);
	return result;
}

bool CBaseCombatWeapon::CanShoot() {
	CCSWeaponData* data = GetWeaponInfo();

	if (!data)
		return false;

	CBasePlayer* owner = (CBasePlayer*)EntityList->GetClientEntityFromHandle(m_hOwner());

	if (!owner)
		return false;

	if (Cheat.freezetime)
		return false;

	if (owner->m_fFlags() & FL_FROZEN)
		return false;

	if (data->iMaxClip1 > 0 && m_iClip() <= 0)
		return false;

	if (owner->m_iShotsFired() > 0 && !data->bFullAuto)
		return false;

	int tick_base = owner->m_nTickBase();
	if (owner == Cheat.LocalPlayer)
		tick_base -= ctx.tickbase_shift;
	const float cur_time = TICKS_TO_TIME(tick_base);

	if (cur_time < m_flNextPrimaryAttack() || cur_time < owner->m_flNextAttack())
		return false;

	return true;
}