#include "ESP.h"

#include <string>
#include <algorithm>

#include "../../SDK/Globals.h"
#include "../../SDK/Misc/CBaseCombatWeapon.h"
#include "../../Utils/Utils.h"
#include "GrenadePrediction.h"
#include "../RageBot/LagCompensation.h"
#include "../RageBot/AnimationSystem.h"
#include "../Misc/UI.h"
#include "../RageBot/Interpolate.h"

ESPInfo_t ESPInfo[64];
GrenadeWarning NadeWarning;
CUtlVector<SndInfo_t> soundList;

void ESP::Draw() {
	if (!config.visuals.esp.enable->get() || !Cheat.InGame || !Cheat.LocalPlayer) return;

	for (int i = 0; i < ClientState->m_nMaxClients; i++) {
		ESP::UpdatePlayer(i);
		ESP::DrawPlayer(i);
	}
}

void ESP::ProcessSounds() {
	if (!config.visuals.esp.dormant->get())
		return;

	soundList.RemoveAll();
	EngineSound->GetActiveSounds(soundList);

	for (int i = 0; i < soundList.Count(); i++) {
		SndInfo_t& sound = soundList[i];

		if (sound.m_nSoundSource == 0)
			continue;

		CBaseEntity* sound_source = EntityList->GetClientEntity(sound.m_nSoundSource);

		if (!sound_source)
			continue;

		CBaseEntity* moveparent = EntityList->GetClientEntityFromHandle(sound_source->moveparent());
		CBasePlayer* player = nullptr;

		if (sound_source->IsPlayer())
			player = reinterpret_cast<CBasePlayer*>(sound_source);
		else if (moveparent && moveparent->IsPlayer())
			player = reinterpret_cast<CBasePlayer*>(moveparent);
		else
			continue;

		Vector origin = *sound.m_pOrigin;

		auto trace = EngineTrace->TraceHull(origin, origin - Vector(0, 0, 128), Vector(-16, -16, 0), Vector(16, 16, 72), MASK_SOLID, player);

		ESPInfo[player->EntIndex()].m_vecOrigin = trace.endpos;
		ESPInfo[player->EntIndex()].m_flLastUpdateTime = GlobalVars->curtime;
		ESPInfo[player->EntIndex()].m_nFlags = player->m_fFlags();
     	ESPInfo[player->EntIndex()].m_nFlags |= (trace.fraction < 0.50f ? FL_DUCKING : 0) | (trace.fraction < 1.0f ? FL_ONGROUND : 0);
		ESPInfo[player->EntIndex()].m_nFlags &= (trace.fraction >= 0.50f ? ~FL_DUCKING : 0) | (trace.fraction >= 1.0f ? ~FL_ONGROUND : 0);
	}
}

void ESP::UpdatePlayer(int id) {
	CBasePlayer* player = (CBasePlayer*)EntityList->GetClientEntity(id);
	ESPInfo_t& info = ESPInfo[id];

	info.m_pEnt = player;
	if (!player || player->m_iTeamNum() == Cheat.LocalPlayer->m_iTeamNum() || !player->IsPlayer() || !player->IsAlive()) {
		info.m_bValid = false;
		return;
	}

	info.m_bDormant = player->m_bDormant();

	auto& records = LagCompensation->records(id);

	if (records.empty()) {
		info.m_bValid = false;
		return;
	}

	LagRecord* latestRecord = &records.back();

	if (latestRecord->m_flDuckAmout > 0.01f && latestRecord->m_flDuckAmout < 0.9 && player->m_flDuckSpeed() == 8)
		info.m_nFakeDuckTicks++;
	else
		info.m_nFakeDuckTicks = 0;

	if (!info.m_bDormant)
		info.m_vecOrigin = Interpolate->GetInterpolated(player);

	float playerHeight = player->m_vecMaxs().z;
	Vector2 head = Render->WorldToScreen(info.m_vecOrigin + Vector(0, 0, playerHeight));
	Vector2 feet = Render->WorldToScreen(info.m_vecOrigin);
	int h = feet.y - head.y;
	int w = (h / playerHeight) * 32;
	int bboxCenter = (feet.x + head.x) * 0.5f;
	info.m_BoundingBox[0] = Vector2(bboxCenter - w * 0.5f, head.y);
	info.m_BoundingBox[1] = Vector2(bboxCenter + w * 0.5f, feet.y);

	info.m_bValid = info.m_BoundingBox[0].x > 0 && info.m_BoundingBox[0].y > 0 && info.m_BoundingBox[1].x < Cheat.ScreenSize.x&& info.m_BoundingBox[1].y < Cheat.ScreenSize.y;

	if (info.m_bDormant) {
		player->m_fFlags() = info.m_nFlags;
		float unupdatedTime = GlobalVars->curtime - info.m_flLastUpdateTime;

		if (unupdatedTime > 5)
			info.m_Alpha = (10 - unupdatedTime) / 5 * 255;
		else
			info.m_Alpha = 255;

		if (unupdatedTime > 10)
			info.m_bValid = false;
	}
	else {
		info.m_Alpha = 255;
		info.m_flLastUpdateTime = GlobalVars->curtime;
	}
}

