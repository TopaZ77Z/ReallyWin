#include "IEngineTrace.h"

#include "../../Utils/Utils.h"
#include "../Misc/CBaseEntity.h"

bool IEngineTrace::ClipRayToHitboxes(const Ray_t& ray, unsigned int fMask, CBaseEntity* pEnt, CGameTrace* pTrace) {
	static auto _ClipRayToHitboxes = reinterpret_cast<bool(__thiscall*)(IEngineTrace*, const Ray_t&, unsigned int, ICollideable*, CGameTrace*)>(Utils::PatternScan("engine.dll", "55 8B EC 83 EC 68 56 57 6A 54 8D 45 98 6A 00 50"));
	return _ClipRayToHitboxes(this, ray, fMask, pEnt->GetCollideable(), pTrace);
}

bool IEngineTrace::ClipRayToPlayer(const Ray_t& ray, unsigned int fMask, CBaseEntity* pEnt, CGameTrace* pTrace) {
	if (!EngineTrace->ClipRayToHitboxes(ray, fMask, pEnt, pTrace))
		return false;

	if (pTrace->DidHit())
		pTrace->hit_entity = pEnt;
	else
		return false;

	return true;
}

Vector IEngineTrace::ClosestPoint(const Vector& start, const Vector& end, const Vector& point) {
	const Vector dir = end - start;
	const Vector diff = point - start;

	const float t = diff.Dot(dir) / dir.LengthSqr();
	const Vector closest_point = start + dir * t;

	return closest_point;
}

float IEngineTrace::DistanceToRay(const Vector& start, const Vector& end, const Vector& point) {
	return (point - ClosestPoint(start, end, point)).Length();
}

float IEngineTrace::SegmentToSegment(const Vector& s1, const Vector& s2, const Vector& k1, const Vector& k2) {
	static auto constexpr epsilon = 0.00000001f;

	auto u = s2 - s1;
	auto v = k2 - k1;
	auto w = s1 - k1;

	auto a = u.Dot(u);
	auto b = u.Dot(v);
	auto c = v.Dot(v);
	auto d = u.Dot(w);
	auto e = v.Dot(w);
	auto D = a * c - b * b;

	auto sn = 0.0f, sd = D;
	auto tn = 0.0f, td = D;

	if (D < epsilon)
	{
		sn = 0.0f;
		sd = 1.0f;
		tn = e;
		td = c;
	}
	else
	{
		sn = b * e - c * d;
		tn = a * e - b * d;

		if (sn < 0.0f)
		{
			sn = 0.0f;
			tn = e;
			td = c;
		}
		else if (sn > sd)
		{
			sn = sd;
			tn = e + b;
			td = c;
		}
	}

	if (tn < 0.0f)
	{
		tn = 0.0f;

		if (-d < 0.0f)
			sn = 0.0f;
		else if (-d > a)
			sn = sd;
		else
		{
			sn = -d;
			sd = a;
		}
	}
	else if (tn > td)
	{
		tn = td;

		if (-d + b < 0.0f)
			sn = 0.0f;
		else if (-d + b > a)
			sn = sd;
		else
		{
			sn = -d + b;
			sd = a;
		}
	}

	auto sc = fabs(sn) < epsilon ? 0.0f : sn / sd;
	auto tc = fabs(tn) < epsilon ? 0.0f : tn / td;

	auto dp = w + u * sc - v * tc;
	return dp.Length();
} 

bool IEngineTrace::IntersectsCylinder(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs, float radius) {
	const Vector rd = end - start;

	const Vector ba = maxs - mins;
	const Vector oc = start - mins;

	const float baba = ba.Dot(ba);
	const float bard = ba.Dot(rd);
	const float baoc = ba.Dot(oc);

	const float k2 = baba - bard * bard;
	const float k1 = baba * oc.Dot(rd) - baoc * bard;
	const float k0 = baba * oc.Dot(oc) - baoc * baoc - radius * radius * baba;

	float h = k1 * k1 - k2 * k0;
	if (h < 0.0f)
		return false;

	h = std::sqrt(h);
	float t = (-k1 - h) / k2;
	// body
	float y = baoc + t * bard;
	if (y > 0.0 && y < baba)
		return true;
	// caps
	t = (((y < 0.0f) ? 0.0f : baba) - baoc) / bard;
	if (abs(k1 + k2 * t) < h) 
	{
		return true;
	}
	return false;
}

bool IEngineTrace::IntersectsCapsule(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs, float radius) {
	const float dist_to_mins = DistanceToRay(start, end, mins);
	const float dist_to_maxs = DistanceToRay(start, end, maxs);

	if (dist_to_mins <= radius || dist_to_maxs <= radius)
		return true;

	const Vector capsule_dir = maxs - mins;
	const float capuse_length = capsule_dir.Length();

	if (dist_to_maxs > radius + capuse_length || dist_to_mins > radius + capuse_length)
		return false;

	return IntersectsCylinder(start, end, mins, maxs, radius);
}