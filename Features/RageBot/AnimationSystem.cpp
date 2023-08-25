#include "AnimationSystem.h"
#include "LagCompensation.h"
#include "../AntiAim/AntiAim.h"
#include "Resolver.h"
#include "Interpolate.h"

void CAnimationSystem::BuildMatrix(CBasePlayer* player, LagRecord* record, matrix3x4_t* boneToWorld, int mask, AnimationLayer* animlayers) {

	std::memcpy(player->GetAnimlayers(), record->animlayers.data(), sizeof(AnimationLayer) * ANIMATION_LAYER_COUNT);

	float curtime = GlobalVars->curtime;
	float realtime = GlobalVars->realtime;
	float absoluteframetime = GlobalVars->absoluteframetime;
	float frametime = GlobalVars->frametime;
	float framecount = GlobalVars->framecount;
	float tickcount = GlobalVars->tickcount;
	float interpolation_amount = GlobalVars->interpolation_amount;

	static auto r_jiggle_bones = CVar->FindVar("r_jiggle_bones");
	auto r_jiggle_bones_backup = r_jiggle_bones->GetInt();

	bool backupMaintainSequenceTransitions = player->m_bMaintainSequenceTransitions();
	int backupEffects = player->m_fEffects();
	const auto backup_occlusion_flags = player->m_nOcclusionFlags();
	const auto backup_occlusion_mask = player->m_nOcclusionMask();
	auto m_nLastSkipFramecount = player->m_nLastSkipFramecount();
	auto m_nClientEffects = player->m_nClientEffects();
	auto GetAbsOrigin = player->GetAbsOrigin();

	//int nSimulationTick = TIME_TO_TICKS(record->m_flSimulationTime);
	//GlobalVars->curtime = record->m_flSimulationTime;
	//GlobalVars->realtime = record->m_flSimulationTime;
	//GlobalVars->frametime = GlobalVars->interval_per_tick;
	//GlobalVars->absoluteframetime = GlobalVars->interval_per_tick;
	//GlobalVars->tickcount = nSimulationTick;
	//GlobalVars->framecount = INT_MAX; /* ShouldSkipAnimationFrame fix */
	//GlobalVars->interpolation_amount = 0.0f;

	player->InvalidateBoneCache();

	r_jiggle_bones->SetInt(0);

	*(uint8_t*)((uintptr_t)this + 0x274) |= FL_ONGROUND;

	player->m_fEffects() |= EF_NOINTERP;
	player->m_bMaintainSequenceTransitions() = false;

	player->m_nClientEffects() |= 2;
	player->m_nOcclusionFlags() = -1;
	player->m_nOcclusionMask() &= ~2;
	player->m_nLastSkipFramecount() = 0;

	player->m_iMostRecentModelBoneCounter() = 0;
	player->m_flLastBoneSetupTime() = -FLT_MAX;

	/*if (boneToWorld == Interpolate->interpolate_data[player->EntIndex()].original_matrix)
		player->SetAbsOrigin(record->m_vecOrigin);*/

	hook_info.setup_bones = true;
	player->SetupBones(boneToWorld, MAXSTUDIOBONES, mask, player->m_flSimulationTime());
	hook_info.setup_bones = false;

	player->m_fEffects() = backupEffects;
	player->m_bMaintainSequenceTransitions() = backupMaintainSequenceTransitions;
	r_jiggle_bones->SetInt(r_jiggle_bones_backup);

	player->m_nClientEffects() = m_nClientEffects;
	player->m_nOcclusionFlags() = backup_occlusion_flags;
	player->m_nOcclusionMask() = backup_occlusion_mask;
	player->m_nLastSkipFramecount() = m_nLastSkipFramecount;
	player->SetAbsOrigin(GetAbsOrigin);

	std::memcpy(player->GetAnimlayers(), record->animlayers.data(), sizeof(AnimationLayer) * ANIMATION_LAYER_COUNT);

	GlobalVars->curtime = curtime;
	GlobalVars->realtime = realtime;
	GlobalVars->absoluteframetime = absoluteframetime;
	GlobalVars->frametime = frametime;
	GlobalVars->framecount = framecount;
	GlobalVars->tickcount = tickcount;
	GlobalVars->interpolation_amount = interpolation_amount;
}

