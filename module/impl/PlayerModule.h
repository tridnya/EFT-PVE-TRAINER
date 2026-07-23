#pragma once
#include <algorithm>
#include <cctype>
#include <string>

#include "../game_state.h"
#include "../Module.h"
#include "../ModuleCategory.h"
#include "../../config/CheckboxValue.h"
#include "../../config/FloatSliderValue.h"
#include "../../config/IntSliderValue.h"
#include "../../config/SelectValue.h"
#include "../../config/ActionRowValue.h"
#include "../../il2cpp/Il2CppObjectInstance.h"
#include "../../il2cpp/unity.h"

static bool do_speed;
static int speed_value;

static bool should_survive;

using fixed_update_sig = void(*)(Il2CppObject*);
static fixed_update_sig o_player_camera_controller_fixed_update = nullptr;

inline void hk_player_camera_controller_fixed_update(Il2CppObject* instance)
{
    if (do_speed)
    {
        Il2CppObjectInstance controller_instance(instance);
        Il2CppObjectInstance main_player(controller_instance.get_method<Il2CppObject*(
            *)(Il2CppObject*)>("get_Player", 0)(instance));
        Il2CppObjectInstance character_controller(main_player.get_field<Il2CppObject*>("_characterController"));
        unity::vector3 velocity = character_controller.get_method<unity::vector3(*)(Il2CppObject*)>("get_velocity", 0)(
            character_controller.get_instance());
        velocity.x /= 100;
        velocity.y = 0;
        velocity.z /= 100;
        for (int i = 0; i < speed_value; ++i)
        {
            character_controller.get_method<int(*)(Il2CppObject*, unity::vector3, float)>("Move", 2)(
                character_controller.get_instance(), velocity, 100.0f);
        }
    }

    o_player_camera_controller_fixed_update(instance);
}

using local_game_stop_sig = void(*)(Il2CppObject*, Il2CppString*, int, Il2CppString*, float);

static local_game_stop_sig o_local_game_stop;

inline void hk_local_game_stop(Il2CppObject* instance, Il2CppString* profile_id, int exit_status,
                               Il2CppString* exit_name, float delay)
{
    if (should_survive) exit_status = 0; // Survived

    o_local_game_stop(instance, profile_id, exit_status, exit_name, delay);
}

class PlayerModule : Module
{
public:
    PlayerModule() : Module("Player", Trainer)
    {
    }

    CheckboxValue* god_mode = conf(new CheckboxValue(false, "God Mode"));
    CheckboxValue* infinite_stamina = conf(new CheckboxValue(false, "Infinite Stamina"));
    CheckboxValue* no_hunger_thirst = conf(new CheckboxValue(false, "No Hunger/Thirst"));
    CheckboxValue* instant_search = conf(new CheckboxValue(false, "Instant Search"));

    SelectValue* exp_type = conf(new SelectValue(0, {
                                                     "ExpDoorBreached", "ExpItemLooting", "ExpDamage", "ExpHeal",
                                                     "ExpExamine", "ExpKillBase", "ExpItemLooting"
                                                 }, "Exp Type"));
    IntSliderValue* exp_amount = conf(new IntSliderValue(10000, 0, 500000, "Exp Amount"));
    ActionRowValue* add_exp = conf(new ActionRowValue([&]
    {
        add_exp_impl();
    }, "Add Exp"));

    SelectValue* fence_rep_source = conf(new SelectValue(0, {
                                                             "Boss Help", "Scav Help", "Scav Kill", "Boss Kill",
                                                             "Traitor Kill", "Exit Standing"
                                                         }, "Fence Rep Source"));
    FloatSliderValue* fence_rep_amount = conf(new FloatSliderValue(0.1f, 0, 5, "Fence Rep Amount"));
    ActionRowValue* add_fence_rep = conf(new ActionRowValue([&]
    {
        add_fence_rep_impl();
    }, "Add Fence Rep"));

    SelectValue* trader_select = conf(new SelectValue(0, {
                                                          "LIGHT_KEEPER_TRADER", "RAGMAN_TRADER",
                                                          "ARENA_MANAGER_TRADER", "PEACEKEEPER", "SKIER",
                                                          "PRAPOR_TRADER", "JAEGER_TRADER", "MECHANIC_TRADER",
                                                          "TERAPEVT_TRADER", "BTR_TRADER"
                                                      }, "Trader"));
    FloatSliderValue* trader_rep_amount = conf(new FloatSliderValue(0.1f, 0, 5, "Trader Rep Amount"));
    ActionRowValue* add_trader_rep = conf(new ActionRowValue([&]
    {
        add_trader_rep_impl();
    }, "Add Trader Rep"));