void ESP::DrawPlayer(int id) {
	ESPInfo_t info = ESPInfo[id];

	if (!info.m_bValid)
		return;

	CBasePlayer* player = info.m_pEnt;

	if (info.m_bDormant && !config.visuals.esp.dormant->get())
		return;

	if (info.m_BoundingBox[0].Invalid() || info.m_BoundingBox[1].Invalid())
		return;

	DrawBox(info);
	DrawHealth(info);
	DrawName(info);
	DrawFlags(info);
	DrawWeapon(info);
}

void ESP::DrawBox(ESPInfo_t info) {
	if (!config.visuals.esp.bounding_box->get())
		return;

	Color clr = config.visuals.esp.box_color->get();
	if (info.m_bDormant) {
		clr = Color(220, 220, 220);
	}
	clr.a = info.m_Alpha;

	Render->Box(info.m_BoundingBox[0], info.m_BoundingBox[1], clr);
	Render->Box(info.m_BoundingBox[0] - Vector2(1, 1), info.m_BoundingBox[1] + Vector2(1, 1), Color(0, 0, 0, 150 * clr.a / 255));
	Render->Box(info.m_BoundingBox[0] + Vector2(1, 1), info.m_BoundingBox[1] - Vector2(1, 1), Color(0, 0, 0, 150 * clr.a / 255));
}

void ESP::DrawHealth(ESPInfo_t info) {
	if (!config.visuals.esp.health_bar->get())
		return;

	Color clr = Color(0, 255, 0);
	float dormantTime = 0.f;

	if (info.m_bDormant) {
		clr = Color(175, 175, 175);
	}

	int health = info.m_pEnt->m_iHealth();
	clr.r = std::clamp(health < 50 ? 250 : (100 - health) / 50.f * 250, 130.f, 230.f);
	clr.g = std::clamp(health > 50 ? 250 : health / 50.f * 250, 100.f, 230.f);
	clr.b = 140;

	if (config.visuals.esp.custom_health->get())
		clr = config.visuals.esp.custom_health_color->get();

	clr.a = info.m_Alpha;

	int h = info.m_BoundingBox[1].y - info.m_BoundingBox[0].y;
	float health_fraction = 1 - health / 100.f;
	Vector2 health_box_start = info.m_BoundingBox[0] - Vector2(7, 1);
	Vector2 health_box_end(info.m_BoundingBox[0].x - 3, info.m_BoundingBox[1].y + 1);

	Render->BoxFilled(health_box_start, health_box_end, Color(15, 15, 15, info.m_Alpha * 0.65f));

	Render->BoxFilled(health_box_start + Vector2(0, 1), Vector2(health_box_start.x + 1, health_box_end.y - 1), Color(10, 10, 10, 255 * info.m_Alpha));
	Render->BoxFilled(Vector2(health_box_end.x - 1, health_box_start.y + 1), Vector2(health_box_end.x, health_box_end.y - 1), Color(10, 10, 10, 255 * info.m_Alpha));
	Render->BoxFilled(health_box_start, Vector2(health_box_end.x, health_box_start.y + 1), Color(10, 10, 10, 255 * info.m_Alpha));
	Render->BoxFilled(Vector2(health_box_start.x, health_box_end.y - 1), health_box_end, Color(10, 10, 10, 255 * info.m_Alpha));

	Render->BoxFilled(health_box_start + Vector2(1, 1 + h * health_fraction), health_box_end - Vector2(1, 1), info.m_bDormant ? Color(220, 220, 220, info.m_Alpha) : clr);

	if (health < 92)
		Render->Text(std::to_string(health), info.m_BoundingBox[0] - Vector2(7.f, -health_fraction * h + 5), Color(230, 230, 230, info.m_Alpha), SmallFont, TEXT_CENTERED | TEXT_OUTLINED);
}

