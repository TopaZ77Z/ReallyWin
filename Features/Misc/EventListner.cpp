#include "EventListner.h"

#include "../../SDK/Interfaces.h"
#include "../../SDK/Globals.h"
#include "../../Utils/Console.h"
#include "../RageBot/DoubleTap.h"
#include "../RageBot/AnimationSystem.h"
#include "../RageBot/LagCompensation.h"
#include "../RageBot/Interpolate.h"
#include "../RageBot/Ragebot.h"

CEventListner* EventListner = new CEventListner;

static std::vector<const char*> s_RgisterEvents = {
	"player_hurt",
	"player_death",
	"bomb_planted",
	"bomb_defused",
	"bomb_begindefuse",
	"bomb_beginplant",
	"bomb_abortplant",
	"bomb_abortdefuse",
	"bomb_exploded",
	"round_start",
	"round_end",
	"item_purchase",
	"round_freeze_end",
	"bullet_impact",
	"item_equip"
};

void CEventListner::FireGameEvent(IGameEvent* event) {
	const std::string name = event->GetName();

	if (name == "player_hurt") {
		CBasePlayer* victim = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(EngineClient->GetPlayerForUserID(event->GetInt("userid"))));

		if (EngineClient->GetPlayerForUserID(event->GetInt("attacker")) == EngineClient->GetLocalPlayer()) {
			if (config.visuals.esp.hitsound->get())
				EngineClient->ExecuteClientCmd("play buttons/arena_switch_press_02.wav");

			if (config.misc.miscellaneous.logs->get(0)) {
				Console->Log(std::format("hurt {}'s {} for {} damage ({} remaining)", victim->GetName(), GetHitgroupName(event->GetInt("hitgroup")), event->GetInt("dmg_health"), event->GetInt("health")));
			}
		}

		if (victim && victim->m_bDormant())
			victim->m_iHealth() = event->GetInt("health");
	}
	else if (name == "player_death") {
		if (EngineClient->GetPlayerForUserID(event->GetInt("userid")) == EngineClient->GetLocalPlayer()) {
			ctx.reset();
			DoubleTap->target_tickbase_shift = 0;
			ctx.tickbase_shift = 0;
		}

		Interpolate->InvalidateInterpolation(EngineClient->GetPlayerForUserID(event->GetInt("userid")));
	}
	else if (name == "round_start") {
		Console->Log(">>>>>>>>>>>>>>>>>>>>\nRound Started\n<<<<<<<<<<<<<<<<<<\n");
		LagCompensation->Reset();
		Cheat.freezetime = true;
	}
	else if (name == "round_freeze_end") {
		Cheat.freezetime = false;
	}
	else if (name == "bullet_impact") {
		if (EngineClient->GetPlayerForUserID(event->GetInt("userid")) == EngineClient->GetLocalPlayer() && config.visuals.effects.server_impacts->get() && Cheat.LocalPlayer) {
			Vector pos = Vector(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));
			Color col = config.visuals.effects.server_impacts_color->get();
			DebugOverlay->AddBoxOverlay(pos, Vector(-1, -1, -1), Vector(1, 1, 1), QAngle(), col.r, col.g, col.b, col.a, config.visuals.effects.impacts_duration->get());
		}
	}
	else if (name == "item_equip") {
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(EntityList->GetClientEntity(EngineClient->GetPlayerForUserID(event->GetInt("userid"))));

		if (player && player->m_bDormant()) {
			long new_weapon = event->GetInt("defindex");
			unsigned long* weapons = player->m_hMyWeapons();

			for (int i = 0; i < MAX_WEAPONS; i++) {
				unsigned long weapon_handle = weapons[i];
				CBaseCombatWeapon* weapon = reinterpret_cast<CBaseCombatWeapon*>(EntityList->GetClientEntityFromHandle(weapon_handle));

				if (!weapon)
					continue;

				if (weapon->m_iItemDefinitionIndex() == new_weapon) {
					player->m_hActiveWeapon() = weapon_handle;
					break;
				}
			}
		}
	}
}

int CEventListner::GetEventDebugID() {
	return 42;
}

void CEventListner::Register() {
	m_iDebugId = 42;

	for (auto name : s_RgisterEvents) {
		GameEventManager->AddListener(this, name, false);
	}
}

void CEventListner::Unregister() {
	GameEventManager->RemoveListener(this);
}