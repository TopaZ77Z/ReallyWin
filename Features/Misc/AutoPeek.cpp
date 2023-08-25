#include "AutoPeek.h"
#include "Prediction.h"
#include "../RageBot/DoubleTap.h"
#include "../../SDK/Globals.h"
#include "../../SDK/Render.h"

CAutoPeek* AutoPeek = new CAutoPeek;


void CAutoPeek::Draw() {
	static bool prev_state = false;

	if (!Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive() || !Cheat.InGame)
		return;

	bool state = (config.ragebot.aimbot.peek_assist_keybind->get() && config.ragebot.aimbot.peek_assist->get());

	if (prev_state != state) {
		if (state) {
			if (Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND) {
				position = Cheat.LocalPlayer->m_vecOrigin();
			}
			else {
				CGameTrace trace = EngineTrace->TraceHull(Cheat.LocalPlayer->m_vecOrigin(), Cheat.LocalPlayer->m_vecOrigin() - Vector(0, 0, 128), Cheat.LocalPlayer->m_vecMins(), Cheat.LocalPlayer->m_vecMaxs(), CONTENTS_SOLID, Cheat.LocalPlayer);

				position = trace.endpos;
			}
		}

		prev_state = state;
	}

	circleAnimation.UpdateAnimation(state);

	float alpha = circleAnimation.GetValue();

	if (alpha > 0.01f)
		Render->Circle3DGradient(position, 22.f, config.ragebot.aimbot.peek_assist_color->get().alpha_modulatef(alpha));
}

void CAutoPeek::CreateMove() {
	if (!Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive() || !Cheat.InGame || !(config.ragebot.aimbot.peek_assist_keybind->get() && config.ragebot.aimbot.peek_assist->get())) {
		returning = false;
		return;
	}

	if (ctx.cmd->buttons & IN_ATTACK && Cheat.LocalPlayer->GetActiveWeapon()->ShootingWeapon() && Cheat.LocalPlayer->GetActiveWeapon()->CanShoot())
		returning = true;
	else if ((Cheat.LocalPlayer->m_vecOrigin() - position).Length2D() < 1)
		returning = false;

	if (returning) {
		QAngle ang = Utils::VectorToAngle(Cheat.LocalPlayer->m_vecOrigin(), position);
		QAngle vang;
		EngineClient->GetViewAngles(&vang);
		ang.yaw = vang.yaw - ang.yaw;
		ang.Normalize();

		Vector dir;
		Utils::AngleVectors(ang, dir);
		dir *= 450;
		ctx.cmd->forwardmove = dir.x;
		ctx.cmd->sidemove = dir.y;
	}
}