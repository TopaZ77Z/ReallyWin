#include "Hooks.h"
#include "../Utils/Utils.h"
#include "../Utils/Hash.h"
#include "../Menu/menu.h"
#include "Config.h"
#include "Globals.h"
#include <intrin.h>
#include "../Utils/Console.h"

#include "../Features/Misc/AutoStrafe.h"
#include "../Features/Visuals/ESP.h"
#include "../Features/Visuals/Glow.h"
#include "../Features/Visuals/Chams.h"
#include "../Features/Visuals/World.h"
#include "../Features/Visuals/GrenadePrediction.h"
#include "../Features/Misc/Prediction.h"
#include "../Features/AntiAim/AntiAim.h"
#include "../Features/RageBot/LagCompensation.h"
#include "../Features/RageBot/DoubleTap.h"
#include "../Features/Misc/AutoPeek.h"
#include "../Features/Misc/UI.h"
#include "../Features/RageBot/Ragebot.h"
#include "../Features/RageBot/AutoWall.h"
#include "../Features/RageBot/Resolver.h"
#include "../Features/RageBot/AnimationSystem.h"
#include "../Features/Misc/Misc.h"
#include "../Features/Misc/EventListner.h"
#include "../Features/RageBot/Interpolate.h"
#include "../Features/RageBot/LocalAnimationSystem.h"

GrenadePrediction NadePrediction;

template <typename T>
inline T HookFunction(void* pTarget, void* pDetour) {
	return (T)DetourFunction((PBYTE)pTarget, (PBYTE)pDetour);
}

inline void RemoveHook(void* original, void* detour) {
	DetourRemove((PBYTE)original, (PBYTE)detour);
}

LRESULT CALLBACK hkWndProc(HWND Hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	if (!Render->IsInitialized() || !Menu->initialized)
		return CallWindowProc(oWndProc, Hwnd, Message, wParam, lParam);;

	if (Message == WM_KEYDOWN) {
		AntiAim->OnKeyPressed(wParam);

		Cheat.KeyStates[wParam] = !Cheat.KeyStates[wParam];
	}

	Menu->WndProc(Message, wParam);

	return CallWindowProc(oWndProc, Hwnd, Message, wParam, lParam);
}

HRESULT __stdcall hkReset(IDirect3DDevice9* thisptr, D3DPRESENT_PARAMETERS* params) {
	static tReset oReset = (tReset)Hooks::DirectXDeviceVMT->GetOriginal(16);

	auto result = oReset(thisptr, params);

	Render->Reset();

	return result;
}

HRESULT __stdcall hkPresent(IDirect3DDevice9* thisptr, const RECT* src, const RECT* dest, HWND window, const RGNDATA* dirtyRegion) {
	if (thisptr != DirectXDevice)
		return oPresent(thisptr, src, dest, window, dirtyRegion);

	if (!Render->IsInitialized()) {
		Render->Init(thisptr);
		Menu->Init();
		return oPresent(thisptr, src, dest, window, dirtyRegion);
	}

	// static auto movups_14a781e4 = Utils::PatternScan("client.dll", "0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9", 3);

	Cheat.InGame = EngineClient->IsConnected() && EngineClient->IsInGame();

	if (Cheat.InGame && !PlayerResource)
		PlayerResource = **(CCSPlayerResource***)(Utils::PatternScan("client.dll", "8B 3D ? ? ? ? 85 FF 0F 84 ? ? ? ? 81 C7", 0x2));
	else if (!Cheat.InGame)
		PlayerResource = nullptr;

	if (thisptr->BeginScene() == D3D_OK) {
		Render->RenderDrawData();
		thisptr->EndScene();
	}

	return oPresent(thisptr, src, dest, window, dirtyRegion);
}

void __fastcall hkHudUpdate(IBaseClientDLL* thisptr, void* edx, bool bActive) {
	static auto oHudUpdate = (tHudUpdate)Hooks::ClientVMT->GetOriginal(11);

	Cheat.LocalPlayer = (CBasePlayer*)EntityList->GetClientEntity(EngineClient->GetLocalPlayer());

	if (!Render->IsInitialized() || !Menu->initialized)
		return;

	Render->BeginFrame();

	Render->UpdateViewMatrix(EngineClient->WorldToScreenMatrix());

	ESP::Draw();
	ESP::DrawGrenades();
	NadePrediction.Draw();
	Menu->Draw();
	AutoPeek->Draw();
	World->Crosshair();
	Ragebot->DrawDebugData();

	InputSystem->EnableInput(!Menu->is_opened());

	Render->EndFrame();

	oHudUpdate(thisptr, edx, bActive);
}

void __fastcall hkLockCursor(ISurface* thisptr, void* edx) {
	static tLockCursor oLockCursor = (tLockCursor)Hooks::SurfaceVMT->GetOriginal(67);

	if (Menu->is_opened())
		return Surface->UnlockCursor();

	oLockCursor(thisptr, edx);
}

void adjust_player_time_base(CBasePlayer* player, int simulation_ticks)
{
	auto nci = EngineClient->GetNetChannelInfo();

	if (simulation_ticks < 0 || !nci)
		return;

	static auto sv_clockcorrection_msecs = CVar->FindVar("sv_clockcorrection_msecs");		/* default to 30 */

	/* adjust tickbase below */
	const auto server_tick = Cheat.ServerTime + TIME_TO_TICKS(nci->GetLatency(1));

	/* clock correction time */
	const auto correction_seconds = std::clamp(sv_clockcorrection_msecs->GetFloat() / 1000.0f, 0.0f, 1.0f);
	const auto correction_ticks = TIME_TO_TICKS(correction_seconds);
	/* find ideal tick */
	const auto ideal_final_tick = server_tick + correction_ticks;

	/* adjust tickbase */
	player->m_nTickBase() = ideal_final_tick - simulation_ticks + GlobalVars->simTicksThisFrame;
}

