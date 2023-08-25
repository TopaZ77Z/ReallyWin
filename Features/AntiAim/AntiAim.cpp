#include "AntiAim.h"
#include "../RageBot/DoubleTap.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"
#include <algorithm>
#include "../Misc/Prediction.h"
#include "../RageBot/AutoWall.h"

void CAntiAim::FakeLag() {
	static ConVar* sv_maxusrcmdprocessticks = CVar->FindVar("sv_maxusrcmdprocessticks");

	if (Cheat.LocalPlayer->m_MoveType() == MOVETYPE_NOCLIP || Cheat.LocalPlayer->m_fFlags() & FL_FROZEN || Cheat.freezetime)
		return;

	CBaseCombatWeapon* activeWeapon = Cheat.LocalPlayer->GetActiveWeapon();

	if (!activeWeapon)
		return;

	if (activeWeapon->IsGrenade() && EnginePrediction->m_fThrowTime > 0) {
		ctx.send_packet = true;
		return;
	}
	else if (ctx.cmd->buttons & IN_ATTACK && activeWeapon->ShootingWeapon() && activeWeapon->CanShoot()) {
		if (!config.antiaim.misc.fake_duck->get())
			ctx.send_packet = true;
		return;
	}

	int fakelagTicks = 0;
	int fakelagLimit = min(14, config.antiaim.fakelag.limit->get());
	int fakelagMin = 1;

	if (ctx.tickbase_shift > 0) {
		fakelagLimit = max((sv_maxusrcmdprocessticks->GetInt() - 2) - ctx.tickbase_shift, 0);
	}

	if (config.antiaim.misc.fake_duck->get())
		fakelagMin = fakelagLimit = 14;

	if (config.antiaim.fakelag.enabled->get()) {
		if (Cheat.LocalPlayer->m_vecVelocity().Length() < 50) {
			fakelagTicks = 2;
		}
		else {
			fakelagTicks = int(64.0f / (Cheat.LocalPlayer->m_vecVelocity().Length() * GlobalVars->interval_per_tick) + 1);
		}
	}

	fakelagMin = min(fakelagMin, fakelagLimit);

	if (config.antiaim.fakelag.variability->get() && config.antiaim.fakelag.enabled->get())
		fakelagTicks = std::clamp(fakelagTicks - abs(rand() % int(config.antiaim.fakelag.variability->get())), fakelagMin, fakelagLimit);
	else
		fakelagTicks = std::clamp(fakelagTicks, fakelagMin, fakelagLimit);
	
	if (ctx.teleported_last_tick)
		ctx.send_packet = true;
	else
		ctx.send_packet = ClientState->m_nChokedCommands >= fakelagTicks;

	//static bool hasPeeked = false;

	//if (IsPeeking()) {
	//	if (!hasPeeked) {
	//		hasPeeked = true;

	//		if (ClientState->m_nChokedCommands > 0)
	//			ctx.send_packet = true;
	//	}
	//}
	//else {
	//	hasPeeked = false;
	//}
}

void CAntiAim::Angles() {
	desyncing = false;

	if (Cheat.LocalPlayer->m_MoveType() == MOVETYPE_NOCLIP || Cheat.LocalPlayer->m_MoveType() == MOVETYPE_LADDER || Cheat.LocalPlayer->m_fFlags() & FL_FROZEN || Cheat.freezetime)
		return;

	CBaseCombatWeapon* activeWeapon = Cheat.LocalPlayer->GetActiveWeapon();

	if (!activeWeapon)
		return;

	if (activeWeapon->IsGrenade() && activeWeapon->m_fThrowTime())
		return;

	if (!activeWeapon->IsGrenade() && (ctx.cmd->buttons & IN_ATTACK && Cheat.LocalPlayer->GetActiveWeapon()->CanShoot()))
		return;

	target = GetNearestTarget();

	if (!(ctx.cmd->buttons & IN_USE)) {
		switch (config.antiaim.anti_aimbot_angles.pitch->get()) {
		case 0:
			break;
		case 1:
			ctx.cmd->viewangles.pitch = 89;
			break;
		case 2:
			float pitch = -89;
			// 2 up
			if (Cheat.LocalPlayer->m_flSimulationTime() <= -1)
				pitch = -89;
			else if (int(GlobalVars->curtime * 26) % 16 == 0 && ClientState->m_nChokedCommands < 2)
				pitch = -89;
			else
				pitch = 89;

			ctx.cmd->viewangles.pitch = pitch;
			break;
		}

		float originalYaw = ctx.cmd->viewangles.yaw;

		switch (config.antiaim.anti_aimbot_angles.yaw->get()) {
		case 0:
			break;
		case 1:
			ctx.cmd->viewangles.yaw -= 180;
			break;
		case 2:
			ctx.cmd->viewangles.yaw = AtTargets();
			break;
		}

		if (manualAngleState) {
			ctx.cmd->viewangles.yaw = originalYaw + ((manualAngleState == 1) ? 90 : -90);
		}

		notModifiedYaw = ctx.cmd->viewangles.yaw;

		if (config.antiaim.anti_aimbot_angles.yaw_jitter->get())
			ctx.cmd->viewangles.yaw += jitter ? -config.antiaim.anti_aimbot_angles.modifier_value->get() : config.antiaim.anti_aimbot_angles.modifier_value->get();
	}

	Desync();
}

