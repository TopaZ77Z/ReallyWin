#include "UI.h"
#include "../../SDK/Globals.h"
#include "../../SDK/Render.h"
#include "../../Utils/Animation.h"
#include "../../Utils/Utils.h"
#include "../Visuals/GrenadePrediction.h"

#include <vector>

//void UI::KeyBinds() {
//	static std::vector<UIKeyBind_t> keybindDrawList{
//		UIKeyBind_t("Fake Duck", &AntiAim.FakeDuckKey),
//		UIKeyBind_t("Double Tap", &RageBot.DoubleTapBind),
//		UIKeyBind_t("Auto Peek", &Misc.AutoPeekBind),
//		UIKeyBind_t("Minimum Damage", &RageBot.MinDamageOverrideBind),
//		UIKeyBind_t("Edge Jump", &Misc.EdgeJumpBind),
//		UIKeyBind_t("Roll Resolver", &RageBot.RollResolverBind)
//	};
//	static float windowAlpha = 0.f;
//	static Vector2 draggingOffset;
//
//	bool renderingBinds = false;
//	Vector2 windowSize(200, 30);
//
//	for (auto& bind : keybindDrawList) {
//		bind.alpha = interpolate(bind.alpha, bind.pointer->GetState() ? 1 : 0, 8);
//
//		windowSize.y += bind.alpha * 18;
//
//		if (bind.alpha > 0.02)
//			renderingBinds = true;
//	}
//
//	if (MenuOld::is_opened)
//		renderingBinds = true;
//
//	windowAlpha = interpolate(windowAlpha, renderingBinds ? 1 : 0, 8);
//
//	if (windowAlpha < 0.02)
//		return;
//
//	if (GetAsyncKeyState(VK_LBUTTON)) {
//		if (Utils::InBounds(Misc.KeybindWindow, Misc.KeybindWindow + windowSize))
//			Misc.KeybindWindow = Utils::GetMousePos() - draggingOffset;
//	}
//	else {
//		draggingOffset = Utils::GetMousePos() - Misc.KeybindWindow;
//	}
//
//	Render->PushClipRect(Misc.KeybindWindow, Misc.KeybindWindow + windowSize);
//
//	Render->BoxFilled(Misc.KeybindWindow, Misc.KeybindWindow + windowSize, Color(10, 10, 30, int(255 * windowAlpha)), 3);
//	Render->Text("Keybinds", Misc.KeybindWindow + Vector2(10, 6), Color(1.f, 1.f, 1.f, windowAlpha), Museo);
//	Render->BoxFilled(Misc.KeybindWindow + Vector2(0, 26), Misc.KeybindWindow + Vector2(windowSize.x, 27), Color(0.7f, 0.7f, 0.7f, 0.2f * windowAlpha));
//
//	int renderOffset = 0;
//
//	for (auto& bind : keybindDrawList) {
//		if (bind.alpha > 0.02) {
//			Render->Text(bind.name, Misc.KeybindWindow + Vector2(6, 30 + renderOffset), Color(255, 255, 255, int(bind.alpha * 255)), Museo);
//
//			Render->Text(bind.pointer->type == 0 ? "toggled" : "hold", Misc.KeybindWindow + Vector2(windowSize.x - 6 - Render->CalcTextSize(bind.pointer->type == 0 ? "toggled" : "hold", Museo).x, 30 + renderOffset), Color(255, 255, 255, int(bind.alpha * 255)), Museo);
//
//			renderOffset += bind.alpha * 18;
//		}
//	}
//
//	Render->PopClipRect();
//}