void ESP::DrawName(ESPInfo_t info) {
	if (!config.visuals.esp.name->get())
		return;

	Color clr = config.visuals.esp.name_color->get();
	if (info.m_bDormant) {
		clr = Color(220, 220, 220);
	}

	clr.a *= info.m_Alpha / 255.f;

	Render->Text(info.m_pEnt->GetName(), Vector2((info.m_BoundingBox[0].x + info.m_BoundingBox[1].x) * 0.5f, info.m_BoundingBox[0].y - 14), clr, Verdana, TEXT_CENTERED | TEXT_DROPSHADOW);
}

void ESP::DrawFlags(ESPInfo_t info) {
	bool dormant = info.m_bDormant;
	LagRecord* record = &LagCompensation->records(info.m_pEnt->EntIndex()).back();

	std::vector<ESPFlag_t> flags;

	if (config.visuals.esp.flags->get(0)) {
		std::string str = "";
		if (info.m_pEnt->m_ArmorValue() > 0)
			str = "K";
		if (info.m_pEnt->m_bHasHelmet())
			str = "HK";
		flags.push_back({ str, Color(240, 240, 240, info.m_Alpha) });
	}


	if (config.visuals.esp.flags->get(6))
		flags.push_back({ std::format("{}%", (int)(record->resolver_data.anim_accuracy * 100)), Color(255, 255, 255) });

	if (config.visuals.esp.flags->get(1) && info.m_pEnt->m_bIsScoped() && !dormant)
		flags.push_back({ "ZOOM", Color(120, 160, 200, info.m_Alpha) });

	if (config.visuals.esp.flags->get(2) && info.m_nFakeDuckTicks > 16 && !dormant)
		flags.push_back({ "FD", Color(240, 240, 240, info.m_Alpha) });

	if (config.visuals.esp.flags->get(3) && record->shifting_tickbase && !dormant)
		flags.push_back({ "X", Color(240, 240, 240, info.m_Alpha) });

	if (config.visuals.esp.flags->get(4) && info.m_pEnt->EntIndex() == PlayerResource->m_iPlayerC4())
		flags.push_back({ "BOMB", Color(230, 40, 40, info.m_Alpha) });

	if (config.visuals.esp.flags->get(5) && record->breaking_lag_comp && !dormant)
		flags.push_back({ "LC", Color(230, 40, 40, info.m_Alpha) });

	if (config.visuals.esp.flags->get(6) && record->resolver_data.resolver_type == ResolverType::ANIM && !dormant)
		flags.push_back({ "ANIM", Color(165, 230, 14, info.m_Alpha) });

	int line_offset = 0;
	for (const auto& flag : flags) {
		Render->Text(flag.flag, Vector2(info.m_BoundingBox[1].x + 3, info.m_BoundingBox[0].y + line_offset), dormant ? Color(240, 240, 240, info.m_Alpha) : flag.color, SmallFont, TEXT_OUTLINED);
		line_offset += 10;
	}
}

