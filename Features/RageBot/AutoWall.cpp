#include "AutoWall.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Misc/CBasePlayer.h"
#include "../../Utils/Utils.h"

inline bool CGameTrace::DidHitWorld() const
{
	return hit_entity->EntIndex() == 0;
}
inline bool CGameTrace::DidHitNonWorldEntity() const
{
	return hit_entity != NULL && !DidHitWorld();
}

void CAutoWall::TraceLine(const Vector& absStart, const Vector& absEnd, unsigned int mask, void* ignore, CGameTrace* ptr)
{
	Ray_t ray;
	ray.Init(absStart, absEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;

	EngineTrace->TraceRay(ray, mask, &filter, ptr);
}

void CAutoWall::ClipTraceToPlayers(const Vector& start, const Vector& end, const unsigned int mask, ITraceFilter* filter, CGameTrace* trace, CBasePlayer* target = nullptr) {
	/*static void* UTIL_ClipTraceToPlayers = Utils::PatternScan("client.dll", "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 0F 57 C9");

	_asm
	{
		MOV		EAX, filter
		LEA		ECX, trace
		PUSH	ECX
		PUSH	EAX
		PUSH	mask
		LEA		EDX, start
		LEA		ECX, end
		CALL	UTIL_ClipTraceToPlayers
		ADD		ESP, 0xC
	}*/

	CGameTrace playerTrace;
	Ray_t ray(start, end);

	if (target) {
		if (!EngineTrace->ClipRayToPlayer(ray, mask, target, &playerTrace))
			return;

		if (playerTrace.fraction < trace->fraction)
			*trace = playerTrace;

		return;
	}

	float minFraction = trace->fraction;
	for (int i = 0; i < ClientState->m_nMaxClients; ++i) {
		CBasePlayer* pl = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(i));

		if (!pl || !pl->IsAlive() || pl->m_bDormant())
			continue;

		if (filter && !filter->ShouldHitEntity((IHandleEntity*)pl, mask))
			continue;

		const Vector mins = pl->m_vecMins();
		const Vector maxs = pl->m_vecMaxs();

		const Vector center = pl->m_vecOrigin() + (maxs + mins) * 0.5f;

		if (EngineTrace->DistanceToRay(start, end, center) > 60.f)
			continue;

		if (!EngineTrace->ClipRayToPlayer(ray, mask, pl, &playerTrace))
			continue;

		if (playerTrace.fraction < minFraction) {
			*trace = playerTrace;
			minFraction = playerTrace.fraction;
		}
	}
}

bool CAutoWall::TraceToExit(CGameTrace& enterTrace, CGameTrace& exitTrace, const Vector& startPosition, const Vector& direction)
{
	Vector start;
	float maxDistance = 90.f, rayExtension = 4.f, currentDistance = 0;
	int firstContents = 0;

	while (currentDistance <= maxDistance)
	{
		currentDistance += rayExtension;

		start = startPosition + direction * currentDistance;

		int pointContents = EngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		if (!firstContents)
		{
			firstContents = pointContents;
		}

		if (pointContents & MASK_SHOT_HULL && (!(pointContents & CONTENTS_HITBOX) || pointContents == firstContents))
			continue;

		TraceLine(start, start - (direction * rayExtension), MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr, &exitTrace);

		if (exitTrace.startsolid && exitTrace.surface.flags & SURF_HITBOX)
		{
			TraceLine(start, startPosition, MASK_SHOT_HULL, exitTrace.hit_entity, &exitTrace);

			if (exitTrace.DidHit() && !exitTrace.startsolid)
			{
				start = exitTrace.endpos;
				return true;
			}
			continue;
		}

		if (exitTrace.DidHit() && !exitTrace.startsolid)
		{
			if (enterTrace.hit_entity->IsBreakable() && exitTrace.hit_entity->IsBreakable())
				return true;

			if (enterTrace.surface.flags & SURF_NODRAW || !(exitTrace.surface.flags & SURF_NODRAW) && (exitTrace.plane.normal.Dot(direction) <= 1.f))
			{
				float multAmount = exitTrace.fraction * 4.f;
				start -= direction * multAmount;
				return true;
			}

			continue;
		}

		if (!exitTrace.DidHit() || exitTrace.startsolid) {
			if (enterTrace.DidHitNonWorldEntity() && enterTrace.hit_entity->IsBreakable()) {
				exitTrace = enterTrace;
				exitTrace.endpos = start + direction;
				return true;
			}

			continue;
		}
	}
	return false;
}

