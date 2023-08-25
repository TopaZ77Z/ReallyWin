#include "Ragebot.h"
#include "AutoWall.h"
#include "../Misc/Prediction.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"
#include "../../Utils/Utils.h"
#include <algorithm>
#include "../Misc/AutoPeek.h"
#include "DoubleTap.h"
#include "../../Utils/Console.h"
#include "AnimationSystem.h"

void CRagebot::CalcSpreadValues() {
	for (int i = 0; i < 50; i++) {
		Utils::RandomSeed(i);

		float a = Utils::RandomFloat(0.f, 1.f);
		float b = Utils::RandomFloat(0.f, 6.2831853071795864f);
		float c = Utils::RandomFloat(0.f, 1.f);
		float d = Utils::RandomFloat(0.f, 6.2831853071795864f);

		spread_values[i].a = a;
		spread_values[i].bcos = std::cos(b);
		spread_values[i].bsin = std::sin(b);
		spread_values[i].c = c;
		spread_values[i].dcos = std::cos(d);
		spread_values[i].dsin = std::sin(d);
	}
}

weapon_settings_t CRagebot::GetWeaponSettings(int weaponId) {
	weapon_settings_t settings = config.ragebot.weapons.global;

	switch (weaponId) {
	case Scar20:
	case G3SG1:
		settings = config.ragebot.weapons.autosniper;
		break;
	case Ssg08:
		settings = config.ragebot.weapons.scout;
		break;
	case Awp:
		settings = config.ragebot.weapons.awp;
		break;
	case Deagle:
	case Revolver:
		settings = config.ragebot.weapons.deagle;
		break;
	case Fiveseven:
	case Glock:
	case Usp_s:
	case Tec9:
		settings = config.ragebot.weapons.pistol;
		break;
	}

	return settings;
}

float CRagebot::CalcMinDamage(CBasePlayer* player) {
	int minimum_damage = settings.minimum_damage->get();
	if (config.ragebot.aimbot.minimum_damage_override_key->get())
		minimum_damage = settings.minimum_damage_override->get();

	if (minimum_damage >= 100) {
		return minimum_damage - 100 + player->m_iHealth();
	}
	else {
		return min(minimum_damage, player->m_iHealth() + 1);
	}
}

