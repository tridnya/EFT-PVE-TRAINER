#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include <Windows.h>

#include "../game_state.h"
#include "../Module.h"
#include "../ModuleCategory.h"
#include "../../config/ActionRowValue.h"
#include "../../config/CheckboxValue.h"
#include "../../config/ColorpickerValue.h"
#include "../../config/FloatSliderValue.h"
#include "../../config/IntSliderValue.h"
#include "../../gui/media/media_widget.h"
#include "../../gui/menu/imgui_addons.h"
#include "../../il2cpp/il2utils.h"
#include "../../il2cpp/Il2CppObjectInstance.h"
#include "../../il2cpp/unity.h"
#include "../../util/globals.h"
#include "../../util/logger.h"

namespace world_visualizer_detail
{
    struct ExfilPoint
    {
        std::string name{};
        int status = 0;
        unity::vector3 position{};
    };

    struct TransitPoint
    {
        std::string name{};
        unity::vector3 position{};
    };

    struct WorldMarker
    {
        std::string name{};
        unity::vector3 position{};
        bool is_crate = false;
        bool is_quest_item = false;
    };

    struct CachedCrate
    {
        Il2CppObject* object = nullptr;
        std::string name{};
    };

    inline std::vector<CachedCrate> cached_crates{};
    inline ULONGLONG cached_crates_last_refresh = 0;

    [[nodiscard]] inline std::string wide_to_utf8(const std::wstring& text)
    {
        if (text.empty())
        {
            return {};
        }

        const int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size <= 0)
        {
            return {};
        }

        std::string utf8(static_cast<std::size_t>(size), '\0');
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, utf8.data(), size, nullptr, nullptr);
        utf8.pop_back();
        return utf8;
    }

    [[nodiscard]] inline unity::vector3 world_to_screen(Il2CppObject* camera, unity::vector3 world_point)
    {
        unity::vector3 screen_point = unity::world_to_screen_point(camera, world_point);
        const float camera_height = static_cast<float>((std::max)(1, unity::camera_pixel_height(camera)));
        const float scale = menu::get_height() / camera_height;
        screen_point.y = menu::get_height() - (screen_point.y * scale);
        screen_point.x *= scale;
        return screen_point;
    }

    [[nodiscard]] inline bool is_visible(const unity::vector3& screen_point)
    {
        return screen_point.z > 0.01f &&
            screen_point.x > -100.0f &&
            screen_point.y > -100.0f &&
            screen_point.x < menu::get_width() + 100.0f &&
            screen_point.y < menu::get_height() + 100.0f;
    }

    [[nodiscard]] inline const char* exfil_status_text(int status)
    {
        switch (status)
        {
        case 8: return "hidden";
        case 7: return "inactive";
        case 6: return "pending";
        case 5:
        case 4: return "open";
        case 2: return "available";
        case 1: return "closed";
        default: return "unknown";
        }
    }

    [[nodiscard]] inline float distance_sq(const unity::vector3& a, const unity::vector3& b)
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        const float dz = a.z - b.z;
        return dx * dx + dy * dy + dz * dz;
    }

    [[nodiscard]] inline ImVec4 color_from_u32(ImU32 color, float alpha = 1.0f)
    {
        ImVec4 value = ImGui::ColorConvertU32ToFloat4(color);
        value.w = alpha;
        return value;
    }

    [[nodiscard]] inline ImVec4 blend_color(const ImVec4& start, const ImVec4& end, float amount)
    {
        return {
            start.x + ((end.x - start.x) * amount),
            start.y + ((end.y - start.y) * amount),
            start.z + ((end.z - start.z) * amount),
            start.w + ((end.w - start.w) * amount)
        };
    }

    [[nodiscard]] inline bool class_name_looks_like_crate(const char* class_name)
    {
        if (!class_name || !class_name[0])
        {
            return false;
        }

        return std::strstr(class_name, "LootableContainer") != nullptr ||
            std::strstr(class_name, "Container") != nullptr ||
            std::strstr(class_name, "Corpse") != nullptr;
    }

    [[nodiscard]] inline std::string localized_short_name(Il2CppObject* item_object, const char* fallback)
    {
        if (!item_object)
        {
            return fallback;
        }

        static auto* assembly_csharp = il2utils::resolve_image("Assembly-CSharp.dll");
        static auto* localization_extensions_class =
            il2utils::resolve_class(assembly_csharp, "EFT", "LocalizationExtensions");
        static auto localized_short_name_method =
            il2utils::get_method<Il2CppString*(*)(Il2CppObject*)>(
                localization_extensions_class,
                "LocalizedShortName",
                1
            );

        if (!localized_short_name_method)
        {
            return fallback;
        }

        Il2CppString* name_string = localized_short_name_method(item_object);
        if (!name_string)
        {
            return fallback;
        }

        std::string name = wide_to_utf8(il2utils::conv_wstring(name_string));
        return name.empty() ? std::string(fallback) : name;
    }

    inline void draw_marker_text(
        ImDrawList* draw_list,
        const unity::vector3& screen_position,
        const ImVec4& color,
        int font_size,
        const std::string& text)
    {
        const float scaled_font_size = static_cast<float>(font_size) * menu::get_scale_factor();
        const ImVec2 center{ screen_position.x, screen_position.y };
        const float outline_offset = (std::max)(1.0f, menu::get_scale_factor());
        const ImVec4 outline_color{ 0.0f, 0.0f, 0.0f, 0.88f };
        const std::array<ImVec2, 8> outline_offsets{
            ImVec2{ -1.0f, -1.0f },
            ImVec2{ 0.0f, -1.0f },
            ImVec2{ 1.0f, -1.0f },
            ImVec2{ -1.0f, 0.0f },
            ImVec2{ 1.0f, 0.0f },
            ImVec2{ -1.0f, 1.0f },
            ImVec2{ 0.0f, 1.0f },
            ImVec2{ 1.0f, 1.0f }
        };

        for (const ImVec2& offset : outline_offsets)
        {
            imgui_addons::centered_text(
                draw_list,
                menu::get_hud_font(),
                scaled_font_size,
                {
                    center.x + (offset.x * outline_offset),
                    center.y + (offset.y * outline_offset)
                },
                outline_color,
                text);
        }

        imgui_addons::centered_text(
            draw_list,
            menu::get_hud_font(),
            scaled_font_size,
            center,
            color,
            text
        );
    }
}

