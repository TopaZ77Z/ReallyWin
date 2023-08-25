#pragma once
#include "Misc/CBasePlayer.h"
#include "Config.h"

class ConVar;

struct CheatState_t {
	CBasePlayer* LocalPlayer = nullptr;
	bool InGame = false;
	Vector2 ScreenSize;
	bool KeyStates[1024];
	bool Unloaded = false;
	QAngle thirdpersonAngles;
	float ServerTime = 0.f;
	float weaponInaccuracy = 0.f;
	float weaponSpread = 0.f;
	bool freezetime = false;
	//int tickbaseshift = 0;
};

struct HooksInfo_t {
	bool update_csa = false;
	bool setup_bones = false;
	bool console_log = false;
	bool disable_interpolation = false;
};

struct Ctx_t {
	CUserCmd* cmd = nullptr;
	bool send_packet = true;
	int tickbase_shift = 0;
	int shifted_last_tick = 0;
	bool teleported_last_tick = false;
	bool should_update_local_anims = false;
	Vector local_velocity;
	Vector last_local_velocity;
	bool fakeducking = false;
	bool revolver_working = false; 
	float next_revolver_time = 0.f;
	int server_tick = 0;
	bool is_peeking = false;
	bool defensive = false;

	std::vector<int> shifted_commands;
	std::vector<int> sented_commands;

	void reset() {
		shifted_commands.clear();
		sented_commands.clear();
		tickbase_shift = 0;
		local_velocity = Vector();
		last_local_velocity = Vector();
	}
};

struct CVars {
	ConVar* r_aspectratio;
	ConVar* mat_postprocessing_enable;
	ConVar* r_DrawSpecificStaticProp;
	ConVar* fog_override;
	ConVar* fog_color;
	ConVar* fog_start;
	ConVar* fog_end;
	ConVar* fog_maxdensity;
	ConVar* molotov_throw_detonate_time;
	ConVar* sv_cheats;
	ConVar* sv_gravity;
	ConVar* sv_jump_impulse;
	ConVar* sv_maxunlag;
	ConVar* sv_maxusrcmdprocessticks;
	ConVar* weapon_recoil_scale;
	ConVar* cl_csm_shadows;
	ConVar* cl_foot_contact_shadows;
	ConVar* cl_lagcompensation;
	ConVar* cl_interp_ratio;
	ConVar* weapon_debug_spread_show;
	ConVar* r_drawsprites;
	ConVar* zoom_sensitivity_ratio_mouse;
};


extern CheatState_t Cheat;
extern CVars cvars;
extern Ctx_t ctx;
extern HooksInfo_t hook_info;