void __stdcall CreateMove(int sequence_number, float sample_frametime, bool active, bool& bSendPacket) {
	static auto oCHLCCreateMove = (tCHLCCreateMove)Hooks::ClientVMT->GetOriginal(22);

	oCHLCCreateMove(Client, sequence_number, sample_frametime, active);

	CUserCmd* cmd = Input->GetUserCmd(sequence_number);
	CVerifiedUserCmd* verified = Input->GetVerifiedCmd(sequence_number);

	Miscelleaneus::Clantag();

	if (!cmd || !cmd->command_number || !Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive())
		return;

	CBaseCombatWeapon* weapon = Cheat.LocalPlayer->GetActiveWeapon();

	if (!cmd || !cmd->command_number)
		return;

	ctx.cmd = cmd;
	ctx.send_packet = true;

	if (config.misc.movement.auto_jump->get()) {
		if (!(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND) && Cheat.LocalPlayer->m_MoveType() != MOVETYPE_NOCLIP && Cheat.LocalPlayer->m_MoveType() != MOVETYPE_LADDER)
			cmd->buttons &= ~IN_JUMP;
	}

	if (config.misc.movement.quick_stop->get() && (cmd->buttons & (IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK | IN_JUMP)) == 0 && Cheat.LocalPlayer->m_vecVelocity().LengthSqr() > 64 && Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND) {
		Vector vec_speed = Cheat.LocalPlayer->m_vecVelocity();
		QAngle direction = Math::VectorAngles(vec_speed);

		QAngle view; EngineClient->GetViewAngles(&view);
		direction.yaw = view.yaw - direction.yaw;
		direction.Normalize();

		Vector forward;
		Math::AngleVectors(direction, forward);

		Vector nigated_direction = forward * -std::clamp(vec_speed.Q_Length2D(), 0.f, 450.f);

		cmd->sidemove = nigated_direction.y;
		cmd->forwardmove = nigated_direction.x;
	}

	Miscelleaneus::AutoStrafe();

	Miscelleaneus::QuickSwitch();

	//adjust_player_time_base(GlobalVars->tickcount);

	if (DoubleTap->IsShifting()) {
		if (ClientState->m_nDeltaTick > 0)
			Prediction->Update(ClientState->m_nDeltaTick, ClientState->m_nDeltaTick > 0, ClientState->m_nLastCommandAck, ClientState->m_nLastOutgoingCommand + ClientState->m_nChokedCommands);

		EnginePrediction->Start(cmd);

		QAngle eyeYaw = cmd->viewangles;

		AutoPeek->CreateMove();
		Ragebot->Run();

		AntiAim->Angles();
		AntiAim->SlowWalk();

		ctx.send_packet = bSendPacket = ctx.tickbase_shift == 1;
		cmd->buttons &= ~(IN_ATTACK | IN_ATTACK2);

		EnginePrediction->End();

		cmd->viewangles.Normalize();
		Utils::FixMovement(cmd, eyeYaw);

		ctx.shifted_commands.emplace_back(cmd->command_number);
		ctx.sented_commands.emplace_back(cmd->command_number);
		ctx.teleported_last_tick = true;

		verified->m_cmd = *cmd;
		verified->m_crc = cmd->GetChecksum();
		return;
	}

	// pre_prediction

	if (config.misc.movement.infinity_duck->get())
		ctx.cmd->buttons |= IN_BULLRUSH;

	AntiAim->SlowWalk();

	QAngle storedAng = cmd->viewangles;

	if (ClientState->m_nDeltaTick > 0)
		Prediction->Update(ClientState->m_nDeltaTick, ClientState->m_nDeltaTick > 0, ClientState->m_nLastCommandAck, ClientState->m_nLastOutgoingCommand + ClientState->m_nChokedCommands);

	EnginePrediction->Start(cmd);

	ctx.last_local_velocity = ctx.local_velocity;
	ctx.local_velocity = Cheat.LocalPlayer->m_vecVelocity();

	Miscelleaneus::CompensateThrowable();

	// prediction

	Cheat.ServerTime = GlobalVars->curtime;

	if (config.misc.movement.edge_jump->get() && !(Cheat.LocalPlayer->m_fFlags() & FL_ONGROUND) && EnginePrediction->m_fFlags & FL_ONGROUND)
		cmd->buttons |= IN_JUMP;

	if (weapon->ShootingWeapon() && !weapon->CanShoot())
		cmd->buttons &= ~IN_ATTACK;

	if (weapon->ShootingWeapon() && weapon->CanShoot() && cmd->buttons & IN_ATTACK)
		DoubleTap->ForceTeleport();

	AntiAim->FakeLag();
	AntiAim->FakeDuck();
	AntiAim->Angles();

	AutoPeek->CreateMove();

	Ragebot->Run();

	cmd->viewangles.Normalize(config.misc.miscellaneous.anti_untrusted->get());

	if (ctx.send_packet) {
		Cheat.thirdpersonAngles = cmd->viewangles;
		if (!config.antiaim.anti_aimbot_angles.body_yaw_options->get(1) || Utils::RandomInt(0, 10) > 5)
			AntiAim->jitter = !AntiAim->jitter;
		ctx.should_update_local_anims = true;
	}

	LocalAnimations->OnCreateMove();

	EnginePrediction->End();

	// createmove

	Utils::FixMovement(cmd, storedAng);

	AntiAim->LegMovement();

	bSendPacket = ctx.send_packet;
	ctx.sented_commands.emplace_back(cmd->command_number);
	ctx.teleported_last_tick = false;

	verified->m_cmd = *cmd;
	verified->m_crc = cmd->GetChecksum();
}

__declspec(naked) void __fastcall hkCHLCCreateMove(IBaseClientDLL* thisptr, void*, int sequence_number, float input_sample_frametime, bool active) {
	__asm {
		push ebp
		mov  ebp, esp
		push ebx
		push esp
		push dword ptr[active]
		push dword ptr[input_sample_frametime]
		push dword ptr[sequence_number]
		call CreateMove
		pop  ebx
		pop  ebp
		retn 0Ch
	}
}

void* __fastcall hkAllocKeyValuesMemory(IKeyValuesSystem* thisptr, void* edx, int iSize)
{
	static auto oAllocKeyValuesMemory = (void*(__fastcall*)(IKeyValuesSystem*, void*, int))Hooks::KeyValuesVMT->GetOriginal(2);

	// return addresses of check function
	// @credits: danielkrupinski
	static const void* uAllocKeyValuesEngine = Utils::PatternScan("engine.dll", "55 8B EC 56 57 8B F9 8B F2 83 FF 11 0F 87 ? ? ? ? 85 F6 0F 84 ? ? ? ?", 0x4A);
	static const void* uAllocKeyValuesClient = Utils::PatternScan("client.dll", "55 8B EC 56 57 8B F9 8B F2 83 FF 11 0F 87 ? ? ? ? 85 F6 0F 84 ? ? ? ?", 0x3E);

	// doesn't call it yet, but have checking function
	//static const std::uintptr_t uAllocKeyValuesMaterialSystem = MEM::FindPattern(MATERIALSYSTEM_DLL, XorStr("FF 52 04 85 C0 74 0C 56")) + 0x3;
	//static const std::uintptr_t uAllocKeyValuesStudioRender = MEM::FindPattern(STUDIORENDER_DLL, XorStr("FF 52 04 85 C0 74 0C 56")) + 0x3;

	if (const void* uReturnAddress = _ReturnAddress(); uReturnAddress == uAllocKeyValuesEngine || uReturnAddress == uAllocKeyValuesClient)
		return nullptr;

	return oAllocKeyValuesMemory(thisptr, edx, iSize);
}