void CAnimationSystem::Extrapolate(CBasePlayer* player, Vector& origin, Vector& velocity, int& flags, bool on_ground)
{
	static const auto sv_gravity = CVar->FindVar(("sv_gravity"));
	static const auto sv_jump_impulse = CVar->FindVar(("sv_jump_impulse"));

	if (!(flags & FL_ONGROUND))
		velocity.z -= TICKS_TO_TIME(sv_gravity->GetFloat());

	else if (player->m_fFlags() & FL_ONGROUND && !on_ground)
		velocity.z = sv_jump_impulse->GetFloat();

	const auto src = origin;
	auto end = src + velocity * GlobalVars->interval_per_tick;

	Ray_t r;
	r.Init(src, end, player->m_vecMins(), player->m_vecMaxs());

	CGameTrace t;
	CTraceFilter filter;
	filter.pSkip = player;

	EngineTrace->TraceRay(r, MASK_PLAYERSOLID, &filter, &t);

	if (t.fraction != 1.f)
	{
		for (auto i = 0; i < 2; i++)
		{
			velocity -= t.plane.normal * velocity.Dot(t.plane.normal);

			const auto dot = velocity.Dot(t.plane.normal);
			if (dot < 0.f)
				velocity -= Vector(dot * t.plane.normal.x,
					dot * t.plane.normal.y, dot * t.plane.normal.z);

			end = t.endpos + velocity * TICKS_TO_TIME(1.f - t.fraction);

			r.Init(t.endpos, end, player->m_vecMins(), player->m_vecMaxs());
			EngineTrace->TraceRay(r, MASK_PLAYERSOLID, &filter, &t);

			if (t.fraction == 1.f)
				break;
		}
	}

	origin = end = t.endpos;
	end.z -= 2.f;

	r.Init(origin, end, player->m_vecMins(), player->m_vecMaxs());
	EngineTrace->TraceRay(r, MASK_PLAYERSOLID, &filter, &t);

	flags &= ~FL_ONGROUND;

	if (t.DidHit() && t.plane.normal.z > .7f)
		flags |= FL_ONGROUND;
}