class WorldVisualizerModule : Module
{
public:
    WorldVisualizerModule() : Module("World", World)
    {
    }

    ActionRowValue* unlock_doors = conf(new ActionRowValue([&]
    {
        unlock_doors_impl();
    }, "Unlock All Doors"));
    CheckboxValue* spotify_mode = conf(new CheckboxValue(false, "Spotify Mode"));

    CheckboxValue* show_extractions = conf(new CheckboxValue(false, "Extractions"));
    ColorpickerValue* extraction_color = conf(new ColorpickerValue({ 0.60f, 0.72f, 1.0f, 1.0f }, "Extraction Color"));
    CheckboxValue* show_transits = conf(new CheckboxValue(false, "Transits"));
    ColorpickerValue* transit_color = conf(new ColorpickerValue({ 0.96f, 0.45f, 0.69f, 1.0f }, "Transit Color"));

    CheckboxValue* show_items = conf(new CheckboxValue(false, "Items"));
    ColorpickerValue* item_color = conf(new ColorpickerValue({ 0.72f, 0.86f, 1.0f, 1.0f }, "Item Color"));
    FloatSliderValue* item_distance = conf(new FloatSliderValue(65.0f, 1.0f, 500.0f, "Item Distance"));
    FloatSliderValue* item_min_distance = conf(new FloatSliderValue(0.0f, 0.0f, 25.0f, "Item Min Distance"));

    CheckboxValue* show_crates = conf(new CheckboxValue(false, "Crates"));
    ColorpickerValue* crate_color = conf(new ColorpickerValue({ 1.0f, 0.68f, 0.34f, 1.0f }, "Crate Color"));
    FloatSliderValue* crate_distance = conf(new FloatSliderValue(120.0f, 1.0f, 500.0f, "Crate Distance"));

    CheckboxValue* show_quest_items = conf(new CheckboxValue(false, "Quest Items"));
    ColorpickerValue* quest_item_color = conf(new ColorpickerValue({ 0.74f, 0.52f, 1.0f, 1.0f }, "Quest Item Color"));
    FloatSliderValue* quest_item_distance = conf(new FloatSliderValue(300.0f, 1.0f, 500.0f, "Quest Item Distance"));

    IntSliderValue* font_size = conf(new IntSliderValue(12, 8, 30, "Marker Size"));

