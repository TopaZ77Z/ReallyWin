#include "menu.h"

#include "../Resources/aim-img.h"
#include "../Resources/visuals-img.h"
#include "../Resources/background.h"
#include "../Resources/misc-img.h"
#include "../Resources/antiaim-img.h"
#include "../Resources/skinchanger-img.h"
#include "../Resources/playerlist-img.h"
#include "../Resources/config-img.h"
#include "../Resources/lua-img.h"

#include "../Features/Visuals/World.h"

#include "../Utils/Utils.h"
#include "../SDK/Config.h"


void CMenu::Init() {
	aimImg = Render->LoadImageFromMemory(aim_img, sizeof(aim_img), Vector2(50, 50));
	visualsImg = Render->LoadImageFromMemory(visuals_image, sizeof(visuals_image), Vector2(50, 50));
	backgroundTexture = Render->LoadImageFromMemory(skeetmenubg, sizeof(skeetmenubg), Vector2(2000, 2000));
	miscImg = Render->LoadImageFromMemory(misc_img, sizeof(misc_img), Vector2(50, 50));
	antiaimImg = Render->LoadImageFromMemory(antiaim_image, sizeof(antiaim_image), Vector2(50, 50));
	skinchangerImg = Render->LoadImageFromMemory(skinchanger_image, sizeof(skinchanger_image), Vector2(50, 50));
	playerListImg = Render->LoadImageFromMemory(playerlist_image, sizeof(playerlist_image), Vector2(50, 50));
	configImg = Render->LoadImageFromMemory(config_image, sizeof(config_image), Vector2(50, 50));
	luaImg = Render->LoadImageFromMemory(lua_image, sizeof(lua_image), Vector2(50, 50));

	AddGroupBox("RAGE", "Aimbot", 1, 0);
	{
		config.ragebot.aimbot.enabled = AddCheckBox("RAGE", "Aimbot", "Enabled");
		config.ragebot.aimbot.extrapolation = AddComboBox("RAGE", "Aimbot", "Extrapolation", { "Disable", "Enable", "Force" }, 1);
		config.ragebot.aimbot.doubletap = AddCheckBox("RAGE", "Aimbot", "Double tap", true);
		config.ragebot.aimbot.doubletap_key = AddKeyBind("RAGE", "Aimbot", "Double tap");
		config.ragebot.aimbot.doubletap_speed = AddSlider("RAGE", "Aimbot", "Speed", 2, 15, 12, "tick");
		config.ragebot.aimbot.lag_exploit = AddCheckBox("RAGE", "Aimbot", "Lag exploit");
		config.ragebot.aimbot.force_teleport = AddKeyBind("RAGE", "Aimbot", "Force teleport");
		config.ragebot.aimbot.force_body_aim = AddKeyBind("RAGE", "Aimbot", "Force body aim");
		config.ragebot.aimbot.minimum_damage_override_key = AddKeyBind("RAGE", "Aimbot", "Minimum damage override");
		config.ragebot.aimbot.peek_assist = AddCheckBox("RAGE", "Aimbot", "Peek assist");
		config.ragebot.aimbot.peek_assist_color = AddColorPicker("RAGE", "Aimbot", "Peek assist");
		config.ragebot.aimbot.peek_assist_keybind = AddKeyBind("RAGE", "Aimbot", "Peek assist bind");
		config.ragebot.aimbot.low_fps_mitigations = AddMultiCombo("RAGE", "Aimbot", "Low fps mitigations", { "Limit targets per tick", "Fast hitchance" });
		config.ragebot.aimbot.show_aimpoints = AddCheckBox("RAGE", "Aimbot", "Show aim points");
		config.ragebot.aimbot.auto_fire = AddCheckBox("RAGE", "Aimbot", "AutoFire");
	}

	AddGroupBox("RAGE", "Weapon settings", 1, 1);
	{
		config.ragebot.selected_weapon = AddComboBox("RAGE", "Weapon settings", "Weapon configuration", { "Global", "AWP", "Autosniper", "Scout", "Deagle", "Pistol" });

		auto setup_weapon_config = [this](weapon_settings_t& settings) {
			settings.hitboxes = AddMultiCombo("RAGE", "Weapon settings", std::format("[{}] {}", settings.weapon_name, "Hitbox"), { "Head", "Chest", "Stomach", "Arms", "Legs", "Feet" });
			settings.multipoints = AddMultiCombo("RAGE", "Weapon settings", std::format("[{}] {}", settings.weapon_name, "Multipoints"), { "Head", "Chest", "Stomach" });
			settings.head_point_scale = AddSlider("RAGE", "Weapon settings", std::format("[{}] {}", settings.weapon_name, "Head scale"), 0, 100, 50, "%");
			settings.body_point_scale = AddSlider("RAGE", "Weapon settings", std::format("[{}] {}", settings.weapon_name, "Body scale"), 0, 100, 50, "%");
			settings.hitchance = AddSlider("RAGE", "Weapon settings", std::format("[{}] {}", settings.weapon_name, "Hitchance"), 0, 100, 50, "%");
			settings.minimum_damage = AddSlider("RAGE", "Weapon settings", std::format("[{}] {}", settings.weapon_name, "Minimum damage"), 1, 130, 30);
			settings.minimum_damage_override = AddSlider("RAGE", "Weapon settings", std::format("[{}] {}", settings.weapon_name, "Minimum damage override"), 1, 130, 10);
			settings.auto_stop = AddMultiCombo("RAGE", "Weapon settings", std::format("[{}] {}", settings.weapon_name, "Auto stop"), { "Full stop", "Early", "Move between shots" });
			settings.auto_scope = AddCheckBox("RAGE", "Weapon settings", std::format("[{}] {}", settings.weapon_name, "Auto scope"));
		};

		setup_weapon_config(config.ragebot.weapons.global);
		setup_weapon_config(config.ragebot.weapons.awp);
		setup_weapon_config(config.ragebot.weapons.autosniper);
		setup_weapon_config(config.ragebot.weapons.scout);
		setup_weapon_config(config.ragebot.weapons.deagle);
		setup_weapon_config(config.ragebot.weapons.pistol);
	}

	AddGroupBox("ANTIAIM", "Anti-aimbot angles", 1, 0);
	{
		config.antiaim.anti_aimbot_angles.pitch = AddComboBox("ANTIAIM", "Anti-aimbot angles", "Pitch", { "Disabled", "Down", "Deffensive"});
		config.antiaim.anti_aimbot_angles.yaw = AddComboBox("ANTIAIM", "Anti-aimbot angles", "Yaw", { "Forward", "Backward", "At target" });
		config.antiaim.anti_aimbot_angles.yaw_jitter = AddCheckBox("ANTIAIM", "Anti-aimbot angles", "Yaw jitter");
		config.antiaim.anti_aimbot_angles.modifier_value = AddSlider("ANTIAIM", "Anti-aimbot angles", "Modifier value", -180, 180, 0, "°", 1, true);
		config.antiaim.anti_aimbot_angles.manual_left = AddKeyBind("ANTIAIM", "Anti-aimbot angles", "Manual left");
		config.antiaim.anti_aimbot_angles.manual_right = AddKeyBind("ANTIAIM", "Anti-aimbot angles", "Manual right");
		config.antiaim.anti_aimbot_angles.body_yaw = AddCheckBox("ANTIAIM", "Anti-aimbot angles", "Body yaw");
		config.antiaim.anti_aimbot_angles.body_yaw_options = AddMultiCombo("ANTIAIM", "Anti-aimbot angles", "Body yaw options", { "Jitter", "Random jitter", "Extended", "Freestand"});
		config.antiaim.anti_aimbot_angles.body_yaw_limit = AddSlider("ANTIAIM", "Anti-aimbot angles", "Limit", 0, 58, 58, "°");
		config.antiaim.anti_aimbot_angles.inverter = AddKeyBind("ANTIAIM", "Anti-aimbot angles", "Inverter");
	}

	AddGroupBox("ANTIAIM", "Fake lag", 0.6f, 1);
	{
		config.antiaim.fakelag.enabled = AddCheckBox("ANTIAIM", "Fake lag", "Enabled");
		config.antiaim.fakelag.limit = AddSlider("ANTIAIM", "Fake lag", "Limit", 1, 15, 13);
		config.antiaim.fakelag.variability = AddSlider("ANTIAIM", "Fake lag", "Variabaility", 1, 14, 1);
		config.antiaim.fakelag.triggers = AddMultiCombo("ANTIAIM", "Fake lag", "Triggers", { "Move", "Air", "Break LC", "Peek" });
	}

	AddGroupBox("ANTIAIM", "Other", 0.4f, 1);
	{
		config.antiaim.misc.fake_duck = AddKeyBind("ANTIAIM", "Other", "Fake duck");
		config.antiaim.misc.slow_walk = AddKeyBind("ANTIAIM", "Other", "Slow walk");
		config.antiaim.misc.slow_walk_type = AddComboBox("ANTIAIM", "Other", "Type", { "Force accuracy", "Custom" }, 0, true);
		config.antiaim.misc.custom_slow_walk = AddSlider("ANTIAIM", "Other", "Custom speed", 0, 100, 50, "%");
		config.antiaim.misc.animations = AddMultiCombo("ANTIAIM", "Other", "Animations", { "Lean", "Static legs in air", "Backward legs" });
		config.antiaim.misc.leg_movement = AddComboBox("ANTIAIM", "Other", "Leg movement", { "Default", "Sliding", "Walking" });
	}

	AddGroupBox("VISUALS", "Player ESP", 0.5f, 0);
	{
		config.visuals.esp.enable = AddCheckBox("VISUALS", "Player ESP", "Enable");
		config.visuals.esp.dormant = AddCheckBox("VISUALS", "Player ESP", "Dormant");
		config.visuals.esp.dynamic_box = AddCheckBox("VISUALS", "Player ESP", "Dynamic box");
		config.visuals.esp.bounding_box = AddCheckBox("VISUALS", "Player ESP", "Bounding box");
		config.visuals.esp.box_color = AddColorPicker("VISUALS", "Player ESP", "Bounding box");
		config.visuals.esp.health_bar = AddCheckBox("VISUALS", "Player ESP", "Health bar");
		config.visuals.esp.custom_health = AddCheckBox("VISUALS", "Player ESP", "Custom health");
		config.visuals.esp.custom_health_color = AddColorPicker("VISUALS", "Player ESP", "Custom health");
		config.visuals.esp.name = AddCheckBox("VISUALS", "Player ESP", "Name");
		config.visuals.esp.name_color = AddColorPicker("VISUALS", "Player ESP", "Name");
		config.visuals.esp.flags = AddMultiCombo("VISUALS", "Player ESP", "Flags", { "Armor", "Zoom", "Fake duck", "Exploit", "Break LC", "Bomb", "Resolver"});
		config.visuals.esp.weapon_text = AddCheckBox("VISUALS", "Player ESP", "Weapon text");
		config.visuals.esp.weapon_text_color = AddColorPicker("VISUALS", "Player ESP", "Weapon text");
		config.visuals.esp.weapon_icon = AddCheckBox("VISUALS", "Player ESP", "Weapon icon");
		config.visuals.esp.weapon_icon_color = AddColorPicker("VISUALS", "Player ESP", "Weapon icon");
		config.visuals.esp.ammo = AddCheckBox("VISUALS", "Player ESP", "Ammo");
		config.visuals.esp.ammo_color = AddColorPicker("VISUALS", "Player ESP", "Ammo", Color(80, 140, 200));
		config.visuals.esp.glow = AddCheckBox("VISUALS", "Player ESP", "Glow");
		config.visuals.esp.glow_color = AddColorPicker("VISUALS", "Player ESP", "Glow", Color(180, 60, 120));
		config.visuals.esp.hitsound = AddCheckBox("VISUALS", "Player ESP", "Hit sound");
		config.visuals.esp.show_server_hitboxes = AddCheckBox("VISUALS", "Player ESP", "Show sever hitboxes");
	}

	AddGroupBox("VISUALS", "Other ESP", 0.4f, 1);
	{
		config.visuals.other_esp.radar = AddCheckBox("VISUALS", "Other ESP", "Radar");
		config.visuals.other_esp.dropped_weapons = AddMultiCombo("VISUALS", "Other ESP", "Dropped weapons", { "Text", "Icon", "Glow" });
		config.visuals.other_esp.dropped_weapons_color = AddColorPicker("VISUALS", "Other ESP", "Dropped weapons", Color(255, 255, 255));
		config.visuals.other_esp.sniper_crosshair = AddCheckBox("VISUALS", "Other ESP", "Sniper crosshair");
		config.visuals.other_esp.penetration_crosshair = AddCheckBox("VISUALS", "Other ESP", "Penetration crosshair");
		config.visuals.other_esp.bomb = AddCheckBox("VISUALS", "Other ESP", "Bomb");
		config.visuals.other_esp.bomb_color = AddColorPicker("VISUALS", "Other ESP", "Bomb", Color(150, 200, 60));
		config.visuals.other_esp.grenades = AddCheckBox("VISUALS", "Other ESP", "Grenades");
		config.visuals.other_esp.molotov_radius = AddCheckBox("VISUALS", "Other ESP", "Molotov radius");
		config.visuals.other_esp.molotov_radius_color = AddColorPicker("VISUALS", "Other ESP", "Molotov radius", Color(255, 0, 0));
		config.visuals.other_esp.grenade_trajecotry = AddCheckBox("VISUALS", "Other ESP", "Grenade trajectory");
		config.visuals.other_esp.grenade_trajectory_color = AddColorPicker("VISUALS", "Other ESP", "Grenade trajectory", Color(250, 60, 60));
		config.visuals.other_esp.grenade_trajectory_hit_color = AddColorPicker("VISUALS", "Other ESP", "Grenade trajectory (hit)", Color(150, 200, 60));
		config.visuals.other_esp.grenade_proximity_warning = AddCheckBox("VISUALS", "Other ESP", "Grenade proximity warning");
		config.visuals.other_esp.grenade_predict_color = AddColorPicker("VISUALS", "Other ESP", "Grenade predict color");
		config.visuals.other_esp.particles = AddMultiCombo("VISUALS", "Other ESP", "Particles", {"Molotov", "Smoke"});
	}

	AddGroupBox("VISUALS", "Effects", 0.6f, 1);
	{
		config.visuals.effects.fov = AddSlider("VISUALS", "Effects", "Field of view", 80, 130, 90);
		config.visuals.effects.removals = AddMultiCombo("VISUALS", "Effects", "Removals", { "Post effects", "Fog", "Shadows", "Smoke", "Flashbang", "Scope", "Blood", "Sprites" });
		config.visuals.effects.world_color_enable = AddCheckBox("VISUALS", "Effects", "World color");
		config.visuals.effects.world_color = AddColorPicker("VISUALS", "Effects", "World color");
		config.visuals.effects.props_color_enable = AddCheckBox("VISUALS", "Effects", "Props color");
		config.visuals.effects.props_color = AddColorPicker("VISUALS", "Effects", "Props color");
		config.visuals.effects.thirdperson = AddCheckBox("VISUALS", "Effects", "Force thirdperson");
		config.visuals.effects.thirdperson_bind = AddKeyBind("VISUALS", "Effects", "Force thirdperson");
		config.visuals.effects.thirdperson_distance = AddSlider("VISUALS", "Effects", "Thirdperson distance", 25, 200, 100, "");
		config.visuals.effects.aspect_ratio = AddSlider("VISUALS", "Effects", "Aspect ratio", 0, 2, 0, "", 0.01f);
		config.visuals.effects.client_impacts = AddCheckBox("VISUALS", "Effects", "Client impacts");
		config.visuals.effects.client_impacts_color = AddColorPicker("VISUALS", "Effects", "Client impacts", Color(255, 0, 0, 125));
		config.visuals.effects.server_impacts = AddCheckBox("VISUALS", "Effects", "Server impacts");
		config.visuals.effects.server_impacts_color = AddColorPicker("VISUALS", "Effects", "Server impacts", Color(0, 0, 255, 125));
		config.visuals.effects.impacts_duration = AddSlider("VISUALS", "Effects", "Duration", 1, 10, 4, "s");
		config.visuals.effects.override_skybox = AddComboBox("VISUALS", "Effects", "Override skybox", { "Disabled", "Night 1", "Night 2", "Night 3" });
		config.visuals.effects.override_fog = AddCheckBox("VISUALS", "Effects", "Override fog");
		config.visuals.effects.fog_color = AddColorPicker("VISUALS", "Effects", "Override fog");
		config.visuals.effects.fog_start = AddSlider("VISUALS", "Effects", "Fog start", 0, 1000, 200, "", 10);
		config.visuals.effects.fog_end = AddSlider("VISUALS", "Effects", "Fog end", 0, 1000, 500, "", 10);
		config.visuals.effects.fog_density = AddSlider("VISUALS", "Effects", "Fog density", 0, 100, 50);
		config.visuals.effects.preserve_killfeed = AddCheckBox("VISUALS", "Effects", "Preserve killfeed");
		config.visuals.effects.optimizations = AddMultiCombo("VISUALS", "Effects", "Additional removals", { "Teammates", "Ragdolls", "Decals" });
	}

	AddGroupBox("VISUALS", "Colored models", 0.5f, 0);
	{
		config.visuals.chams.enemy = AddCheckBox("VISUALS", "Colored models", "Player");
		config.visuals.chams.enemy_invisible = AddCheckBox("VISUALS", "Colored models", "Player behind wall");
		config.visuals.chams.enemy_color = AddColorPicker("VISUALS", "Colored models", "Player", Color(150, 190, 70));
		config.visuals.chams.enemy_invisible_color = AddColorPicker("VISUALS", "Colored models", "Player behind wall", Color(60, 120, 160));
		config.visuals.chams.enemy_type = AddComboBox("VISUALS", "Colored models", "Enemy type", { "Default", "Solid", "Glow" }, 0, true);
		config.visuals.chams.enemy_second_color = AddColorPicker("VISUALS", "Colored models", "Enemy type");
		config.visuals.chams.enemy_glow_thickness = AddSlider("VISUALS", "Colored models", "Glow thickness", 0.1f, 9.f, 1.f, "", 0.1f);
		config.visuals.chams.local_player = AddCheckBox("VISUALS", "Colored models", "Local player");
		config.visuals.chams.local_player_color = AddColorPicker("VISUALS", "Colored models", "Local player", Color(100, 100, 100));
		config.visuals.chams.local_player_type = AddComboBox("VISUALS", "Colored models", "Local type", { "Default", "Solid", "Glow" }, 0, true);
		config.visuals.chams.local_glow_thickness = AddSlider("VISUALS", "Colored models", "Glow thickness", 0.1f, 9.f, 1.f, "", 0.1f);
		config.visuals.chams.attachments = AddCheckBox("VISUALS", "Colored models", "Attachments");
		config.visuals.chams.attachments_color = AddColorPicker("VISUALS", "Colored models", "Attachments");
		config.visuals.chams.attachments_type = AddComboBox("VISUALS", "Colored models", "Attachments type", { "Default", "Solid", "Glow" }, 0, true);
		config.visuals.chams.attachemtns_second_color = AddColorPicker("VISUALS", "Colored models", "Attachments type");
		config.visuals.chams.attachments_glow_thickness = AddSlider("VISUALS", "Colored models", "Glow thickness", 0.1f, 9.f, 1.f, "", 0.1f);
		config.visuals.chams.scope_blend = AddSlider("VISUALS", "Colored models", "Scope blend", 0, 100, 100);
		config.visuals.chams.disable_model_occlusion = AddCheckBox("VISUALS", "Colored models", "Disable model occlusion");
	}

	AddGroupBox("MISC", "Miscellaneous", 1, 0);
	{
		config.misc.miscellaneous.anti_untrusted = AddCheckBox("MISC", "Miscellaneous", "Anti untrusted");
		config.misc.miscellaneous.logs = AddMultiCombo("MISC", "Miscellaneous", "Logs", { "Damage", "Aimbot", "Purchuases" });
		config.misc.miscellaneous.auto_buy = AddMultiCombo("MISC", "Miscellaneous", "Auto buy", { "AWP", "Scout", "Autosniper", "Deagle / R8", "Five-Seven / Tec-9", "Taser", "Armor", "Smoke", "Molotov", "HeGrenade", "Flashbang", "Defuse kit" });
		config.misc.miscellaneous.filter_console = AddCheckBox("MISC", "Miscellaneous", "Filter console");
		config.misc.miscellaneous.clantag = AddCheckBox("MISC", "Miscellaneous", "Clantag");
		config.misc.miscellaneous.shot_effect = AddCheckBox("MISC", "Miscellaneous", "Shot Effect");
		config.misc.miscellaneous.quick_switch = AddCheckBox("MISC", "Miscellaneous", "Quick Switch");
	}

	AddGroupBox("MISC", "Movement", 0.5f, 1);
	{
		config.misc.movement.auto_jump = AddCheckBox("MISC", "Movement", "Auto jump");
		config.misc.movement.auto_strafe = AddCheckBox("MISC", "Movement", "Auto strafe");
		config.misc.movement.auto_strafe_smooth = AddSlider("MISC", "Movement", "Auto strafe smooth", 0, 100, 50, "%");
		config.misc.movement.compensate_throwable = AddCheckBox("MISC", "Movement", "Compensate throwable");
		config.misc.movement.edge_jump = AddCheckBox("MISC", "Movement", "Edge jump");
		config.misc.movement.edge_jump_key = AddKeyBind("MISC", "Movement", "Edge jump");
		config.misc.movement.infinity_duck = AddCheckBox("MISC", "Movement", "Infinity duck");
		config.misc.movement.quick_stop = AddCheckBox("MISC", "Movement", "Quick stop");
	}

	AddGroupBox("MISC", "Settings", 0.5f, 1);
	{
		menu_color_picker = AddColorPicker("MISC", "Settings", "Menu color", Color(150, 210, 25));
		menu_key_bind = AddKeyBind("MISC", "Settings", "Menu key", VK_INSERT);
		AddButton("MISC", "Settings", "Unload");
	}

	AddGroupBox("SKINS", "Weapon skin", 0.6f, 0);
	{

	}

	AddGroupBox("SKINS", "Knife options", 1, 1);
	{

	}

	AddGroupBox("SKINS", "Glove options", 0.4f, 0);
	{

	}

	AddGroupBox("PLAYERS", "Players", 1, 0);
	{

	}

	AddGroupBox("PLAYERS", "Adjustements", 1, 1);
	{

	}

	AddGroupBox("CONFIG", "Presets", 1, 0);
	{
		Config->config_list = AddListBox("CONFIG", "Presets", "cfglist", {});
		Config->config_name = AddInputBox("CONFIG", "Presets", "Config");
		Config->load_button = AddButton("CONFIG", "Presets", "Load");
		Config->save_button = AddButton("CONFIG", "Presets", "Save");
		AddButton("CONFIG", "Presets", "Delete");
		AddButton("CONFIG", "Presets", "Reset");
		AddButton("CONFIG", "Presets", "Import from clipboard");
		AddButton("CONFIG", "Presets", "Export to clipboard");
	}

	AddGroupBox("CONFIG", "Lua", 1, 1);
	{

	}

	AddGroupBox("LUA", "A", 1, 0);
	AddGroupBox("LUA", "B", 1, 1);

	menu_key_bind->toggled = true;

	Config->Init();

	config.ragebot.selected_weapon->set_callback([]() {
		const int selected_weapon = config.ragebot.selected_weapon->get();
		auto& weapon_configs = config.ragebot.weapons;

		weapon_configs.global.set_visible(selected_weapon == 0);
		weapon_configs.awp.set_visible(selected_weapon == 1);
		weapon_configs.autosniper.set_visible(selected_weapon == 2);
		weapon_configs.scout.set_visible(selected_weapon == 3);
		weapon_configs.deagle.set_visible(selected_weapon == 4);
		weapon_configs.pistol.set_visible(selected_weapon == 5);
	});

	auto world_modulation_callback = []() {
		World->Modulation();
	};

	config.visuals.effects.world_color_enable->set_callback(world_modulation_callback);
	config.visuals.effects.world_color->set_callback(world_modulation_callback);
	config.visuals.effects.props_color_enable->set_callback(world_modulation_callback);
	config.visuals.effects.props_color->set_callback(world_modulation_callback);

	config.visuals.effects.override_skybox->set_callback([]() {
		World->SkyBox();
	});

	auto world_fog_callback = []() {
		World->Fog();

		config.visuals.effects.fog_density->set_visible(config.visuals.effects.override_fog->get());
		config.visuals.effects.fog_start->set_visible(config.visuals.effects.override_fog->get());
		config.visuals.effects.fog_end->set_visible(config.visuals.effects.override_fog->get());
	};

	config.visuals.effects.override_fog->set_callback(world_fog_callback);
	config.visuals.effects.fog_color->set_callback(world_fog_callback);
	config.visuals.effects.fog_density->set_callback(world_fog_callback);
	config.visuals.effects.fog_start->set_callback(world_fog_callback);
	config.visuals.effects.fog_end->set_callback(world_fog_callback);

	config.visuals.effects.removals->set_callback([]() {
		World->RemoveBlood();
		World->Smoke();
	});

	initialized = true;
}