#pragma once
#include <Windows.h> // [NEW] Required for Sleep()
#include "../assets/weapon_finish_assets.h"
#include "ModuleManager.h"
#include "always_on.h"
#include "game_state.h"
#include "../runtime/mcp/runtime_bridge.h"

namespace common_hooks
{
    static Il2CppImage* assembly_csharp;
    static const Il2CppClass* game_world_class;

    using update_sig = void(*)(Il2CppObject*);
    static update_sig o_tarkov_application_update = nullptr;
    static update_sig o_game_world_update = nullptr;

    static const Il2CppClass* is_in_raid;
    static Il2CppObject* weapon_finish_load_object = nullptr;
    static bool weapon_finish_loaded = false;
    static bool previous_in_raid = false;
    static Il2CppObject* active_game_world = nullptr;
    static Il2CppObject* active_main_player = nullptr;

    static void reset_scene_owned_module_state()
    {
        ModuleManager::get_instance()->apply([](Module* module)
            {
                module->reset_gameworld_state();
            });
    }

    // [TEMP TRACE] file-local only, no cross-file/shared state. Dense for the first 20
    // calls since hook install, then a sparse ~1/sec heartbeat (every 60th call) so the
    // window leading up to and including the freeze is still covered without flooding.
    //static int trace_call_count = 0;

    static void hk_tarkov_application_update(Il2CppObject* instance)
    {
        //++trace_call_count;
       // const bool trace_active = trace_call_count <= 20 || (trace_call_count % 60) == 0;
       // if (trace_active)
       // {
       //     //logger::info("[TRACE] APPUPDATE_ENTER build=CONTROL seq=" + std::to_string(trace_call_count) +
            //    " tid=" + std::to_string(GetCurrentThreadId()));
       // }

        if (!weapon_finish_load_object)
        {
            logger::info("Loading weapon finish assets");
            weapon_finish_load_object = weapon_finish_assets::start_load();
        }
        else if (!weapon_finish_loaded)
        {
            weapon_finish_loaded = weapon_finish_assets::check_progress(weapon_finish_load_object);
        }

        // set ingame status and release scene-owned state after a raid ends
        const bool current_in_raid = il2utils::get_static_field<bool>(is_in_raid, "InRaid");
        if (previous_in_raid && !current_in_raid)
        {
            reset_scene_owned_module_state();
            always_on::reset_raid_runtime_state();
            active_game_world = nullptr;
            active_main_player = nullptr;
            logger::info("[Lifecycle] reset scene-owned module state after raid");
        }

        game_state::is_in_raid = current_in_raid;
        previous_in_raid = current_in_raid;

        runtime::mcp::pump();

        ModuleManager::get_instance()->apply([](Module* m)
            {
                m->application_update();
            });

       // if (trace_active)
       // {
       //     logger::info("[TRACE] APPUPDATE_BEFORE_ORIGINAL build=CONTROL seq=" + std::to_string(trace_call_count) +
       //       " tid=" + std::to_string(GetCurrentThreadId()));
       // }

        o_tarkov_application_update(instance);

       // if (trace_active)
      ////  {
       //     logger::info("[TRACE] APPUPDATE_ORIGINAL_RETURNED build=CONTROL seq=" + std::to_string(trace_call_count) +
       //         " tid=" + std::to_string(GetCurrentThreadId()));
        //    logger::info("[TRACE] APPUPDATE_EXIT build=CONTROL seq=" + std::to_string(trace_call_count) +
       //         " tid=" + std::to_string(GetCurrentThreadId()));
        // }
    }

    static void hk_game_world_update(Il2CppObject* instance)
    {
        o_game_world_update(instance);

        // no modules outside of raid
        if (!game_state::is_in_raid) return;

        Il2CppObjectInstance game_world_instance(instance);
        const auto main_player_object = game_world_instance.get_field<Il2CppObject*>("MainPlayer");

        // if player is not loaded yet
        if (main_player_object == nullptr) return;

        if (active_game_world != instance || active_main_player != main_player_object)
        {
            active_game_world = instance;
            active_main_player = main_player_object;
            reset_scene_owned_module_state();
            logger::info("[Lifecycle] reset scene-owned module state for new GameWorld");
            return;
        }

        Il2CppObjectInstance main_player(main_player_object);

        ModuleManager::get_instance()->apply([game_world_instance, main_player](Module* m)
            {
                m->gameworld_update(game_world_class, game_world_instance, main_player);
            });

        always_on::dump_icebreaker_live_container_state();
    }

    inline void init()
    {
        // [MODIFIED] Wait for the game to load Assembly-CSharp.dll before we try to hook things inside it
        while (!il2utils::resolve_image("Assembly-CSharp.dll"))
        {
            Sleep(50);
        }
        assembly_csharp = il2utils::resolve_image("Assembly-CSharp.dll");

        // ingame status class
        is_in_raid = il2utils::resolve_class(assembly_csharp, "EFT", "InGameStatus");

        // hook game world update
        game_world_class = il2utils::resolve_class(assembly_csharp, "EFT", "GameWorld");
        il2utils::hook_method(game_world_class, "Update", 0, hk_game_world_update,
            &o_game_world_update);

        // hook tarkov application update
        const Il2CppClass* tarkov_application_class = il2utils::resolve_class(assembly_csharp, "EFT", "TarkovApplication");
        il2utils::hook_method(tarkov_application_class, "Update", 0, hk_tarkov_application_update,
            &o_tarkov_application_update);
    }
}