bool __fastcall hkSetSignonState(void* thisptr, void* edx, int state, int count, const void* msg) {
	bool result = oSetSignonState(thisptr, edx, state, count, msg);

	if (state == 6) { // SIGNONSTATE_FULL
		World->Modulation();
		World->SkyBox();
		World->Fog();
		World->RemoveBlood();
		World->Smoke();

		static ConVar* cl_threaded_bone_setup = CVar->FindVar("cl_threaded_bone_setup");

		cl_threaded_bone_setup->SetInt(1);

		GrenadePrediction::PrecacheParticles();
	}

	return result;
}

void __fastcall hkLevelShutdown(IBaseClientDLL* thisptr, void* edx) {
	static auto oLevelShutdown = (void(__thiscall*)(IBaseClientDLL*))Hooks::ClientVMT->GetOriginal(7);

	DoubleTap->target_tickbase_shift = ctx.tickbase_shift = 0;
	ctx.reset();
	LagCompensation->Reset();
	Interpolate->ResetInterpolation();

	oLevelShutdown(thisptr);
}

void __fastcall hkOverrideView(IClientMode* thisptr, void* edx, CViewSetup* setup) {
	static tOverrideView oOverrideView = (tOverrideView)Hooks::ClientModeVMT->GetOriginal(18);

	if (!Menu->initialized)
		return oOverrideView(thisptr, edx, setup);

	if (setup->fov == 90 || config.visuals.effects.removals->get(5)) {
		setup->fov = config.visuals.effects.fov->get();
	}

	World->ProcessCamera(setup);

	if (Cheat.InGame && Cheat.LocalPlayer && Cheat.LocalPlayer->m_iHealth() > 0) {
		NadePrediction.Start(setup->angles, setup->origin);
	}

	if (Cheat.LocalPlayer && Cheat.LocalPlayer->m_iHealth() > 0 && config.antiaim.misc.fake_duck->get())
		setup->origin = Cheat.LocalPlayer->GetAbsOrigin() + Vector(0, 0, 64);

	setup->angles.roll = 0;

	oOverrideView(thisptr, edx, setup);
}

void __fastcall hkPaintTraverse(IPanel* thisptr, void* edx, unsigned int panel, bool bForceRepaint, bool bForce) {
	static tPaintTraverse oPaintTraverse = (tPaintTraverse)Hooks::PanelVMT->GetOriginal(41);
	static unsigned int hud_zoom_panel = 0;

	if (!Menu->initialized || !Render->IsInitialized())
		return oPaintTraverse(thisptr, edx, panel, bForceRepaint, bForce);

	if (!hud_zoom_panel) {
		std::string panelName = VPanel->GetName(panel);

		if (panelName == "HudZoom")
			hud_zoom_panel = panel;
	}

	if (hud_zoom_panel == panel && config.visuals.effects.removals->get(5))
		return;

	oPaintTraverse(thisptr, edx, panel, bForceRepaint, bForce);
}

void __fastcall hkDoPostScreenEffects(IClientMode* thisptr, void* edx, CViewSetup* setup) {
	static tDoPostScreenEffects oDoPostScreenEffects = (tDoPostScreenEffects)Hooks::ClientModeVMT->GetOriginal(44);

	Glow::Run();

	oDoPostScreenEffects(thisptr, edx, setup);
}

bool __fastcall hkIsPaused(IVEngineClient* thisptr, void* edx) {
	static void* addr = Utils::PatternScan("client.dll", "FF D0 A1 ?? ?? ?? ?? B9 ?? ?? ?? ?? D9 1D ?? ?? ?? ?? FF 50 34 85 C0 74 22 8B 0D ?? ?? ?? ??", 0x29);
	static tIsPaused oIsPaused = (tIsPaused)Hooks::EngineVMT->GetOriginal(90);

	if (_ReturnAddress() == addr)
		return true;

	return oIsPaused(thisptr, edx);
}

void __fastcall hkDrawModelExecute(IVModelRender* thisptr, void* edx, void* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld) {
	static tDrawModelExecute oDrawModelExecute = (tDrawModelExecute)Hooks::ModelRenderVMT->GetOriginal(21);

	std::string modelName = ModelInfoClient->GetModelName(pInfo.pModel);
	CBaseEntity* entity = EntityList->GetClientEntity(pInfo.entity_index);

	bool isPlayer = entity && entity->IsPlayer();
	bool isWeapon = !isPlayer && modelName.find("weapons/v_") != std::string::npos && modelName.find("arms") == std::string::npos;

	if (StudioRender->IsForcedMaterialOverride() && !isWeapon)
		return oDrawModelExecute(thisptr, edx, ctx, state, pInfo, pCustomBoneToWorld);

	Chams->OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld, edx);

	oDrawModelExecute(thisptr, edx, ctx, state, pInfo, pCustomBoneToWorld);

	StudioRender->ForcedMaterialOverride(nullptr);
}

int __fastcall hkGetBool(ConVar* thisptr, void* edx) {
	static tGetBool oGetBool = (tGetBool)Hooks::ConVarVMT->GetOriginal(13);
	static void* cameraThink = Utils::PatternScan("client.dll", "85 C0 75 30 38 86");

	if (_ReturnAddress() == cameraThink && config.visuals.effects.thirdperson->get())
		return 1;

	return oGetBool(thisptr, edx);
}