void CRagebot::AutoStop() {
	if (!(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND)) return;

	Vector vec_speed = Cheat.LocalPlayer->m_vecVelocity();
	QAngle direction = Math::VectorAngles(vec_speed);

	float target_speed = active_weapon->MaxSpeed() * 0.3f;

	if (vec_speed.Q_Length() < target_speed + 3.f && !settings.auto_stop->get(0)) {
		float cmd_speed = Math::Q_sqrt(ctx.cmd->forwardmove * ctx.cmd->forwardmove + ctx.cmd->sidemove * ctx.cmd->sidemove);
	
		if (cmd_speed > target_speed) {
			float factor = 40.f / target_speed;
			ctx.cmd->forwardmove *= factor;
			ctx.cmd->sidemove *= factor;
		}

		return;
	}

	QAngle view; EngineClient->GetViewAngles(&view);
	direction.yaw = view.yaw - direction.yaw;
	direction.Normalize();

	Vector forward;
	Math::AngleVectors(direction, forward);

	Vector nigated_direction = forward * -std::clamp(vec_speed.Q_Length2D(), 0.f, 450.f);

	ctx.cmd->sidemove = nigated_direction.y;
	ctx.cmd->forwardmove = nigated_direction.x;
}
/*

bool CRagebot::calculate_hitchance(Vector& aim_angle, int& final_hitchance) {

	settings = GetWeaponSettings(active_weapon->m_iItemDefinitionIndex());

	const auto hitchance_cfg = settings.hitchance->get();

	auto forward = Vector(0, 0, 0);
	auto right = Vector(0, 0, 0);
	auto up = Vector(0, 0, 0);

	angle_vectors(aim_angle, &forward, &right, &up);

	fast_vec_normalize(forward);
	fast_vec_normalize(right);
	fast_vec_normalize(up);

	auto total_hits = 0;

	float spread_value, inaccuracy_value;
	Vector spread_view, direction, end;

	for (auto i = 0; i < 256; i++) {
		float a = Utils::RandomFloat(0.f, 1.f);
		float b = Utils::RandomFloat(0.f, M_PI * 2.f);
		float c = Utils::RandomFloat(0.f, 1.f);
		float d = Utils::RandomFloat(0.f, M_PI * 2.f);

		inaccuracy_value = a * EnginePrediction->WeaponInaccuracy();
		spread_value = c * EnginePrediction->WeaponSpread();

		spread_view = ((cos(b) * inaccuracy_value) + (cos(d) * spread_value), (sin(b) * inaccuracy_value) + (sin(d) * spread_value), 0);

		direction.x = forward.x + (spread_view.x * right.x) + (spread_view.y * up.x);
		direction.y = forward.y + (spread_view.x * right.y) + (spread_view.y * up.y);
		direction.z = forward.z + (spread_view.x * right.z) + (spread_view.y * up.z);

		end = eye_position + (direction * 8192);

		if ((static_cast<float>(total_hits) / 256.f) * 100.f >= hitchance_cfg)
		{
			final_hitchance = (static_cast<float>(total_hits) / 256.f) * 100.f;
			return true;
		}
	}

	return false;
}

*/
void fast_rsqrt(float a, float* out)
{
	const auto xx = _mm_load_ss(&a);
	auto xr = _mm_rsqrt_ss(xx);
	auto xt = _mm_mul_ss(xr, xr);
	xt = _mm_mul_ss(xt, xx);
	xt = _mm_sub_ss(_mm_set_ss(3.f), xt);
	xt = _mm_mul_ss(xt, _mm_set_ss(0.5f));
	xr = _mm_mul_ss(xr, xt);
	_mm_store_ss(out, xr);
}
float fast_vec_normalize(Vector& vec)
{
	const auto sqrlen = vec.LengthSqr() + 1.0e-10f;
	float invlen;
	fast_rsqrt(sqrlen, &invlen);
	vec.x *= invlen;
	vec.y *= invlen;
	vec.z *= invlen;
	return sqrlen * invlen;
}
static const int iTotalSeeds = 255;
static std::vector<std::tuple<float, float, float>> PreComputedSeeds = {};
void BulidSeedTable() {

	if (!PreComputedSeeds.empty()) return;

	for (auto i = 0; i < iTotalSeeds; i++) {
		Utils::RandomSeed(i + 1);

		const auto pi_seed = Utils::RandomFloat(0.f, M_PI * 2);

		PreComputedSeeds.emplace_back(Utils::RandomFloat(0.f, 1.f), sin(pi_seed), cos(pi_seed));
	}
}

