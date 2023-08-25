#include "Resolver.h"

#include <algorithm>
#include "../../SDK/Interfaces.h"
#include "../../SDK/Misc/CBasePlayer.h"
#include "LagCompensation.h"
#include "../../SDK/Globals.h"
#include "AnimationSystem.h"


CResolver* Resolver = new CResolver;

void CResolver::Reset(CBasePlayer* pl) {
	if (pl) {
		m_CachedRollAngle[pl->EntIndex()] = 0.f;
		return;
	}

	for (int i = 0; i < 64; ++i)
		m_CachedRollAngle[i] = 0.f;
}

float CResolver::ResolveRollAnimation(CBasePlayer* player, const LagRecord* prevRecord) {
	int plIdx = player->EntIndex();

	float eyeYaw = player->m_angEyeAngles().yaw;
	if (!prevRecord) {
		return 0.f;
	}

	float prevEyeYaw = prevRecord->m_viewAngle.yaw;
	float delta = Math::AngleDiff(eyeYaw, prevEyeYaw);

	return delta;
}

float CResolver::GetRollAngle(CBasePlayer* player) {
	if (player->IsTeammate())
		return 0.f;

	return m_CachedRollAngle[player->EntIndex()];
}

void CResolver::SetRollAngle(CBasePlayer* player, float angle) {
	m_CachedRollAngle[player->EntIndex()] = angle;
	player->m_angEyeAngles().roll = angle;
	player->v_angle().roll = angle;
}

R_PlayerState CResolver::DetectPlayerState(CBasePlayer* player, AnimationLayer* animlayers) {
	if (!(player->m_fFlags() & FL_ONGROUND))
		return R_PlayerState::AIR;

	CCSGOPlayerAnimationState* animstate = player->GetAnimstate();

	if (player->m_vecVelocity().Length2DSqr() > 200.f && animstate->flWalkToRunTransition > 0.5f && animlayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate > 0.0001f)
		return R_PlayerState::MOVING;

	return R_PlayerState::STANDING;
}

R_AntiAimType CResolver::DetectAntiAim(CBasePlayer* player, const std::deque<LagRecord>& records) {
	if (records.size() < 6 || records.back().m_nChokedTicks < 1)
		return R_AntiAimType::NONE;

	int jitteredRecords = 0;
	float avgDelta = 0.f;
	float prevEyeYaw = player->m_angEyeAngles().yaw;

	for (int i = records.size() - 1; i > records.size() - 3; i--) {
		const LagRecord* record = &records.at(i);
		float eyeYaw = record->m_viewAngle.yaw;

		float delta = std::fabs(Math::AngleDiff(eyeYaw, prevEyeYaw));

		avgDelta += delta;

		float maxDeltaDiff = record->m_nChokedTicks > 2 ? 30 : 15;

		if (std::fabs(delta - 60.f) < maxDeltaDiff)
			jitteredRecords++;

		prevEyeYaw = eyeYaw;
	}

	if (jitteredRecords > 1)
		return R_AntiAimType::JITTER;

	if (avgDelta * 0.5f < 30.f)
		return R_AntiAimType::STATIC;

	return R_AntiAimType::UNKNOWN;
}

void CResolver::SetupMoveLayer(CBasePlayer* player) {

}

void CResolver::SetupResolverLayers(CBasePlayer* player, LagRecord* record) {
	CCSGOPlayerAnimationState* animstate = player->GetAnimstate();
	CCSGOPlayerAnimationState originalAnimstate = *animstate;

	float eyeYaw = player->m_angEyeAngles().yaw;

	// zero delta
	animstate->SetTickInterval();
	animstate->flGoalFeetYaw = Math::AngleNormalize(eyeYaw);

	player->UpdateClientSideAnimation();
	memcpy(record->resolver_data.animlayers[0], player->GetAnimlayers(), sizeof(AnimationLayer) * 13);
	*animstate = originalAnimstate;

	// positive delta
	animstate->SetTickInterval();
	animstate->flGoalFeetYaw = Math::AngleNormalize(eyeYaw + player->GetMaxDesyncDelta());

	player->UpdateClientSideAnimation();
	memcpy(record->resolver_data.animlayers[1], player->GetAnimlayers(), sizeof(AnimationLayer) * 13);
	*animstate = originalAnimstate;

	// negative delta
	animstate->SetTickInterval();
	animstate->flGoalFeetYaw = Math::AngleNormalize(eyeYaw - player->GetMaxDesyncDelta());

	player->UpdateClientSideAnimation();
	memcpy(record->resolver_data.animlayers[2], player->GetAnimlayers(), sizeof(AnimationLayer) * 13);
	*animstate = originalAnimstate;
}