void __fastcall hkFrameStageNotify(IBaseClientDLL* thisptr, void* edx, EClientFrameStage stage) {
	static auto oFrameStageNotify = (tFrameStageNotify)Hooks::ClientVMT->GetOriginal(37);
	Cheat.LocalPlayer = (CBasePlayer*)EntityList->GetClientEntity(EngineClient->GetLocalPlayer());

	switch (stage) {
	case FRAME_RENDER_START:
		Interpolate->RunInterpolation();

		cvars.r_aspectratio->SetFloat(config.visuals.effects.aspect_ratio->get());
		cvars.mat_postprocessing_enable->SetInt(!config.visuals.effects.removals->get(0));
		cvars.cl_csm_shadows->SetInt(!config.visuals.effects.removals->get(2));
		cvars.cl_foot_contact_shadows->SetInt(0);
		cvars.r_drawsprites->SetInt(!config.visuals.effects.removals->get(7));

		if (config.visuals.effects.removals->get(5))
			cvars.zoom_sensitivity_ratio_mouse->SetFloat(0.f);
		else
			cvars.zoom_sensitivity_ratio_mouse->SetFloat(1.f);

		break;
	case FRAME_NET_UPDATE_END:
		ctx.server_tick = TIME_TO_TICKS(GlobalVars->curtime);

		LagCompensation->OnNetUpdate();
		ESP::ProcessSounds();
		if (Cheat.InGame) {
			EngineClient->FireEvents();
		}
		break;
	}

	LocalAnimations->FrameStageNotify(stage);

	oFrameStageNotify(thisptr, edx, stage);
}

void __fastcall hkUpdateClientSideAnimation(CBasePlayer* thisptr, void* edx) {
	if (hook_info.update_csa)
		return oUpdateClientSideAnimation(thisptr, edx);
	if (thisptr == Cheat.LocalPlayer)
		return oUpdateClientSideAnimation(thisptr, edx);
	if (!thisptr->IsPlayer())
		return oUpdateClientSideAnimation(thisptr, edx);
}

bool __fastcall hkShouldSkipAnimationFrame(void* thisptr, void* edx) {
	return false;
}

bool __fastcall hkInterpolateViewmodel(CBaseEntity* thisptr, void* edx, float time) {
	if (!DoubleTap->ShouldCharge())
		return oInterpolateViewmodel(thisptr, edx, time);

	const auto owner = EntityList->GetClientEntityFromHandle(thisptr->m_hOwner());
	if (owner != Cheat.LocalPlayer)
		return oInterpolateViewmodel(thisptr, edx, time);

	const float lerp_amount = GlobalVars->interpolation_amount;

	GlobalVars->interpolation_amount = 0.f;
	const bool result = oInterpolateViewmodel(thisptr, edx, time);
	GlobalVars->interpolation_amount = lerp_amount;

	return result;
}

bool __fastcall hkShouldInterpolate(CBasePlayer* thisptr, void* edx) {
	if (!DoubleTap->ShouldCharge() || thisptr != Cheat.LocalPlayer)
		return oShouldInterpolate(thisptr, edx);

	Interpolate->DisableInterpolationFlags(thisptr);

	return false;
}

void __fastcall hkDoExtraBoneProcessing(CBaseEntity* player, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* contex) {

}

void __fastcall hkStandardBlendingRules(CBasePlayer* thisptr, void* edx, void* hdr, void* pos, void* q, float current_time, int bone_mask) {
	if (thisptr == Cheat.LocalPlayer)
		thisptr->m_fEffects() |= EF_NOINTERP;
	oStandardBlendingRules(thisptr, edx, hdr, pos, q, current_time, bone_mask);
	if (thisptr == Cheat.LocalPlayer)
		thisptr->m_fEffects() &= ~EF_NOINTERP;
}

bool __fastcall hkIsHLTV(IVEngineClient* thisptr, void* edx) {
	static auto oIsHLTV = (tIsHLTV)Hooks::EngineVMT->GetOriginal(93);

	if (hook_info.setup_bones)
		return true;

	if (hook_info.update_csa)
		return true;

	static const auto setup_velocity = Utils::PatternScan("client.dll", "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0");
	static const auto accumulate_layers = Utils::PatternScan("client.dll", "84 C0 75 0D F6 87");

	if (_ReturnAddress() == setup_velocity || _ReturnAddress() == accumulate_layers)
		return true;

	return oIsHLTV(thisptr, edx);
}

void __fastcall hkBuildTransformations(CBaseEntity* thisptr, void* edx, void* hdr, void* pos, void* q, const void* camera_transform, int bone_mask, void* bone_computed) {
	if (thisptr)
		thisptr->m_isJiggleBonesEnabled() = false;

	oBuildTransformations(thisptr, edx, hdr, pos, q, camera_transform, bone_mask, bone_computed);
}

void __fastcall hkSetUpLean(CCSGOPlayerAnimationState* thisptr, void* edx) {
	if (config.antiaim.misc.animations->get(0) && thisptr->pEntity == Cheat.LocalPlayer)
		oSetUpLean(thisptr, edx);
}

bool __fastcall hkSetupBones(CBaseEntity* thisptr, void* edx, matrix3x4_t* pBoneToWorld, int maxBones, int mask, float curTime) {
	if (hook_info.setup_bones || !thisptr)
		return oSetupBones(thisptr, edx, pBoneToWorld, maxBones, mask, curTime);

	CBasePlayer* ent = (CBasePlayer*)((uintptr_t)thisptr - 0x4);

	if (!ent->IsPlayer() || !ent->IsAlive())
		return oSetupBones(thisptr, edx, pBoneToWorld, maxBones, mask, curTime);

	if (ent == Cheat.LocalPlayer)
		return oSetupBones(thisptr, edx, pBoneToWorld, maxBones, mask, curTime);

	if (!hook_info.disable_interpolation && mask & BONE_USED_BY_ATTACHMENT) {
		Interpolate->InterpolateModel(ent, ent->GetCachedBoneData().Base());

		//const auto backup_abs_origin = ent->GetAbsOrigin();
		ent->SetAbsOrigin(Interpolate->GetInterpolated(ent));
		//ent->m_BoneAccessor().m_ReadableBones |= BONE_USED_BY_ATTACHMENT;
		//ent->m_BoneAccessor().m_WritableBones |= BONE_USED_BY_ATTACHMENT;
		//ent->CopyBones(ent->m_BoneAccessor().m_pBones);
		ent->SetupBones_AttachmentHelper();
		//ent->SetAbsOrigin(backup_abs_origin);
	}

	if (pBoneToWorld && maxBones != -1) {
		ent->CopyBones(pBoneToWorld);
	}

	return true;
}

