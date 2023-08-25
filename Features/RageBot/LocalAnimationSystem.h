#pragma once

#include "../../SDK/Globals.h"
#include "../../SDK/Interfaces.h"

class CLocalAnimations
{
	struct stored_local_anims_t {
		// animstate vars
		float flEyeYaw = 0.f;
		float flEyePitch = 0.f;
		float flGoalFeetYaw = 0.f;
		float flMoveYaw = 0.f;
		float flLeanAmount = 0.f;
		float flJumpFallVelocity = 0.f;
		bool bOnGround = true;
		float flDurationInAir = 0.f;
		float flAffectedFraction = 0.f;

		std::array<float, 24> poseparams{};

		bool filled = false;
	} stored_local_anims;

	AnimationLayer local_animlayers[13];
	QAngle local_abs_angles;
public:

	void	FrameStageNotify(EClientFrameStage stage);
	void	OnCreateMove();
	void	UpdateLocalAnimations();

	void	StoreLocalAnims();
	void	RestoreLocalAnims();
};

extern CLocalAnimations* LocalAnimations;
