#pragma once
#include <vector>
#include "../../SDK/Misc/Vector.h"
#include "../../SDK/Misc/QAngle.h"
#include "../../SDK/Misc/Matrix.h"
#include "../../SDK/Misc/CBasePlayer.h"
#include "Resolver.h"

class CBasePlayer;

struct LagRecord {
	CBasePlayer* player = nullptr;

	matrix3x4_t boneMatrix[128];
	matrix3x4_t rollMatrix[128];

	std::array < AnimationLayer, ANIMATION_LAYER_COUNT > animlayers = { };

	Vector m_vecOrigin = Vector(0, 0, 0);
	Vector m_vecVelocity = Vector(0, 0, 0);
	Vector m_vecMins = Vector(0, 0, 0);
	Vector m_vecMaxs = Vector(0, 0, 0);
	Vector m_vecAbsOrigin = Vector(0, 0, 0);
	Vector m_vecAbsVelocity = Vector(0, 0, 0);

	QAngle m_viewAngle = QAngle(0, 0, 0);
	QAngle m_angAbsAngles = QAngle(0, 0, 0);

	float m_flSimulationTime = 0.f;
	float m_flDuckAmout = 0.f;
	float m_flDuckSpeed = 0.f;
	float m_flCycle = 0.f;
	float roll = 0.f;
	float m_flCollisionChangeTime = 0.f;
	float m_flCollisionZ = 0.f;
	float m_flLowerBodyYaw = 0.f;
	float m_flThirdPersonRecoil = 0.f;
	float m_flOldSimulationTime = 0.f;
	float m_flVelocityModifier = 0.f;
	float m_flTimeOfLastInjury = 0.f;
	float m_flGroundAccelLinearFracLastTime = 0.f;

	int m_nSequence = 0;
	int m_fFlags = 0;
	int m_nChokedTicks = 0;
	int m_nShotTick = 0;
	int m_iEFlags = 0;

	bool shifting_tickbase = false;
	bool breaking_lag_comp = false;
	bool boneMatrixFilled = false;
	bool rollMatrixFilled = false;
	bool m_DidShot = false;
	bool m_IsFakePlayer = false;
	bool m_bIsWalking = false;


	std::array<float, 24> flPoseParamaters;

	ResolverData_t resolver_data;
};

class CLagCompensation {
	std::array<std::deque<LagRecord>, 64> lag_records;
	float max_simulation_time[64];
	int last_update_tick[64];
public:

	__forceinline std::deque<LagRecord>& records(int index) { return lag_records[index]; };

	LagRecord* BackupData(CBasePlayer* player);

	void RecordDataIntoTrack(CBasePlayer* player, LagRecord* record);
	void BacktrackEntity(LagRecord* record);
	void OnNetUpdate();
	void Reset();

	// Record helpers
	float GetLerpTime();
	bool ValidRecord(LagRecord* record);
	void RebulidCrouchAnimation(CBasePlayer* pPlayer);
};

extern CLagCompensation* LagCompensation;