void __fastcall hkRunCommand(IPrediction* thisptr, void* edx, CBasePlayer* player, CUserCmd* cmd, IMoveHelper* moveHelper) {
	static auto oRunCommand = (tRunCommand)(Hooks::PredictionVMT->GetOriginal(19));

	if (!player || !cmd || player != Cheat.LocalPlayer)
		return oRunCommand(thisptr, edx, player, cmd, moveHelper);

	const float backup_velocity_modifier = player->m_flVelocityModifier();

	oRunCommand(thisptr, edx, player, cmd, moveHelper);

	player->m_flVelocityModifier() = backup_velocity_modifier;

	MoveHelper = moveHelper;
}

void __fastcall hkPhysicsSimulate(CBasePlayer* thisptr, void* edx) {
	const int tick_base = thisptr->m_nTickBase();
	C_CommandContext* c_ctx = thisptr->GetCommandContext();
	auto simulation_tick = *(int*)((uintptr_t)thisptr + 0x2AC);

	if (thisptr != Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive() || thisptr->m_nSimulationTick() == GlobalVars->tickcount || !c_ctx->needsprocessing || GlobalVars->tickcount == simulation_tick)
		return oPhysicsSimulate(thisptr, edx);

	auto& local_data = EnginePrediction->GetLocalData(c_ctx->command_number);

	if (c_ctx->cmd.tick_count > GlobalVars->tickcount * 2) {
		c_ctx->cmd.hasbeenpredicted = true;
		Cheat.LocalPlayer->m_nTickBase()++;
		return;
	}

	if (c_ctx->cmd.command_number == DoubleTap->charged_command)
		adjust_player_time_base(Cheat.LocalPlayer, DoubleTap->charged_command + 1);

	EnginePrediction->RestoreNetvars(c_ctx->command_number % 150);

	oPhysicsSimulate(thisptr, edx);

	EnginePrediction->StoreNetvars(c_ctx->command_number % 150);
}

void __fastcall hkPacketStart(CClientState* thisptr, void* edx, int incoming_sequence, int outgoing_acknowledged) {
	if (!Cheat.InGame || !Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive())
		return oPacketStart(thisptr, edx, incoming_sequence, outgoing_acknowledged);

	for (auto it = ctx.sented_commands.begin(); it != ctx.sented_commands.end(); it++) {
		if (*it == outgoing_acknowledged) {
			oPacketStart(thisptr, edx, incoming_sequence, outgoing_acknowledged);
			break;
		}
	}

	ctx.sented_commands.erase(
		std::remove_if(
			ctx.sented_commands.begin(),
			ctx.sented_commands.end(),
			[&](auto const& command) { return command < outgoing_acknowledged; }),
		ctx.sented_commands.end());
}

void __fastcall hkPacketEnd(CClientState* thisptr, void* edx) {
	if (!Cheat.LocalPlayer || ClientState->m_ClockDriftMgr.m_nServerTick != ClientState->m_nDeltaTick)
	{
		ctx.reset(); 		return oPacketEnd(thisptr, edx);
	}

	oPacketEnd(thisptr, edx);
}

void __fastcall hkClampBonesInBBox(CBasePlayer* thisptr, void* edx, matrix3x4_t* bones, int boneMask) {
	if (thisptr->m_fFlags() & FL_FROZEN) {
		thisptr->m_vecMaxs().z = 72.f; // abobus fix

		auto collidable = thisptr->GetCollideable();

		if (collidable)
			collidable->OBBMaxs().z = 72.f;
	}

	return oClampBonesInBBox(thisptr, edx, bones, boneMask);
}

void __cdecl hkCL_Move(float accamulatedExtraSamples, bool bFinalTick) {
	Cheat.LocalPlayer = (CBasePlayer*)EntityList->GetClientEntity(EngineClient->GetLocalPlayer());

	if (!Cheat.LocalPlayer || !Cheat.LocalPlayer->IsAlive())
		return oCL_Move(accamulatedExtraSamples, bFinalTick);

	DoubleTap->Run();

	if (DoubleTap->ShouldCharge()) {
		ctx.tickbase_shift++;
		ctx.shifted_last_tick++;
		return;
	}

	oCL_Move(accamulatedExtraSamples, bFinalTick);

	DoubleTap->HandleTeleport(oCL_Move, accamulatedExtraSamples);
}

QAngle* __fastcall hkGetEyeAngles(CBasePlayer* thisptr, void* edx) {
	if (thisptr != Cheat.LocalPlayer)
		return oGetEyeAngles(thisptr, edx);

	static void* EyeAnglesPitch = Utils::PatternScan("client.dll", "8B CE F3 0F 10 00 8B 06 F3 0F 11 45 ? FF 90 ? ? ? ? F3 0F 10 55 ?");
	static void* EyeAnglesYaw = Utils::PatternScan("client.dll", "F3 0F 10 55 ? 51 8B 8E ? ? ? ?");
	static void* ShitAnimRoll = Utils::PatternScan("client.dll", "8B 55 0C 8B C8 E8 ? ? ? ? 83 C4 08 5E 8B E5");

	if (_ReturnAddress() != EyeAnglesPitch && _ReturnAddress() != EyeAnglesYaw && _ReturnAddress() != ShitAnimRoll)
		return oGetEyeAngles(thisptr, edx);

	return &Cheat.thirdpersonAngles;
}

