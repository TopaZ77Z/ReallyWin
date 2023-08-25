#include "Interpolate.h"
#include "AnimationSystem.h"

void CInterpolate::DisableInterpolationFlags(CBasePlayer* player) {
	auto& var_mapping = player->m_VarMapping();

	for (int i = 0; i < var_mapping.m_nInterpolatedEntries; ++i)
		var_mapping.m_Entries[i].m_bNeedsToInterpolate = false;
}

Vector CInterpolate::GetInterpolated(CBasePlayer* player) {
	return interpolate_data[player->EntIndex()].origin;
}

void CInterpolate::RunInterpolation() {
	for (int i = 0; i < 64; i++) {
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(i));

		if (!player || player == Cheat.LocalPlayer || !player->IsAlive() || player->m_bDormant())
			continue;

		const Vector origin = player->m_vecOrigin();

		interpolate_data_t* data = &interpolate_data[i];

		if (!data->valid || (origin - data->origin).LengthSqr() > 8192) {
			data->origin = origin;
			data->valid = true;
			continue;
		}

		data->origin += (origin - data->origin) * std::clamp(GlobalVars->frametime * 32, 0.f, 0.8f);
	}
}

void CInterpolate::InterpolateModel(CBasePlayer* player, matrix3x4_t* matrix) {
	if (player == Cheat.LocalPlayer)
		return;

	interpolate_data_t* data = &interpolate_data[player->EntIndex()];

	if (!data->valid)
		return;

	Utils::MatrixMove(data->original_matrix, matrix, player->GetCachedBoneData().Count(), player->m_vecOrigin(), data->origin);
}

void CInterpolate::ResetInterpolation() {
	for (int i = 0; i < 64; i++)
		interpolate_data[i].valid = false;
}

void CInterpolate::InvalidateInterpolation(int i) {
	interpolate_data[i].valid = false;
}

CInterpolate* Interpolate = new CInterpolate;