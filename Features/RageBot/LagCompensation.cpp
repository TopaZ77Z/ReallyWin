#include "LagCompensation.h"
#include "AnimationSystem.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"
#include "../AntiAim/AntiAim.h"
#include "DoubleTap.h"
#include "../../Utils/Utils.h"
#include <algorithm>

LagRecord* CLagCompensation::BackupData(CBasePlayer* player) {
	LagRecord* record = new LagRecord;

	record->player = player;
	RecordDataIntoTrack(player, record);

	return record;
}

void CLagCompensation::RecordDataIntoTrack(CBasePlayer* player, LagRecord* record) {

	record->player = player;
	record->m_viewAngle = player->m_angEyeAngles();
	record->m_flSimulationTime = player->m_flSimulationTime();
	record->m_flOldSimulationTime = player->m_flOldSimulationTime();
	record->m_vecOrigin = player->m_vecOrigin();
	record->m_vecAbsVelocity = player->m_vecAbsVelocity();
	record->m_vecAbsOrigin = player->GetAbsOrigin();
	record->m_angAbsAngles = player->GetAbsAngles();
	record->m_fFlags = player->m_fFlags();
	record->m_iEFlags = player->m_iEFlags();
	record->m_flCycle = player->m_flCycle();
	record->m_nSequence = player->m_nSequence();
	record->m_flDuckAmout = player->m_flDuckAmount();
	record->m_flDuckSpeed = player->m_flDuckSpeed();
	record->m_vecMins = player->m_vecMins();
	record->m_vecMaxs = player->m_vecMaxs();
	record->m_flCollisionChangeTime = player->m_flCollisionChangeTime();
	record->m_flCollisionZ = player->m_flCollisionChangeOrigin();
	record->m_vecVelocity = player->m_vecVelocity();
	record->m_flThirdPersonRecoil = player->m_flThirdpersonRecoil();
	record->m_flLowerBodyYaw = player->m_flLowerBodyYawTarget();
	record->m_bIsWalking = player->m_bIsWalking();
	//record->m_flVelocityModifier = player->m_flVelocityModifier();
	record->m_flTimeOfLastInjury = player->m_flTimeOfLastInjury();
	record->m_flGroundAccelLinearFracLastTime = player->m_flGroundAccelLinearFracLastTime();

	CBaseCombatWeapon* pWeapon = player->GetActiveWeapon();
	if (pWeapon)
	{
		if (pWeapon->m_flLastShotTime() <= player->m_flSimulationTime() && pWeapon->m_flLastShotTime() > player->m_flOldSimulationTime())
		{
			record->m_DidShot = true;
			record->m_nShotTick = TIME_TO_TICKS(pWeapon->m_flLastShotTime());
		}
	}

	player_info_t m_PlayerInfo;
	EngineClient->GetPlayerInfo(player->EntIndex(), &m_PlayerInfo);

	if (m_PlayerInfo.fakeplayer)
		record->m_IsFakePlayer = true;

	memcpy(record->boneMatrix, player->GetCachedBoneData().Base(), sizeof(matrix3x4_t) * player->GetCachedBoneData().Count());

	std::memcpy(record->animlayers.data(), player->GetAnimlayers(), sizeof(AnimationLayer) * ANIMATION_LAYER_COUNT);
	std::memcpy(record->flPoseParamaters.data(), player->m_flPoseParameter().data(), sizeof(float) * MAXSTUDIOPOSEPARAM);
}

void CLagCompensation::BacktrackEntity(LagRecord* record) {

	CBasePlayer* player = record->player;

	float flSimulationTime = player->m_flSimulationTime();

	player->m_vecMins() = record->m_vecMins;
	player->m_vecMaxs() = record->m_vecMaxs;
	player->m_angEyeAngles() = record->m_viewAngle;
	player->SetAbsAngles(record->m_angAbsAngles);
	player->SetAbsOrigin(record->m_vecAbsOrigin);
	player->m_vecOrigin() = record->m_vecOrigin;
	player->m_flSimulationTime() = record->m_flSimulationTime;
	player->m_fFlags() = record->m_fFlags;
	player->m_flCollisionChangeTime() = record->m_flCollisionChangeTime;
	player->m_flCollisionChangeOrigin() = record->m_flCollisionZ;
	player->m_flDuckAmount() = record->m_flDuckAmout;
	player->m_flDuckSpeed() = record->m_flDuckSpeed;
	player->m_nSequence() = record->m_nSequence;
	player->m_flTimeOfLastInjury() = record->m_flTimeOfLastInjury;
    player->m_flGroundAccelLinearFracLastTime() = record->m_flGroundAccelLinearFracLastTime;

	std::memcpy(player->GetAnimlayers(), record->animlayers.data(), sizeof(AnimationLayer) * ANIMATION_LAYER_COUNT);
	std::memcpy(player->m_flPoseParameter().data(), record->flPoseParamaters.data(), sizeof(float) * MAXSTUDIOPOSEPARAM);

	if (record->rollMatrixFilled && abs(flSimulationTime - record->m_flSimulationTime) > 0.002f)
		memcpy(player->GetCachedBoneData().Base(), record->rollMatrix, player->GetCachedBoneData().Count() * sizeof(matrix3x4_t));
	else
		memcpy(player->GetCachedBoneData().Base(), record->boneMatrix, player->GetCachedBoneData().Count() * sizeof(matrix3x4_t));

	return player->InvalidateBoneCache();
}