float CRagebot::CalcHitchance(QAngle angles, CBasePlayer* target, int damagegroup) {
	auto weapon = Cheat.LocalPlayer->GetActiveWeapon();
	auto local = Cheat.LocalPlayer;
	settings = GetWeaponSettings(active_weapon->m_iItemDefinitionIndex());
	float hit_chance_cfg = settings.hitchance->get();
	int needed_hits = static_cast<int>(iTotalSeeds * (hit_chance_cfg / 100.f));
	const auto allowed_misses = 256 - needed_hits;

	if (!local)
		return false;

	if (!weapon)
		return false;

	BulidSeedTable();

	Vector forward, right, up = Vector(0, 0, 0);;

	Math::AngleVectors(angles, forward, right, up);

	fast_vec_normalize(forward);
	fast_vec_normalize(right);
	fast_vec_normalize(up);
	int hits = 0;

	CGameTrace tr;
	Ray_t ray;

	weapon->UpdateAccuracyPenality();
	const auto weapon_inaccuracy = weapon->GetInaccuracy();
	const auto weapon_spread = weapon->GetSpread();

	for (int i = 0; i < iTotalSeeds; i++) {

		float a = Utils::RandomFloat(0.f, 1.f);
		float b = Utils::RandomFloat(0.f, M_PI * 2.f);
		float c = Utils::RandomFloat(0.f, 1.f);
		float d = Utils::RandomFloat(0.f, M_PI * 2.f);

		float inaccuracy = a * weapon_inaccuracy;
		float spread = c * weapon_spread;

		if (active_weapon->m_iItemDefinitionIndex() == 64)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}

		Vector spread_view((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0), direction;

		direction.x = forward.x + (spread_view.x * right.x) + (spread_view.y * up.x);
		direction.y = forward.y + (spread_view.x * right.y) + (spread_view.y * up.y);
		direction.z = forward.z + (spread_view.x * right.z) + (spread_view.y * up.z);
		direction.Normalized();

		fast_vec_normalize(direction);

		ray.Init(eye_position, eye_position + (direction * 8192));

		EngineTrace->ClipRayToPlayer(ray, MASK_SHOT | CONTENTS_HITBOX, target, &tr);

		//DebugOverlay->AddLineOverlay(eye_position, tr.endpos, 255, 255, 255, false, 0.01f);

		if (tr.hit_entity == target)
			hits++;

		if ((iTotalSeeds - i + hits) < needed_hits)
			return false;

		const auto hitchance = (static_cast<float>(hits) / 255.f) * 100.f;
		if (hitchance >= hit_chance_cfg)
			return hitchance;

		if (i - hits > allowed_misses)
			return hitchance;
	}

	return 0.f;
}

void CRagebot::FindTargets() {
	targets.clear();
	targets.reserve(ClientState->m_nMaxClients);

	for (int i = 0; i < ClientState->m_nMaxClients; i++) {
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(i));

		if (!player || !player->IsAlive() || player->IsTeammate() || player->m_bDormant() || player->m_bGunGameImmunity())
			continue;

		if (config.ragebot.aimbot.low_fps_mitigations->get(0) && GlobalVars->frametime > 0.02f && (i + GlobalVars->tickcount % 2) % 2)
			continue;

		targets.emplace_back(player);
	}
}

bool CRagebot::CompareRecords(LagRecord* a, LagRecord* b) {
	const Vector vec_diff = a->m_vecOrigin - b->m_vecOrigin;

	if (vec_diff.LengthSqr() > 64)
		return false;

	QAngle angle_diff = a->m_viewAngle - b->m_viewAngle;
	angle_diff.Normalize();

	if (angle_diff.yaw > 90.f)
		return false;

	if (angle_diff.pitch > 10.f)
		return false;

	if (a->breaking_lag_comp != b->breaking_lag_comp)
		return false;

	if (a->shifting_tickbase != b->shifting_tickbase)
		return false;

	return true;
}

std::vector<LagRecord*> CRagebot::SelectRecords(CBasePlayer* player){
	std::vector<LagRecord*> target_records;
	auto& records = LagCompensation->records(player->EntIndex());

	if (records.empty())
		return target_records;

	LagRecord* last_valid_record{ nullptr };
	for (auto i = records.rbegin(); i != records.rend(); i = std::next(i)) {
		const auto record = &*i;
		if (!LagCompensation->ValidRecord(record)) {
			if (record->breaking_lag_comp)
				break;

			continue;
		}

		if (target_records.empty()) {
			target_records.emplace_back(record);
		} else {
			last_valid_record = record;
		}
	}

	if (last_valid_record) {
		if (!target_records.empty() && !CompareRecords(target_records.back(), last_valid_record))
			target_records.emplace_back(last_valid_record);
	}

	if (target_records.empty()) {
		// TODO: should be extrapolation here

		LagRecord* mostRecentRecord = &records.back();
		if (!mostRecentRecord->shifting_tickbase)
			target_records.emplace_back(mostRecentRecord);
	}

	return target_records;
}