bool CAutoWall::HandleBulletPenetration(CBasePlayer* attacker, CCSWeaponData* weaponData, CGameTrace& enterTrace, surfacedata_t* enterSurfaceData, Vector& eyePosition, Vector direction, int& possibleHitsRemaining, float& currentDamage)
{
	static ConVar* damageReductionBullets = CVar->FindVar("ff_damage_reduction_bullets");
	static ConVar* damageBulletPenetration = CVar->FindVar("ff_damage_bullet_penetration");

	const float ff_damage_reduction_bullets = damageReductionBullets->GetFloat();
	const float ff_damage_bullet_penetration = damageBulletPenetration->GetFloat();

	int enterMaterial = enterSurfaceData->game.material;

	if (possibleHitsRemaining == 0 && enterMaterial != CHAR_TEX_GRATE && enterMaterial != CHAR_TEX_GLASS && !(enterTrace.surface.flags & SURF_NODRAW))
		return false;

	if (weaponData->flPenetration <= 0.0f || possibleHitsRemaining <= 0)
		return false;

	CGameTrace exitTrace;
	if (!TraceToExit(enterTrace, exitTrace, enterTrace.endpos, direction) && !(EngineTrace->GetPointContents(enterTrace.endpos, MASK_SHOT_HULL, nullptr) & MASK_SHOT_HULL))
		return false;

	float finalDamageModifier = 0.16f;
	float combinedPenetrationModifier;

	surfacedata_t* exitSurfaceData = PhysicSurfaceProps->GetSurfaceData(exitTrace.surface.surfaceProps);
	int exitMaterial = exitSurfaceData->game.material;

	float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;
	float exitSurfPenetrationModifier = exitSurfaceData->game.flPenetrationModifier;

	if (enterMaterial == CHAR_TEX_GRATE || enterMaterial == CHAR_TEX_GLASS)
	{
		combinedPenetrationModifier = 3.f;
		finalDamageModifier = 0.05f;
	}
	else if (((enterTrace.contents >> 3) & CONTENTS_SOLID) || ((enterTrace.surface.flags >> 7) & SURF_LIGHT))
	{
		combinedPenetrationModifier = 1.f;
	}
	else if (enterMaterial == CHAR_TEX_FLESH && enterTrace.hit_entity && reinterpret_cast<CBasePlayer*>(enterTrace.hit_entity)->IsTeammate() && ff_damage_reduction_bullets == 0.f)
	{
		if (ff_damage_bullet_penetration == 0.f)
			return false;

		combinedPenetrationModifier = ff_damage_bullet_penetration;
		finalDamageModifier = ff_damage_bullet_penetration;
	}
	else
	{
		combinedPenetrationModifier = (enterSurfPenetrationModifier + exitSurfPenetrationModifier) * 0.5f;
	}

	if (enterMaterial == exitMaterial)
	{
		if (exitMaterial == CHAR_TEX_CARDBOARD || exitMaterial == CHAR_TEX_WOOD)
		{
			combinedPenetrationModifier = 3.f;
		}
		else if (exitMaterial == CHAR_TEX_PLASTIC)
		{
			combinedPenetrationModifier = 2.f;
		}
	}

	const float thickness = (exitTrace.endpos - enterTrace.endpos).LengthSqr();
	const float modifier = (combinedPenetrationModifier > 0.f) ? (1.f / combinedPenetrationModifier) : 0.f;

	const float lostDamage = (currentDamage * finalDamageModifier + (weaponData->flPenetration > 0.0f ? 3.75f / weaponData->flPenetration : 0.0f) * (modifier * 3.0f)) + ((modifier * thickness) / 24.0f);

	if (lostDamage > currentDamage)
		return false;

	if (lostDamage > 0.f)
	{
		currentDamage -= lostDamage;
	}

	if (currentDamage < 1.f)
		return false;

	eyePosition = exitTrace.endpos;
	--possibleHitsRemaining;
	return true;
}

bool CAutoWall::FireBullet(CBasePlayer* attacker, const Vector& start, const Vector& end, FireBulletData_t& data, CBasePlayer* target) {
	static ConVar* mp_friendlyfire = CVar->FindVar("mp_friendlyfire");

	Vector eyePosition = start;
	Vector direction = (end - start).Q_Normalized();

	CBaseCombatWeapon* weapon = attacker->GetActiveWeapon();
	CCSWeaponData* weaponData = weapon->GetWeaponInfo();

	if (!weaponData)
		return false;

	CTraceFilter filter;
	filter.pSkip = attacker;
	
	float currentDistance = 0.f;
	float maxRange = weaponData->flRange;

	int possibleHitsRemaining = 4; 
	data.damage = weaponData->iDamage;
	Ray_t ray;

	bool friendlyFire = mp_friendlyfire->GetInt();

	while (possibleHitsRemaining > 0 && data.damage >= 1.f) {
		maxRange -= currentDistance;

		Vector end = eyePosition + direction * maxRange;

		ray.Init(eyePosition, end);
		EngineTrace->TraceRay(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &data.enterTrace);

		//TraceLine(eyePosition, end, MASK_SHOT_HULL | CONTENTS_HITBOX, attacker, &data.enterTrace)
		ClipTraceToPlayers(eyePosition, end + direction * 40.f, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &data.enterTrace);

		data.impacts.emplace_back(data.enterTrace.endpos);

		surfacedata_t* enterSurfaceData = PhysicSurfaceProps->GetSurfaceData(data.enterTrace.surface.surfaceProps);
		const float enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;

		if (data.enterTrace.fraction == 1.f)
			break;

		currentDistance += data.enterTrace.fraction * maxRange;

		data.damage *= std::powf(weaponData->flRangeModifier, (currentDistance * 0.002f));

		if (currentDistance > 3000.f || enterSurfPenetrationModifier < 0.1f)
			break;

		CBasePlayer* hit_player = reinterpret_cast<CBasePlayer*>(data.enterTrace.hit_entity);

		if (hit_player && hit_player->IsTeammate())
			return false;

		if (data.enterTrace.hit_entity && data.enterTrace.hit_entity->IsPlayer())
		{
			hit_player->ScaleDamage(data.enterTrace.hitgroup, weaponData, data.damage);
			return true;
		}

		if (!HandleBulletPenetration(attacker, weaponData, data.enterTrace, enterSurfaceData, eyePosition, direction, possibleHitsRemaining, data.damage))
			break;
	}

	return false;
}

CAutoWall* AutoWall = new CAutoWall;