    unity::vector3 local_position{};
    std::vector<world_visualizer_detail::ExfilPoint> extraction_points{};
    std::vector<world_visualizer_detail::TransitPoint> transit_points{};
    std::vector<world_visualizer_detail::WorldMarker> world_markers{};
    bool queue_unlock_doors = false;

    void unlock_doors_impl()
    {
        if (!game_state::is_in_raid)
        {
            return;
        }

        queue_unlock_doors = true;
    }

    void draw_overlay(ImDrawList* draw_list) override
    {
        if (!game_state::is_in_raid)
        {
            return;
        }

        Il2CppObject* camera = unity::get_current_camera();
        if (!camera)
        {
            return;
        }

        const bool use_spotify_colors = spotify_mode->get_value();
        const ImVec4 media_primary = world_visualizer_detail::color_from_u32(media_widget::get_accent_color());
        const ImVec4 media_secondary = world_visualizer_detail::color_from_u32(media_widget::get_secondary_color());
        const ImVec4 media_blend = world_visualizer_detail::blend_color(media_primary, media_secondary, 0.55f);
        const ImVec4 resolved_extraction_color = use_spotify_colors ? media_primary : extraction_color->get_value();
        const ImVec4 resolved_transit_color = use_spotify_colors ? media_secondary : transit_color->get_value();
        const ImVec4 resolved_item_color = use_spotify_colors ? media_primary : item_color->get_value();
        const ImVec4 resolved_crate_color = use_spotify_colors ? media_blend : crate_color->get_value();
        const ImVec4 resolved_quest_item_color = use_spotify_colors ? media_secondary : quest_item_color->get_value();

        if (show_extractions->get_value())
        {
            for (const world_visualizer_detail::ExfilPoint& point : extraction_points)
            {
                const unity::vector3 screen_position = world_visualizer_detail::world_to_screen(camera, point.position);
                if (!world_visualizer_detail::is_visible(screen_position))
                {
                    continue;
                }

                const int distance = static_cast<int>(local_position.distance(point.position));
                std::string text = std::string("extract [") + std::to_string(distance) + "m] " +
                    point.name + " (" + world_visualizer_detail::exfil_status_text(point.status) + ")";
                world_visualizer_detail::draw_marker_text(
                    draw_list,
                    screen_position,
                    resolved_extraction_color,
                    font_size->get_value(),
                    text
                );
            }
        }

        if (show_transits->get_value())
        {
            for (const world_visualizer_detail::TransitPoint& point : transit_points)
            {
                const unity::vector3 screen_position = world_visualizer_detail::world_to_screen(camera, point.position);
                if (!world_visualizer_detail::is_visible(screen_position))
                {
                    continue;
                }

                const int distance = static_cast<int>(local_position.distance(point.position));
                std::string text = std::string("transit [") + std::to_string(distance) + "m] " + point.name;
                world_visualizer_detail::draw_marker_text(
                    draw_list,
                    screen_position,
                    resolved_transit_color,
                    font_size->get_value(),
                    text
                );
            }
        }

        const bool items_enabled = show_items->get_value();
        const bool crates_enabled = show_crates->get_value();
        const bool quest_items_enabled = show_quest_items->get_value();
        const float item_max = item_distance->get_value();
        const float crate_max = crate_distance->get_value();
        const float quest_max = quest_item_distance->get_value();

        if (!items_enabled && !crates_enabled && !quest_items_enabled)
        {
            return;
        }

        for (const world_visualizer_detail::WorldMarker& marker : world_markers)
        {
            const float dist_sq = world_visualizer_detail::distance_sq(local_position, marker.position);
            const bool show_item_marker = items_enabled && !marker.is_crate && !marker.is_quest_item &&
                dist_sq <= item_max * item_max;
            const bool show_crate_marker = crates_enabled && marker.is_crate && dist_sq <= crate_max * crate_max;
            const bool show_quest_marker = quest_items_enabled && marker.is_quest_item && dist_sq <= quest_max * quest_max;

            if (!show_item_marker && !show_crate_marker && !show_quest_marker)
            {
                continue;
            }

            const unity::vector3 screen_position = world_visualizer_detail::world_to_screen(camera, marker.position);
            if (!world_visualizer_detail::is_visible(screen_position))
            {
                continue;
            }

            const int distance = static_cast<int>(std::sqrt(dist_sq));
            const char* prefix = show_quest_marker ? "quest" : (marker.is_crate ? "crate" : "item");
            const ImVec4 color = show_quest_marker
                ? resolved_quest_item_color
                : (marker.is_crate ? resolved_crate_color : resolved_item_color);
            std::string text = std::string(prefix) + " [" + std::to_string(distance) + "m] " + marker.name;
            world_visualizer_detail::draw_marker_text(
                draw_list,
                screen_position,
                color,
                font_size->get_value(),
                text
            );
        }
    }

