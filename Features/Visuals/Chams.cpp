#include "Chams.h"
#include "../../SDK/Hooks.h"
#include "../../SDK/Globals.h"
#include "../../Utils/Utils.h"
#include "../RageBot/LagCompensation.h"

#include <fstream>

Vector interpolatedBacktrackChams[64];
CChams* Chams = new CChams;

bool CChams::IsLocalPlayerAttachment(CBaseEntity* entity) {
	if (!Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive())
		return false;

	if (EntityList->GetClientEntityFromHandle(entity->moveparent()) == Cheat.LocalPlayer)
		return true;

	if (EntityList->GetClientEntityFromHandle(entity->m_hOwnerEntity()) == Cheat.LocalPlayer)
		return true;

	if (EntityList->GetClientEntityFromHandle(entity->m_hOwner()) == Cheat.LocalPlayer)
		return true;

	if (entity->m_pMoveParent() == Cheat.LocalPlayer)
		return true;

	unsigned long* myWeapons = Cheat.LocalPlayer->m_hMyWeapons();

	for (int i = 0; i < MAX_WEAPONS; i++) {
		if (EntityList->GetClientEntityFromHandle(myWeapons[i]) == entity)
			return true;
	}

	//unsigned long* vm = Cheat.LocalPlayer->m_hViewModel();

	//for (int i = 0; i < MAX_VIEWMODELS; i++) {
	//	if (EntityList->GetClientEntityFromHandle(vm[i]) == entity)
	//		return true;
	//}

	return false;
}

void CChams::LoadChams() {
	std::ofstream("csgo\\materials\\glow.vmt") << R"#("VertexLitGeneric"
  {
		"$envmap"	 "models/effects/cube_white"
		"$envmaptint"	 "[0.25 0.25 0.4]"
		"$envmapfresnel"	 "1"
		"$envmapfresnelminmaxexp"	 "[0 1 2]"
		"$alpha"	 "1"
		"$model"	"1"
		"$ignorez"		"0"
	} )#";
}

void CChams::OverrideMaterial(int type, bool z, Color color, float glowThickness) {
	static IMaterial* materials[] = { 
		MaterialSystem->FindMaterial("debug/debugambientcube"),
		MaterialSystem->FindMaterial("debug/debugdrawflat"),
		MaterialSystem->FindMaterial("glow.vmt"),
	};

	IMaterial* material = materials[type];

	if (type > 1) {
		float thickness = 10.f - glowThickness;
		bool found;

		IMaterialVar* var = material->FindVar("$envmapfresnelminmaxexp", &found);

		if (found)
			var->SetVecValue(0, 1, thickness);
		else
			return;
	}

	material->IncrementReferenceCount();
	material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, z);
	ColorModulate(material, color);
	material->AlphaModulate(color.a / 255.f);

	StudioRender->ForcedMaterialOverride(material);
}

void CChams::ColorModulate(IMaterial* mat, Color color) {
	auto found = false;
	IMaterialVar* var = mat->FindVar("$envmaptint", &found);

	if (found)
		var->SetVecValue(color.r / 255.f, color.g / 255.f, color.b / 255.f);
	else
		return;

	RenderView->SetColorModulation(color.r / 255.f, color.g / 255.f, color.b / 255.f);
}

