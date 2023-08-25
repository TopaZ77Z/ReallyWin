#include "World.h"
#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"
#include <string>
#include "../../Utils/Utils.h"
#include "../RageBot/AnimationSystem.h"
#include "../RageBot/AutoWall.h"
#include "../RageBot/Interpolate.h"

void CWorld::Modulation() {
	if (!Cheat.InGame)
		return;

	for (auto i = MaterialSystem->FirstMaterial(); i != MaterialSystem->InvalidMaterial(); i = MaterialSystem->NextMaterial(i)) {
		IMaterial* material = MaterialSystem->GetMaterial(i);
			
		if (!material)
			continue;

		if (material->IsErrorMaterial())
			continue;

		if (strstr(material->GetTextureGroupName(), "World")) {
			if (config.visuals.effects.world_color_enable->get()) {
				const Color clr = config.visuals.effects.world_color->get();
				material->ColorModulate(clr.r / 255.f, clr.g / 255.f, clr.b / 255.f);
				material->AlphaModulate(clr.a / 255.f);
			}
			else {
				material->ColorModulate(1, 1, 1);
				material->AlphaModulate(1);
			}
		}
		else if (strstr(material->GetTextureGroupName(), "StaticProp")) {
			if (config.visuals.effects.props_color_enable->get()) {
				const Color clr = config.visuals.effects.props_color->get();
				material->ColorModulate(clr.r / 255.f, clr.g / 255.f, clr.b / 255.f);
				material->AlphaModulate(clr.a / 255.f);
				material->SetMaterialVarFlag(MATERIAL_VAR_TRANSLUCENT, false);
				cvars.r_DrawSpecificStaticProp->SetInt(1);
			}
			else {
				material->ColorModulate(1, 1, 1);
				material->AlphaModulate(1);
			}
		}
	}
}

void CWorld::Fog() {
	if (config.visuals.effects.override_fog->get()) {
		cvars.fog_override->SetInt(1);
		cvars.fog_start->SetInt(config.visuals.effects.fog_start->get() * 8);
		cvars.fog_end->SetInt(config.visuals.effects.fog_end->get() * 8);
		cvars.fog_maxdensity->SetFloat(config.visuals.effects.fog_density->get() * 0.01f);
		cvars.fog_color->SetString((std::to_string(config.visuals.effects.fog_color->get().r) + " " + std::to_string(config.visuals.effects.fog_color->get().g) + " " + std::to_string(config.visuals.effects.fog_color->get().b)).c_str());
	}
	else {
		cvars.fog_override->SetInt(0);
	}
}

void CWorld::SkyBox() {
	static auto load_skybox = reinterpret_cast<void(__fastcall*)(const char*)>(Utils::PatternScan("engine.dll", "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45"));

	if (!Cheat.InGame)
		return;

	switch (config.visuals.effects.override_skybox->get()) {
	case 1:
		load_skybox("sky_csgo_cloudy01");
		break;
	case 2:
		load_skybox("sky_csgo_night02");
		break;
	case 3:
		load_skybox("sky_csgo_night02b");
		break;
	}
}

void CWorld::ProcessCamera(CViewSetup* view_setup) {
	if (!Cheat.InGame || !Cheat.LocalPlayer)
		return;

	if (config.visuals.effects.thirdperson->get() && Cheat.LocalPlayer->IsAlive()) {
		Input->m_fCameraInThirdPerson = true;
		QAngle angles; EngineClient->GetViewAngles(&angles);
		QAngle backAngle = QAngle(angles.yaw - 180, -angles.pitch, 0);
		backAngle.Normalize();
		Vector cameraDirection = Math::AngleVectors(angles);

		CGameTrace trace;
		CTraceFilterWorldOnly filter;
		Ray_t ray;
		Vector eyePos = Cheat.LocalPlayer->m_vecViewOffset() + Cheat.LocalPlayer->GetAbsOrigin();
		ray.Init(eyePos, eyePos - cameraDirection * config.visuals.effects.thirdperson_distance->get(), Vector(-16, -16, -16), Vector(16, 16, 16));

		EngineTrace->TraceRay(ray, CONTENTS_SOLID, &filter, &trace);

		float distance = trace.fraction * config.visuals.effects.thirdperson_distance->get();

		Input->m_vecCameraOffset = Vector(angles.pitch, angles.yaw, distance);
	}
	else {
		Input->m_fCameraInThirdPerson = false;
	}

	if (Cheat.LocalPlayer && (!Cheat.LocalPlayer->IsAlive() || Cheat.LocalPlayer->m_iTeamNum() == 1) && Cheat.LocalPlayer->m_iObserverMode() == OBS_MODE_CHASE) { // disable spectators interpolation
		CBasePlayer* observer = (CBasePlayer*)EntityList->GetClientEntityFromHandle(Cheat.LocalPlayer->m_hObserverTarget());

		if (observer) {
			Vector dir = Math::AngleVectors(view_setup->angles);
			Vector origin = Interpolate->GetInterpolated(observer) + Vector(0, 0, 64 - observer->m_flDuckAmount() * 28);
			CGameTrace trace = EngineTrace->TraceHull(origin, 
				origin - dir * config.visuals.effects.thirdperson_distance->get(), Vector(-20, -20, -20), Vector(20, 20, 20), CONTENTS_SOLID, observer);
			view_setup->origin = trace.endpos;
		}
	}
}