    SelectValue* skill_select = conf(new SelectValue(0, {
                                                         "Pistol", "Revolver", "SMG", "Assault", "Shotgun", "Sniper",
                                                         "LMG", "HMG", "Launcher", "AttachedLauncher", "Misc", "Melee",
                                                         "DMR", "Throwing", "AimDrills", "RecoilControl",
                                                         "TroubleShooting", "Perception", "Attention", "Intellect",
                                                         "Immunity", "Charisma", "Memory", "Endurance", "Strength",
                                                         "Vitality", "Health", "Metabolism", "StressResistance",
                                                         "Sniping", "CovertMovement", "ProneMovement", "LightVests",
                                                         "HeavyVests", "WeaponModding", "AdvancedModding",
                                                         "Lockpicking", "Surgery", "Search", "WeaponTreatment",
                                                         "MagDrills", "Freetrading", "Barter", "Crafting",
                                                         "HideoutManagement", "BearAssaultoperations", "BearAuthority",
                                                         "BearAksystems", "BearHeavycaliber", "BearRawpower",
                                                         "UsecArsystems", "UsecDeepweaponmodding",
                                                         "UsecLongrangeoptics", "UsecNegotiations", "UsecTactics"
                                                     }, "Skill"));
    IntSliderValue* skill_level = conf(new IntSliderValue(51, 0, 51, "Skill Level"));
    ActionRowValue* set_skill = conf(new ActionRowValue([&]
    {
        set_skill_impl();
    }, "Set Skill"));

    CheckboxValue* speed_hack = conf(new CheckboxValue(false, "Speed Hack"));
    IntSliderValue* speed = conf(new IntSliderValue(2, 1, 10, "Speed"));

    CheckboxValue* noclip = conf(new CheckboxValue(false, "Noclip"));
    FloatSliderValue* noclip_speed = conf(new FloatSliderValue(8.0f, 1.0f, 30.0f, "Fly Speed"));
    FloatSliderValue* noclip_boost = conf(new FloatSliderValue(3.0f, 1.0f, 10.0f, "Boost Multiplier"));

    CheckboxValue* always_survive = conf(new CheckboxValue(false, "Always Survive"));

    bool queue_add_exp = false;
    bool queue_add_fence_rep = false;
    bool queue_add_trader_rep = false;
    bool queue_set_skill = false;

    using get_delta_time_sig = float(*)();
    inline static get_delta_time_sig get_delta_time = nullptr;

    bool noclip_active = false;
    bool noclip_collision_state_saved = false;
    bool noclip_original_detect_collisions = true;
    Il2CppObject* noclip_player = nullptr;
    Il2CppObject* noclip_controller = nullptr;
    unity::vector3 noclip_position{};
    bool noclip_toggle_logged = false;
    bool noclip_layout_dumped = false;
    bool noclip_has_last_written_position = false;
    int noclip_probe_frame = 0;
    int noclip_probe_frames_remaining = 0;
    unity::vector3 noclip_last_written_position{};

    static bool key_is_down(const int virtual_key)
    {
        return (GetAsyncKeyState(virtual_key) & 0x8000) != 0;
    }

    static std::string noclip_class_name(const Il2CppClass* klass)
    {
        if (!klass)
        {
            return "null";
        }

        const std::string namespace_name = klass->namespaze ? klass->namespaze : "";
        const std::string name = klass->name ? klass->name : "<unnamed>";
        return namespace_name.empty() ? name : namespace_name + "." + name;
    }

    static bool noclip_member_name_matches(const char* value)
    {
        if (!value)
        {
            return false;
        }

        std::string name = value;
        std::transform(name.begin(), name.end(), name.begin(),
            [](const unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            });

        static constexpr const char* keywords[] = {
            "character", "controller", "movement", "motor", "position",
            "transform", "gravity", "velocity", "speed", "teleport"
        };
        for (const char* keyword : keywords)
        {
            if (name.find(keyword) != std::string::npos)
            {
                return true;
            }
        }

        return false;
    }