void CAntiAim::Desync() {
	if (!config.antiaim.anti_aimbot_angles.body_yaw->get() || DoubleTap->IsShifting())
		return;

	bool inverter = config.antiaim.anti_aimbot_angles.inverter->get();

	if (config.antiaim.anti_aimbot_angles.body_yaw_options->get(0))
		inverter = jitter;

	if (config.antiaim.anti_aimbot_angles.body_yaw_options->get(3)) {
		int fs_side = DesyncFreestand();

		if (fs_side != 0)
			inverter = fs_side == -1;
	}

	float desyncAngle = 0.f;

	if (config.antiaim.anti_aimbot_angles.body_yaw_limit->get() < 58)
		desyncAngle = inverter ? -config.antiaim.anti_aimbot_angles.body_yaw_limit->get() : config.antiaim.anti_aimbot_angles.body_yaw_limit->get();
	else
		desyncAngle = inverter ? -120 : 120;

	if (!ctx.send_packet) {
		realAngle = Math::AngleNormalize(ctx.cmd->viewangles.yaw + std::clamp(desyncAngle, -Cheat.LocalPlayer->GetMaxDesyncDelta(), Cheat.LocalPlayer->GetMaxDesyncDelta()));

		ctx.cmd->viewangles.yaw += desyncAngle;
	}

	if (config.antiaim.anti_aimbot_angles.body_yaw_options->get(2) && ctx.send_packet && (!ctx.tickbase_shift || ctx.cmd->buttons & IN_DUCK || config.antiaim.misc.slow_walk->get() || !(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND && Cheat.LocalPlayer->m_vecVelocity().LengthSqr() > 400.f))) {
		ctx.cmd->viewangles.roll = desyncAngle < 0 ? 64: -64;
	}

	desyncing = true;

	if (ctx.local_velocity.LengthSqr() < 5.f)
		ctx.cmd->sidemove = ctx.cmd->buttons & IN_DUCK ? (ctx.cmd->tick_count % 2 == 0 ? -3.01f : 3.01f) : (ctx.cmd->tick_count % 2 == 0 ? -1.01f : 1.01f);
}

CBasePlayer* CAntiAim::GetNearestTarget() {
	float nearestFov = 360.f;
	Vector eyePos = Cheat.LocalPlayer->GetEyePosition();
	QAngle viewAngle; EngineClient->GetViewAngles(&viewAngle);
	CBasePlayer* nearestPlayer = nullptr;

	for (int i = 0; i < ClientState->m_nMaxClients; i++) {
		CBasePlayer* pl = (CBasePlayer*)EntityList->GetClientEntity(i);

		if (!pl)
			continue;

		if (!pl->IsPlayer() || pl->IsTeammate() || !pl->IsAlive())
			continue;

		if (GlobalVars->curtime - pl->m_flSimulationTime() > 5.f) // old dormant
			continue;

		QAngle angleToPlayer = Math::VectorAngles(pl->m_vecOrigin() - eyePos);

		float fov = Utils::GetFOV(viewAngle, angleToPlayer);

		if (fov < nearestFov) {
			nearestFov = fov;
			nearestPlayer = pl;
		}
	}

	return nearestPlayer;
}

float CAntiAim::AtTargets() {
	if (!target)
		return ctx.cmd->viewangles.yaw - 180.f;

	QAngle backwardAngle = Math::VectorAngles(Cheat.LocalPlayer->m_vecOrigin() - target->m_vecOrigin());
	return backwardAngle.yaw;
}

int CAntiAim::DesyncFreestand() {
	if (!target)
		return 0;

	Vector forward = (target->m_vecOrigin() - Cheat.LocalPlayer->m_vecOrigin()).Q_Normalized();
	Vector eyePos = Cheat.LocalPlayer->GetEyePosition();

	Vector right = Math::AngleVectors(QAngle(0, notModifiedYaw + 90.f, 0));

	Vector negPos = eyePos - right * 16.f;
	Vector posPos = eyePos + right * 16.f;

	CGameTrace negTrace = EngineTrace->TraceRay(negPos, negPos + forward * 100.f, CONTENTS_SOLID, Cheat.LocalPlayer);
	CGameTrace posTrace = EngineTrace->TraceRay(posPos, posPos + forward * 100.f, CONTENTS_SOLID, Cheat.LocalPlayer);

	if (negTrace.startsolid && posTrace.startsolid)
		return 0;
	else if (negTrace.startsolid)
		return -1;
	else if (posTrace.startsolid)
		return 1;

	if (negTrace.fraction == 1.f && posTrace.fraction == 1.f)
		return 0;

	return negTrace.fraction < posTrace.fraction ? -1 : 1;
}