void CRagebot::ScanTargets() {
	scanned_targets.clear();
	scanned_targets.reserve(targets.size());

	for (int i = 0; i < targets.size(); i++) {
		auto target = targets[i];
		scanned_targets.emplace_back(ScanTarget(target));
	}
}

void CRagebot::GetMultipoints(LagRecord* record, int hitbox_id, float scale, std::vector<AimPoint_t>& points) {
	studiohdr_t* studiomodel = ModelInfoClient->GetStudioModel(record->player->GetModel());

	if (!studiomodel)
		return;

	mstudiobbox_t* hitbox = studiomodel->GetHitboxSet(record->player->m_nHitboxSet())->GetHitbox(hitbox_id);

	if (!hitbox)
		return;

	if (hitbox->flCapsuleRadius <= 0) // do not scan multipoints for feet
		return;

	matrix3x4_t boneMatrix = record->boneMatrix[hitbox->bone];

	Vector mins, maxs;
	Math::VectorTransform(hitbox->bbmin, boneMatrix, &mins);
	Math::VectorTransform(hitbox->bbmax, boneMatrix, &maxs);

	Vector center = (mins + maxs) * 0.5f;
	Vector sideDirection = center - mins;
	const float width = sideDirection.Normalize() + hitbox->flCapsuleRadius;
	const float radius = hitbox->flCapsuleRadius;

	//DebugOverlay->AddCapsuleOverlay(mins, maxs, radius, 255, 255, 255, 255, 4.f);

	Vector verts[]{
		Vector(0, radius * scale, 0),
		Vector(0, -radius * scale, 0),
		Vector(0, 0, width * scale),
		Vector(0, 0, -width * scale),
	};

	if (hitbox_id == HITBOX_HEAD) {
		verts[2].z = radius * scale;
		verts[3].z = -radius * scale;
	}

	for (const auto& vert : verts)
		points.emplace_back(Math::VectorTransform(vert, boneMatrix));

	if (hitbox_id == HITBOX_HEAD)
		points.emplace_back(Vector(center.x, center.y, center.z + width * scale * 0.85f));
}

int CRagebot::CalcPointsCount() {
	int count = 0;

	for (int hitbox = 0; hitbox < HITBOX_MAX; hitbox++) {
		if (!hitbox_enabled(hitbox))
			continue;

		count++;

		if (multipoints_enabled(hitbox)) {
			if (hitbox == HITBOX_HEAD)
				count += 5;
			else if (hitbox >= HITBOX_PELVIS && hitbox <= HITBOX_UPPER_CHEST)
				count += 4;
		}
	}

	return count;
}

bool CRagebot::IsArmored(int hitbox) {
	switch (hitbox)
	{
	case HITBOX_LEFT_CALF:
	case HITBOX_LEFT_FOOT:
	case HITBOX_LEFT_THIGH:
	case HITBOX_RIGHT_THIGH:
	case HITBOX_RIGHT_FOOT:
	case HITBOX_RIGHT_CALF:
		return false;
	default:
		return true;
	}
}

std::vector<AimPoint_t> CRagebot::SelectPoints(LagRecord* record, bool backtrack_scan) {
	std::vector<AimPoint_t> points;

	points.reserve(CalcPointsCount());

	for (int hitbox = 0; hitbox < HITBOX_MAX; hitbox++) {
		if (!hitbox_enabled(hitbox))
			continue;

		float max_possible_damage = weapon_data->iDamage;
		record->player->ScaleDamage(HitboxToHitgroup(hitbox), weapon_data, max_possible_damage);

		if (!settings.auto_stop->get(1) && max_possible_damage < CalcMinDamage(record->player))
			continue;

		points.emplace_back(AimPoint_t({ record->player->GetHitboxCenter(hitbox, record->boneMatrix), hitbox }));

		if (multipoints_enabled(hitbox))
			GetMultipoints(record, hitbox, hitbox == HITBOX_HEAD ? settings.head_point_scale->get() * 0.01f : settings.body_point_scale->get() * 0.01f, points);
	}

	return points;
}