    static void dump_noclip_class_members(
        const Il2CppClass* klass,
        const std::string& label)
    {
        size_t logged_members = 0;
        size_t depth = 0;
        for (const Il2CppClass* cursor = klass;
            cursor != nullptr && depth < 8 && logged_members < 96;
            cursor = cursor->parent, ++depth)
        {
            logger::info("[NoclipProbe] " + label + " class='" +
                noclip_class_name(cursor) +
                "' depth=" + std::to_string(depth) +
                " fields=" + std::to_string(cursor->field_count) +
                " methods=" + std::to_string(cursor->method_count));

            if (cursor->fields)
            {
                for (uint16_t index = 0;
                    index < cursor->field_count && logged_members < 96;
                    ++index)
                {
                    const FieldInfo& field = cursor->fields[index];
                    if (!noclip_member_name_matches(field.name))
                    {
                        continue;
                    }

                    logger::info("[NoclipProbeField] owner='" +
                        noclip_class_name(cursor) +
                        "' name='" + std::string(field.name ? field.name : "<null>") +
                        "' offset=" + std::to_string(field.offset) +
                        " type=" + std::to_string(
                            field.type ? static_cast<int>(field.type->type) : -1));
                    ++logged_members;
                }
            }

            if (cursor->methods)
            {
                for (uint16_t index = 0;
                    index < cursor->method_count && logged_members < 96;
                    ++index)
                {
                    const MethodInfo* method = cursor->methods[index];
                    if (!method || !noclip_member_name_matches(method->name))
                    {
                        continue;
                    }

                    logger::info("[NoclipProbeMethod] owner='" +
                        noclip_class_name(cursor) +
                        "' name='" + std::string(method->name ? method->name : "<null>") +
                        "' params=" + std::to_string(method->parameters_count));
                    ++logged_members;
                }
            }
        }
    }

    void clear_noclip_state()
    {
        noclip_active = false;
        noclip_collision_state_saved = false;
        noclip_original_detect_collisions = true;
        noclip_player = nullptr;
        noclip_controller = nullptr;
        noclip_position = {};
    }

    void disable_noclip_for_current_player()
    {
        clear_noclip_state();
    }

    void update_noclip(Il2CppObjectInstance main_player)
    {
        Il2CppObject* player = main_player.get_instance();
        if (!noclip->get_value())
        {
            if (noclip_active && noclip_player == player)
            {
                disable_noclip_for_current_player();
            }
            else if (noclip_player != player)
            {
                clear_noclip_state();
            }
            return;
        }

        if (!noclip_layout_dumped)
        {
            noclip_layout_dumped = true;
            noclip_probe_frame = 0;
            noclip_probe_frames_remaining = 20;
            noclip_has_last_written_position = false;
            logger::info("[NoclipProbe] entered gameworld path before object reads");
        }

        const FieldInfo* character_controller_field =
            player != nullptr && player->klass != nullptr ?
            il2utils::resolve_field(player->klass, "_characterController") :
            nullptr;
        Il2CppObject* character_controller =
            character_controller_field != nullptr && character_controller_field->offset > 0 ?
            *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<uintptr_t>(player) + character_controller_field->offset) :
            nullptr;
        Il2CppObject* player_game_object = unity::component_get_game_object(player);
        Il2CppObject* player_transform = player_game_object ?
            unity::gameobject_get_transform(player_game_object) :
            nullptr;

        if (!player_transform)
        {
            logger::warn("[NoclipProbe] player transform is unavailable; no position write performed");
            return;
        }

        if (!noclip_active || noclip_player != player)
        {
            clear_noclip_state();
            noclip_active = true;
            noclip_player = player;
            noclip_controller = character_controller;
            noclip_position = unity::transform_get_pos(player_transform);
            logger::info("[NoclipProbe] activation player='" +
                noclip_class_name(player != nullptr ? player->klass : nullptr) +
                "' controller_field=" +
                std::to_string(character_controller_field != nullptr ? 1 : 0) +
                " controller='" +
                noclip_class_name(
                    character_controller != nullptr ? character_controller->klass : nullptr) +
                "' game_object=" + std::to_string(player_game_object != nullptr ? 1 : 0) +
                " transform=" + std::to_string(player_transform != nullptr ? 1 : 0) +
                " delta_time_method=" + std::to_string(get_delta_time != nullptr ? 1 : 0));
            if (character_controller)
            {
                logger::info("[NoclipProbe] activation controller='" +
                    noclip_class_name(character_controller->klass) +
                    "'; Unity CharacterController collision API skipped");
            }
            else
            {
                logger::warn("[NoclipProbe] _characterController resolved but its live value is null");
            }
        }
        else if (character_controller)
        {
            noclip_controller = character_controller;
        }