    void application_update() override
    {
    }

    void reset_gameworld_state() override
    {
        local_position = {};
        extraction_points.clear();
        transit_points.clear();
        world_markers.clear();
        world_visualizer_detail::cached_crates.clear();
        world_visualizer_detail::cached_crates_last_refresh = 0;
    }

    void gameworld_update(const Il2CppClass* game_world_class, Il2CppObjectInstance game_world_instance,
                          Il2CppObjectInstance main_player) override
    {
        if (queue_unlock_doors)
        {
            queue_unlock_doors = false;
            unlock_doors_in_world(game_world_class);
        }

        Il2CppObject* main_player_game_object = unity::component_get_game_object(main_player.get_instance());
        Il2CppObject* main_player_transform = main_player_game_object ? unity::gameobject_get_transform(main_player_game_object) : nullptr;
        if (!main_player_transform)
        {
            return;
        }

        local_position = unity::transform_get_pos(main_player_transform);

        if (show_extractions->get_value())
        {
            refresh_extractions(game_world_instance, main_player);
        }

        if (show_transits->get_value())
        {
            refresh_transits(game_world_instance);
        }

        if (show_items->get_value() || show_crates->get_value() || show_quest_items->get_value())
        {
            refresh_world_markers(game_world_instance);
        }
        else
        {
            world_markers.clear();
            world_visualizer_detail::cached_crates.clear();
            world_visualizer_detail::cached_crates_last_refresh = 0;
        }
    }

    void init() override
    {
    }

private:
    void unlock_doors_in_world(const Il2CppClass* game_world_class)
    {
        if (!game_world_class || !game_world_class->image)
        {
            return;
        }

        Il2CppObject* door_system_type = il2utils::get_system_type(game_world_class->image, "EFT.Interactive", "Door");
        if (!door_system_type)
        {
            return;
        }

        std::size_t found_doors = 0;
        Il2CppObject** doors = unity::find_objects_of_type(door_system_type, &found_doors);
        if (!doors || found_doors == 0)
        {
            return;
        }

        for (std::size_t index = 0; index < found_doors; ++index)
        {
            if (!doors[index])
            {
                continue;
            }

            Il2CppObjectInstance door_instance(doors[index]);
            const auto set_door_state = door_instance.get_method<void(*)(Il2CppObject*, int)>("set_DoorState", 1);
            if (set_door_state)
            {
                set_door_state(door_instance.get_instance(), 2);
            }
        }

        if (globals::verbose)
        {
            logger::info("Unlocked doors: " + std::to_string(found_doors));
        }
    }

    void refresh_extractions(Il2CppObjectInstance& game_world_instance, Il2CppObjectInstance& main_player)
    {
        Il2CppObject* exfiltration_controller_object =
            game_world_instance.get_field<Il2CppObject*>("<ExfiltrationController>k__BackingField");
        if (!exfiltration_controller_object)
        {
            extraction_points.clear();
            return;
        }

        const auto get_profile = main_player.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_Profile", 0);
        Il2CppObject* profile = get_profile(main_player.get_instance());
        if (!profile)
        {
            extraction_points.clear();
            return;
        }

        Il2CppObjectInstance exfiltration_controller(exfiltration_controller_object);
        const auto eligible_points = exfiltration_controller.get_method<Il2CppArray*(*)(Il2CppObject*, Il2CppObject*)>(
            "EligiblePoints",
            1
        )(exfiltration_controller.get_instance(), profile);

        std::vector<world_visualizer_detail::ExfilPoint> next_points{};
        if (!eligible_points)
        {
            extraction_points.clear();
            return;
        }

        next_points.reserve(eligible_points->max_length);
        for (std::size_t index = 0; index < eligible_points->max_length; ++index)
        {
            Il2CppObject* exfil_object = reinterpret_cast<Il2CppObject**>(&eligible_points->data)[index];
            if (!exfil_object)
            {
                continue;
            }

            Il2CppObjectInstance exfiltration_point(exfil_object);
            Il2CppObjectInstance settings(exfiltration_point.get_field<Il2CppObject*>("Settings"));
            const std::string name = il2utils::conv_string(settings.get_field<Il2CppString*>("Name"));

            Il2CppObject* game_object = unity::component_get_game_object(exfiltration_point.get_instance());
            Il2CppObject* transform = game_object ? unity::gameobject_get_transform(game_object) : nullptr;
            if (!transform)
            {
                continue;
            }

            next_points.push_back({
                name,
                exfiltration_point.get_field<int>("_status"),
                unity::transform_get_pos(transform)
            });
        }

        extraction_points.swap(next_points);
    }

