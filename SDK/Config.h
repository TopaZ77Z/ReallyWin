#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>

#include "../Menu/menu.h"
#include "../Utils/json.hpp"

struct weapon_settings_t {
    std::string weapon_name;

    CMultiCombo* hitboxes{ nullptr };
    CMultiCombo* multipoints{ nullptr };
    CSlider* head_point_scale{ nullptr };
    CSlider* body_point_scale{ nullptr };
    CSlider* hitchance{ nullptr };
    CSlider* minimum_damage{ nullptr };
    CSlider* minimum_damage_override{ nullptr };
    CMultiCombo* auto_stop{ nullptr };
    CCheckbox* auto_scope{ nullptr };

    weapon_settings_t() {}
    weapon_settings_t(const std::string& name) : weapon_name(name) {}

    inline void set_visible(bool visible) {
        hitboxes->set_visible(visible);
        multipoints->set_visible(visible);
        head_point_scale->set_visible(visible);
        body_point_scale->set_visible(visible);
        hitchance->set_visible(visible);
        minimum_damage->set_visible(visible);
        minimum_damage_override->set_visible(visible);
        auto_stop->set_visible(visible);
        auto_scope->set_visible(visible);
    };
};

struct config_t {
    struct ragebot_t {
        struct aimbot_t {
            CCheckbox* enabled;
            CComboBox* extrapolation;
            CCheckbox* doubletap;
            CKeyBind* doubletap_key;
            CSlider* doubletap_speed;
            CCheckbox* lag_exploit;
            CKeyBind* force_teleport;
            CKeyBind* force_body_aim;
            CKeyBind* minimum_damage_override_key;
            CCheckbox* peek_assist;
            CColorPicker* peek_assist_color;
            CKeyBind* peek_assist_keybind;
            CMultiCombo* low_fps_mitigations;
            CCheckbox* show_aimpoints;
            CCheckbox* auto_fire;
        } aimbot;

        CComboBox* selected_weapon;

        struct weapons_t {
            weapon_settings_t global{ "Global" };
            weapon_settings_t awp{ "AWP" };
            weapon_settings_t autosniper{ "Auto" };
            weapon_settings_t scout{ "Scout" };
            weapon_settings_t deagle{ "Deagle" };
            weapon_settings_t pistol{ "Pistol" };
        } weapons;
    } ragebot;

    struct antiaim_t {
        struct anti_aimbot_angles_t {
            CComboBox* pitch;
            CComboBox* yaw;
            CCheckbox* yaw_jitter;
            CSlider* modifier_value;
            CKeyBind* manual_left;
            CKeyBind* manual_right;
            CCheckbox* body_yaw;
            CMultiCombo* body_yaw_options;
            CSlider* body_yaw_limit;
            CKeyBind* inverter;
        } anti_aimbot_angles;

        struct fakelag_t {
            CCheckbox* enabled;
            CSlider* limit;
            CSlider* variability;
            CMultiCombo* triggers;
        } fakelag;

        struct misc_t {
            CKeyBind* fake_duck;
            CKeyBind* slow_walk;
            CComboBox* slow_walk_type;
            CSlider* custom_slow_walk;
            CMultiCombo* animations;
            CComboBox* leg_movement;
        } misc;
    } antiaim;

    struct visuals_t {
        struct esp_t {
            CCheckbox* enable;
            CCheckbox* dormant;
            CCheckbox* dynamic_box;
            CCheckbox* bounding_box;
            CColorPicker* box_color;
            CCheckbox* health_bar;
            CCheckbox* custom_health;
            CColorPicker* custom_health_color;
            CCheckbox* name;
            CColorPicker* name_color;
            CMultiCombo* flags;
            CCheckbox* weapon_text;
            CColorPicker* weapon_text_color;
            CCheckbox* weapon_icon;
            CColorPicker* weapon_icon_color;
            CCheckbox* ammo;
            CColorPicker* ammo_color;
            CCheckbox* glow;
            CColorPicker* glow_color;
            CCheckbox* hitsound;
            CCheckbox* show_server_hitboxes;
        } esp;

