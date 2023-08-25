#pragma once
#include "../../SDK/Misc/Studio.h"
#include "LagCompensation.h"
#include "../../SDK/Globals.h"
#include <vector>

class CUserCmd;

struct ScannedPoint_t {
	LagRecord* record = nullptr;
	Vector point;
	int hitbox = -1;
	bool multipoint = false;
	float damage = 0.f;
	std::vector<Vector> impacts;
};

struct ScannedTarget_t {
	CBasePlayer* player = nullptr;
	std::vector<ScannedPoint_t> points;
	ScannedPoint_t best_point;
	QAngle angle;
	float hitchance = 0.f;
	float minimum_damage = 0.f;
};

struct AimPoint_t {
	Vector point;
	int hitbox = 0;
	bool multipoint = false;
};

struct ragebot_debug_data_t {
	std::string target;
	bool autostop = false;
	float hitchance = 0.f;
	float damage = 0.f;
};

class CRagebot {
private:
	struct SpreadValues_t {
		float a;
		float bcos;
		float bsin;
		float c;
		float dcos;
		float dsin;
	};

	SpreadValues_t spread_values[50];

	weapon_settings_t settings;
	bool doubletap_stop = false;
	float doubletap_stop_speed = 0.f;

	CBaseCombatWeapon* active_weapon = nullptr;
	Vector eye_position;
	CCSWeaponData* weapon_data = nullptr;
	std::vector<CBasePlayer*> targets;
	std::vector<ScannedTarget_t> scanned_targets;
	CBasePlayer* last_target = nullptr;
	int last_target_shot = 0;

	ragebot_debug_data_t debug_data;

	inline bool hitbox_enabled(int hitbox) {
		switch (hitbox)
		{
		case HITBOX_HEAD:
			return settings.hitboxes->get(0);
		case HITBOX_PELVIS:
		case HITBOX_STOMACH:
			return settings.hitboxes->get(2);
		case HITBOX_CHEST:
		case HITBOX_UPPER_CHEST:
			return settings.hitboxes->get(1);
		case HITBOX_RIGHT_THIGH:
		case HITBOX_LEFT_THIGH:
		case HITBOX_RIGHT_CALF:
		case HITBOX_LEFT_CALF:
			return settings.hitboxes->get(4);
		case HITBOX_RIGHT_FOOT:
		case HITBOX_LEFT_FOOT:
			return settings.hitboxes->get(5);
		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_FOREARM:
		case HITBOX_RIGHT_FOREARM:
			return settings.hitboxes->get(3);
		default:
			return false;
		}
	}

	inline bool multipoints_enabled(int hitbox) {
		switch (hitbox)
		{
		case HITBOX_HEAD:
			return settings.multipoints->get(0);
		case HITBOX_PELVIS:
		case HITBOX_STOMACH:
			return settings.multipoints->get(2);
		case HITBOX_CHEST:
		case HITBOX_UPPER_CHEST:
			return settings.multipoints->get(1);
		default:
			return false;
		}
	}

public:
	void				CalcSpreadValues();
	void				AutoStop();
	float				CalcHitchance(QAngle angles, CBasePlayer* target, int hitbox);
	float				CalcMinDamage(CBasePlayer* target);
	weapon_settings_t	GetWeaponSettings(int weaponId);
	bool				IsArmored(int hitbox);

	int					CalcPointsCount();
	void				GetMultipoints(LagRecord* record, int hitbox, float scale, std::vector<AimPoint_t>& points);
	void				FindTargets();
	std::vector<LagRecord*> SelectRecords(CBasePlayer* player);
	std::vector<AimPoint_t> SelectPoints(LagRecord* record, bool backtrack_scan);
	bool				CompareRecords(LagRecord* a, LagRecord* b);

	ScannedPoint_t		SelectBestPoint(ScannedTarget_t target);
	void				ScanTargets();
	ScannedTarget_t		ScanTarget(CBasePlayer* target);

	void                AutoRevolver();

	void				Run();
	void				DrawDebugData();
	void				Zeusbot();
};

extern CRagebot* Ragebot;