        const float yaw = unity::transform_get_rotation(player_transform).to_euler().y * PI / 180.0f;
        const unity::vector3 forward{ sinf(yaw), 0.0f, cosf(yaw) };
        const unity::vector3 right{ cosf(yaw), 0.0f, -sinf(yaw) };
        unity::vector3 direction{};

        if (key_is_down('W')) direction = direction.add(forward);
        if (key_is_down('S')) direction = direction.add({ -forward.x, 0.0f, -forward.z });
        if (key_is_down('D')) direction = direction.add(right);
        if (key_is_down('A')) direction = direction.add({ -right.x, 0.0f, -right.z });
        if (key_is_down(VK_SPACE)) direction.y += 1.0f;
        if (key_is_down(VK_CONTROL)) direction.y -= 1.0f;

        const unity::vector3 observed_before_write =
            unity::transform_get_pos(player_transform);
        const float magnitude = sqrtf(
            direction.x * direction.x +
            direction.y * direction.y +
            direction.z * direction.z);
        if (magnitude > 0.0f)
        {
            direction.x /= magnitude;
            direction.y /= magnitude;
            direction.z /= magnitude;
            float movement_speed = noclip_speed->get_value();
            if (key_is_down(VK_SHIFT))
            {
                movement_speed *= noclip_boost->get_value();
            }

            const float delta_time = get_delta_time ? get_delta_time() : (1.0f / 60.0f);
            noclip_position.x += direction.x * movement_speed * delta_time;
            noclip_position.y += direction.y * movement_speed * delta_time;
            noclip_position.z += direction.z * movement_speed * delta_time;
        }

        unity::transform_set_pos(player_transform, noclip_position);
        const unity::vector3 observed_after_write =
            unity::transform_get_pos(player_transform);