//bool __fastcall hkWriteUserCmdDeltaToBuffer(IBaseClientDLL* thisptr, void* edx, int slot, void* buf, int from, int to, bool isnewcommand) {
//	static auto oWriteUserCmdDeltaToBuffer = (tWriteUserCmdDeltaToBuffer)Hooks::ClientVMT->GetOriginal(24);
//
//	if (!Cheat.InGame || !Cheat.LocalPlayer /*|| !Cheat.ticksToShift*/)
//		return oWriteUserCmdDeltaToBuffer(thisptr, edx, slot, buf, from, to, isnewcommand);
//
//	if (from != -1)
//		return true;
//
//	auto p_new_commands = (int*)((DWORD)buf - 0x2C);
//	auto p_backup_commands = (int*)((DWORD)buf - 0x30);
//	auto new_commands = *p_new_commands;
//
//	auto next_cmd_nr = ClientState->m_nLastOutgoingCommand + ClientState->GetChokedCommands() + 1;
//
//	//auto total_new_commands = std::clamp(Cheat.ticksToShift, 0, 16);
//	//Cheat.ticksToShift -= total_new_commands;
//
//	from = -1;
//
//	*p_new_commands = 0/*total_new_commands*/;
//	*p_backup_commands = 0;
//
//	for (to = next_cmd_nr - new_commands + 1; to <= next_cmd_nr; to++)
//	{
//		if (!oWriteUserCmdDeltaToBuffer(thisptr, edx, slot, buf, from, to, true))
//			return false;
//
//		from = to;
//	}
//
//	CUserCmd* last_real_cmd = Input->GetUserCmd(slot, from);
//	CUserCmd from_cmd;
//
//	if (last_real_cmd)
//		memcpy(&from_cmd, last_real_cmd, sizeof(CUserCmd));
//
//	CUserCmd to_cmd;
//	memcpy(&to_cmd, &from_cmd, sizeof(CUserCmd));
//
//	to_cmd.command_number++;
//	to_cmd.tick_count += 192;
//
//	for (int i = new_commands; i <= 0/*total_new_commands*/; i++)
//	{
//		Utils::WriteUserCmd(buf, &to_cmd, &from_cmd);
//		memcpy(&from_cmd, &to_cmd, sizeof(CUserCmd));
//		to_cmd.command_number++;
//		to_cmd.tick_count++;
//	}
//
//	return true;
//}

bool __fastcall hkWriteUserCmdDeltaToBuffer(void* ecx, void* edx, int slot, bf_write* buf, int from, int to, bool is_new_command)
{
	static auto oWriteUserCmdDeltaToBuffer = (tWriteUserCmdDeltaToBuffer)Hooks::ClientVMT->GetOriginal(24);

	if (!ctx.tickbase_shift)
		return oWriteUserCmdDeltaToBuffer(ecx, slot, buf, from, to, is_new_command);

	if (from != -1)
		return true;

	auto final_from = -1;

	uintptr_t frame_ptr;
	__asm mov frame_ptr, ebp;

	auto backup_commands = reinterpret_cast <int*> (frame_ptr + 0xFD8);
	auto new_commands = reinterpret_cast <int*> (frame_ptr + 0xFDC);

	auto newcmds = *new_commands;
	auto shift = ctx.tickbase_shift;

	ctx.tickbase_shift = 0;
	*backup_commands = 0;

	auto choked_modifier = newcmds + shift;

	if (choked_modifier > 62)
		choked_modifier = 62;

	*new_commands = choked_modifier;

	auto next_cmdnr = ClientState->m_nChokedCommands + ClientState->m_nLastOutgoingCommand + 1;
	auto final_to = next_cmdnr - newcmds + 1;

	if (final_to <= next_cmdnr)
	{
		while (oWriteUserCmdDeltaToBuffer(ecx, slot, buf, final_from, final_to, true))
		{
			final_from = final_to++;

			if (final_to > next_cmdnr)
				goto next_cmd;
		}

		return false;
	}
next_cmd:

	auto user_cmd = Input->GetUserCmd(final_from);

	if (!user_cmd)
		return true;

	CUserCmd to_cmd;
	CUserCmd from_cmd;

	from_cmd = *user_cmd;
	to_cmd = from_cmd;

	to_cmd.command_number++;
	to_cmd.tick_count += 200;

	if (newcmds > choked_modifier)
		return true;

	for (auto i = choked_modifier - newcmds + 1; i > 0; --i)
	{
		Utils::WriteUserCmd(buf, &to_cmd, &from_cmd);

		from_cmd = to_cmd;
		to_cmd.command_number++;
		to_cmd.tick_count++;
	}

	return true;
}


bool __fastcall hkSendNetMsg(INetChannel* thisptr, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice) {
	static auto oSendNetMsg = (bool(__fastcall*)(INetChannel*, void*, INetMessage&, bool, bool))Hooks::ClientVMT->GetOriginal(40);

	if (Cheat.LocalPlayer && Cheat.InGame) {
		if (msg.GetType() == 14) // Return and don't send messsage if its FileCRCCheck
			return true;

		if (msg.GetGroup() == 9) // Fix lag when transmitting voice and fakelagging
			bVoice = true;
	}

	return oSendNetMsg(thisptr, edx, msg, bForceReliable, bVoice);
}

void __fastcall hkCalculateView(CBasePlayer* thisptr, void* edx, Vector& eyeOrigin, QAngle& eyeAngle, float& z_near, float& z_far, float& fov) {
	if (!thisptr || thisptr != Cheat.LocalPlayer)
		return oCalculateView(thisptr, edx, eyeOrigin, eyeAngle, z_near, z_far, fov);

	static const uintptr_t m_bUseNewAnimstate_offset = *(uintptr_t*)Utils::PatternScan("client.dll", "80 BE ? ? ? ? ? 0F 84 ? ? ? ? 83 BE ? ? ? ? ? 0F 84", 0x2);

	bool& m_bUseNewAnimstate = *(bool*)(thisptr + m_bUseNewAnimstate_offset);
	const bool backup = m_bUseNewAnimstate;

	m_bUseNewAnimstate = false;
	oCalculateView(thisptr, edx, eyeOrigin, eyeAngle, z_near, z_far, fov);
	m_bUseNewAnimstate = backup;
}

void __fastcall hkRenderSmokeOverlay(void* thisptr, void* edx, bool bPreViewModel) {
	if (!config.visuals.effects.removals->get(3))
		oRenderSmokeOverlay(thisptr, edx, bPreViewModel);
}

void __stdcall hkFX_FireBullets(
	CBaseCombatWeapon* weapon,
	int iPlayer,
	int nItemDefIndex,
	const Vector& vOrigin,
	const QAngle& vAngles,
	int	iMode,
	int iSeed,
	float fInaccuracy,
	float fSpread,
	float fAccuracyFishtail,
	float flSoundTime,
	int sound_type,
	float flRecoilIndex) // TODO: find correct signature
{
	oFX_FireBullets(weapon, iPlayer, nItemDefIndex, vOrigin, vAngles, iMode, iSeed, fInaccuracy, fSpread, fAccuracyFishtail, flSoundTime, sound_type, flRecoilIndex);
}

void __fastcall hkProcessMovement(IGameMovement* thisptr, void* edx, CBasePlayer* player, CMoveData* mv) {
	mv->bGameCodeMovedPlayer = false;
	oProcessMovement(thisptr, edx, player, mv);
}