ScannedPoint_t CRagebot::SelectBestPoint(ScannedTarget_t target) {
	ScannedPoint_t best_body_point;
	ScannedPoint_t best_head_point;

	float player_sim_time = target.player->m_flSimulationTime();

	for (const auto& point : target.points) {
		float modified_damage = point.damage;

		if (point.multipoint)
			modified_damage -= 1.5f;
		if (std::fabs(point.record->m_flSimulationTime - player_sim_time) > 0.05f)
			modified_damage -= 6.f;

		if (point.hitbox == HITBOX_HEAD && modified_damage > best_head_point.damage) 
			best_head_point = point;
		else if (point.hitbox != HITBOX_HEAD && modified_damage > best_body_point.damage)
			best_body_point = point;
	}

	//if (best_body_point.damage < target.player->m_iHealth() && 
	//	best_head_point.damage > 0.f && 
	//	best_head_point.record->m_viewAngle.pitch < 80 &&
	//	best_head_point.damage > target.player->m_iHealth()) // go onshot
	//	return best_head_point;

	if (target.player != last_target && ctx.tickbase_shift == 0 && config.ragebot.aimbot.doubletap->get() && !config.ragebot.aimbot.force_teleport->get()) {
		if (best_body_point.damage > target.player->m_iHealth())
			return best_body_point;

		return best_head_point;
	}

	if (best_body_point.damage > target.minimum_damage)
		return best_body_point;

	return best_head_point;
}

ScannedTarget_t CRagebot::ScanTarget(CBasePlayer* target) {
	std::vector<LagRecord*> records = SelectRecords(target);

	if (records.empty())
		return ScannedTarget_t{};

	float minimum_damage = CalcMinDamage(target);

	ScannedTarget_t result;
	result.player = target;
	result.minimum_damage = minimum_damage;

	LagRecord* backup_record = LagCompensation->BackupData(target);

	for (int i = 0; i < records.size(); i++) {
		LagRecord* record = records[i];
		std::vector<AimPoint_t> points = SelectPoints(record, i > 0);

		LagCompensation->BacktrackEntity(record);

		for (const auto& point : points) {
			if (config.ragebot.aimbot.show_aimpoints->get())
				DebugOverlay->AddBoxOverlay(point.point, Vector(-1, -1, -1), Vector(1, 1, 1), QAngle(0, 0, 0), 255, 255, 255, 200, GlobalVars->interval_per_tick * 2);

			FireBulletData_t bullet;
			if (!AutoWall->FireBullet(Cheat.LocalPlayer, eye_position, point.point, bullet, target))
				continue;

			result.points.emplace_back(ScannedPoint_t{
				record,
				point.point,
				point.hitbox,
				point.multipoint,
				bullet.damage,
				bullet.impacts
			});

			if (bullet.damage > 5.f)
				DoubleTap->block_charge = true;
		}
	}

	result.best_point = SelectBestPoint(result);
	if (!result.best_point.record || result.best_point.damage < 5) {
		LagCompensation->BacktrackEntity(backup_record);
		delete backup_record;
		return result;
	}
	LagCompensation->BacktrackEntity(result.best_point.record);

	result.angle = Math::VectorAngles(result.best_point.point - eye_position);
	result.hitchance = CalcHitchance(result.angle, target, HitboxToDamagegroup(result.best_point.hitbox));

	LagCompensation->BacktrackEntity(backup_record);
	delete backup_record;

	return result;
}

