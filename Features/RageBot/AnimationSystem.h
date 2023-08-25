#pragma once
#include <array>

#include "../../SDK/Globals.h"
#include "../../SDK/Interfaces.h"

struct LagRecord;

class CAnimationSystem {

public:
	void	BuildMatrix(CBasePlayer* player, LagRecord* record, matrix3x4_t* boneToWorld, int mask, AnimationLayer* animlayers);
	void Extrapolate(CBasePlayer* player, Vector& origin, Vector& velocity, int& flags, bool on_ground);
	//bool    BuildRebuildedMatrix(CBasePlayer* p_Player, LagRecord* p_Record, matrix3x4_t* m_Matrix, int Mask, AnimationLayer* AnimLayers);
	void	UpdateAnimations(CBasePlayer* player, LagRecord* record, std::deque<LagRecord>& records);
};

extern CAnimationSystem* AnimationSystem;