void CResolver::DetectFreestand(CBasePlayer* player, LagRecord* record) {
	Vector eyePos = player->GetEyePosition();

	QAngle negativeAngle = QAngle(0, player->m_angEyeAngles().yaw - 60.f, 0);
	QAngle positiveAngle = QAngle(0, player->m_angEyeAngles().yaw + 60.f, 0);

	negativeAngle.Normalize();
	positiveAngle.Normalize();

	Vector negPos = eyePos + Math::AngleVectors(negativeAngle) * 13.f;
	Vector posPos = eyePos + Math::AngleVectors(positiveAngle) * 13.f;

	Vector localDir = Cheat.LocalPlayer->GetWorldPosition() - eyePos;
	localDir.Q_Normalize();

	CGameTrace negTrace = EngineTrace->TraceRay(negPos, negPos + localDir * 400.f, MASK_SOLID, player);
	CGameTrace posTrace = EngineTrace->TraceRay(posPos, posPos + localDir * 400.f, MASK_SOLID, player);

	if ((negTrace.fraction == 1.f || negTrace.hit_entity == Cheat.LocalPlayer) && (posTrace.fraction == 1.f || posTrace.hit_entity == Cheat.LocalPlayer)) {
		record->resolver_data.side = 0;
		record->resolver_data.resolver_type = ResolverType::NONE;
		return;
	}

	record->resolver_data.side = negTrace.fraction < posTrace.fraction ? -1 : 1;
	record->resolver_data.resolver_type = ResolverType::FREESTAND;
}

void CResolver::Run(CBasePlayer* player, LagRecord* record, std::deque<LagRecord>& records) {
	if (Cheat.freezetime || player->m_fFlags() & FL_FROZEN)
		return;

	LagRecord* prevRecord = nullptr;
	if (records.size() > 2)
		prevRecord = &records.back();

	record->roll = ResolveRollAnimation(player, prevRecord);

	record->resolver_data.player_state = DetectPlayerState(player, record->animlayers.data());
	record->resolver_data.antiaim_type = DetectAntiAim(player, records);

	SetupResolverLayers(player, record);

	record->resolver_data.resolver_type = ResolverType::NONE;
	record->resolver_data.delta_center = abs(record->animlayers[6].m_flPlaybackRate - record->resolver_data.animlayers[0][6].m_flPlaybackRate) * 100.f;
	record->resolver_data.delta_positive = abs(record->animlayers[6].m_flPlaybackRate - record->resolver_data.animlayers[1][6].m_flPlaybackRate) * 100.f;
	record->resolver_data.delta_negative = abs(record->animlayers[6].m_flPlaybackRate - record->resolver_data.animlayers[2][6].m_flPlaybackRate) * 100.f;

	float flLastDelta = 1.f;
	float flMaxDelta = max(max(record->resolver_data.delta_center, record->resolver_data.delta_negative), record->resolver_data.delta_positive);

	if (record->resolver_data.player_state == R_PlayerState::MOVING) {
		record->resolver_data.resolver_type = ResolverType::ANIM;

		if (record->resolver_data.delta_center < flLastDelta) {
			record->resolver_data.resolver_type = ResolverType::NONE;
			record->resolver_data.side = 0;
			flLastDelta = record->resolver_data.delta_center;
		}

		if (record->resolver_data.delta_positive < flLastDelta) {
			record->resolver_data.resolver_type = ResolverType::ANIM;
			record->resolver_data.side = 1;
			flLastDelta = record->resolver_data.delta_positive;
		}

		if (record->resolver_data.delta_negative < flLastDelta) {
			record->resolver_data.resolver_type = ResolverType::ANIM;
			record->resolver_data.side = -1;
			flLastDelta = record->resolver_data.delta_negative;
		}
	}
	else if (record->resolver_data.antiaim_type == R_AntiAimType::JITTER && prevRecord) {
		float eyeYaw = player->m_angEyeAngles().yaw;
		float prevEyeYaw = prevRecord->m_viewAngle.yaw;
		float delta = Math::AngleDiff(eyeYaw, prevEyeYaw);

		if (delta > 0.f)
			record->resolver_data.side = -1;
		else
			record->resolver_data.side = 1;

		record->resolver_data.resolver_type = ResolverType::LOGIC;
	}
	else {
		DetectFreestand(player, record);
	}

	if (record->resolver_data.player_state == R_PlayerState::AIR)
	{
		player->m_flPoseParameter()[2] = Utils::RandomInt(0, 4) * 0.25f;
		player->m_flPoseParameter()[11] = Utils::RandomInt(1, 3) * 0.25f;
	}

	if (record->resolver_data.side != 0)
	{
		if (record->resolver_data.antiaim_type == R_AntiAimType::JITTER && prevRecord)
			player->GetAnimstate()->flGoalFeetYaw = std::clamp(player->GetAnimstate()->flGoalFeetYaw, player->m_angEyeAngles().yaw - player->GetMaxDesyncDelta(), player->m_angEyeAngles().yaw + player->GetMaxDesyncDelta());
		else
			player->GetAnimstate()->flGoalFeetYaw = Math::NormalizeYaw(player->m_angEyeAngles().yaw + (player->GetMaxDesyncDelta() * record->resolver_data.side));
	}

	return player->InvalidatePhysicsRecursive(ANGLES_CHANGED);
}