int __fastcall hkLogDirect(void* loggingSystem, void* edx, int channel, int serverity, Color color, const char* text) {
	if (hook_info.console_log)
		return oLogDirect(loggingSystem, edx, channel, serverity, color, text);

	if (!config.misc.miscellaneous.filter_console->get())
		return oLogDirect(loggingSystem, edx, channel, serverity, color, text);

	return 0;
}

CNewParticleEffect* hkCreateNewParticleEffect(void* pDef, CBaseEntity* pOwner, Vector const& vecAggregatePosition, const char* pDebugName, int nSplitScreenUser) {
	CNewParticleEffect* result;

	__asm {
		mov edx, pDef
		push nSplitScreenUser
		push pDebugName
		push vecAggregatePosition
		push pOwner
		call oCreateNewParticleEffect
		mov result, eax
	}

	if (IEFFECTS::bCaptureEffect)
		IEFFECTS::pCapturedEffect = result;

	return result;
}

__declspec(naked) void hkCreateNewParticleEffect_proxy() {
	__asm
	{
		push edx
		call hkCreateNewParticleEffect
		pop edx
		retn
	}
}

void __fastcall hkEstimateAbsVelocity(void* plr, void* edx, Vector& Velocity)
{
	auto Player = (CBasePlayer*)plr;

	if (!Player || Player == Cheat.LocalPlayer)
		return oEstimateAbsVelocity(plr, edx, Velocity);

	const float OldSimTime = *reinterpret_cast<float*>(Player + 0x268);
	if (OldSimTime > Player->m_flSimulationTime())
		return;

	oEstimateAbsVelocity(plr, edx, Velocity);
}

int __fastcall hkInterpolationList(void* ecx, void* edx)
{
	static auto allow_extrapolation = *(bool**)((DWORD)Utils::PatternScan("client.dll", "A2 ? ? ? ? 8B 45 E8") + 0x1);
	if (allow_extrapolation)
		*allow_extrapolation = false;

	return oInterpolationList(ecx, edx);
}

bool __stdcall InPrediction()
{
	static auto oRunCommand = (tInPrediction)(Hooks::PredictionVMT->GetOriginal(14));

	static auto maintain_sequence_transitions = (void*)Utils::PatternScan("client.dll", "84 C0 74 17 8B 87");
	static auto setupbones_timing = (void*)Utils::PatternScan("client.dll", "84 C0 74 0A F3 0F 10 05 ? ? ? ? EB 05");
	static void* calcplayerview_return = (void*)Utils::PatternScan("client.dll", "84 C0 75 0B 8B 0D ? ? ? ? 8B 01 FF 50 4C");

	if (maintain_sequence_transitions && hook_info.setup_bones && _ReturnAddress() == maintain_sequence_transitions)
		return true;

	if (setupbones_timing && _ReturnAddress() == setupbones_timing)
		return false;

	if (EngineClient->IsInGame()) {
		if (_ReturnAddress() == calcplayerview_return)
			return true;
	}

	return oRunCommand(Prediction);
}

void Hooks::Initialize() {
	oWndProc = (WNDPROC)(SetWindowLongPtr(FindWindowA("Valve001", nullptr), GWL_WNDPROC, (LONG_PTR)hkWndProc));

	DirectXDeviceVMT = new VMT(DirectXDevice);
	SurfaceVMT = new VMT(Surface);
	ClientModeVMT = new VMT(ClientMode);
	PanelVMT = new VMT(VPanel);
	EngineVMT = new VMT(EngineClient);
	ModelRenderVMT = new VMT(ModelRender);
	ConVarVMT = new VMT(cvars.sv_cheats);
	ClientVMT = new VMT(Client);
	PredictionVMT = new VMT(Prediction);
	KeyValuesVMT = new VMT(KeyValuesSystem);

	// vmt hooking for directx doesnt work for some reason
	oPresent = HookFunction<tPresent>(Utils::PatternScan("gameoverlayrenderer.dll", "55 8B EC 83 EC 4C 53"), hkPresent);
	//oReset = HookFunction<tReset>(Utils::PatternScan("d3d9.dll", "8B FF 55 8B EC 83 E4 F8 81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 ? ? ? ? 53 8B 5D 08 8B CB"), hkReset);

	while (!Menu->initialized)
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	DirectXDeviceVMT->Hook(16, hkReset);
	SurfaceVMT->Hook(67, hkLockCursor);
	ClientModeVMT->Hook(18, hkOverrideView);
	ClientModeVMT->Hook(44, hkDoPostScreenEffects);
	PanelVMT->Hook(41, hkPaintTraverse);
	EngineVMT->Hook(90, hkIsPaused);
	EngineVMT->Hook(93, hkIsHLTV);
	ModelRenderVMT->Hook(21, hkDrawModelExecute);
	ConVarVMT->Hook(13, hkGetBool);
	ClientVMT->Hook(37, hkFrameStageNotify);
	ClientVMT->Hook(11, hkHudUpdate);
	ClientVMT->Hook(22, hkCHLCCreateMove);
	ClientVMT->Hook(40, hkSendNetMsg);
	ClientVMT->Hook(7, hkLevelShutdown);
	//ClientVMT->Hook(24, hkWriteUserCmdDeltaToBuffer);
	PredictionVMT->Hook(19, hkRunCommand);
	KeyValuesVMT->Hook(2, hkAllocKeyValuesMemory);
	PredictionVMT->Hook(14, InPrediction);

	oUpdateClientSideAnimation = HookFunction<tUpdateClientSideAnimation>(Utils::PatternScan("client.dll", "55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74"), hkUpdateClientSideAnimation);
	oDoExtraBoneProcessing = HookFunction<tDoExtraBoneProcessing>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 8B F1 57 89 74 24 1C"), hkDoExtraBoneProcessing);
	oShouldSkipAnimationFrame = HookFunction<tShouldSkipAnimationFrame>(Utils::PatternScan("client.dll", "57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02"), hkShouldSkipAnimationFrame);
	oStandardBlendingRules = HookFunction<tStandardBlendingRules>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6"), hkStandardBlendingRules);
	oBuildTransformations = HookFunction<tBuildTransformations>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F0 81 ? ? ? ? ? 56 57 8B F9 8B"), hkBuildTransformations);
	oSetUpLean = HookFunction<tSetUpLean>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F8 A1 ? ? ? ? 83 EC 20 F3"), hkSetUpLean);
	oSetupBones = HookFunction<tSetupBones>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 57"), hkSetupBones);
	oCL_Move = HookFunction<tCL_Move>(Utils::PatternScan("engine.dll", "55 8B EC 81 EC ? ? ? ? 53 56 8A F9 F3 0F 11 45 ? 8B 4D 04"), hkCL_Move);
	oPhysicsSimulate = HookFunction<tPhysicsSimulate>(Utils::PatternScan("client.dll", "56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 23 0F B7 C1 C1 E0 04 05 ? ? ? ?"), hkPhysicsSimulate);
	oClampBonesInBBox = HookFunction<tClampBonesInBBox>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 38"), hkClampBonesInBBox);
	oCalculateView = HookFunction<tCalculateView>(Utils::PatternScan("client.dll", "55 8B EC 83 EC 14 53 56 57 FF 75 18"), hkCalculateView);
	oGetEyeAngles = HookFunction<tGetEyeAngles>(Utils::PatternScan("client.dll", "56 8B F1 85 F6 74 32"), hkGetEyeAngles);
	oRenderSmokeOverlay = HookFunction<tRenderSmokeOverlay>(Utils::PatternScan("client.dll", "55 8B EC 83 EC 30 80 7D 08 00"), hkRenderSmokeOverlay);
	oShouldInterpolate = HookFunction<tShouldInterpolate>(Utils::PatternScan("client.dll", "56 8B F1 E8 ? ? ? ? 3B F0"), hkShouldInterpolate);
	oPacketStart = HookFunction<tPacketStart>(Utils::PatternScan("engine.dll", "55 8B EC 8B 45 08 89 81 ? ? ? ? 8B 45 0C 89 81 ? ? ? ? 5D C2 08 00 CC CC CC CC CC CC CC 56"), hkPacketStart);
	oPacketEnd = HookFunction<tPacketEnd>(Utils::PatternScan("engine.dll", "56 8B F1 E8 ? ? ? ? 8B 8E ? ? ? ? 3B 8E ? ? ? ? 75 34"), hkPacketEnd);
	oInterpolateViewmodel = HookFunction<tInterpolateViewmodel>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F8 83 EC 0C 53 56 8B F1 57 83 BE ? ? ? ? ?"), hkInterpolateViewmodel);
	//oFX_FireBullets = HookFunction<tFX_FireBullets>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 C0 F3 0F 10 45"), hkFX_FireBullets);
	oProcessMovement = HookFunction<tProcessMovement>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 C0 83 EC 38 A1 ? ? ? ?"), hkProcessMovement);
	oLogDirect = HookFunction<tLogDirect>(Utils::PatternScan("tier0.dll", "55 8B EC 83 E4 F8 8B 45 08 83 EC 14 53 56 8B F1 57 85 C0 0F 88 ? ? ? ?"), hkLogDirect);
	oSetSignonState = HookFunction<tSetSignonState>(Utils::PatternScan("engine.dll", "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 57 FF 75 10"), hkSetSignonState);
	oCreateNewParticleEffect = HookFunction<tCreateNewParticleEffect>(Utils::PatternScan("client.dll", "55 8B EC 83 EC 0C 53 56 8B F2 89 75 F8 57"), hkCreateNewParticleEffect_proxy);
	oEstimateAbsVelocity = HookFunction<tEstimateAbsVelocity>(Utils::PatternScan("client.dll", "55 8B EC 83 E4 F8 83 EC 0C 56 8B F1 85"), hkEstimateAbsVelocity);
	oInterpolationList = HookFunction< tInterpolationList>(Utils::PatternScan("client.dll", "0F ? ? ? ? ? ? 3D ? ? ? ? 74 3F"), hkInterpolationList);

	EventListner->Register();
}