void CLagCompensation::OnNetUpdate() {
	if (!Cheat.InGame)
		return;

	for (int i = 0; i < ClientState->m_nMaxClients; i++) {
		CBasePlayer* pl = (CBasePlayer*)EntityList->GetClientEntity(i);

		if (!pl || !pl->IsAlive() || pl == Cheat.LocalPlayer || pl->m_bDormant())
			continue;

		auto& records = lag_records[pl->EntIndex()];

		if (records.empty() || pl->m_flSimulationTime() != pl->m_flOldSimulationTime()) {
			LagRecord* prev_record = !records.empty() ? &records.back() : nullptr;
			LagRecord* new_record = &records.emplace_back();

			new_record->m_nChokedTicks = TIME_TO_TICKS(pl->m_flSimulationTime() - pl->m_flOldSimulationTime());
			last_update_tick[i] = GlobalVars->tickcount;

			RecordDataIntoTrack(pl, new_record);

			RebulidCrouchAnimation(pl);

			AnimationSystem->UpdateAnimations(pl, new_record, records);

			if (prev_record)
				new_record->breaking_lag_comp = (prev_record->m_vecOrigin - new_record->m_vecOrigin).LengthSqr() > 4096.f;

			new_record->shifting_tickbase = max_simulation_time[i] > new_record->m_flSimulationTime;

			if (new_record->m_flSimulationTime > max_simulation_time[i] || abs(max_simulation_time[i] - new_record->m_flSimulationTime) > 1.f)
				max_simulation_time[i] = new_record->m_flSimulationTime;

			while (records.size() > 64) {
				records.pop_front();
			}
		}

		INetChannelInfo* nci = EngineClient->GetNetChannelInfo();
		if (config.visuals.esp.show_server_hitboxes->get() && nci && nci->IsLoopback())
			pl->DrawServerHitboxes(GlobalVars->interval_per_tick, true);
	}
}

float CLagCompensation::GetLerpTime() {
	static auto cl_interp = CVar->FindVar("cl_interp");
	static auto cl_interp_ratio = CVar->FindVar("cl_interp_ratio");
	static auto cl_updaterate = CVar->FindVar("cl_updaterate");
	static auto sv_minupdaterate = CVar->FindVar("sv_minupdaterate");
	static auto sv_maxupdaterate = CVar->FindVar("sv_maxupdaterate");
	static auto sv_client_min_interp_ratio = CVar->FindVar("sv_client_min_interp_ratio");
	static auto sv_client_max_interp_ratio = CVar->FindVar("sv_client_max_interp_ratio");

	int ud_rate = cl_updaterate->GetInt();

	if (sv_minupdaterate && sv_maxupdaterate)
		ud_rate = sv_maxupdaterate->GetInt();

	auto ratio = cl_interp_ratio->GetFloat();

	if (!ratio)
		ratio = 1.0f;

	if (sv_client_min_interp_ratio && sv_client_max_interp_ratio && sv_client_min_interp_ratio->GetFloat() != 1.0f)
		ratio = std::clamp(ratio, sv_client_min_interp_ratio->GetFloat(), sv_client_max_interp_ratio->GetFloat());

	return std::max<float>(cl_interp->GetFloat(), (ratio / ud_rate));
}

bool CLagCompensation::ValidRecord(LagRecord* record) {
	if (record->shifting_tickbase || record->breaking_lag_comp)
		return false;

	const auto nci = EngineClient->GetNetChannelInfo();

	if (!nci || !Cheat.LocalPlayer)
		return false;

	const auto correct = std::clamp(nci->GetLatency(0) + nci->GetLatency(1) + GetLerpTime(), 0.0f, cvars.sv_maxunlag->GetFloat());

	auto server_time = Cheat.LocalPlayer->IsAlive() ? TICKS_TO_TIME(Cheat.LocalPlayer->m_nTickBase()) : GlobalVars->curtime;
	
	if (abs(correct - (server_time - record->m_flSimulationTime)) > 0.2f)
		return false;

	return true;
}

void CLagCompensation::RebulidCrouchAnimation(CBasePlayer* pPlayer) {

	auto BackupPose = pPlayer->m_flPoseParameter();
	auto BackupCycle = pPlayer->GetAnimlayers()[3].m_flCycle;
	auto BackupWeight = pPlayer->GetAnimlayers()[3].m_flWeight;

	if (pPlayer->m_fFlags() & FL_ONGROUND && pPlayer->m_vecVelocity().Length2D() < 0.15f) {

		if (pPlayer->GetAnimlayers()[3].m_nSequence == 979) {

			pPlayer->GetAnimlayers()[3].m_flCycle = 0.0f;
			pPlayer->GetAnimlayers()[3].m_flWeight = 0.0f;
		}
	}

	pPlayer->m_flPoseParameter() = BackupPose;
	pPlayer->GetAnimlayers()[3].m_flCycle = BackupCycle;
	pPlayer->GetAnimlayers()[3].m_flWeight = BackupWeight;
}

void CLagCompensation::Reset() {
	for (int i = 0; i < lag_records.size(); i++) {
		lag_records[i].clear();
		max_simulation_time[i] = 0.f;
		last_update_tick[i] = 0;
	}
}

CLagCompensation* LagCompensation = new CLagCompensation;