void CRagebot::AutoRevolver()
{
	if (config.ragebot.aimbot.enabled->get())
	{
		if (Cheat.LocalPlayer && Cheat.LocalPlayer->IsAlive())
		{
			ctx.revolver_working = true;
			ctx.cmd->buttons &= ~IN_ATTACK2;

			if (ctx.revolver_working && active_weapon->CanShoot())
			{
				if (GlobalVars->curtime < ctx.next_revolver_time)
					ctx.cmd->buttons |= IN_ATTACK;
				else
				{
					if (GlobalVars->curtime < TICKS_TO_TIME(Cheat.LocalPlayer->m_nTickBase()))
						ctx.cmd->buttons |= IN_ATTACK2;
					else
						ctx.next_revolver_time = GlobalVars->curtime + 0.234375;
				}

				ctx.revolver_working = GlobalVars->curtime > ctx.next_revolver_time;
			}
			else
			{
				ctx.cmd->buttons &= ~IN_ATTACK;

				ctx.next_revolver_time = GlobalVars->curtime + 0.234375;
				ctx.revolver_working = false;
			}
		}
	}
}
__forceinline QAngle CalcAngle(const Vector& src, const Vector& dst)
{
	QAngle vAngle;
	Vector delta((src.x - dst.x), (src.y - dst.y), (src.z - dst.z));
	double hyp = sqrt(delta.x * delta.x + delta.y * delta.y);

	vAngle.pitch = float(atanf(float(delta.z / hyp)) * 57.295779513082f);
	vAngle.yaw = float(atanf(float(delta.y / delta.x)) * 57.295779513082f);
	vAngle.roll = 0.0f;

	if (delta.x >= 0.0)
		vAngle.yaw += 180.0f;

	return vAngle;
}
void CRagebot::Run() {
	if (!config.ragebot.aimbot.enabled->get())
		return;

	if (DoubleTap->IsShifting()) {
		if (doubletap_stop) {
			float current_vel = Math::Q_sqrt(ctx.cmd->sidemove * ctx.cmd->sidemove + ctx.cmd->forwardmove + ctx.cmd->forwardmove);
			const float max_speed = doubletap_stop_speed * 0.75f;

			if (current_vel > 1.f) {
				float factor = max_speed / current_vel;
				ctx.cmd->sidemove *= factor;
				ctx.cmd->forwardmove *= factor;
			}
		}
		return;
	}

	if (Cheat.LocalPlayer->m_fFlags() & FL_FROZEN)
		return;

	active_weapon = Cheat.LocalPlayer->GetActiveWeapon();
	eye_position = Cheat.LocalPlayer->GetShootPosition();

	if (!active_weapon || active_weapon->IsGrenade())
		return;
	
	if (active_weapon->m_iItemDefinitionIndex() == Taser) {
		Zeusbot();
		return;
	}

	/*if (active_weapon->m_iItemDefinitionIndex() == Revolver)
	{
		AutoRevolver();

		if (ctx.revolver_working)
			active_weapon->m_flPostponeFireReadyTime() = ctx.next_revolver_time;
	}*/

	weapon_data = active_weapon->GetWeaponInfo();

	if (!active_weapon->ShootingWeapon())
		return;

	settings = GetWeaponSettings(active_weapon->m_iItemDefinitionIndex());

	doubletap_stop = false;

	FindTargets();

	hook_info.disable_interpolation = true;
	ScanTargets();
	hook_info.disable_interpolation = false;

	ScannedTarget_t best_target;
	bool should_autostop = false;

	debug_data.autostop = false;
	debug_data.damage = 0;
	debug_data.hitchance = 0;
	debug_data.target = "null";

	for (const auto& target : scanned_targets) {
		if (target.best_point.damage > target.minimum_damage && ctx.cmd->command_number - last_target_shot < 150 && target.player == last_target) {
			if (target.hitchance > settings.hitchance->get()) {
				best_target = target;
				break;
			}
			else {
				should_autostop = true;
				best_target.player = nullptr;
			}
		}

		if (target.hitchance > settings.hitchance->get() && target.best_point.damage > max(best_target.best_point.damage, target.minimum_damage))
			best_target = target;

		if (target.best_point.damage > target.minimum_damage) {
			should_autostop = true;
			debug_data.hitchance = target.hitchance;
			debug_data.damage = target.best_point.damage;
			debug_data.target = target.player->GetName();

			if (settings.auto_scope->get() && !Cheat.LocalPlayer->m_bIsScoped() && weapon_data->nWeaponType)
				ctx.cmd->buttons |= IN_ATTACK2;
		}

		if (target.best_point.damage > 15) {
			if (settings.auto_stop->get(1))
				should_autostop = true;
			DoubleTap->block_charge = true;
		}
	}

	if (settings.auto_stop->get(2) && !active_weapon->CanShoot())
		return;

	if (should_autostop)
	{
		ctx.is_peeking = true;
		AutoStop();
	}
	else
		ctx.is_peeking = false;

	debug_data.autostop = should_autostop;

	if (!best_target.player || !active_weapon->CanShoot())
		return;

	Vector vecViewOffset = Cheat.LocalPlayer->m_vecViewOffset();
	if (vecViewOffset.z <= 46.05f)
		vecViewOffset.z = 46.0f;
	else if (vecViewOffset.z > 64.0f)
		vecViewOffset.z = 64.0f;

	auto m_vecShootPosition = Cheat.LocalPlayer->m_vecOrigin() + vecViewOffset;

	QAngle angAimbotAngles = CalcAngle(m_vecShootPosition, best_target.best_point.point);

	//const float flWeaponRecoilScale = cvars.weapon_recoil_scale->GetFloat();
	//if (flWeaponRecoilScale > 0.0f)
	//	angAimbotAngles = Cheat.LocalPlayer->m_aimPunchAngle() * flWeaponRecoilScale;
	//ctx.cmd->viewangles = best_target.angle - Cheat.LocalPlayer->m_aimPunchAngle() * cvars.weapon_recoil_scale->GetFloat();
	ctx.cmd->viewangles = angAimbotAngles;

	if (config.ragebot.aimbot.auto_fire->get())
		ctx.cmd->buttons |= IN_ATTACK;

	ctx.cmd->tick_count = TIME_TO_TICKS(best_target.best_point.record->m_flSimulationTime + LagCompensation->GetLerpTime());

	if (!settings.auto_stop->get(2) && ctx.tickbase_shift > 0) {
		doubletap_stop = true;
		doubletap_stop_speed = active_weapon->MaxSpeed() * 0.3f;
	}

	DoubleTap->ForceTeleport();
	if (!config.antiaim.misc.fake_duck->get())
		ctx.send_packet = true;

	AutoPeek->returning = true;
	last_target = best_target.player;
	last_target_shot = ctx.cmd->command_number;

	if (config.visuals.effects.client_impacts->get()) {
		for (const auto& impact : best_target.best_point.impacts)
		{
			DebugOverlay->AddBoxOverlay(impact, Vector(-1, -1, -1), Vector(1, 1, 1), QAngle(),
				config.visuals.effects.client_impacts_color->get().r,
				config.visuals.effects.client_impacts_color->get().g,
				config.visuals.effects.client_impacts_color->get().b,
				config.visuals.effects.client_impacts_color->get().a,
				config.visuals.effects.impacts_duration->get());
		}
	}

	if (config.misc.miscellaneous.shot_effect->get())
	{
		for (const auto& impact : best_target.best_point.impacts)
			Effects->Sparks(impact, 2, 2);
	}

	LagRecord* record = best_target.best_point.record;

	if (config.misc.miscellaneous.logs->get(1)) {
		Console->Log(std::format("shot at {}'s {} [dmg: {:d}] [hc: {}] [bt: {}] [res: {:.1f}deg {}%]", 
			best_target.player->GetName(), 
			GetHitboxName(best_target.best_point.hitbox), 
			static_cast<int>(best_target.best_point.damage), 
			best_target.hitchance, 
			TIME_TO_TICKS(best_target.player->m_flSimulationTime() - record->m_flSimulationTime),
			record->resolver_data.side * best_target.player->GetMaxDesyncDelta(),
			static_cast<int>(record->resolver_data.anim_accuracy * 100.f)
		));
	}
}