void CBombInfo::EventHandler(IGameEvent* event) {
	if (!strcmp(event->GetName(), "bomb_beginplant")) {
		bombState = BOMBSTATE_PLANTING;
		timeStamp = GlobalVars->realtime;
		Vector asite = PlayerResource->m_bombsiteCenterA();
		Vector bsite = PlayerResource->m_bombsiteCenterB();
		bombPosition = EntityList->GetClientEntity(EngineClient->GetPlayerForUserID(event->GetInt("userid")))->GetAbsOrigin();

		float adist = (asite - bombPosition).Length();
		float bdist = (bsite - bombPosition).Length();

		bombSite = adist < bdist ? 0 : 1;
	}
	else if (!strcmp(event->GetName(), "bomb_abortplant")) {
		bombState = BOMBSTATE_NONE;
	}
	else if (!strcmp(event->GetName(), "bomb_planted")) {
		bombState = BOMBSTATE_PLANTED;
		timeStamp = GlobalVars->realtime;
		bombSite = event->GetInt("site");
	}
	else if (!strcmp(event->GetName(), "bomb_begindefuse")) {
		bombState = BOMBSTATE_DEFUSING;
		defuseTimeStamp = GlobalVars->realtime;
		defuseKits = event->GetBool("haskit");
	}
	else if (!strcmp(event->GetName(), "bomb_abortdefuse")) {
		bombState = BOMBSTATE_PLANTED;
	}
	else if (!strcmp(event->GetName(), "bomb_desfused") || !strcmp(event->GetName(), "bomb_exploded")) {
		bombState = BOMBSTATE_NONE;
	}
	else if (!strcmp(event->GetName(), "round_start")) {
		bombState = BOMBSTATE_NONE;
	} 
}
//
//void CBombInfo::Draw() {
//	static Vector2 draggingOffset;
//
//	if (!Cheat.InGame)
//		bombState = BOMBSTATE_NONE;
//
//	alpha = interpolate(alpha, (bombState > 0 || MenuOld::is_opened) ? 1 : 0, 8);
//
//	if (alpha < 0.02f)
//		return;
//
//	if (GetAsyncKeyState(VK_LBUTTON)) {
//		if (Utils::InBounds(Misc.BombWindow, Misc.BombWindow + Vector2(150, 45)))
//			Misc.BombWindow = Utils::GetMousePos() - draggingOffset;
//	}
//	else {
//		draggingOffset = Utils::GetMousePos() - Misc.BombWindow;
//	}
//
//	float circleFraction = 0.f;
//	float secondCircleFraction = 0.f;
//	bool canDefuse = false;
//	float timer = 0.f;
//	int damage = 0;
//
//	auto flDamage = 500.f;
//	auto flBombRadius = flDamage * 3.5f;
//	auto flDistanceToLocalPlayer = 0.f;
//	auto fSigma = flBombRadius / 3.0f;
//	auto fGaussianFalloff = 0.f;
//	auto flAdjustedDamage = 0.f;
//
//	switch (bombState) {
//	case BOMBSTATE_PLANTING:
//		circleFraction = (GlobalVars->realtime - timeStamp) / 3.f;
//		timer = 3.f - (GlobalVars->realtime - timeStamp);
//		damage = 0;
//		break;
//	case BOMBSTATE_PLANTED:
//		circleFraction = 1.f - (Cheat.ServerTime - timeStamp) / 40.f;
//		timer = 40.f - (Cheat.ServerTime - timeStamp);
//		damage = 0;
//		if (!Cheat.LocalPlayer)
//			break;
//		flDamage = 500.f; // 500 - default, if radius is not written on the map https://i.imgur.com/mUSaTHj.png
//		flBombRadius = flDamage * 3.5f;
//		flDistanceToLocalPlayer = (bombPosition + Vector(0, 0, 5) - Cheat.LocalPlayer->GetEyePosition()).Length();// ((c4bomb origin + viewoffset) - (localplayer origin + viewoffset))
//		fSigma = flBombRadius / 3.0f;
//		fGaussianFalloff = exp(-flDistanceToLocalPlayer * flDistanceToLocalPlayer / (2.0f * fSigma * fSigma));
//		flAdjustedDamage = flDamage * fGaussianFalloff;
//
//		flAdjustedDamage = CSGO_Armor(flAdjustedDamage, Cheat.LocalPlayer->m_ArmorValue());
//
//		damage = flAdjustedDamage;
//		break;
//	case BOMBSTATE_DEFUSING:
//		circleFraction = 1.f - (Cheat.ServerTime - timeStamp) / 40.f;
//		secondCircleFraction = 1.f - (GlobalVars->realtime - defuseTimeStamp) / (defuseKits ? 5.f : 10.f);
//		timer = (defuseKits ? 5.f : 10.f) - (GlobalVars->realtime - defuseTimeStamp);
//		damage = 0;
//		if (!Cheat.LocalPlayer)
//			break;
//		flDamage = 500.f; // 500 - default, if radius is not written on the map https://i.imgur.com/mUSaTHj.png
//		flBombRadius = flDamage * 3.5f;
//		flDistanceToLocalPlayer = (bombPosition + Vector(0, 0, 5) - Cheat.LocalPlayer->GetEyePosition()).Length();// ((c4bomb origin + viewoffset) - (localplayer origin + viewoffset))
//		fSigma = flBombRadius / 3.0f;
//		fGaussianFalloff = exp(-flDistanceToLocalPlayer * flDistanceToLocalPlayer / (2.0f * fSigma * fSigma));
//		flAdjustedDamage = flDamage * fGaussianFalloff;
//
//		flAdjustedDamage = CSGO_Armor(flAdjustedDamage, Cheat.LocalPlayer->m_ArmorValue());
//
//		damage = flAdjustedDamage;
//
//		canDefuse = (40.f - (Cheat.ServerTime - timeStamp)) > timer;
//		break;
//	case BOMBSTATE_NONE:
//		break;
//	}
//
//	Render->BoxFilled(Misc.BombWindow, Misc.BombWindow + Vector2(150, 50), Color(10, 10, 30, int(200 * alpha)), 4);
//	Render->Text(format("%.1f", timer), Misc.BombWindow + Vector2(30, 18), Color(255, 255, 255, int(255 * alpha)), Museo, TEXT_CENTERED);
//	Render->Circle(Misc.BombWindow + Vector2(30, 25), 18, Color(40, 40, 40, int(200 * alpha)));
//	Render->Circle(Misc.BombWindow + Vector2(30, 25), 18, Color(0, 145, 255, int(255 * alpha)), -1, 90.f + (360.f - circleFraction * 360.f), 450.f);
//
//	if (bombState == BOMBSTATE_DEFUSING) {
//		Render->Circle(Misc.BombWindow + Vector2(30, 25), 15, canDefuse ? Color(0, 255, 0, int(255 * alpha)) : Color(255, 0, 0, int(255 * alpha)), -1, 90.f + (360.f - secondCircleFraction * 360.f), 450.f);
//	}
//
//	Render->Text(format("Damage: %d", damage), Misc.BombWindow + Vector2(60, 10), Color(255, 255, 255, int(255 * alpha)), Museo);
//	Render->Text(bombSite == 0 ? "Site: A" : "Site: B", Misc.BombWindow + Vector2(60, 24), Color(255, 255, 255, int(255 * alpha)), Museo);
//}

CBombInfo* UI::BombInfo = new CBombInfo;