void CChams::OnDrawModelExecute(void* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* pBoneToWorld, void* edx) {
	static tDrawModelExecute oDME = (tDrawModelExecute)Hooks::ModelRenderVMT->GetOriginal(21);

	const model_t* mdl = info.pModel;
	studiohdr_t* studio = (studiohdr_t*)state.m_pStudioHdr;

	CBaseEntity* ent = EntityList->GetClientEntity(info.entity_index);

	if (!ent)
		return;

	if (ent->IsPlayer()) {
		CBasePlayer* player = (CBasePlayer*)ent;

		if (player == Cheat.LocalPlayer) {
			float scopeBlend = player->m_bIsScoped() ? config.visuals.chams.scope_blend->get() / 100.f : 1.0f;
			float chamsAlpha = config.visuals.chams.local_player->get() ? config.visuals.chams.local_player_color->get().a : 1.f;

			RenderView->SetBlend(min(scopeBlend, chamsAlpha));

			if (config.visuals.chams.local_player->get()) {
				OverrideMaterial(config.visuals.chams.local_player_type->get(), false, config.visuals.chams.local_player_color->get());
			}
		}
		else {
			if (player->IsTeammate())
				return;

			//if (Players.EnableBacktrackChams) {
			//	LagRecord* lastRecord = nullptr;
			//	auto& records = LagCompensation->records(player->EntIndex());

			//	const auto rend = records.rend();
			//	for (auto i = records.rbegin(); i != rend; i = std::next(i)) {
			//		if (!LagCompensation->ValidRecord(*i)) {
			//			if ((*i)->breaking_lag_comp) {
			//				break;
			//			}

			//			continue;
			//		}

			//		lastRecord = *i;
			//	}

			//	if (lastRecord) {
			//		if (interpolatedBacktrackChams[player->EntIndex()].Zero())
			//			interpolatedBacktrackChams[player->EntIndex()] = lastRecord->m_vecOrigin;

			//		interpolatedBacktrackChams[player->EntIndex()].Interpolate(lastRecord->m_vecOrigin, GlobalVars->frametime * 16);

			//		matrix3x4_t backtrackMatrix[256];
			//		memcpy(backtrackMatrix, lastRecord->boneMatrix, 48 * player->GetCachedBoneData().Count());

			//		Utils::MatrixMove(backtrackMatrix, player->GetCachedBoneData().Count(), lastRecord->m_vecOrigin, interpolatedBacktrackChams[player->EntIndex()]);

			//		float delta = (player->m_vecOrigin() - interpolatedBacktrackChams[player->EntIndex()]).Length();

			//		Color clr = *Players.BacktrackChamsColor;

			//		if (delta < 10)
			//			clr.a *= delta / 10;
			//		OverrideMaterial(Players.BacktrackChams.value, true, clr);
			//		oDME(ModelRender, edx, ctx, state, info, backtrackMatrix);
			//	}
			//}

			if (config.visuals.chams.enemy_invisible->get()) {
				// handle glow filled somehow

				if (config.visuals.chams.enemy_type->get() == 2) {
					OverrideMaterial(0, true, config.visuals.chams.enemy_invisible_color->get());
					oDME(ModelRender, edx, ctx, state, info, pBoneToWorld);
				}

				OverrideMaterial(config.visuals.chams.enemy_type->get(), true, config.visuals.chams.enemy_second_color->get(), config.visuals.chams.enemy_glow_thickness->get());
				oDME(ModelRender, edx, ctx, state, info, pBoneToWorld);
			}

			if (config.visuals.chams.enemy->get()) {
				// handle glow filled somehow

				if (config.visuals.chams.enemy_type->get() == 2) {
					OverrideMaterial(0, false, config.visuals.chams.enemy_color->get());
					oDME(ModelRender, edx, ctx, state, info, pBoneToWorld);
				}

				OverrideMaterial(config.visuals.chams.enemy_type->get(), false, config.visuals.chams.enemy_second_color->get(), config.visuals.chams.enemy_glow_thickness->get());
			}
		}
	}
	else if (IsLocalPlayerAttachment(ent)) {
		float scopeBlend = Cheat.LocalPlayer->m_bIsScoped() ? config.visuals.chams.scope_blend->get() : 100.0f;
		RenderView->SetBlend(scopeBlend * 0.01f);

		if (config.visuals.chams.attachments->get()) {
			if (config.visuals.chams.attachments_type->get() == 2) {
				OverrideMaterial(0, false, config.visuals.chams.attachments_color->get());
				oDME(ModelRender, edx, ctx, state, info, pBoneToWorld);
			}

			OverrideMaterial(config.visuals.chams.attachments_type->get(), false, config.visuals.chams.attachemtns_second_color->get(), config.visuals.chams.attachments_glow_thickness->get());
		}
	}
}