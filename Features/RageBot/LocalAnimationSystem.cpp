#include "LocalAnimationSystem.h"
#include "../AntiAim/AntiAim.h"
#include "Interpolate.h"
#include "AnimationSystem.h"

void CLocalAnimations::StoreLocalAnims() {
	auto animstate = Cheat.LocalPlayer->GetAnimstate();

	stored_local_anims.bOnGround = animstate->bOnGround;
	stored_local_anims.flAffectedFraction = animstate->flAffectedFraction;
	stored_local_anims.flDurationInAir = animstate->flDurationInAir;
	stored_local_anims.flEyePitch = animstate->flEyePitch;
	stored_local_anims.flEyeYaw = animstate->flEyeYaw;
	stored_local_anims.flGoalFeetYaw = animstate->flGoalFeetYaw;
	stored_local_anims.flJumpFallVelocity = animstate->flJumpFallVelocity;
	stored_local_anims.flLeanAmount = animstate->flLeanAmount;
	stored_local_anims.flMoveYaw = animstate->flMoveYaw;
	stored_local_anims.poseparams = Cheat.LocalPlayer->m_flPoseParameter();
	stored_local_anims.filled = true;
}

void CLocalAnimations::RestoreLocalAnims() {
	if (!stored_local_anims.filled)
		return;

	auto animstate = Cheat.LocalPlayer->GetAnimstate();

	animstate->bOnGround = stored_local_anims.bOnGround;
	animstate->flAffectedFraction = stored_local_anims.flAffectedFraction;
	animstate->flDurationInAir = stored_local_anims.flDurationInAir;
	animstate->flEyePitch = stored_local_anims.flEyePitch;
	animstate->flEyeYaw = stored_local_anims.flEyeYaw;
	animstate->flGoalFeetYaw = stored_local_anims.flGoalFeetYaw;
	animstate->flJumpFallVelocity = stored_local_anims.flJumpFallVelocity;
	animstate->flLeanAmount = stored_local_anims.flLeanAmount;
	animstate->flMoveYaw = stored_local_anims.flMoveYaw;
	Cheat.LocalPlayer->m_flPoseParameter() = stored_local_anims.poseparams;
	Cheat.LocalPlayer->SetAbsAngles(QAngle(0, stored_local_anims.flGoalFeetYaw, 0));
}

void CLocalAnimations::OnCreateMove() {
	static float old_sim_time = 0.f;

	CCSGOPlayerAnimationState* animstate = Cheat.LocalPlayer->GetAnimstate();

	if (AntiAim->desyncing)
		animstate->flGoalFeetYaw = AntiAim->realAngle;

	memcpy(local_animlayers, Cheat.LocalPlayer->GetAnimlayers(), sizeof(AnimationLayer) * 13);

	animstate->bOnGround = Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND;
	if (!animstate->bOnGround) {
		const float jumpImpulse = cvars.sv_jump_impulse->GetFloat();
		const float gravity = cvars.sv_gravity->GetFloat();
		const float speed = Cheat.LocalPlayer->m_flFallVelocity();
		// speed = jumpImpulse - gravity * flDurationInAir
		animstate->flDurationInAir = (jumpImpulse - speed) / gravity;
	}

	animstate->iLastUpdateFrame = 0;
	Cheat.LocalPlayer->UpdateAnimationState(animstate, Cheat.thirdpersonAngles, true);

	if (AntiAim->desyncing)
		animstate->flGoalFeetYaw = AntiAim->realAngle;

	if (ctx.send_packet) {
		old_sim_time = Cheat.LocalPlayer->m_flSimulationTime();
		local_abs_angles = QAngle(0, animstate->flGoalFeetYaw, 0);
		stored_local_anims.poseparams = Cheat.LocalPlayer->m_flPoseParameter();

	}

	memcpy(Cheat.LocalPlayer->GetAnimlayers(), local_animlayers, sizeof(AnimationLayer) * 13);
}

void CLocalAnimations::UpdateLocalAnimations() {
	static int last_update_tick = 0;
	static float old_sim_time = 0.f;

	CCSGOPlayerAnimationState* animstate = Cheat.LocalPlayer->GetAnimstate();

	animstate->iLastUpdateFrame = GlobalVars->framecount;

	animstate->bOnGround = stored_local_anims.bOnGround;
	animstate->flAffectedFraction = stored_local_anims.flAffectedFraction;
	animstate->flDurationInAir = stored_local_anims.flDurationInAir;
	animstate->flEyePitch = stored_local_anims.flEyePitch;
	animstate->flEyeYaw = stored_local_anims.flEyeYaw;
	animstate->flGoalFeetYaw = stored_local_anims.flGoalFeetYaw;
	animstate->flJumpFallVelocity = stored_local_anims.flJumpFallVelocity;
	animstate->flLeanAmount = stored_local_anims.flLeanAmount;
	animstate->flMoveYaw = stored_local_anims.flMoveYaw;
	Cheat.LocalPlayer->SetAbsAngles(local_abs_angles);
	Cheat.LocalPlayer->m_flPoseParameter() = stored_local_anims.poseparams;
}

void CLocalAnimations::FrameStageNotify(EClientFrameStage stage) {
	switch (stage)
	{
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		for (int i = 0; i < ClientState->m_nMaxClients; ++i) {
			CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(i));

			if (player && player != Cheat.LocalPlayer)
				Interpolate->DisableInterpolationFlags(player);
		}
		break;
	case FRAME_NET_UPDATE_END:
		break;
	case FRAME_RENDER_START:
		if (Cheat.LocalPlayer && Cheat.LocalPlayer->IsAlive()) {
			UpdateLocalAnimations();
		}
		break;
	default:
		break;
	}
}

CLocalAnimations* LocalAnimations = new CLocalAnimations;