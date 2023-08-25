#include "Config.h"

#include "../Utils/Utils.h"

config_t config;
CConfig* Config = new CConfig;

// callbacks
void on_config_list_changed() {
    Config->config_name->input = Config->config_list->get();

    Config->config_list->UpdateList(Config->get_all_configs());
};

void on_load() {
    std::string name = Config->config_name->input;

    if (name == "")
        return;

    const std::string file_path = std::filesystem::current_path().string() + "/rw/" + name + ".cfg";
    if (!std::filesystem::exists(file_path)) {
        Utils::Print("[reallywin] config doesnt exists: %s\n", name);
        return;
    }

    std::ifstream file(file_path);
    nlohmann::json config_json;
    file >> config_json;

    Config->parse(config_json);
}

void on_save() {
    std::string name = Config->config_name->input;

    if (name == "")
        return;

    const std::string file_path = std::filesystem::current_path().string() + "/rw/" + name + ".cfg";

    std::ofstream file(file_path);
    nlohmann::json config_json = Config->dump();

    file << config_json;
};

void on_delete() {};
void on_import() {};
void on_export() {};

void CConfig::Init() {
    add(config.ragebot.aimbot.enabled);
    add(config.ragebot.aimbot.extrapolation);
    add(config.ragebot.aimbot.doubletap);
    add(config.ragebot.aimbot.doubletap_key);
    add(config.ragebot.aimbot.doubletap_speed);
    add(config.ragebot.aimbot.lag_exploit);
    add(config.ragebot.aimbot.force_teleport);
    add(config.ragebot.aimbot.force_body_aim);
    add(config.ragebot.aimbot.minimum_damage_override_key);
    add(config.ragebot.aimbot.peek_assist);
    add(config.ragebot.aimbot.peek_assist_color);
    add(config.ragebot.aimbot.peek_assist_keybind);
    add(config.ragebot.aimbot.low_fps_mitigations);
    add(config.ragebot.aimbot.show_aimpoints);
    add(config.ragebot.aimbot.auto_fire);
    add(config.ragebot.weapons.global.hitboxes);
    add(config.ragebot.weapons.global.multipoints);
    add(config.ragebot.weapons.global.head_point_scale);
    add(config.ragebot.weapons.global.body_point_scale);
    add(config.ragebot.weapons.global.hitchance);
    add(config.ragebot.weapons.global.minimum_damage);
    add(config.ragebot.weapons.global.minimum_damage_override);
    add(config.ragebot.weapons.global.auto_stop);
    add(config.ragebot.weapons.global.auto_scope);
    add(config.ragebot.weapons.awp.hitboxes);
    add(config.ragebot.weapons.awp.multipoints);
    add(config.ragebot.weapons.awp.head_point_scale);
    add(config.ragebot.weapons.awp.body_point_scale);
    add(config.ragebot.weapons.awp.hitchance);
    add(config.ragebot.weapons.awp.minimum_damage);
    add(config.ragebot.weapons.awp.minimum_damage_override);
    add(config.ragebot.weapons.awp.auto_stop);
    add(config.ragebot.weapons.awp.auto_scope);
    add(config.ragebot.weapons.autosniper.hitboxes);
    add(config.ragebot.weapons.autosniper.multipoints);
    add(config.ragebot.weapons.autosniper.head_point_scale);
    add(config.ragebot.weapons.autosniper.body_point_scale);
    add(config.ragebot.weapons.autosniper.hitchance);
    add(config.ragebot.weapons.autosniper.minimum_damage);
    add(config.ragebot.weapons.autosniper.minimum_damage_override);
    add(config.ragebot.weapons.autosniper.auto_stop);
    add(config.ragebot.weapons.autosniper.auto_scope);
    add(config.ragebot.weapons.scout.hitboxes);
    add(config.ragebot.weapons.scout.multipoints);
    add(config.ragebot.weapons.scout.head_point_scale);
    add(config.ragebot.weapons.scout.body_point_scale);
    add(config.ragebot.weapons.scout.hitchance);
    add(config.ragebot.weapons.scout.minimum_damage);
    add(config.ragebot.weapons.scout.minimum_damage_override);
    add(config.ragebot.weapons.scout.auto_stop);
    add(config.ragebot.weapons.scout.auto_scope);
    add(config.ragebot.weapons.deagle.hitboxes);
    add(config.ragebot.weapons.deagle.multipoints);
    add(config.ragebot.weapons.deagle.head_point_scale);
    add(config.ragebot.weapons.deagle.body_point_scale);
    add(config.ragebot.weapons.deagle.hitchance);
    add(config.ragebot.weapons.deagle.minimum_damage);
    add(config.ragebot.weapons.deagle.minimum_damage_override);
    add(config.ragebot.weapons.deagle.auto_stop);
    add(config.ragebot.weapons.deagle.auto_scope);
    add(config.ragebot.weapons.pistol.hitboxes);
    add(config.ragebot.weapons.pistol.multipoints);
    add(config.ragebot.weapons.pistol.head_point_scale);
    add(config.ragebot.weapons.pistol.body_point_scale);
    add(config.ragebot.weapons.pistol.hitchance);
    add(config.ragebot.weapons.pistol.minimum_damage);
    add(config.ragebot.weapons.pistol.minimum_damage_override);
    add(config.ragebot.weapons.pistol.auto_stop);
    add(config.ragebot.weapons.pistol.auto_scope);
    add(config.antiaim.anti_aimbot_angles.pitch);
    add(config.antiaim.anti_aimbot_angles.yaw);
    add(config.antiaim.anti_aimbot_angles.yaw_jitter);
    add(config.antiaim.anti_aimbot_angles.modifier_value);
    add(config.antiaim.anti_aimbot_angles.manual_left);
    add(config.antiaim.anti_aimbot_angles.manual_right);
    add(config.antiaim.anti_aimbot_angles.body_yaw);
    add(config.antiaim.anti_aimbot_angles.body_yaw_options);
    add(config.antiaim.anti_aimbot_angles.body_yaw_limit);
    add(config.antiaim.anti_aimbot_angles.inverter);
    add(config.antiaim.fakelag.enabled);
    add(config.antiaim.fakelag.limit);
    add(config.antiaim.fakelag.variability);
    add(config.antiaim.fakelag.triggers);
    add(config.antiaim.misc.fake_duck);
    add(config.antiaim.misc.slow_walk);
    add(config.antiaim.misc.slow_walk_type);
    add(config.antiaim.misc.custom_slow_walk);
    add(config.antiaim.misc.animations);
    add(config.antiaim.misc.leg_movement);
    add(config.visuals.esp.enable);
    add(config.visuals.esp.dormant);
    add(config.visuals.esp.dynamic_box);
    add(config.visuals.esp.bounding_box);
    add(config.visuals.esp.box_color);
    add(config.visuals.esp.health_bar);
    add(config.visuals.esp.custom_health);
    add(config.visuals.esp.custom_health_color);
    add(config.visuals.esp.name);
    add(config.visuals.esp.name_color);
    add(config.visuals.esp.flags);
    add(config.visuals.esp.weapon_text);
    add(config.visuals.esp.weapon_text_color);
    add(config.visuals.esp.weapon_icon);
    add(config.visuals.esp.weapon_icon_color);
    add(config.visuals.esp.ammo);
    add(config.visuals.esp.ammo_color);
    add(config.visuals.esp.glow);
    add(config.visuals.esp.glow_color);
    add(config.visuals.esp.hitsound);
    add(config.visuals.chams.enemy);
    add(config.visuals.chams.enemy_color);
    add(config.visuals.chams.enemy_second_color);
    add(config.visuals.chams.enemy_invisible);
    add(config.visuals.chams.enemy_invisible_color);
    add(config.visuals.chams.enemy_type);
    add(config.visuals.chams.enemy_glow_thickness);
    add(config.visuals.chams.local_player);
    add(config.visuals.chams.local_player_color);
    add(config.visuals.chams.local_player_type);
    add(config.visuals.chams.local_glow_thickness);
    add(config.visuals.chams.attachments);
    add(config.visuals.chams.attachments_color);
    add(config.visuals.chams.attachments_type);
    add(config.visuals.chams.attachemtns_second_color);
    add(config.visuals.chams.attachments_glow_thickness);
    add(config.visuals.chams.scope_blend);
    add(config.visuals.chams.disable_model_occlusion);
    add(config.visuals.other_esp.radar);
    add(config.visuals.other_esp.dropped_weapons);
    add(config.visuals.other_esp.dropped_weapons_color);
    add(config.visuals.other_esp.sniper_crosshair);
    add(config.visuals.other_esp.penetration_crosshair);
    add(config.visuals.other_esp.bomb);
    add(config.visuals.other_esp.bomb_color);
    add(config.visuals.other_esp.grenades);
    add(config.visuals.other_esp.grenade_trajecotry);
    add(config.visuals.other_esp.grenade_trajectory_color);
    add(config.visuals.other_esp.grenade_trajectory_hit_color);
    add(config.visuals.other_esp.grenade_proximity_warning);
    add(config.visuals.other_esp.grenade_predict_color);
    add(config.visuals.other_esp.particles);
    add(config.visuals.effects.fov);
    add(config.visuals.effects.removals);
    add(config.visuals.effects.world_color_enable);
    add(config.visuals.effects.world_color);
    add(config.visuals.effects.props_color_enable);
    add(config.visuals.effects.props_color);
    add(config.visuals.effects.thirdperson);
    add(config.visuals.effects.thirdperson_bind);
    add(config.visuals.effects.thirdperson_distance);
    add(config.visuals.effects.aspect_ratio);
    add(config.visuals.effects.client_impacts);
    add(config.visuals.effects.client_impacts_color);
    add(config.visuals.effects.server_impacts);
    add(config.visuals.effects.server_impacts_color);
    add(config.visuals.effects.impacts_duration);
    add(config.visuals.effects.override_skybox);
    add(config.visuals.effects.override_fog);
    add(config.visuals.effects.fog_color);
    add(config.visuals.effects.fog_start);
    add(config.visuals.effects.fog_end);
    add(config.visuals.effects.fog_density);
    add(config.visuals.effects.preserve_killfeed);
    add(config.visuals.effects.optimizations);
    add(config.misc.miscellaneous.anti_untrusted);
    add(config.misc.miscellaneous.logs);
    add(config.misc.miscellaneous.auto_buy);
    add(config.misc.miscellaneous.filter_console);
    add(config.misc.miscellaneous.clantag);
    add(config.misc.miscellaneous.shot_effect);
    add(config.misc.miscellaneous.quick_switch);
    add(config.misc.movement.auto_jump);
    add(config.misc.movement.auto_strafe);
    add(config.misc.movement.compensate_throwable);
    add(config.misc.movement.auto_strafe_smooth);
    add(config.misc.movement.edge_jump);
    add(config.misc.movement.edge_jump_key);
    add(config.misc.movement.infinity_duck);
    add(config.misc.movement.quick_stop);

    add(Menu->menu_color_picker);
    add(Menu->menu_key_bind);

    load_button->set_callback(on_load);
    save_button->set_callback(on_save);
    config_list->set_callback(on_config_list_changed);
}