void CAntiAim::SlowWalk() {
	if (!config.antiaim.misc.slow_walk->get() || !Cheat.LocalPlayer || Cheat.LocalPlayer->m_iHealth() == 0 || !(GetAsyncKeyState(VK_SHIFT) & 0x8000))
		return;

	CBaseCombatWeapon* weapon = Cheat.LocalPlayer->GetActiveWeapon();

	ctx.cmd->buttons &= ~IN_WALK;

	if (!weapon)
		return;

	CCSWeaponData* data = weapon->GetWeaponInfo();

	float maxSpeed = (Cheat.LocalPlayer->m_bIsScoped() ? data->flMaxSpeedAlt : data->flMaxSpeed) * 0.3f;

	float movespeed = Math::Q_sqrt(ctx.cmd->sidemove * ctx.cmd->sidemove + ctx.cmd->forwardmove * ctx.cmd->forwardmove);
	
	if (movespeed == 0)
		return;

	float modifier = maxSpeed / movespeed;

	ctx.cmd->sidemove = min(modifier * ctx.cmd->sidemove, 450);
	ctx.cmd->forwardmove = min(modifier * ctx.cmd->forwardmove, 450);
}

void CAntiAim::FakeDuck() {
	if (!Cheat.LocalPlayer || Cheat.LocalPlayer->m_iHealth() == 0 || !config.antiaim.misc.fake_duck->get())
		return;

	if (!ctx.fakeducking && ClientState->m_nChokedCommands != 7)
		return;

	if (ClientState->GetChokedCommands() < 7)
		ctx.cmd->buttons &= ~IN_DUCK;
	else
		ctx.cmd->buttons |= IN_DUCK;

	ctx.fakeducking = true;
}

void CAntiAim::LegMovement() {
	if (!Cheat.LocalPlayer || Cheat.LocalPlayer->m_MoveType() == MOVETYPE_LADDER)
		return;

	ctx.cmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);

	switch (config.antiaim.misc.leg_movement->get()) {
	case 0:
		if (ctx.cmd->forwardmove > 0)
			ctx.cmd->buttons |= IN_FORWARD;
		else
			ctx.cmd->buttons |= IN_BACK;
		if (ctx.cmd->sidemove > 0)
			ctx.cmd->buttons |= IN_MOVERIGHT;
		else
			ctx.cmd->buttons |= IN_MOVELEFT;
		break;
	case 1:
		if (ctx.cmd->forwardmove < 0)
			ctx.cmd->buttons |= IN_FORWARD;
		else
			ctx.cmd->buttons |= IN_BACK;
		if (ctx.cmd->sidemove < 0)
			ctx.cmd->buttons |= IN_MOVERIGHT;
		else
			ctx.cmd->buttons |= IN_MOVELEFT;
		break;
	default:
		break;
	}
}

bool CAntiAim::IsPeeking() {
	Vector velocity = Cheat.LocalPlayer->m_vecVelocity().Normalized();

	Vector scan_points[] = {
		Cheat.LocalPlayer->GetHitboxCenter(HITBOX_HEAD) + velocity * 15,
		Cheat.LocalPlayer->GetHitboxCenter(HITBOX_LEFT_FOOT) + velocity * 15,
		Cheat.LocalPlayer->GetHitboxCenter(HITBOX_RIGHT_FOOT) + velocity * 15,
		Cheat.LocalPlayer->GetHitboxCenter(HITBOX_PELVIS) + velocity * 15,
	};

	CBasePlayer* nearest = GetNearestTarget();

	if (!nearest)
		return false;

	FireBulletData_t data;

	for (auto& point : scan_points) {
		if (AutoWall->FireBullet(nearest, nearest->GetEyePosition(), point, data, nearest) && data.damage >= 5.f)
			return true;
	}

	return false;
}

void CAntiAim::OnKeyPressed(WPARAM key) {
	if (key == config.antiaim.anti_aimbot_angles.manual_left->key) {
		if (manualAngleState == 0 || manualAngleState == 2)
			manualAngleState = 1;
		else if (manualAngleState == 1)
			manualAngleState = 0;
	}
	else if (key == config.antiaim.anti_aimbot_angles.manual_right->key) {
		if (manualAngleState == 0 || manualAngleState == 1)
			manualAngleState = 2;
		else if (manualAngleState == 2)
			manualAngleState = 0;
	}
}

CAntiAim* AntiAim = new CAntiAim();