    void refresh_transits(Il2CppObjectInstance& game_world_instance)
    {
        Il2CppObject* transit_controller_object =
            game_world_instance.get_field<Il2CppObject*>("<TransitController>k__BackingField");
        if (!transit_controller_object)
        {
            transit_points.clear();
            return;
        }

        Il2CppObjectInstance transit_controller(transit_controller_object);
        const auto transits_by_id = transit_controller.get_field<unity::dict<int, Il2CppObject*>*>("pointsById");
        if (!transits_by_id || !transits_by_id->m_p_entries)
        {
            transit_points.clear();
            return;
        }

        std::vector<world_visualizer_detail::TransitPoint> next_points{};
        next_points.reserve(static_cast<std::size_t>((std::max)(0, transits_by_id->m_i_count)));

        for (int index = 0; index < transits_by_id->m_i_count; ++index)
        {
            Il2CppObject* transit_object = transits_by_id->get_value_by_index(index);
            if (!transit_object)
            {
                continue;
            }

            Il2CppObjectInstance transit(transit_object);
            Il2CppObject* parameters_object = transit.get_field<Il2CppObject*>("parameters");
            if (!parameters_object)
            {
                continue;
            }

            Il2CppObjectInstance parameters(parameters_object);
            const std::string location = il2utils::conv_string(parameters.get_field<Il2CppString*>("location"));

            Il2CppObject* game_object = unity::component_get_game_object(transit.get_instance());
            Il2CppObject* transform = game_object ? unity::gameobject_get_transform(game_object) : nullptr;
            if (!transform)
            {
                continue;
            }

            next_points.push_back({
                location,
                unity::transform_get_pos(transform)
            });
        }

        transit_points.swap(next_points);
    }

    void refresh_world_markers(Il2CppObjectInstance& game_world_instance)
    {
        std::vector<world_visualizer_detail::WorldMarker> next_markers{};
        const float min_distance = item_min_distance->get_value();
        const float min_distance_sq = min_distance * min_distance;

        if (show_items->get_value())
        {
            refresh_items(game_world_instance, next_markers, min_distance_sq);
        }

        if (show_quest_items->get_value())
        {
            refresh_quest_items(game_world_instance, next_markers);
        }

        if (show_crates->get_value())
        {
            refresh_crates(game_world_instance, next_markers, min_distance_sq);
        }
        else
        {
            world_visualizer_detail::cached_crates.clear();
            world_visualizer_detail::cached_crates_last_refresh = 0;
        }

        world_markers.swap(next_markers);
    }

    void refresh_items(
        Il2CppObjectInstance& game_world_instance,
        std::vector<world_visualizer_detail::WorldMarker>& next_markers,
        float min_distance_sq)
    {
        Il2CppObject* loot_hydra_object = game_world_instance.get_field<Il2CppObject*>("LootItems");
        if (!loot_hydra_object)
        {
            return;
        }

        Il2CppObjectInstance loot_hydra(loot_hydra_object);
        unity::list* loot_list = loot_hydra.get_field<unity::list*>("_iteration");
        if (!loot_list || !loot_list->m_p_list_array)
        {
            return;
        }

        Il2CppArray* loot_array = loot_list->m_p_list_array;
        const int count = static_cast<int>(loot_array->max_length);
        const float max_distance = item_distance->get_value();
        const float max_distance_sq = max_distance * max_distance;

        next_markers.reserve(next_markers.size() + static_cast<std::size_t>((std::max)(0, count)));
        for (int index = 0; index < count; ++index)
        {
            Il2CppObject* loot_object = reinterpret_cast<Il2CppObject**>(&loot_array->data)[index];
            if (!loot_object)
            {
                continue;
            }

            Il2CppObject* game_object = unity::component_get_game_object(loot_object);
            Il2CppObject* transform = game_object ? unity::gameobject_get_transform(game_object) : nullptr;
            if (!transform)
            {
                continue;
            }

            const unity::vector3 position = unity::transform_get_pos(transform);
            const float marker_distance_sq = world_visualizer_detail::distance_sq(local_position, position);
            if (marker_distance_sq < min_distance_sq || marker_distance_sq > max_distance_sq)
            {
                continue;
            }

            Il2CppObjectInstance loot_instance(loot_object);
            Il2CppObject* item_object = loot_instance.get_field<Il2CppObject*>("_item");

            next_markers.push_back({
                world_visualizer_detail::localized_short_name(item_object, "Item"),
                position,
                false,
                false
            });
        }
    }