void CWorld::Smoke() {
	if (!Cheat.InGame)
		return;

	static const char* smokeMaterials[] = {
		"particle/vistasmokev1/vistasmokev1_emods",
		"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		"particle/vistasmokev1/vistasmokev1_fire",
		"particle/vistasmokev1/vistasmokev1_smokegrenade"
	};

	for (const char* mat : smokeMaterials) {
		IMaterial* material = MaterialSystem->FindMaterial(mat);
		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, config.visuals.effects.removals->get(3));
	}
}

void CWorld::Crosshair() {
	cvars.weapon_debug_spread_show->SetInt((config.visuals.other_esp.sniper_crosshair->get() && Cheat.LocalPlayer && Cheat.LocalPlayer->IsAlive() && !Cheat.LocalPlayer->m_bIsScoped()) ? 2 : 0);

	if (Cheat.InGame && Cheat.LocalPlayer->IsAlive() && config.visuals.effects.removals->get(5) && Cheat.LocalPlayer->m_bIsScoped()) {
		Render->BoxFilled(Vector2(Cheat.ScreenSize.x / 2, 0), Vector2(Cheat.ScreenSize.x / 2 + 1, Cheat.ScreenSize.y), Color(10, 10, 10, 255));
		Render->BoxFilled(Vector2(0, Cheat.ScreenSize.y / 2), Vector2(Cheat.ScreenSize.x, Cheat.ScreenSize.y / 2 + 1), Color(10, 10, 10, 255));
	}

	if (!config.visuals.other_esp.penetration_crosshair->get())
		return;

	if (!Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive())
		return;

	CBaseCombatWeapon* activeWeapon = Cheat.LocalPlayer->GetActiveWeapon();

	if (!activeWeapon || !activeWeapon->ShootingWeapon())
		return;

	Vector eyePos = Cheat.LocalPlayer->GetEyePosition();

	QAngle viewAngle;
	EngineClient->GetViewAngles(&viewAngle);

	Vector direction = Math::AngleVectors(viewAngle);

	FireBulletData_t fb_data;
	bool hit = AutoWall->FireBullet(Cheat.LocalPlayer, eyePos, eyePos + direction * 8192, fb_data);

	Color crosshairColor(255, 20, 20);

	if (fb_data.impacts.size() > 1) {
		crosshairColor = Color(20, 255, 20);

		if (hit)
			crosshairColor = Color(56, 209, 255);
	}

	Render->BoxFilled(Cheat.ScreenSize * 0.5f - Vector2(0, 2), Cheat.ScreenSize * 0.5f + Vector2(1, 3), crosshairColor);
	Render->BoxFilled(Cheat.ScreenSize * 0.5f - Vector2(2, 0), Cheat.ScreenSize * 0.5f + Vector2(3, 1), crosshairColor);
}

void CWorld::RemoveBlood() {
	if (!Cheat.InGame)
		return;

	static const char* bloodMaterials[] = {
		"decals/blood8",
		"decals/blood7",
		"decals/blood6",
		"decals/blood5",
		"decals/blood4",
		"decals/blood3",
		"decals/blood2",
		"decals/blood1",
		//"decals/blood6_subrect",
		//"decals/blood5_subrect",
		//"decals/blood4_subrect",
		//"decals/blood3_subrect",
		//"decals/blood2_subrect",
		//"decals/blood1_subrect",
	};

	for (const char* mat : bloodMaterials) {
		IMaterial* material = MaterialSystem->FindMaterial(mat);

		if (!material || material->IsErrorMaterial())
			continue;

		material->IncrementReferenceCount();
		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, config.visuals.effects.removals->get(6));
	}
}

CWorld* World = new CWorld;