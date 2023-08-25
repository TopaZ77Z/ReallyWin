#include "DoubleTap.h"
#include "../Misc/Prediction.h"
#include "../../SDK/Globals.h"
#include "../AntiAim/AntiAim.h"
#include "../../Utils/Console.h"

void CDoubleTap::Run() {
	if (!ctx.cmd)
		return;

	if (teleport_next_tick) {
		teleport_next_tick = false;
		target_tickbase_shift = 0;
		last_teleport_time = GlobalVars->realtime;
		return;
	}

	if (!config.ragebot.aimbot.doubletap->get() || config.antiaim.misc.fake_duck->get() || config.ragebot.aimbot.force_teleport->get()) {
		target_tickbase_shift = 0;
		return;
	}

	const int max_tickbase_charge = MaxTickbaseShift();
	CBaseCombatWeapon* weapon = Cheat.LocalPlayer->GetActiveWeapon();

	if (target_tickbase_shift < max_tickbase_charge) {
		if (!(weapon->IsGrenade() || GlobalVars->realtime - last_teleport_time < 0.4f || GetAsyncKeyState(VK_LBUTTON) & 0x8000 || block_charge || !ctx.send_packet)) {
			target_tickbase_shift = max_tickbase_charge;
			charged_command = ctx.cmd->command_number;
			ctx.shifted_last_tick = 0;
		}
	}
	else if (target_tickbase_shift > max_tickbase_charge) {
		target_tickbase_shift = max_tickbase_charge;
	}

	block_charge = false;
}

bool CDoubleTap::ShouldCharge() {
	if (GlobalVars->realtime - Cheat.LocalPlayer->m_flSpawnTime() < 0.2f)
		return false;

	return ctx.tickbase_shift < target_tickbase_shift;
}

void CDoubleTap::HandleTeleport(CL_Move_t cl_move, float extra_samples) {
	shifting_tickbase = true;

	for (; ctx.tickbase_shift > target_tickbase_shift; --ctx.tickbase_shift) {
		cl_move(extra_samples, ctx.tickbase_shift == target_tickbase_shift); // bFinalTick does nothing to be honest
	}

	shifting_tickbase = false;
}

int CDoubleTap::MaxTickbaseShift() {
	CBaseCombatWeapon* activeWeapon = Cheat.LocalPlayer->GetActiveWeapon();

	if (!activeWeapon)
		return config.ragebot.aimbot.doubletap_speed->get();

	switch (activeWeapon->m_iItemDefinitionIndex()) {
	case Tec9:
	case Fiveseven:
	case Elite:
		return 7;
	default:
		return config.ragebot.aimbot.doubletap_speed->get();
	}
}

void CDoubleTap::ForceTeleport() {
	target_tickbase_shift = 0;
	last_teleport_time = GlobalVars->realtime;
	teleport_next_tick = false;
}

void CTickBase::Store(int tickbase, int cmd, int shift, bool restore, int cmd_diff)
{
	this->data.tickbase = tickbase;
	this->data.command_number = cmd;
	this->data.shift_amount = shift;
	this->data.restore_tickbase = restore;
	this->data.cmd_diff = cmd_diff;
}

void CTickBase::Fix(int new_command_number, int& tickbase)
{
	auto d = this->data;
	if (d.command_number <= 0)
		return;

	if (d.command_number == new_command_number)
		tickbase = d.tickbase - d.shift_amount + GlobalVars->simTicksThisFrame;

	if (d.restore_tickbase && d.command_number + d.cmd_diff == new_command_number)
		tickbase += d.shift_amount - GlobalVars->simTicksThisFrame;
}

CTickBase* TickBase = new CTickBase;

CDoubleTap* DoubleTap = new CDoubleTap;