void CConfig::parse(nlohmann::json& cfg) {
    for (auto& item : items) {
        try {
            IBaseElement* e = item.item;
            auto& val = cfg[item.name];

            switch (item.item->GetItemType()) {
            case CHECKBOX:
                ((CCheckbox*)e)->value = val;
                break;
            case COLORPCIKER:
                ((CColorPicker*)e)->Set(val);
                break;
            case KEYBIND:
                ((CKeyBind*)e)->bindType = val[0];
                ((CKeyBind*)e)->bindState = val[1];
                ((CKeyBind*)e)->key = val[2];
                break;
            case SLIDER:
                ((CSlider*)e)->value = val;
                break;
            case COMBO:
                ((CComboBox*)e)->value = val;
                break;
            case MULTICOMBO:
                ((CMultiCombo*)e)->values = val;
                break;
            case INPUTBOX:
                ((CInputBox*)e)->input = val;
                break;
            case LISTBOX:
                ((CListBox*)e)->active = val;
                break;
            default:
                break;
            }

            if (e->callback)
                e->callback();
        }
        catch (nlohmann::json::exception& e) {
            Utils::Print("[reallywin] config load: item not found %s\n", item.name.c_str());
        }
    }

    try {
        Menu->size = Vector2(static_cast<int>(cfg["menu_size"][0]), static_cast<int>(cfg["menu_size"][1]));
        //Menu->menuPos = Vector2(static_cast<int>(cfg["menu_pos"][0]), static_cast<int>(cfg["menu_pos"][1]));
    }
    catch (nlohmann::json::exception& e) {
        Utils::Print("[reallywin] error when loading menu state\n");
    }
}

nlohmann::json CConfig::dump() {
    nlohmann::json result;

    for (const auto& item : items) {
        IBaseElement* e = item.item;
        std::string name = item.name;

        switch (e->GetItemType()) {
        case CHECKBOX:
            result[name] = ((CCheckbox*)e)->value;
            break;
        case COLORPCIKER:
            result[name] = ((CColorPicker*)e)->get().to_int32();
            break;
        case KEYBIND:
            result[name] = { ((CKeyBind*)e)->bindType, ((CKeyBind*)e)->bindState, ((CKeyBind*)e)->key };
            break;
        case SLIDER:
            result[name] = ((CSlider*)e)->value;
            break;
        case COMBO:
            result[name] = ((CComboBox*)e)->value;
            break;
        case MULTICOMBO:
            result[name] = ((CMultiCombo*)e)->values;
            break;
        case INPUTBOX:
            result[name] = ((CInputBox*)e)->input;
            break;
        case LISTBOX:
            result[name] = ((CListBox*)e)->active;
            break;
        default:
            // handle any unexpected values of ElementType here
            break;
        }
    }

    result["menu_size"] = { Menu->size.x, Menu->size.y };
    //result["menu_pos"] = { Menu->menuPos.x, Menu->menuPos.y };

    return result;
}