void CRagebot::DrawDebugData() {
	Render->Text("target: " + debug_data.target, Vector2(210, 10), Color(255, 255, 255), Verdana, TEXT_DROPSHADOW);
	Render->Text("autostop: " + std::to_string(debug_data.autostop), Vector2(210, 23), Color(255, 255, 255), Verdana, TEXT_DROPSHADOW);
	Render->Text("damage: " + std::to_string(debug_data.damage), Vector2(210, 36), Color(255, 255, 255), Verdana, TEXT_DROPSHADOW);
	Render->Text("hitchance: " + std::to_string(debug_data.hitchance), Vector2(210, 49), Color(255, 255, 255), Verdana, TEXT_DROPSHADOW);
	if (config.ragebot.aimbot.minimum_damage_override_key->get())
		Render->Text("dmg override", Vector2(210, 62), Color(255, 255, 255), Verdana, TEXT_DROPSHADOW);
	if (config.ragebot.aimbot.doubletap_key->get())
		Render->Text("doubletap", Vector2(210, 75), Color(255, 255, 255), Verdana, TEXT_DROPSHADOW);
	if (ctx.is_peeking)
		Render->Text("is peeking", Vector2(210, 88), Color(255, 255, 255), Verdana, TEXT_DROPSHADOW);
	if (ctx.defensive)
		Render->Text("defensive", Vector2(210, 101), Color(255, 255, 255), Verdana, TEXT_DROPSHADOW);
}

