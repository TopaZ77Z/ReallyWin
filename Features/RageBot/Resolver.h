#pragma once

#include <deque>

#include "../../SDK/Misc/CBasePlayer.h"

struct LagRecord;

enum class R_PlayerState {
	STANDING,
	MOVING,
	AIR
};

enum class R_AntiAimType {
	NONE,
	STATIC,
	JITTER,
	UNKNOWN,
};

enum class ResolverType {
	NONE,
	FREESTAND,
	LOGIC,
	ANIM,
};

struct ResolverData_t {
	R_PlayerState player_state;
	R_AntiAimType antiaim_type;
	ResolverType resolver_type;

	AnimationLayer animlayers[3][13];

	float delta_positive = 0.f;
	float delta_negative = 0.f;
	float delta_center = 0.f;

	matrix3x4_t matrix_middle[MAXSTUDIOBONES];
	matrix3x4_t matrix_left[MAXSTUDIOBONES];
	matrix3x4_t matrix_right[MAXSTUDIOBONES];

	float anim_accuracy = 0.f;

	int side = 0;
};

class CResolver {
	float m_CachedRollAngle[64];

public:
	CResolver() {
		for (int i = 0; i < 64; ++i)
			m_CachedRollAngle[i] = 0.f;
	}	

	void			Reset(CBasePlayer* pl = nullptr);

	float			ResolveRollAnimation(CBasePlayer* player, const LagRecord* prevRecord);
	float			GetRollAngle(CBasePlayer* player);
	void			SetRollAngle(CBasePlayer* player, float angle);

	R_PlayerState	DetectPlayerState(CBasePlayer* player, AnimationLayer* animlayers);
	R_AntiAimType	DetectAntiAim(CBasePlayer* player, const std::deque<LagRecord>& records);

	void			SetupMoveLayer(CBasePlayer* player);
	void			SetupResolverLayers(CBasePlayer* player, LagRecord* record);

	void			DetectFreestand(CBasePlayer* player, LagRecord* record);

	void			Run(CBasePlayer* player, LagRecord* record, std::deque<LagRecord>& records);
};

extern CResolver* Resolver;