void CAnimationSystem::UpdateAnimations(CBasePlayer* player, LagRecord* record, std::deque<LagRecord>& records) {
	CCSGOPlayerAnimationState* animstate = player->GetAnimstate();
	const int idx = player->EntIndex();

	if (!animstate)
		return;

	LagRecord* prevRecord = nullptr;

	if (records.size() > 2)
		prevRecord = &records.back();

	float_t flCurTime = GlobalVars->curtime;
	float_t flRealTime = GlobalVars->realtime;
	int32_t iFrameCount = GlobalVars->framecount;
	int32_t iTickCount = GlobalVars->tickcount;
	float_t flFrameTime = GlobalVars->frametime;
	float_t flAbsFrameTime = GlobalVars->absoluteframetime;
	float_t flInterpolation = GlobalVars->interpolation_amount;

	float_t flDuckAmount = player->m_flDuckAmount();
	int32_t iEFlags = player->m_iEFlags();
	QAngle angEyeAngles = player->m_angEyeAngles();
	float_t flLowerBodyYaw = player->m_flLowerBodyYawTarget();

	float_t flFeetCycle = player->GetAnimstate()->flFeetCycle;
	float_t flFeetWeight = player->GetAnimstate()->flFeetWeight;

	player->m_iEFlags() &= ~EFL_DIRTY_ABSVELOCITY;
	player->m_fFlags() = record->m_fFlags;
	player->m_vecVelocity() = record->m_vecVelocity;
	player->m_vecAbsVelocity() = record->m_vecVelocity;

	memcpy(record->animlayers.data(), player->GetAnimlayers(), 13 * sizeof(AnimationLayer));

	player->SetAbsOrigin(record->m_vecOrigin);

	if (record->m_nChokedTicks <= 1)
	{
		GlobalVars->curtime = record->m_flSimulationTime;
		GlobalVars->realtime = record->m_flSimulationTime;
		GlobalVars->frametime = GlobalVars->interval_per_tick;
		GlobalVars->absoluteframetime = GlobalVars->interval_per_tick;
		GlobalVars->framecount = TIME_TO_TICKS(record->m_flSimulationTime);
		GlobalVars->tickcount = TIME_TO_TICKS(record->m_flSimulationTime);
		GlobalVars->interpolation_amount = 0.0f;

		animstate->SetTickInterval();

		player->m_angEyeAngles() = record->m_viewAngle;
		player->m_flLowerBodyYawTarget() = record->m_flLowerBodyYaw;
		player->m_flDuckAmount() = record->m_flDuckAmout;

		for (int i = NULL; i < ANIMATION_LAYER_COUNT; i++)
			player->GetAnimlayers()[i].m_pOwner = player;

		player->UpdateClientSideAnimation();

		GlobalVars->curtime = flCurTime;
		GlobalVars->realtime = flRealTime;
		GlobalVars->frametime = flFrameTime;
		GlobalVars->absoluteframetime = flAbsFrameTime;
		GlobalVars->framecount = iFrameCount;
		GlobalVars->tickcount = iTickCount;
		GlobalVars->interpolation_amount = flInterpolation;
	}
	else
	{
		int32_t iSimulationTick = 1;
		do
		{
			float_t flSimulationTime = player->m_flOldSimulationTime() + TICKS_TO_TIME(iSimulationTick);

			GlobalVars->curtime = flSimulationTime;
			GlobalVars->realtime = flSimulationTime;
			GlobalVars->frametime = GlobalVars->interval_per_tick;
			GlobalVars->absoluteframetime = GlobalVars->interval_per_tick;
			GlobalVars->framecount = TIME_TO_TICKS(flSimulationTime);
			GlobalVars->tickcount = TIME_TO_TICKS(flSimulationTime);
			GlobalVars->interpolation_amount = 0.0f;

			animstate->SetTickInterval();

			for (int i = NULL; i < ANIMATION_LAYER_COUNT; i++)
				player->GetAnimlayers()[i].m_pOwner = player;

			player->UpdateClientSideAnimation();

			GlobalVars->curtime = flCurTime;
			GlobalVars->realtime = flRealTime;
			GlobalVars->frametime = flFrameTime;
			GlobalVars->absoluteframetime = flAbsFrameTime;
			GlobalVars->framecount = iFrameCount;
			GlobalVars->tickcount = iTickCount;
			GlobalVars->interpolation_amount = flInterpolation;

			// increase simulation tick
			iSimulationTick++;
		} while (iSimulationTick <= record->m_nChokedTicks);
	}

	Resolver->SetRollAngle(player, 0.f);

	CCSGOPlayerAnimationState originalAnimstate = *animstate;

	if (!player->IsTeammate() && !record->m_IsFakePlayer) {
		Resolver->Run(player, record, records);
	}

	BuildMatrix(player, record,Interpolate->interpolate_data[idx].original_matrix, BONE_USED_BY_ANYTHING, record->animlayers.data());
	memcpy(record->boneMatrix, Interpolate->interpolate_data[idx].original_matrix, player->GetCachedBoneData().Count() * sizeof(matrix3x4_t));
	record->boneMatrixFilled = true;

	Resolver->SetRollAngle(player, record->roll);

	animstate->SetTickInterval();

	BuildMatrix(player, record, record->rollMatrix, BONE_USED_BY_HITBOX, record->animlayers.data());
	record->rollMatrixFilled = true;

	player->UpdateClientSideAnimation();

	GlobalVars->curtime = flCurTime;
	GlobalVars->realtime = flRealTime;
	GlobalVars->frametime = flFrameTime;
	GlobalVars->absoluteframetime = flAbsFrameTime;
	GlobalVars->framecount = iFrameCount;
	GlobalVars->tickcount = iTickCount;
	GlobalVars->interpolation_amount = flInterpolation;

	player->GetAnimstate()->flFeetCycle = flFeetCycle;
	player->GetAnimstate()->flFeetWeight = flFeetWeight;

	player->m_angEyeAngles() = angEyeAngles;
	player->m_flLowerBodyYawTarget() = flLowerBodyYaw;
	player->m_flDuckAmount() = flDuckAmount;
	player->m_iEFlags() = iEFlags;

	memcpy(player->GetAnimlayers(), record->animlayers.data(), sizeof(AnimationLayer) * 13);
	memcpy(player->GetCachedBoneData().Base(), record->boneMatrix, player->GetCachedBoneData().Count() * sizeof(matrix3x4_t));

	return player->InvalidatePhysicsRecursive(ANIMATION_CHANGED);
}

CAnimationSystem* AnimationSystem = new CAnimationSystem;