    void refresh_quest_items(
        Il2CppObjectInstance& game_world_instance,
        std::vector<world_visualizer_detail::WorldMarker>& next_markers)
    {
        auto* quest_set = game_world_instance.get_field<unity::hashset<Il2CppObject*>*>("QuestItemsList");
        if (!quest_set || !quest_set->m_p_slots)
        {
            return;
        }

        const float max_distance = quest_item_distance->get_value();
        const float max_distance_sq = max_distance * max_distance;
        auto* slots = quest_set->get_slots();
        const int last_index = quest_set->m_i_last_index;

        for (int index = 0; index < last_index; ++index)
        {
            if (slots[index].m_i_hash_code < 0)
            {
                continue;
            }

            Il2CppObject* loot_object = slots[index].m_t_value;
            if (!loot_object)
            {
                continue;
            }

            Il2CppObject* game_object = unity::component_get_game_object(loot_object);
            Il2CppObject* transform = game_object ? unity::gameobject_get_transform(game_object) : nullptr;
            if (!transform)
            {
                continue;
            }

            const unity::vector3 position = unity::transform_get_pos(transform);
            if (world_visualizer_detail::distance_sq(local_position, position) > max_distance_sq)
            {
                continue;
            }

            Il2CppObjectInstance quest_loot_instance(loot_object);
            Il2CppObject* item_object = quest_loot_instance.get_field<Il2CppObject*>("_item");

            next_markers.push_back({
                world_visualizer_detail::localized_short_name(item_object, "Quest Item"),
                position,
                false,
                true
            });
        }
    }

    void refresh_crates(
        Il2CppObjectInstance& game_world_instance,
        std::vector<world_visualizer_detail::WorldMarker>& next_markers,
        float min_distance_sq)
    {
        const ULONGLONG now = GetTickCount64();
        if (now - world_visualizer_detail::cached_crates_last_refresh >= 15000 ||
            world_visualizer_detail::cached_crates.empty())
        {
            world_visualizer_detail::cached_crates_last_refresh = now;
            world_visualizer_detail::cached_crates.clear();

            unity::list* loot_list = game_world_instance.get_field<unity::list*>("LootList");
            if (loot_list && loot_list->m_p_list_array)
            {
                Il2CppArray* loot_array = loot_list->m_p_list_array;
                const int count = static_cast<int>(loot_array->max_length);
                for (int index = 0; index < count; ++index)
                {
                    Il2CppObject* object = reinterpret_cast<Il2CppObject**>(&loot_array->data)[index];
                    if (!object)
                    {
                        continue;
                    }

                    const char* class_name = (object->klass && object->klass->name) ? object->klass->name : "";
                    if (!world_visualizer_detail::class_name_looks_like_crate(class_name))
                    {
                        continue;
                    }

                    std::string name = unity::get_name(object);
                    if (name.empty())
                    {
                        name = class_name;
                    }

                    world_visualizer_detail::cached_crates.push_back({
                        object,
                        std::move(name)
                    });
                }
            }
        }

        const float max_distance = crate_distance->get_value();
        const float max_distance_sq = max_distance * max_distance;
        next_markers.reserve(next_markers.size() + world_visualizer_detail::cached_crates.size());

        for (const world_visualizer_detail::CachedCrate& crate : world_visualizer_detail::cached_crates)
        {
            if (!crate.object)
            {
                continue;
            }

            Il2CppObject* game_object = unity::component_get_game_object(crate.object);
            Il2CppObject* transform = game_object ? unity::gameobject_get_transform(game_object) : nullptr;
            if (!transform)
            {
                continue;
            }

            const unity::vector3 position = unity::transform_get_pos(transform);
            const float marker_distance_sq = world_visualizer_detail::distance_sq(local_position, position);
            if (marker_distance_sq < min_distance_sq || marker_distance_sq > max_distance_sq)
            {
                continue;
            }

            next_markers.push_back({
                crate.name,
                position,
                true,
                false
            });
        }
    }
};
