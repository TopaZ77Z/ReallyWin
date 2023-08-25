#pragma once

#include "../SDK/Interfaces.h"
#include "../Utils/VMTHook.h"
#include "../detours.h"

class CBasePlayer;
class CBaseCombatWeapon;

typedef HRESULT(__stdcall* tEndScene)(IDirect3DDevice9*);
typedef HRESULT(__stdcall* tReset)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
typedef HRESULT(__stdcall* tPresent)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
typedef void(__fastcall* tLockCursor)(ISurface*, void*);
typedef void(__fastcall* tOverrideView)(IClientMode*, void*, CViewSetup*);
typedef void(__fastcall* tPaintTraverse)(IPanel*, void*, unsigned int, bool, bool);
typedef void(__fastcall* tDoPostScreenEffects)(IClientMode*, void*, CViewSetup*);
typedef bool(__fastcall* tIsPaused)(IVEngineClient*, void*);
typedef void(__fastcall* tDrawModelExecute)(IVModelRender*, void*, void*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
typedef int(__fastcall* tGetBool)(ConVar*, void*);
typedef void(__fastcall* tFrameStageNotify)(IBaseClientDLL*, void*, EClientFrameStage);
typedef void(__fastcall* tRunCommand)(IPrediction*, void*, CBasePlayer*, CUserCmd*, IMoveHelper*);
typedef void(__fastcall* tUpdateClientSideAnimation)(CBasePlayer*, void*);
typedef bool(__fastcall* tShouldSkipAnimationFrame)(void*, void*);
typedef bool(__fastcall* tShouldInterpolate)(CBasePlayer*, void*);
typedef void(__fastcall* tDoExtraBoneProcessing)(CBaseEntity*, void*, CStudioHdr*, Vector*, Quaternion*, const matrix3x4_t&, uint8_t*, void*);
typedef void(__fastcall* tStandardBlendingRules)(void*, void*, void*, void*, void*, float, int);
typedef bool(__fastcall* tIsHLTV)(IVEngineClient*, void*);
typedef void(__fastcall* tBuildTransformations)(CBaseEntity*, void*, void*, void*, void*, const void*, int, void*);
typedef void(__fastcall* tSetUpLean)(void*, void*);
typedef void(__fastcall* tCheckForSequenceChange)(void*, void*, void*, int, bool, bool);
typedef bool(__fastcall* tSetupBones)(CBaseEntity*, void*, matrix3x4_t*, int, int, float);
typedef void(__fastcall* tUpdateAttachments)(CBasePlayer*, void*);
typedef void(__cdecl* tCL_Move)(float, bool);
typedef void(__fastcall* tHudUpdate)(IBaseClientDLL*, void*, bool);
typedef void(__thiscall* tCHLCCreateMove)(IBaseClientDLL*, int, float, bool);
typedef bool(__thiscall* tWriteUserCmdDeltaToBuffer)(void*, int, void*, int, int, bool);
typedef void(__fastcall* tPhysicsSimulate)(CBasePlayer*, void*);
typedef void(__fastcall* tClampBonesInBBox)(CBasePlayer*, void*, matrix3x4_t*, int);
typedef void(__fastcall* tCalculateView)(CBasePlayer*, void*, Vector&, QAngle&, float&, float&, float&);
typedef QAngle*(__fastcall* tGetEyeAngles)(CBasePlayer*, void*);
typedef void(__fastcall* tRenderSmokeOverlay)(void*, void*, bool);
typedef void(__fastcall* tPacketEnd)(CClientState*, void*);
typedef void(__fastcall* tPacketStart)(CClientState*, void*, int, int);
typedef bool(__fastcall* tInterpolateViewmodel)(CBaseEntity*, void*, float);
typedef bool(__fastcall* tFireGameEvent)(IGameEventManager2*, void*, IGameEvent*, bool, bool);
typedef void(__stdcall* tFX_FireBullets)(CBaseCombatWeapon*, int, unsigned short, const Vector&, const QAngle&, int, int, float, float, float, float, int, float);
typedef void(__fastcall* tProcessMovement)(IGameMovement*, void*, CBasePlayer*, CMoveData*);
typedef int(__fastcall* tLogDirect)(void*, void*, int, int, Color, const char*);
typedef void(__fastcall* tEyePositionAndVectors)(uintptr_t, void*, Vector*, Vector*, Vector*, Vector*);
typedef void(*tCL_FullyConnected)();
typedef bool(__fastcall* tSetSignonState)(void*, void*, int, int, const void*);
typedef CNewParticleEffect*(__fastcall* tCreateNewParticleEffect)(CBaseEntity*, void*, Vector const&, const char*, int);
typedef void(__fastcall* tEstimateAbsVelocity)(void*, void*, Vector&);
typedef int(__fastcall* tInterpolationList)(void*, void*);
typedef bool(__thiscall* tInPrediction)(void*);

inline WNDPROC oWndProc;
inline tEndScene oEndScene;
inline tPresent oPresent;
inline tUpdateClientSideAnimation oUpdateClientSideAnimation;
inline tShouldSkipAnimationFrame oShouldSkipAnimationFrame;
inline tDoExtraBoneProcessing oDoExtraBoneProcessing;
inline tStandardBlendingRules oStandardBlendingRules;
inline tBuildTransformations oBuildTransformations;
inline tSetUpLean oSetUpLean;
inline tCheckForSequenceChange oCheckForSequenceChange;
inline tSetupBones oSetupBones;
inline tUpdateAttachments oUpdateAttachments;
inline tCL_Move oCL_Move;
inline tPhysicsSimulate oPhysicsSimulate;
inline tClampBonesInBBox oClampBonesInBBox;
inline tCalculateView oCalculateView;
inline tGetEyeAngles oGetEyeAngles;
inline tRenderSmokeOverlay oRenderSmokeOverlay;
inline tShouldInterpolate oShouldInterpolate;
inline tPacketEnd oPacketEnd;
inline tPacketStart oPacketStart;
inline tInterpolateViewmodel oInterpolateViewmodel;
inline tFireGameEvent oFireGameEvent;
inline tFX_FireBullets oFX_FireBullets;
inline tProcessMovement oProcessMovement;
inline tLogDirect oLogDirect;
inline tEyePositionAndVectors oEyePositionAndVectors;
inline tCL_FullyConnected oCL_FullyConnected;
inline tSetSignonState oSetSignonState;
inline tCreateNewParticleEffect oCreateNewParticleEffect;
inline tEstimateAbsVelocity oEstimateAbsVelocity;
inline tInterpolationList oInterpolationList;

namespace Hooks {
	inline VMT* DirectXDeviceVMT;
	inline VMT* SurfaceVMT;
	inline VMT* ClientModeVMT;
	inline VMT* PanelVMT;
	inline VMT* EngineVMT;
	inline VMT* ModelRenderVMT;
	inline VMT* ConVarVMT;
	inline VMT* ClientVMT;
	inline VMT* PredictionVMT;
	inline VMT* KeyValuesVMT;

	void Initialize();
	void End();
}