void ESP::DrawWeapon(ESPInfo_t info) {
	if (!config.visuals.esp.weapon_text->get())
		return;

	CBaseCombatWeapon* activeWeapon = info.m_pEnt->GetActiveWeapon();

	if (!activeWeapon)
		return;

	const char* weap = activeWeapon->GetName();

	Render->Text(std::string(weap), Vector2((info.m_BoundingBox[0].x + info.m_BoundingBox[1].x) / 2, info.m_BoundingBox[1].y + 2), info.m_bDormant ? Color(220, 220, 220, info.m_Alpha) : config.visuals.esp.weapon_text_color->get(), SmallFont, TEXT_OUTLINED | TEXT_CENTERED);
}

void ESP::DrawGrenades() {
	static auto mp_friendlyfire = CVar->FindVar("mp_friendlyfire");

	if (!Cheat.InGame || !config.visuals.other_esp.grenades->get()) return;
	
	for (int i = 0; i < EntityList->GetMaxEntities(); i++) {
		CBaseGrenade* ent = reinterpret_cast<CBaseGrenade*>(EntityList->GetClientEntity(i));

		if (!ent)
			continue;

		auto classId = ent->GetClientClass();

		if (!classId)
			continue;

		if (classId->m_ClassID == C_INFERNO) {
			CBasePlayer* owner = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntityFromHandle(ent->m_hOwnerEntity()));

			if (owner != Cheat.LocalPlayer && owner->IsTeammate() && !mp_friendlyfire->GetInt())
				continue;

			Vector2 pos = Render->WorldToScreen(ent->GetAbsOrigin());

			if (pos.Invalid())
				continue;

			float distance = (Cheat.LocalPlayer->GetAbsOrigin() - ent->GetAbsOrigin()).Q_Length();

			if (distance > 700)
				continue;

			float distance_alpha = std::clamp(1.f - (distance - 600.f) / 100.f, 0.f, 1.f);

			float alpha = std::clamp((7.03125f - (GlobalVars->curtime - ent->m_flInfernoSpawnTime())) * 2.f, 0.f, 1.f) * distance_alpha;

			Render->CircleFilled(pos, 30, Color(16, 16, 16, 190 * alpha));
			Render->GlowCircle2(pos, 27, Color(40, 40, 40, 255 * alpha), Color(20, 20, 20, 255 * alpha));
			Render->GlowCircle(pos, 25, Color(255, 50, 50, std::clamp((430 - distance) / 250.f, 0.f, 1.f) * 255 * alpha));

			Render->Image(Resources::Inferno, pos - Vector2(15, 15), Color(255, 255, 255, 230 * alpha));

			continue;
		}

		const model_t* model = ent->GetModel();

		if (!model)
			continue;

		studiohdr_t* studioModel = ModelInfoClient->GetStudioModel(model);

		if (!studioModel)
			continue;

		if (strstr(studioModel->szName, "thrown") || classId->m_ClassID == C_BASE_CS_GRENADE_PROJECTILE || classId->m_ClassID == C_MOLOTOV_PROJECTILE || classId->m_ClassID == C_DECOY_PROJECTILE) {
			int weapId;

			CBasePlayer* thrower = EntityList->GetClientEntity(ent->m_hThrower())->GetPlayer();

			if (thrower && thrower->IsTeammate() && mp_friendlyfire->GetInt() == 0)
				continue;

			if (strstr(studioModel->szName, "fraggrenade"))
			{
				if (ent->GetCreationTime() + 1.625f <= TICKS_TO_TIME(GlobalVars->tickcount))
					continue;

				weapId = HeGrenade;
			}
			else if (strstr(studioModel->szName, "incendiarygrenade") || strstr(studioModel->szName, "molotov"))
			{
				weapId = Molotov;
			}
			else
				continue;

			if (config.visuals.other_esp.grenade_proximity_warning->get()) {
				NadeWarning.Warning(ent, weapId);
			}
		}
	}
}