        struct chams_t {
            CCheckbox* enemy;
            CColorPicker* enemy_color;
            CColorPicker* enemy_second_color;
            CCheckbox* enemy_invisible;
            CColorPicker* enemy_invisible_color;
            CComboBox* enemy_type;
            CSlider* enemy_glow_thickness;

            CCheckbox* local_player;
            CColorPicker* local_player_color;
            CComboBox* local_player_type;
            CSlider* local_glow_thickness;

            CCheckbox* attachments;
            CColorPicker* attachments_color;
            CComboBox* attachments_type;
            CColorPicker* attachemtns_second_color;
            CSlider* attachments_glow_thickness;

            CSlider* scope_blend;
            CCheckbox* disable_model_occlusion;
        } chams;

        struct other_esp_t {
            CCheckbox* radar;
            CMultiCombo* dropped_weapons;
            CColorPicker* dropped_weapons_color;
            CCheckbox* sniper_crosshair;
            CCheckbox* penetration_crosshair;
            CCheckbox* bomb;
            CColorPicker* bomb_color;
            CCheckbox* grenades;
            CCheckbox* molotov_radius;
            CColorPicker* molotov_radius_color;
            CCheckbox* grenade_trajecotry;
            CColorPicker* grenade_trajectory_color;
            CColorPicker* grenade_trajectory_hit_color;
            CCheckbox* grenade_proximity_warning;
            CColorPicker* grenade_predict_color;
            CMultiCombo* particles;
        } other_esp;

        struct effects_t {
            CSlider* fov;
            CMultiCombo* removals;
            CCheckbox* world_color_enable;
            CColorPicker* world_color;
            CCheckbox* props_color_enable;
            CColorPicker* props_color;
            CCheckbox* thirdperson;
            CKeyBind* thirdperson_bind;
            CSlider* thirdperson_distance;
            CSlider* aspect_ratio;
            CCheckbox* client_impacts;
            CColorPicker* client_impacts_color;
            CCheckbox* server_impacts;
            CColorPicker* server_impacts_color;
            CSlider* impacts_duration;
            CComboBox* override_skybox;
            CCheckbox* override_fog;
            CColorPicker* fog_color;
            CSlider* fog_start;
            CSlider* fog_end;
            CSlider* fog_density;
            CCheckbox* preserve_killfeed;
            CMultiCombo* optimizations;
        } effects;
    } visuals;

    struct misc_t {
        struct miscellaneous_t {
            CCheckbox* anti_untrusted;
            CMultiCombo* logs;
            CMultiCombo* auto_buy;
            CCheckbox* filter_console;
            CCheckbox* clantag;
            CCheckbox* shot_effect;
            CCheckbox* quick_switch;
        } miscellaneous;

        struct movement_t {
            CCheckbox* auto_jump;
            CCheckbox* auto_strafe;
            CCheckbox* compensate_throwable;
            CSlider* auto_strafe_smooth;
            CCheckbox* edge_jump;
            CKeyBind* edge_jump_key;
            CCheckbox* infinity_duck;
            CCheckbox* quick_stop;
        } movement;
    } misc;
};

extern config_t config;

struct config_item_t {
    IBaseElement* item;
    std::string name;
};

class CConfig {
    std::vector<config_item_t> items;

    inline void add(IBaseElement* item) { 
        items.emplace_back(config_item_t{ item, item->parent->name + "_" + item->name + "_" + std::to_string(item->GetItemType()) });
    };

public:
    CListBox* config_list;
    CInputBox* config_name;
    CButton* load_button;
    CButton* save_button;
    CButton* delete_button;
    CButton* import_button;
    CButton* export_button;

    void parse(nlohmann::json& cfg);
    nlohmann::json dump();

    std::vector<std::string> get_all_configs() {
        std::vector<std::string> result;

        for (const auto& file : std::filesystem::directory_iterator(std::filesystem::current_path().string() + "/rw"))
            if (file.path().extension() == ".cfg")
                result.push_back(file.path().stem().string());

        return result;
    }

    void Init();
};

extern CConfig* Config;