void Hooks::End() {
	SetWindowLongPtr(FindWindowA("Valve001", nullptr), GWL_WNDPROC, (LONG_PTR)oWndProc);

	EventListner->Unregister();

	DirectXDeviceVMT->UnHook(16);
	SurfaceVMT->UnHook(67);
	ClientModeVMT->UnHook(18);
	ClientModeVMT->UnHook(44);
	PanelVMT->UnHook(41);
	EngineVMT->UnHook(90);
	EngineVMT->UnHook(93);
	ModelRenderVMT->UnHook(21);
	ConVarVMT->UnHook(13);
	ClientVMT->UnHook(37);
	ClientVMT->UnHook(11);
	ClientVMT->UnHook(22);
	ClientVMT->UnHook(7);
	ClientVMT->UnHook(24);
	PredictionVMT->UnHook(19);
	ClientVMT->UnHook(40);
	KeyValuesVMT->UnHook(2);
	PredictionVMT->UnHook(14);

	RemoveHook(oPresent, hkPresent);
	//RemoveHook(oReset, hkReset);
	RemoveHook(oUpdateClientSideAnimation, hkUpdateClientSideAnimation);
	RemoveHook(oDoExtraBoneProcessing, hkDoExtraBoneProcessing);
	RemoveHook(oShouldSkipAnimationFrame, hkShouldSkipAnimationFrame);
	RemoveHook(oStandardBlendingRules, hkStandardBlendingRules);
	RemoveHook(oBuildTransformations, hkBuildTransformations);
	RemoveHook(oSetUpLean, hkSetUpLean);
	RemoveHook(oSetupBones, hkSetupBones);
	RemoveHook(oCL_Move, hkCL_Move);
	RemoveHook(oPhysicsSimulate, hkPhysicsSimulate);
	RemoveHook(oClampBonesInBBox, hkClampBonesInBBox);
	RemoveHook(oCalculateView, hkCalculateView);
	RemoveHook(oGetEyeAngles, hkGetEyeAngles);
	RemoveHook(oRenderSmokeOverlay, hkRenderSmokeOverlay);
	RemoveHook(oShouldInterpolate, hkShouldInterpolate);
	RemoveHook(oPacketStart, hkPacketStart);
	RemoveHook(oPacketEnd, hkPacketEnd);
	RemoveHook(oInterpolateViewmodel, hkInterpolateViewmodel);
	//RemoveHook(oFX_FireBullets, hkFX_FireBullets);
	RemoveHook(oProcessMovement, hkProcessMovement);
	RemoveHook(oLogDirect, hkLogDirect);
	RemoveHook(oSetSignonState, hkSetSignonState);
	RemoveHook(oCreateNewParticleEffect, hkCreateNewParticleEffect_proxy);
	RemoveHook(oEstimateAbsVelocity, hkEstimateAbsVelocity);
	RemoveHook(oInterpolationList, hkInterpolationList);
}