        if (noclip_probe_frames_remaining > 0)
        {
            const unity::vector3 overwrite_delta = noclip_has_last_written_position ?
                unity::vector3{
                    observed_before_write.x - noclip_last_written_position.x,
                    observed_before_write.y - noclip_last_written_position.y,
                    observed_before_write.z - noclip_last_written_position.z
                } :
                unity::vector3{};
            logger::info("[NoclipProbe] frame=" + std::to_string(noclip_probe_frame) +
                " keys=" +
                    std::to_string(key_is_down('W') ? 1 : 0) +
                    std::to_string(key_is_down('A') ? 1 : 0) +
                    std::to_string(key_is_down('S') ? 1 : 0) +
                    std::to_string(key_is_down('D') ? 1 : 0) +
                " up=" + std::to_string(key_is_down(VK_SPACE) ? 1 : 0) +
                " down=" + std::to_string(key_is_down(VK_CONTROL) ? 1 : 0) +
                " before=(" + std::to_string(observed_before_write.x) + "," +
                    std::to_string(observed_before_write.y) + "," +
                    std::to_string(observed_before_write.z) + ")" +
                " target=(" + std::to_string(noclip_position.x) + "," +
                    std::to_string(noclip_position.y) + "," +
                    std::to_string(noclip_position.z) + ")" +
                " after=(" + std::to_string(observed_after_write.x) + "," +
                    std::to_string(observed_after_write.y) + "," +
                    std::to_string(observed_after_write.z) + ")" +
                " transform_set=1" +
                " next_frame_overwrite_delta=(" +
                    std::to_string(overwrite_delta.x) + "," +
                    std::to_string(overwrite_delta.y) + "," +
                    std::to_string(overwrite_delta.z) + ")" +
                " controller='" +
                    noclip_class_name(
                        character_controller != nullptr ? character_controller->klass : nullptr) +
                    "'");
            noclip_last_written_position = noclip_position;
            noclip_has_last_written_position = true;
            ++noclip_probe_frame;
            --noclip_probe_frames_remaining;
        }
    }

    void add_exp_impl()
    {
        if (!game_state::is_in_raid) return;
        queue_add_exp = true;
    }

    void add_fence_rep_impl()
    {
        if (!game_state::is_in_raid) return;
        queue_add_fence_rep = true;
    }

    void add_trader_rep_impl()
    {
        if (!game_state::is_in_raid) return;
        queue_add_trader_rep = true;
    }

    void set_skill_impl()
    {
        if (!game_state::is_in_raid) return;
        queue_set_skill = true;
    }

    void draw_overlay(ImDrawList* draw_list) override
    {
    }

    void application_update() override
    {
        if (noclip->get_value())
        {
            if (!noclip_toggle_logged)
            {
                noclip_toggle_logged = true;
                logger::info("[NoclipProbe] toggle=1 in_raid=" +
                    std::to_string(game_state::is_in_raid ? 1 : 0));
            }
        }
        else
        {
            noclip_toggle_logged = false;
            noclip_layout_dumped = false;
            noclip_probe_frame = 0;
            noclip_probe_frames_remaining = 0;
            noclip_has_last_written_position = false;
        }

        if (!game_state::is_in_raid)
        {
            clear_noclip_state();
        }
    }

    void reset_gameworld_state() override
    {
        clear_noclip_state();
        noclip_layout_dumped = false;
        noclip_probe_frame = 0;
        noclip_probe_frames_remaining = 0;
        noclip_has_last_written_position = false;
        noclip_last_written_position = {};
    }

    void gameworld_update(const Il2CppClass* game_world_class, Il2CppObjectInstance game_world_instance,
                          Il2CppObjectInstance main_player) override
    {
        const auto get_profile = main_player.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_Profile", 0);
        Il2CppObjectInstance main_profile(get_profile(main_player.get_instance()));

        update_noclip(main_player);

        if (infinite_stamina->get_value())
        {
            Il2CppObjectInstance physical(main_player.get_field<Il2CppObject*>("Physical"));

            Il2CppObjectInstance stamina(physical.get_field<Il2CppObject*>("Stamina"));
            stamina.set_field("Current", 100.0f);

            Il2CppObjectInstance stamina_hands(physical.get_field<Il2CppObject*>("HandsStamina"));
            stamina_hands.set_field("Current", 100.0f);

            Il2CppObjectInstance oxygen(physical.get_field<Il2CppObject*>("Oxygen"));
            oxygen.set_field("Current", 100.0f);
        }

        Il2CppObjectInstance health_controller(
            main_player.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_ActiveHealthController", 0)(
                main_player.get_instance()));
        static bool god_was_on = false;
        static float saved_fall_height = 0.f;
        static bool fall_saved = false;

        if (god_mode->get_value())
        {
            // cache the real fall-safe height once so we can restore it later
            if (!fall_saved)
            {
                saved_fall_height = health_controller.get_field<float>("_fallSafeHeight");
                fall_saved = true;
            }

            health_controller.get_method<void(*)(Il2CppObject*, float)>("SetDamageCoeff", 1)(
                health_controller.get_instance(), -1.0f);
            health_controller.get_method<void(*)(Il2CppObject*, int)>("RemoveNegativeEffects", 1)(
                health_controller.get_instance(), 7); // 7 = Common
            health_controller.get_method<void(*)(Il2CppObject*)>("RestoreFullHealth", 0)(
                health_controller.get_instance());
            health_controller.set_field("_fallSafeHeight", 99999999.0f);
            god_was_on = true;
        }
        else if (god_was_on)
        {
            // god mode turned off -> restore normal damage + fall height (once)
            health_controller.get_method<void(*)(Il2CppObject*, float)>("SetDamageCoeff", 1)(
                health_controller.get_instance(), 1.0f);
            if (fall_saved)
                health_controller.set_field("_fallSafeHeight", saved_fall_height);
            god_was_on = false;
        }

        if (no_hunger_thirst->get_value())
        {
            health_controller.get_method<void(*)(Il2CppObject*, float)>("ChangeEnergy", 1)(
                health_controller.get_instance(), 100.0f);
            health_controller.get_method<void(*)(Il2CppObject*, float)>("ChangeHydration", 1)(
                health_controller.get_instance(), 100.0f);
        }

        Il2CppObjectInstance skill_manager(
            main_player.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_Skills", 0)(main_player.get_instance()));
        if (instant_search->get_value())
        {
            Il2CppObjectInstance attention_elite_lucky_search(
                skill_manager.get_field<Il2CppObject*>("AttentionEliteLuckySearch"));

            attention_elite_lucky_search.set_field("Value", 100000.0f);
        }

        if (queue_add_exp)
        {
            queue_add_exp = false;
            Il2CppObjectInstance eft_stats(
                main_profile.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_EftStats", 0)(
                    main_profile.get_instance()));

            Il2CppObjectInstance session_counters(eft_stats.get_field<Il2CppObject*>("SessionCounters"));

            const Il2CppImage* assembly_csharp = il2utils::resolve_image("Assembly-CSharp.dll");
            const Il2CppClass* predefined_counters = il2utils::resolve_class(assembly_csharp, "EFT.Counters",
                                                                             "PredefinedCounters");
            const auto reason = il2utils::get_static_field<Il2CppObject
                *>(predefined_counters, exp_type->get_selected().c_str());

            session_counters.get_method<void(*)(Il2CppObject*, int, Il2CppObject*)>("AddInt", 2)(
                session_counters.get_instance(), exp_amount->get_value(), reason);

            if (globals::verbose) logger::info("Added EXP");
        }

        if (queue_add_fence_rep)
        {
            queue_add_fence_rep = false;
            Il2CppObjectInstance loyalty_data(
                main_player.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_Loyalty", 0)(
                    main_player.get_instance()));

            loyalty_data.get_method<void(*)(Il2CppObject*, double, int)>("KillAsSavage", 2)(
                loyalty_data.get_instance(), fence_rep_amount->get_value(), fence_rep_source->get_value());

            if (globals::verbose) logger::info("Added Fence Rep");
        }

        if (queue_add_trader_rep)
        {
            queue_add_trader_rep = false;

            // The *_ID constants on Profile.TraderInfo are C# `const string` literals, so they
            // have NO static storage slot (offset 0x00 in metadata). Reading them via
            // il2cpp_field_static_get_value returns garbage and makes TryGetTraderInfo fail or
            // crash. Since the values are compile-time constants, we hardcode them from the dump
            // and build the Il2CppString directly with il2cpp_string_new.
            static const std::unordered_map<std::string, const char*> trader_ids = {
                { "LIGHT_KEEPER_TRADER",  "638f541a29ffd1183d187f57" },
                { "RAGMAN_TRADER",        "5ac3b934156ae10c4430e83c" },
                { "ARENA_MANAGER_TRADER", "6617beeaa9cfa777ca915b7c" },
                { "PEACEKEEPER",          "5935c25fb3acc3127c3d8cd9" },
                { "SKIER",                "58330581ace78e27b8b10cee" },
                { "PRAPOR_TRADER",        "54cb50c76803fa8b248b4571" },
                { "JAEGER_TRADER",        "5c0647fdd443bc2504c2d371" },
                { "MECHANIC_TRADER",      "5a7c2eca46aef81a7ca2145d" },
                { "TERAPEVT_TRADER",      "54cb57776803fa99248b456e" },
                { "BTR_TRADER",           "656f0f98d80a697f855d34b1" },
            };

            const std::string trader_key = trader_select->get_selected();
            const auto id_it = trader_ids.find(trader_key);

            const Il2CppClass* profile_class = il2utils::resolve_class(game_world_class->image, "EFT", "Profile");

            if (id_it == trader_ids.end())
            {
                logger::error("Unknown trader selection: " + trader_key);
                if (globals::verbose) logger::info("Trader Rep Failed: no ID mapping");
            }
            else if (!profile_class)
            {
                logger::error("Failed to resolve Profile class");
                if (globals::verbose) logger::info("Trader Rep Failed: Profile class not found");
            }
            else
            {
                // Resolve methods manually first: il2utils::get_method dereferences
                // method->methodPointer without a null-check, so a missing method would crash.
                const MethodInfo* try_get_method =
                    il2utils::resolve_method(profile_class, "TryGetTraderInfo", 2);

                if (!try_get_method || !try_get_method->methodPointer)
                {
                    logger::error("Failed to resolve TryGetTraderInfo method");
                    if (globals::verbose) logger::info("Trader Rep Failed: TryGetTraderInfo method not found");
                }
                else
                {
                    // ---- Step-by-step diagnostic logging (always on, flushed) ----
                    logger::info("[TR] step 1: building string for " + trader_key + " id=" + id_it->second);
                    Il2CppString* trader_id = il2cpp::il2cpp_string_new(id_it->second);
                    logger::info("[TR] step 2: string ok ptr=" +
                                 std::to_string(reinterpret_cast<uintptr_t>(trader_id)));

                    const auto try_get_trader_info =
                        static_cast<bool(*)(Il2CppObject*, Il2CppString*, Il2CppObject**)>(
                            try_get_method->methodPointer);

                    logger::info("[TR] step 3: calling TryGetTraderInfo, profile=" +
                                 std::to_string(reinterpret_cast<uintptr_t>(main_profile.get_instance())));
                    Il2CppObject* trader_info = nullptr;
                    const bool got = try_get_trader_info(main_profile.get_instance(), trader_id, &trader_info);
                    logger::info("[TR] step 4: TryGetTraderInfo returned " + std::to_string(got) +
                                 " info=" + std::to_string(reinterpret_cast<uintptr_t>(trader_info)));

                    if (got && trader_info)
                    {
                        // Read current standing by reading the backing field directly (no method
                        // call side-effects), then write it back the same way. This avoids the
                        // virtual SetStanding -> UpdateLevel/event path which can touch UI/Settings
                        // state that may be absent in-raid (a likely crash source).
                        logger::info("[TR] step 5: resolving <Standing>k__BackingField on klass=" +
                                     std::string(trader_info->klass && trader_info->klass->name
                                                     ? trader_info->klass->name : "?"));
                        const FieldInfo* standing_field =
                            il2utils::resolve_field(trader_info->klass, "<Standing>k__BackingField");

                        if (!standing_field)
                        {
                            logger::error("[TR] FAILED: <Standing>k__BackingField not found");
                        }
                        else
                        {
                            logger::info("[TR] step 6: field offset=0x" +
                                         [&]{ char b[16]; snprintf(b, sizeof(b), "%X", standing_field->offset); return std::string(b); }());

                            auto* standing_ptr = reinterpret_cast<double*>(
                                reinterpret_cast<UINT64>(trader_info) + standing_field->offset);

                            const double current_standing = *standing_ptr;
                            const double new_standing = current_standing + trader_rep_amount->get_value();
                            *standing_ptr = new_standing;

                            logger::info("[TR] DONE [" + trader_key + "]: " +
                                         std::to_string(current_standing) + " -> " +
                                         std::to_string(new_standing));
                        }
                    }
                    else
                    {
                        logger::warn("[TR] FAILED: TryGetTraderInfo false/null for " + trader_key +
                                     " (trader probablement pas charge sur le profil in-raid)");
                    }
                }
            }
        }

        if (queue_set_skill)
        {
            queue_set_skill = false;

            Il2CppObjectInstance skill_controller(
                main_player.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_Skills", 0)(main_player.get_instance()));

            if (const auto skill_ptr = skill_controller.get_field<Il2CppObject*>(skill_select->get_selected().c_str()))
            {
                Il2CppObjectInstance skill_instance(skill_ptr);

                skill_instance.get_method<void(*)(Il2CppObject*, int)>("SetLevel", 1)(
                    skill_instance.get_instance(), skill_level->get_value());

                if (globals::verbose)
                    logger::info(
                        "Set Skill: " + skill_select->get_selected() + " Level: " +
                        std::to_string(skill_level->get_value()));
            }
            else
            {
                if (globals::verbose) logger::warn("Skill not found: " + skill_select->get_selected());
            }
        }

        do_speed = speed_hack->get_value() && !noclip->get_value();
        speed_value = speed->get_value();

        should_survive = always_survive->get_value();
    }

    void init() override
    {
        const Il2CppImage* image = il2utils::resolve_image("Assembly-CSharp.dll");
        const Il2CppImage* unity_engine = il2utils::resolve_image("UnityEngine.CoreModule.dll");
        const Il2CppClass* time_class = il2utils::resolve_class(unity_engine, "UnityEngine", "Time");
        const MethodInfo* delta_time_method = time_class ?
            il2utils::resolve_method(time_class, "get_deltaTime", 0) :
            nullptr;
        if (delta_time_method && delta_time_method->methodPointer)
        {
            get_delta_time = reinterpret_cast<get_delta_time_sig>(delta_time_method->methodPointer);
        }

        // need a fixed update here so speed hack is consistent, this is a good one i found with access to player
        const Il2CppClass* player_camera_controller_class = il2utils::resolve_class(
            image, "EFT.CameraControl", "PlayerCameraController");
        il2utils::hook_method(player_camera_controller_class, "FixedUpdate", 0,
                              hk_player_camera_controller_fixed_update,
                              &o_player_camera_controller_fixed_update);

        // local game stop
        const Il2CppClass* local_game = il2utils::resolve_class(image, "EFT", "LocalGame");
        il2utils::hook_method(local_game, "Stop", 4, hk_local_game_stop, &o_local_game_stop);
    }
};
