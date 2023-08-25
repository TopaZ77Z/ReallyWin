#include "ShotManager.h"

#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"


void CShotManager::OnCreateMove() {
	CBaseCombatWeapon* active_weapon = Cheat.LocalPlayer->GetActiveWeapon();

	if (!active_weapon)
		return;

}