void CRagebot::Zeusbot() {
	const Vector shoot_pos = Cheat.LocalPlayer->GetShootPosition();
	const float inaccuracy_tan = std::tan(active_weapon->GetInaccuracy());

	if (!active_weapon->CanShoot())
		return;

	for (int i = 0; i < ClientState->m_nMaxClients; i++) {
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(i));
		auto& records = LagCompensation->records(i);

		if (!player || player->IsTeammate() || !player->IsAlive() || player->m_bDormant())
			continue;

		for (auto i = records.rbegin(); i != records.rend(); i = std::next(i)) {
			const auto record = &*i;
			if (!LagCompensation->ValidRecord(record)) {
				if (record->breaking_lag_comp)
					break;

				continue;
			}

			float distance = ((record->m_vecOrigin + (record->m_vecMaxs + record->m_vecMins) * 0.5f) - shoot_pos).LengthSqr();

			if (distance > 180 * 180)
				continue;

			const Vector points[]{
				player->GetHitboxCenter(HITBOX_STOMACH, record->boneMatrix),
				player->GetHitboxCenter(HITBOX_CHEST, record->boneMatrix),
				player->GetHitboxCenter(HITBOX_UPPER_CHEST, record->boneMatrix),
				player->GetHitboxCenter(HITBOX_LEFT_UPPER_ARM, record->boneMatrix),
				player->GetHitboxCenter(HITBOX_RIGHT_UPPER_ARM, record->boneMatrix)
			};

			for (const auto& point : points) {
				CGameTrace trace = EngineTrace->TraceRay(shoot_pos, point, MASK_SHOT | CONTENTS_GRATE, Cheat.LocalPlayer);

				if (trace.hit_entity != player)
					continue;

				QAngle angle = Math::VectorAngles(point - shoot_pos);

				float hitchance = min(7 / ((shoot_pos - point).Q_Length() * inaccuracy_tan), 1.f);

				if (hitchance < 0.6f)
					continue;

				ctx.cmd->viewangles = angle;
				ctx.cmd->buttons |= IN_ATTACK;
				ctx.cmd->tick_count = TIME_TO_TICKS(record->m_flSimulationTime + LagCompensation->GetLerpTime());

				Console->Log(std::format("shot at {} [hc: {}] [bt: {}]", player->GetName(), (int)(hitchance * 100.f), TIME_TO_TICKS(player->m_flSimulationTime() - record->m_flSimulationTime)));

				return;
			}
		}
	}
}

CRagebot* Ragebot = new CRagebot;