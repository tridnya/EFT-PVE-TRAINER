#pragma once
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../il2cpp/Il2CppObjectInstance.h"
#include "../il2cpp/il2utils.h"
#include "../il2cpp/unity.h"
#include "../util/logger.h"
#include "game_state.h"

namespace always_on
{
    static inline Il2CppImage* assembly_csharp;
    static inline bool dumped_location_fields = false;
    static inline bool dumped_location_access_fields = false;
    static inline bool dumped_matchmaker_screen_class = false;
    static inline bool dumped_raid_settings_class = false;
    static inline bool dumped_matchmaker_instance_class = false;
    static inline bool dumped_client_backend_session_class = false;
    static inline bool dumped_create_raid_group_params_class = false;
    static inline bool dumped_update_status_params_class = false;
    static inline bool dumped_send_raid_settings_params_class = false;
    static inline bool dumped_local_raid_started_params_class = false;
    static inline bool dumped_local_raid_started_return_task_class = false;
    static inline bool dumped_local_raid_started_result_class = false;
    static inline bool dumped_local_raid_started_settings_class = false;
    static inline bool dumped_local_raid_started_profile_insurance_class = false;
    static inline bool dumped_local_raid_started_location_loot_class = false;
    static inline bool dumped_local_raid_started_transition_settings_class = false;
    static inline bool dumped_location_settings_class = false;
    static inline bool dumped_locations_dictionary_class = false;
    static inline bool dumped_location_dictionary_keys = false;
    static inline bool dumped_dictionary_key_collection_class = false;
    static inline bool dumped_forced_location_snapshot = false;
    static inline bool dumped_backend_result_before_patch = false;
    static inline bool dumped_backend_result_after_patch = false;
    static inline bool dumped_local_raid_started_backend_clone_values = false;
    static inline bool dumped_backend_safe_local_raid_task_class = false;
    static inline bool dumped_real_local_raid_started_result = false;
    static inline bool logged_backend_safe_local_raid_cached_result_missing = false;
    static inline bool logged_real_local_raid_cached_result_missing = false;
    static inline bool dumped_game_world_init_level_params = false;
    static inline bool dumped_game_world_init_level_world = false;
    static inline bool dumped_game_world_init_level_config = false;
    static inline bool dumped_game_world_init_level_config_after_patch = false;
    static inline bool dumped_local_game_class = false;
    static inline bool dumped_local_game_create_location_before = false;
    static inline bool dumped_local_game_create_location_after = false;
    static inline bool dumped_local_game_create_raid_settings = false;
    static inline bool dumped_receive_raid_settings_params_class = false;
    static inline bool dumped_matching_start_notification_params_class = false;
    static inline bool logged_local_game_create_hook = false;
    static inline bool dumped_icebreaker_scene_services = false;
    static inline bool logged_local_raid_started_return_type = false;
    static inline bool logged_local_raid_started_task_result_patch = false;
    static inline bool logged_raid_settings_offline_fields = false;
    static inline bool logged_raid_settings_state = false;
    static inline bool dumped_icebreaker_json_loot_item = false;
    static inline bool dumped_icebreaker_loot_point_configuration = false;
    static inline bool dumped_icebreaker_container_configuration = false;
    static inline bool dumped_icebreaker_loot_record_probe = false;
    static inline bool dumped_icebreaker_live_container_state = false;
    static inline bool dumped_icebreaker_container_registration_schema = false;
    static inline bool dumped_icebreaker_item_address_probe = false;
    static inline bool dumped_icebreaker_member_item_probe = false;
    static inline bool dumped_icebreaker_root_grid_probe = false;
    static inline bool dumped_icebreaker_root_item_collection_probe = false;
    static inline bool icebreaker_loot_remapped = false;
    static inline bool icebreaker_cutscene_started = false;
    static inline std::string icebreaker_timeline_id;
    static inline bool selected_location_forces_offline = false;
    static inline bool selected_forced_location_inserted_into_settings = false;
    static inline Il2CppObject* selected_forced_location_object = nullptr;
    static inline Il2CppObject* selected_forced_location_settings = nullptr;
    static inline Il2CppObject* selected_forced_location_from_settings = nullptr;
    static inline Il2CppObject* active_backend_safe_location_object = nullptr;
    static inline std::string active_backend_safe_location_id;
    static inline Il2CppObject* last_backend_safe_local_raid_session = nullptr;
    static inline Il2CppObject* last_backend_safe_local_raid_settings = nullptr;
    static inline Il2CppObject* last_backend_safe_local_raid_task = nullptr;
    static inline Il2CppObject* last_real_local_raid_task = nullptr;
    static inline std::string last_backend_safe_local_raid_location_id;
    static inline std::string last_real_local_raid_location_id;
    static inline std::unordered_set<uintptr_t> patched_local_raid_started_results;
    static inline std::string selected_forced_location_id;
    static inline const MethodInfo* local_raid_started_method = nullptr;

    static constexpr const char* labs_id = "laboratory";
    static constexpr const char* labs_dark_id = "laboratory_dark";
    static constexpr const char* ice_breaker_id = "icebreaker";
    static constexpr const char* develop_id = "develop";
    static constexpr const char* hideout_id = "hideout";
    static constexpr const char* labyrinth_id = "labyrinth";
    static constexpr const char* terminal_id = "terminal";
    static constexpr const char* terminal_ui_id = "terminal_ui";
    static constexpr const char* test_id = "test";
    static constexpr const char* backend_safe_location_id = "factory4_day";

    inline void reset_raid_runtime_state()
    {
        selected_location_forces_offline = false;
        selected_forced_location_inserted_into_settings = false;
        selected_forced_location_object = nullptr;
        selected_forced_location_settings = nullptr;
        selected_forced_location_from_settings = nullptr;
        selected_forced_location_id.clear();

        active_backend_safe_location_object = nullptr;
        active_backend_safe_location_id.clear();
        last_backend_safe_local_raid_session = nullptr;
        last_backend_safe_local_raid_settings = nullptr;
        last_backend_safe_local_raid_task = nullptr;
        last_backend_safe_local_raid_location_id.clear();
        last_real_local_raid_task = nullptr;
        last_real_local_raid_location_id.clear();
        patched_local_raid_started_results.clear();

        dumped_backend_result_before_patch = false;
        dumped_backend_result_after_patch = false;
        dumped_local_raid_started_backend_clone_values = false;
        dumped_backend_safe_local_raid_task_class = false;
        dumped_real_local_raid_started_result = false;
        logged_backend_safe_local_raid_cached_result_missing = false;
        logged_real_local_raid_cached_result_missing = false;
        logged_local_raid_started_task_result_patch = false;
        logged_raid_settings_offline_fields = false;
        logged_raid_settings_state = false;

        dumped_icebreaker_scene_services = false;
        dumped_icebreaker_json_loot_item = false;
        dumped_icebreaker_loot_point_configuration = false;
        dumped_icebreaker_container_configuration = false;
        dumped_icebreaker_loot_record_probe = false;
        dumped_icebreaker_live_container_state = false;
        dumped_icebreaker_container_registration_schema = false;
        dumped_icebreaker_item_address_probe = false;
        dumped_icebreaker_member_item_probe = false;
        dumped_icebreaker_root_grid_probe = false;
        dumped_icebreaker_root_item_collection_probe = false;
        icebreaker_loot_remapped = false;
        icebreaker_cutscene_started = false;
        icebreaker_timeline_id.clear();
    }

    struct LocationDictionaryEntry
    {
        int32_t hash_code;
        int32_t next;
        Il2CppString* key;
        Il2CppObject* value;
    };

    struct MongoIdValue
    {
        uint32_t time_stamp;
        uint32_t alignment;
        uint64_t counter;
        Il2CppString* string_id;
    };

    struct IcebreakerLootCandidate
    {
        Il2CppObject* record = nullptr;
        Il2CppObject* item = nullptr;
        Il2CppObject* item_template = nullptr;
        std::string template_id;
    };

    struct IcebreakerLootContainerGroup
    {
        std::string source_id;
        std::string template_id;
        Il2CppObject* root_record = nullptr;
        int contained_item_count = 0;
        std::vector<Il2CppObject*> records;
    };

    static_assert(sizeof(MongoIdValue) == 24);

    static_assert(sizeof(LocationDictionaryEntry) == 24);

    inline std::string lower_copy(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
        return value;
    }

    inline bool should_return_original_transit(const std::string& location, const std::string& name)
    {
        const std::string lower_location = lower_copy(location);
        const std::string lower_name = lower_copy(name);
        return lower_location == labs_id || lower_name == labs_id ||
            lower_location == labs_dark_id || lower_name == labs_dark_id ||
            lower_location == ice_breaker_id || lower_name == ice_breaker_id;
    }

    inline std::string safe_string(const Il2CppString* value)
    {
        if (value == nullptr)
        {
            return "";
        }

        return il2utils::conv_string(value);
    }

    inline bool should_force_display_location(const std::string& value)
    {
        const std::string lower_value = lower_copy(value);
        return lower_value == labs_id || lower_value == labs_dark_id ||
            lower_value == terminal_id || lower_value == terminal_ui_id ||
            lower_value == labyrinth_id || lower_value == ice_breaker_id ||
            lower_value == test_id || lower_value == develop_id ||
            lower_value == "development" || lower_value == hideout_id;
    }

    inline bool should_force_location_offline(const std::string& value)
    {
        const std::string lower_value = lower_copy(value);
        return lower_value == labs_id || lower_value == labs_dark_id ||
            lower_value == ice_breaker_id;
    }

    inline bool selected_forced_location_is_labs()
    {
        const std::string selected_id =
            lower_copy(selected_forced_location_id);
        return selected_id == labs_id || selected_id == labs_dark_id;
    }

    inline bool selected_forced_location_is_icebreaker()
    {
        return lower_copy(selected_forced_location_id) == ice_breaker_id;
    }

    inline void dump_class_layout(const Il2CppClass* klass, const std::string& label);
    inline void dump_object_field_values(Il2CppObject* object, const std::string& label);
    inline void dump_object_class_once(Il2CppObject* object, const std::string& label, bool& dumped);
    inline void dump_object_class_and_values_once(Il2CppObject* object, const std::string& label, bool& dumped);
    inline void dump_location_snapshot(Il2CppObject* location, const std::string& label);
    inline void dump_object_summary(Il2CppObject* object, const std::string& label);
    inline int dictionary_count(Il2CppObject* dictionary);

    inline bool should_force_display_location_object(Il2CppObject* location)
    {
        if (location == nullptr || location->klass == nullptr)
        {
            return false;
        }

        Il2CppObjectInstance location_instance(location);
        return should_force_display_location(safe_string(location_instance.get_field<Il2CppString*>("Name"))) ||
            should_force_display_location(safe_string(location_instance.get_field<Il2CppString*>("Id"))) ||
            should_force_display_location(safe_string(location_instance.get_field<Il2CppString*>("_Id")));
    }

    inline bool should_force_location_offline_object(Il2CppObject* location)
    {
        if (location == nullptr || location->klass == nullptr)
        {
            return false;
        }

        Il2CppObjectInstance location_instance(location);
        return should_force_location_offline(safe_string(location_instance.get_field<Il2CppString*>("Name"))) ||
            should_force_location_offline(safe_string(location_instance.get_field<Il2CppString*>("Id"))) ||
            should_force_location_offline(safe_string(location_instance.get_field<Il2CppString*>("_Id")));
    }

    inline std::string get_location_identifier(Il2CppObject* location)
    {
        if (location == nullptr || location->klass == nullptr)
        {
            return "";
        }

        Il2CppObjectInstance location_instance(location);
        const std::string id = safe_string(location_instance.get_field<Il2CppString*>("Id"));
        const std::string internal_id = safe_string(location_instance.get_field<Il2CppString*>("_Id"));
        const std::string name = safe_string(location_instance.get_field<Il2CppString*>("Name"));

        if (should_force_location_offline(id))
        {
            return lower_copy(id);
        }

        if (should_force_location_offline(internal_id))
        {
            return lower_copy(internal_id);
        }

        if (should_force_location_offline(name))
        {
            return lower_copy(name);
        }

        if (!id.empty())
        {
            return id;
        }

        if (!internal_id.empty())
        {
            return internal_id;
        }

        return name;
    }

    inline void update_selected_forced_location(Il2CppObject* location)
    {
        if (location == nullptr || location->klass == nullptr)
        {
            return;
        }

        const bool should_force_offline =
            should_force_location_offline_object(location);
        std::string location_id = get_location_identifier(location);
        if (game_state::offline_labs_blackout &&
            lower_copy(location_id) == labs_id)
        {
            location_id = labs_dark_id;
        }

        if (should_force_offline)
        {
            if (!selected_location_forces_offline || selected_forced_location_id != location_id)
            {
                logger::info("[RaidSettings] selected forced-offline location '" + location_id + "'");
            }

            selected_location_forces_offline = true;
            selected_forced_location_inserted_into_settings = false;
            selected_forced_location_object = location;
            selected_forced_location_id = location_id;
            active_backend_safe_location_object = nullptr;
            active_backend_safe_location_id.clear();
            dumped_icebreaker_json_loot_item = false;
            dumped_icebreaker_loot_point_configuration = false;
            dumped_icebreaker_container_configuration = false;
            dumped_icebreaker_loot_record_probe = false;
            dumped_icebreaker_live_container_state = false;
            dumped_icebreaker_container_registration_schema = false;
            dumped_icebreaker_item_address_probe = false;
            dumped_icebreaker_member_item_probe = false;
            dumped_icebreaker_root_grid_probe = false;
            dumped_icebreaker_root_item_collection_probe = false;
            icebreaker_loot_remapped = false;
            icebreaker_cutscene_started = false;
            icebreaker_timeline_id.clear();
            return;
        }

        if (selected_location_forces_offline)
        {
            logger::info("[RaidSettings] cleared forced-offline selected location with '" + location_id + "'");
        }

        selected_location_forces_offline = false;
        selected_forced_location_inserted_into_settings = false;
        selected_forced_location_object = nullptr;
        selected_forced_location_settings = nullptr;
        selected_forced_location_from_settings = nullptr;
        active_backend_safe_location_object = nullptr;
        active_backend_safe_location_id.clear();
        dumped_icebreaker_json_loot_item = false;
        dumped_icebreaker_loot_point_configuration = false;
        dumped_icebreaker_container_configuration = false;
        dumped_icebreaker_loot_record_probe = false;
        dumped_icebreaker_live_container_state = false;
        dumped_icebreaker_container_registration_schema = false;
        dumped_icebreaker_item_address_probe = false;
        dumped_icebreaker_member_item_probe = false;
        dumped_icebreaker_root_grid_probe = false;
        dumped_icebreaker_root_item_collection_probe = false;
        icebreaker_loot_remapped = false;
        icebreaker_cutscene_started = false;
        icebreaker_timeline_id.clear();
        selected_forced_location_id.clear();
    }

    inline bool has_forced_selected_location(Il2CppObject* instance)
    {
        if (instance == nullptr || instance->klass == nullptr)
        {
            return false;
        }

        Il2CppObjectInstance screen_instance(instance);
        Il2CppObject* selected_location = screen_instance.get_field<Il2CppObject*>("_selectedLocation");
        return should_force_display_location_object(selected_location);
    }

    inline void force_location_offline(Il2CppObjectInstance& location_instance)
    {
        location_instance.set_field("ForceOnlineRaidInPVE", false);
        location_instance.set_field("ForceOfflineRaidInPVE", true);
        location_instance.set_field("SavageForceOnlineRaidInPVE", false);
        location_instance.set_field("SavageForceOfflineRaidInPVE", true);
    }

    inline std::string class_name(const Il2CppClass* klass)
    {
        if (klass == nullptr)
        {
            return "<null>";
        }

        return std::string(klass->namespaze ? klass->namespaze : "") + "." +
            std::string(klass->name ? klass->name : "<unknown>");
    }

    inline void clear_il2cpp_list(Il2CppObject* list, const char* label)
    {
        if (list == nullptr || list->klass == nullptr)
        {
            return;
        }

        const MethodInfo* clear_method = il2utils::resolve_method(list->klass, "Clear", 0);
        if (clear_method == nullptr || clear_method->methodPointer == nullptr)
        {
            logger::warn("[LocationAccess] unable to clear " + std::string(label) +
                " class=" + class_name(list->klass));
            return;
        }

        using clear_sig = void(*)(Il2CppObject*);
        reinterpret_cast<clear_sig>(clear_method->methodPointer)(list);
    }

    inline void clear_location_access_requirements(Il2CppObjectInstance& location_instance)
    {
        Il2CppObject* access = location_instance.get_field<Il2CppObject*>("access");
        if (access == nullptr || access->klass == nullptr)
        {
            return;
        }

        Il2CppObjectInstance access_instance(access);
        clear_il2cpp_list(access_instance.get_field<Il2CppObject*>("menu_PVP"), "menu_PVP");
        clear_il2cpp_list(access_instance.get_field<Il2CppObject*>("menu_PVE"), "menu_PVE");
        clear_il2cpp_list(access_instance.get_field<Il2CppObject*>("transit_PVP"), "transit_PVP");
        clear_il2cpp_list(access_instance.get_field<Il2CppObject*>("transit_PVE"), "transit_PVE");
    }

    inline Il2CppObject* get_object_field(Il2CppObject* object, const char* field_name)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return nullptr;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr)
        {
            return nullptr;
        }

        return *reinterpret_cast<Il2CppObject**>(reinterpret_cast<UINT64>(object) + field->offset);
    }

    inline Il2CppString* get_string_field(Il2CppObject* object, const char* field_name)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return nullptr;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr || field->type->type != IL2CPP_TYPE_STRING)
        {
            return nullptr;
        }

        return *reinterpret_cast<Il2CppString**>(reinterpret_cast<UINT64>(object) + field->offset);
    }

    inline int get_int_field_or(Il2CppObject* object, const char* field_name, const int default_value)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return default_value;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr ||
            (field->type->type != IL2CPP_TYPE_I4 && field->type->type != IL2CPP_TYPE_VALUETYPE))
        {
            return default_value;
        }

        return *reinterpret_cast<int*>(reinterpret_cast<UINT64>(object) + field->offset);
    }

    inline float get_float_field_or(
        Il2CppObject* object,
        const char* field_name,
        const float default_value)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return default_value;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr || field->type->type != IL2CPP_TYPE_R4)
        {
            return default_value;
        }

        return *reinterpret_cast<float*>(reinterpret_cast<UINT64>(object) + field->offset);
    }

    inline uint8_t get_byte_field_or(
        Il2CppObject* object,
        const char* field_name,
        const uint8_t default_value)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return default_value;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr ||
            (field->type->type != IL2CPP_TYPE_U1 && field->type->type != IL2CPP_TYPE_I1))
        {
            return default_value;
        }

        return *reinterpret_cast<uint8_t*>(reinterpret_cast<UINT64>(object) + field->offset);
    }

    inline std::string get_mongo_id_field_string(
        Il2CppObject* object,
        const char* field_name)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return {};
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr ||
            (field->type->type != IL2CPP_TYPE_VALUETYPE && !field->type->valuetype))
        {
            return {};
        }

        const auto* value = reinterpret_cast<const MongoIdValue*>(
            reinterpret_cast<UINT64>(object) + field->offset);
        return safe_string(value->string_id);
    }

    inline Il2CppObject* get_inventory_item_template(Il2CppObject* item)
    {
        return get_object_field(item, "<Template>k__BackingField");
    }

    inline std::string get_inventory_item_template_id(Il2CppObject* item)
    {
        return get_mongo_id_field_string(
            get_inventory_item_template(item),
            "<_id>k__BackingField");
    }

    inline int get_il2cpp_list_size(Il2CppObject* list)
    {
        return get_int_field_or(list, "_size", -1);
    }

    inline int get_location_loot_size(Il2CppObject* location)
    {
        return get_il2cpp_list_size(get_object_field(location, "Loot"));
    }

    inline bool get_bool_field_or(Il2CppObject* object, const char* field_name, const bool default_value)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return default_value;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr || field->type->type != IL2CPP_TYPE_BOOLEAN)
        {
            return default_value;
        }

        return *reinterpret_cast<bool*>(reinterpret_cast<UINT64>(object) + field->offset);
    }

    inline bool set_bool_field_if_exists(Il2CppObject* object, const char* field_name, const bool value)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr || field->type->type != IL2CPP_TYPE_BOOLEAN)
        {
            return false;
        }

        *reinterpret_cast<bool*>(reinterpret_cast<UINT64>(object) + field->offset) = value;
        return true;
    }

    inline bool set_int_field_if_exists(Il2CppObject* object, const char* field_name, const int value)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr || field->type->type != IL2CPP_TYPE_I4)
        {
            return false;
        }

        *reinterpret_cast<int*>(reinterpret_cast<UINT64>(object) + field->offset) = value;
        return true;
    }

    inline bool set_float_field_if_exists(Il2CppObject* object, const char* field_name, const float value)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr || field->type->type != IL2CPP_TYPE_R4)
        {
            return false;
        }

        *reinterpret_cast<float*>(reinterpret_cast<UINT64>(object) + field->offset) = value;
        return true;
    }

    inline bool set_vector3_field_if_exists(
        Il2CppObject* object,
        const char* field_name,
        const unity::vector3& value)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr)
        {
            return false;
        }

        if (field->type->valuetype || field->type->type == IL2CPP_TYPE_VALUETYPE)
        {
            std::memcpy(
                reinterpret_cast<uint8_t*>(object) + field->offset,
                &value,
                sizeof(value));
            return true;
        }

        const int type = field->type->type;
        if (type != IL2CPP_TYPE_CLASS && type != IL2CPP_TYPE_OBJECT &&
            type != IL2CPP_TYPE_GENERICINST)
        {
            return false;
        }

        Il2CppObject* vector_object = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<UINT64>(object) + field->offset);
        if (vector_object == nullptr)
        {
            return false;
        }

        const bool x_set = set_float_field_if_exists(vector_object, "x", value.x);
        const bool y_set = set_float_field_if_exists(vector_object, "y", value.y);
        const bool z_set = set_float_field_if_exists(vector_object, "z", value.z);
        return x_set && y_set && z_set;
    }

    inline bool set_string_field_if_exists(Il2CppObject* object, const char* field_name, const std::string& value)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr || field->type->type != IL2CPP_TYPE_STRING)
        {
            return false;
        }

        *reinterpret_cast<Il2CppString**>(reinterpret_cast<UINT64>(object) + field->offset) =
            il2cpp::il2cpp_string_new(value.c_str());
        return true;
    }

    inline bool set_string_field_pointer_if_exists(Il2CppObject* object, const char* field_name, Il2CppString* value)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr || field->type->type != IL2CPP_TYPE_STRING)
        {
            return false;
        }

        *reinterpret_cast<Il2CppString**>(reinterpret_cast<UINT64>(object) + field->offset) = value;
        return true;
    }

    inline bool set_reference_field_if_exists(Il2CppObject* object, const char* field_name, Il2CppObject* value)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr)
        {
            return false;
        }

        const int type = field->type->type;
        if (type != IL2CPP_TYPE_CLASS && type != IL2CPP_TYPE_OBJECT &&
            type != IL2CPP_TYPE_ARRAY && type != IL2CPP_TYPE_SZARRAY &&
            type != IL2CPP_TYPE_GENERICINST)
        {
            return false;
        }

        *reinterpret_cast<Il2CppObject**>(reinterpret_cast<UINT64>(object) + field->offset) = value;
        return true;
    }

    inline bool is_object_reference_type(const Il2CppType* type)
    {
        if (type == nullptr)
        {
            return false;
        }

        if (type->valuetype)
        {
            return false;
        }

        return type->type == IL2CPP_TYPE_CLASS || type->type == IL2CPP_TYPE_OBJECT ||
            type->type == IL2CPP_TYPE_ARRAY || type->type == IL2CPP_TYPE_SZARRAY ||
            type->type == IL2CPP_TYPE_GENERICINST;
    }

    inline bool should_rewrite_backend_safe_location_string(const std::string& value)
    {
        const std::string lower_value = lower_copy(value);
        return lower_value == backend_safe_location_id ||
            (!active_backend_safe_location_id.empty() &&
                lower_value == lower_copy(active_backend_safe_location_id));
    }

    inline bool should_recurse_backend_callback_object(Il2CppObject* object)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        const char* namespaze = object->klass->namespaze ? object->klass->namespaze : "";
        if (std::string(namespaze).rfind("System", 0) == 0 ||
            std::string(namespaze).rfind("UnityEngine", 0) == 0)
        {
            return false;
        }

        return object->klass->field_count > 0 && object->klass->fields != nullptr;
    }

    inline bool rewrite_backend_safe_location_strings(
        Il2CppObject* object,
        const std::string& label,
        std::unordered_set<uintptr_t>& visited,
        const int depth = 0)
    {
        if (!selected_location_forces_offline || selected_forced_location_id.empty() ||
            !should_recurse_backend_callback_object(object) || depth > 3)
        {
            return false;
        }

        const uintptr_t address = reinterpret_cast<uintptr_t>(object);
        if (visited.contains(address))
        {
            return false;
        }
        visited.insert(address);

        bool changed = false;
        for (uint16_t index = 0; index < object->klass->field_count; ++index)
        {
            const FieldInfo& field = object->klass->fields[index];
            if (field.offset <= 0 || field.type == nullptr || field.name == nullptr)
            {
                continue;
            }

            if (field.type->type == IL2CPP_TYPE_STRING)
            {
                Il2CppString** value_ptr = reinterpret_cast<Il2CppString**>(
                    reinterpret_cast<UINT64>(object) + field.offset);
                const std::string value = safe_string(*value_ptr);
                if (should_rewrite_backend_safe_location_string(value))
                {
                    *value_ptr = il2cpp::il2cpp_string_new(selected_forced_location_id.c_str());
                    changed = true;
                    logger::info("[BackendCallback] " + label + " rewrote field '" +
                        std::string(field.name) + "' '" + value + "' -> '" +
                        selected_forced_location_id + "'");
                }
                continue;
            }

            if (is_object_reference_type(field.type))
            {
                Il2CppObject* child = *reinterpret_cast<Il2CppObject**>(
                    reinterpret_cast<UINT64>(object) + field.offset);
                changed |= rewrite_backend_safe_location_strings(child, label, visited, depth + 1);
            }
        }

        return changed;
    }

    inline bool copy_string_field_if_exists(Il2CppObject* target, Il2CppObject* source, const char* field_name)
    {
        return set_string_field_pointer_if_exists(target, field_name, get_string_field(source, field_name));
    }

    inline bool copy_string_field_if_empty(Il2CppObject* target, Il2CppObject* source, const char* field_name)
    {
        if (!safe_string(get_string_field(target, field_name)).empty())
        {
            return false;
        }

        return copy_string_field_if_exists(target, source, field_name);
    }

    inline bool copy_reference_field_if_exists(Il2CppObject* target, Il2CppObject* source, const char* field_name)
    {
        return set_reference_field_if_exists(target, field_name, get_object_field(source, field_name));
    }

    inline bool copy_int_field_if_exists(Il2CppObject* target, Il2CppObject* source, const char* field_name)
    {
        if (target == nullptr || target->klass == nullptr || source == nullptr || source->klass == nullptr)
        {
            return false;
        }

        const FieldInfo* source_field = il2utils::resolve_field(source->klass, field_name);
        const FieldInfo* target_field = il2utils::resolve_field(target->klass, field_name);
        if (source_field == nullptr || target_field == nullptr ||
            source_field->type == nullptr || target_field->type == nullptr)
        {
            return false;
        }

        const int source_type = source_field->type->type;
        const int target_type = target_field->type->type;
        const bool source_is_int = source_type == IL2CPP_TYPE_I4 || source_type == IL2CPP_TYPE_VALUETYPE;
        const bool target_is_int = target_type == IL2CPP_TYPE_I4 || target_type == IL2CPP_TYPE_VALUETYPE;
        if (!source_is_int || !target_is_int)
        {
            return false;
        }

        *reinterpret_cast<int*>(reinterpret_cast<UINT64>(target) + target_field->offset) =
            *reinterpret_cast<int*>(reinterpret_cast<UINT64>(source) + source_field->offset);
        return true;
    }

    inline void call_parameterless_constructor_if_exists(Il2CppObject* object)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return;
        }

        const MethodInfo* constructor = il2utils::resolve_method(object->klass, ".ctor", 0);
        if (constructor == nullptr || constructor->methodPointer == nullptr)
        {
            return;
        }

        using constructor_sig = void(*)(Il2CppObject*);
        reinterpret_cast<constructor_sig>(constructor->methodPointer)(object);
    }

    inline const MethodInfo* resolve_method_in_hierarchy(Il2CppClass* klass, const char* name, const int arg_count)
    {
        while (klass != nullptr)
        {
            const MethodInfo* method = il2utils::resolve_method(klass, name, arg_count);
            if (method != nullptr && method->methodPointer != nullptr)
            {
                return method;
            }

            klass = klass->parent;
        }

        return nullptr;
    }

    inline bool hook_method_in_hierarchy(
        Il2CppClass* klass,
        const char* name,
        const int arg_count,
        void* hook,
        void** original,
        const char* label)
    {
        const MethodInfo* method = resolve_method_in_hierarchy(klass, name, arg_count);
        if (method == nullptr || method->methodPointer == nullptr)
        {
            logger::warn("[" + std::string(label) + "] hook missing method '" +
                std::string(name) + "' params=" + std::to_string(arg_count));
            return false;
        }

        hooks::hook(method->methodPointer, hook, reinterpret_cast<PVOID*>(original));
        logger::info("[" + std::string(label) + "] hooked '" + std::string(name) +
            "' params=" + std::to_string(arg_count));
        return true;
    }

    inline bool call_bool_setter_if_exists(Il2CppObject* object, const char* method_name, const bool value)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        const MethodInfo* method = il2utils::resolve_method(object->klass, method_name, 1);
        if (method == nullptr || method->methodPointer == nullptr)
        {
            return false;
        }

        using bool_setter_sig = void(*)(Il2CppObject*, bool);
        reinterpret_cast<bool_setter_sig>(method->methodPointer)(object, value);
        return true;
    }

    inline bool raid_settings_forces_offline(Il2CppObject* raid_settings)
    {
        if (selected_location_forces_offline)
        {
            return true;
        }

        if (raid_settings == nullptr || raid_settings->klass == nullptr)
        {
            return false;
        }

        if (should_force_location_offline_object(get_object_field(raid_settings, "_selectedLocation")))
        {
            return true;
        }

        return should_force_location_offline(safe_string(get_string_field(raid_settings, "LocationId")));
    }

    inline void log_raid_settings_state(Il2CppObject* raid_settings)
    {
        if (logged_raid_settings_state || raid_settings == nullptr || raid_settings->klass == nullptr)
        {
            return;
        }

        logged_raid_settings_state = true;
        logger::info("[RaidSettingsState] class=" + class_name(raid_settings->klass) +
            " location_id='" + safe_string(get_string_field(raid_settings, "LocationId")) +
            "' raid_mode=" + std::to_string(get_int_field_or(raid_settings, "<RaidMode>k__BackingField", -1)) +
            " is_pve_offline=" + std::to_string(get_bool_field_or(raid_settings, "IsPveOffline", false)) +
            " selected_location_forces_offline=" + std::to_string(selected_location_forces_offline) +
            " selected_forced_location_id='" + selected_forced_location_id + "'" +
            " forced_offline=" + std::to_string(raid_settings_forces_offline(raid_settings)));
    }

    inline void force_raid_settings_offline(Il2CppObject* raid_settings)
    {
        if (raid_settings == nullptr || raid_settings->klass == nullptr)
        {
            return;
        }

        log_raid_settings_state(raid_settings);

        bool changed = false;
        if (selected_location_forces_offline && !selected_forced_location_id.empty())
        {
            changed |= set_string_field_if_exists(raid_settings, "LocationId", selected_forced_location_id);
        }

        changed |= set_bool_field_if_exists(raid_settings, "Local", true);
        changed |= set_bool_field_if_exists(raid_settings, "IsLocal", true);
        changed |= set_bool_field_if_exists(raid_settings, "Offline", true);
        changed |= set_bool_field_if_exists(raid_settings, "IsOffline", true);
        changed |= set_bool_field_if_exists(raid_settings, "IsPveOffline", true);
        changed |= set_bool_field_if_exists(raid_settings, "ForceOffline", true);
        changed |= set_bool_field_if_exists(raid_settings, "PlayOnServers", false);
        changed |= set_bool_field_if_exists(raid_settings, "PlayOnBsgServers", false);
        changed |= set_bool_field_if_exists(raid_settings, "IsOnline", false);
        changed |= set_bool_field_if_exists(raid_settings, "Online", false);
        changed |= call_bool_setter_if_exists(raid_settings, "set_OnlinePveRaid", false);

        if (changed && !logged_raid_settings_offline_fields)
        {
            logged_raid_settings_offline_fields = true;
            logger::info("[RaidSettings] forced offline bool fields on " + class_name(raid_settings->klass));
        }
    }

    inline void force_backend_params_offline(Il2CppObject* params, const std::string& label)
    {
        if (!selected_location_forces_offline || selected_forced_location_id.empty() ||
            params == nullptr || params->klass == nullptr)
        {
            return;
        }

        bool changed = false;
        changed |= set_string_field_if_exists(params, "LocationId", selected_forced_location_id);
        changed |= set_string_field_if_exists(params, "locationId", selected_forced_location_id);
        changed |= set_string_field_if_exists(params, "Location", selected_forced_location_id);
        changed |= set_string_field_if_exists(params, "location", selected_forced_location_id);

        changed |= set_bool_field_if_exists(params, "Local", true);
        changed |= set_bool_field_if_exists(params, "local", true);
        changed |= set_bool_field_if_exists(params, "IsLocal", true);
        changed |= set_bool_field_if_exists(params, "isLocal", true);
        changed |= set_bool_field_if_exists(params, "Offline", true);
        changed |= set_bool_field_if_exists(params, "offline", true);
        changed |= set_bool_field_if_exists(params, "IsOffline", true);
        changed |= set_bool_field_if_exists(params, "isOffline", true);
        changed |= set_bool_field_if_exists(params, "IsPveOffline", true);
        changed |= set_bool_field_if_exists(params, "isPveOffline", true);
        changed |= set_bool_field_if_exists(params, "PveOffline", true);
        changed |= set_bool_field_if_exists(params, "pveOffline", true);

        changed |= set_bool_field_if_exists(params, "Online", false);
        changed |= set_bool_field_if_exists(params, "online", false);
        changed |= set_bool_field_if_exists(params, "IsOnline", false);
        changed |= set_bool_field_if_exists(params, "isOnline", false);
        changed |= set_bool_field_if_exists(params, "OnlinePveRaid", false);
        changed |= set_bool_field_if_exists(params, "onlinePveRaid", false);
        changed |= set_bool_field_if_exists(params, "PlayOnServers", false);
        changed |= set_bool_field_if_exists(params, "playOnServers", false);
        changed |= set_bool_field_if_exists(params, "PlayOnBsgServers", false);
        changed |= set_bool_field_if_exists(params, "playOnBsgServers", false);

        if (changed)
        {
            logger::info("[RaidSettings] forced offline backend params for " + label +
                " class=" + class_name(params->klass) +
                " location='" + selected_forced_location_id + "'");
        }
    }

    inline void force_backend_callback_location(Il2CppObject* payload, const std::string& label, bool& dumped)
    {
        if (!selected_location_forces_offline || selected_forced_location_id.empty() ||
            payload == nullptr || payload->klass == nullptr)
        {
            return;
        }

        force_backend_params_offline(payload, label);

        bool changed = false;
        if (selected_forced_location_object != nullptr)
        {
            changed |= set_reference_field_if_exists(payload, "_selectedLocation", selected_forced_location_object);
            changed |= set_reference_field_if_exists(payload, "selectedLocation", selected_forced_location_object);
            changed |= set_reference_field_if_exists(payload, "SelectedLocation", selected_forced_location_object);
        }

        std::unordered_set<uintptr_t> visited;
        changed |= rewrite_backend_safe_location_strings(payload, label, visited);

        dump_object_class_and_values_once(payload, label, dumped);

        if (changed)
        {
            logger::info("[BackendCallback] " + label + " forced callback location to '" +
                selected_forced_location_id + "'");
        }
    }

    struct LocalRaidStartedSnapshot
    {
        Il2CppString* location = nullptr;
        Il2CppString* server_id = nullptr;
        std::string location_text;
        std::string server_id_text;
    };

    inline LocalRaidStartedSnapshot capture_local_raid_started_snapshot(Il2CppObject* raid_settings)
    {
        LocalRaidStartedSnapshot snapshot;
        snapshot.location = get_string_field(raid_settings, "location");
        snapshot.server_id = get_string_field(raid_settings, "serverId");
        snapshot.location_text = safe_string(snapshot.location);
        snapshot.server_id_text = safe_string(snapshot.server_id);
        return snapshot;
    }

    inline Il2CppObject* selected_location_for_local_raid_settings()
    {
        if (selected_forced_location_from_settings != nullptr)
        {
            return selected_forced_location_from_settings;
        }

        return selected_forced_location_object;
    }

    inline bool force_local_raid_started_selected_location(Il2CppObject* raid_settings)
    {
        if (!selected_location_forces_offline || selected_forced_location_id.empty() ||
            raid_settings == nullptr || raid_settings->klass == nullptr)
        {
            return false;
        }

        bool changed = false;
        changed |= set_string_field_if_exists(raid_settings, "location", selected_forced_location_id);
        if (safe_string(get_string_field(raid_settings, "serverId")).empty())
        {
            changed |= set_string_field_if_exists(raid_settings, "serverId", "local");
        }

        Il2CppObject* selected_location = selected_location_for_local_raid_settings();
        if (selected_location != nullptr)
        {
            changed |= set_reference_field_if_exists(raid_settings, "selectedLocation", selected_location);
            changed |= set_reference_field_if_exists(raid_settings, "_selectedLocation", selected_location);
            changed |= set_reference_field_if_exists(raid_settings, "SelectedLocation", selected_location);
        }

        if (changed)
        {
            logger::info("[BackendSession] forced LocalRaidStarted local settings location='" +
                selected_forced_location_id + "' selectedLocation=" +
                (selected_location != nullptr ? class_name(selected_location->klass) : std::string("null")));
        }

        return changed;
    }

    inline void alias_local_raid_started_for_backend(
        Il2CppObject* raid_settings,
        const LocalRaidStartedSnapshot& snapshot,
        Il2CppObject* backend_selected_location,
        const std::string& backend_location_id)
    {
        if (!selected_location_forces_offline || raid_settings == nullptr || raid_settings->klass == nullptr)
        {
            return;
        }

        bool changed = false;
        changed |= set_string_field_if_exists(raid_settings, "location", backend_location_id);
        changed |= set_reference_field_if_exists(
            raid_settings,
            "selectedLocation",
            backend_selected_location);
        changed |= set_reference_field_if_exists(
            raid_settings,
            "_selectedLocation",
            backend_selected_location);
        changed |= set_reference_field_if_exists(
            raid_settings,
            "SelectedLocation",
            backend_selected_location);
        if (backend_selected_location != nullptr)
        {
            logger::info("[BackendSession] LocalRaidStarted backend clone selectedLocation='" +
                safe_string(get_string_field(backend_selected_location, "Id")) + "' class=" +
                class_name(backend_selected_location->klass));
        }
        else
        {
            logger::warn("[BackendSession] LocalRaidStarted backend clone cleared selectedLocation because '" +
                backend_location_id + "' was not found");
        }

        if (snapshot.server_id_text.empty())
        {
            changed |= set_string_field_if_exists(raid_settings, "serverId", "local");
        }

        if (changed)
        {
            logger::info("[BackendSession] aliased LocalRaidStarted backend location '" +
                snapshot.location_text + "' -> '" + backend_location_id + "'");
        }
    }

    inline Il2CppObject* clone_local_raid_started_for_backend(
        Il2CppObject* raid_settings,
        const LocalRaidStartedSnapshot& snapshot,
        Il2CppObject* backend_selected_location,
        const std::string& backend_location_id)
    {
        if (raid_settings == nullptr || raid_settings->klass == nullptr)
        {
            return nullptr;
        }

        Il2CppObject* clone = il2cpp::il2cpp_object_new(raid_settings->klass);
        if (clone == nullptr)
        {
            logger::warn("[BackendSession] failed to allocate LocalRaidStarted backend clone");
            return nullptr;
        }

        call_parameterless_constructor_if_exists(clone);

        bool copied = false;
        copied |= copy_string_field_if_exists(clone, raid_settings, "serverId");
        copied |= copy_string_field_if_exists(clone, raid_settings, "location");
        copied |= copy_int_field_if_exists(clone, raid_settings, "timeVariant");
        copied |= copy_int_field_if_exists(clone, raid_settings, "mode");
        copied |= copy_int_field_if_exists(clone, raid_settings, "playerSide");
        copied |= copy_int_field_if_exists(clone, raid_settings, "transitionType");
        copied |= copy_reference_field_if_exists(clone, raid_settings, "transition");
        copied |= copy_reference_field_if_exists(clone, raid_settings, "selectedLocation");

        if (!copied)
        {
            logger::warn("[BackendSession] LocalRaidStarted backend clone copied no fields");
        }

        alias_local_raid_started_for_backend(
            clone,
            snapshot,
            backend_selected_location,
            backend_location_id);
        if (!dumped_local_raid_started_backend_clone_values)
        {
            dumped_local_raid_started_backend_clone_values = true;
            dump_object_field_values(clone, "LocalRaidStartedBackendCloneAfterAlias");
            dump_location_snapshot(
                get_object_field(clone, "selectedLocation"),
                "LocalRaidStartedBackendCloneSelectedLocation");
        }

        logger::info("[BackendSession] created LocalRaidStarted backend clone local_location='" +
            snapshot.location_text + "' backend_location='" +
            safe_string(get_string_field(clone, "location")) + "'");
        return clone;
    }

    inline const Il2CppClass* resolve_task_class()
    {
        static const Il2CppClass* task_class = nullptr;
        static bool attempted = false;
        if (attempted)
        {
            return task_class;
        }

        attempted = true;
        const char* image_names[] = { "mscorlib.dll", "System.Private.CoreLib.dll", "System.Runtime.dll" };
        for (const char* image_name : image_names)
        {
            const Il2CppImage* image = il2utils::resolve_image(image_name);
            if (image == nullptr)
            {
                continue;
            }

            task_class = il2utils::resolve_class(image, "System.Threading.Tasks", "Task");
            if (task_class != nullptr)
            {
                return task_class;
            }
        }

        logger::warn("[BackendSession] failed to resolve System.Threading.Tasks.Task");
        return nullptr;
    }

    inline Il2CppObject* get_completed_task()
    {
        static Il2CppObject* completed_task = nullptr;
        if (completed_task != nullptr)
        {
            return completed_task;
        }

        const Il2CppClass* task_class = resolve_task_class();
        if (task_class == nullptr)
        {
            return nullptr;
        }

        const MethodInfo* completed_task_getter = il2utils::resolve_method(task_class, "get_CompletedTask", 0);
        if (completed_task_getter == nullptr || completed_task_getter->methodPointer == nullptr)
        {
            logger::warn("[BackendSession] failed to resolve Task.CompletedTask");
            return nullptr;
        }

        using completed_task_sig = Il2CppObject * (*)();
        completed_task = reinterpret_cast<completed_task_sig>(completed_task_getter->methodPointer)();
        if (completed_task == nullptr)
        {
            logger::warn("[BackendSession] Task.CompletedTask returned null");
        }

        return completed_task;
    }

    inline Il2CppClass* class_from_type(const Il2CppType* type)
    {
        if (type == nullptr)
        {
            return nullptr;
        }

        if (type->valuetype)
        {
            return nullptr;
        }

        if (type->type == IL2CPP_TYPE_GENERICINST && type->data.generic_class != nullptr)
        {
            Il2CppClass* klass = type->data.generic_class->cached_class;
            if (klass != nullptr)
            {
                return klass;
            }
        }

        return il2cpp::il2cpp_class_from_type(type);
    }

    inline std::string type_name(const Il2CppType* type)
    {
        if (type == nullptr)
        {
            return "<null>";
        }

        char* raw_name = il2cpp::il2cpp_type_get_name(type);
        if (raw_name != nullptr)
        {
            const std::string name = raw_name;
            il2cpp::il2cpp_free(raw_name);
            return name;
        }

        const Il2CppClass* klass = class_from_type(type);
        if (klass != nullptr)
        {
            return class_name(klass);
        }

        return "type_" + std::to_string(static_cast<int>(type->type));
    }

    inline Il2CppClass* resolve_class_from_full_name(const std::string& full_name)
    {
        if (assembly_csharp == nullptr || full_name.empty() || full_name == "<null>")
        {
            return nullptr;
        }

        if (full_name.size() >= 2 && full_name.compare(full_name.size() - 2, 2, "[]") == 0)
        {
            return nullptr;
        }

        const size_t name_split = full_name.rfind('.');
        if (name_split == std::string::npos || name_split + 1 >= full_name.size())
        {
            return il2cpp::il2cpp_class_from_name(assembly_csharp, "", full_name.c_str());
        }

        const std::string namespaze = full_name.substr(0, name_split);
        const std::string name = full_name.substr(name_split + 1);
        Il2CppClass* klass = il2cpp::il2cpp_class_from_name(assembly_csharp, namespaze.c_str(), name.c_str());
        if (klass != nullptr)
        {
            return klass;
        }

        Il2CppClass* parent = resolve_class_from_full_name(namespaze);
        if (parent == nullptr)
        {
            return nullptr;
        }

        void* iterator = nullptr;
        while (Il2CppClass* nested = il2cpp::il2cpp_class_get_nested_types(parent, &iterator))
        {
            if (nested->name != nullptr && name == nested->name)
            {
                return nested;
            }
        }

        return nullptr;
    }

    inline const Il2CppClass* resolve_system_string_class()
    {
        static const Il2CppClass* string_class = nullptr;
        static bool attempted = false;
        if (attempted)
        {
            return string_class;
        }

        attempted = true;
        const char* image_names[] = { "mscorlib.dll", "System.Private.CoreLib.dll", "System.Runtime.dll" };
        for (const char* image_name : image_names)
        {
            const Il2CppImage* image = il2utils::resolve_image(image_name);
            if (image == nullptr)
            {
                continue;
            }

            string_class = il2utils::resolve_class(image, "System", "String");
            if (string_class != nullptr)
            {
                return string_class;
            }
        }

        logger::warn("[BackendSession] failed to resolve System.String");
        return nullptr;
    }

    inline Il2CppObject* create_empty_string_array()
    {
        const Il2CppClass* string_class = resolve_system_string_class();
        if (string_class == nullptr)
        {
            return nullptr;
        }

        return reinterpret_cast<Il2CppObject*>(
            il2cpp::il2cpp_array_new(const_cast<Il2CppClass*>(string_class), 0));
    }

    inline const Il2CppType* get_array_element_type(const Il2CppType* array_type)
    {
        if (array_type == nullptr)
        {
            return nullptr;
        }

        if (array_type->type == IL2CPP_TYPE_SZARRAY)
        {
            return array_type->data.type;
        }

        if (array_type->type == IL2CPP_TYPE_ARRAY && array_type->data.array != nullptr)
        {
            return array_type->data.array->etype;
        }

        return nullptr;
    }

    inline Il2CppClass* resolve_class_from_type_for_allocation(const Il2CppType* type)
    {
        if (type == nullptr)
        {
            return nullptr;
        }

        if (type->type == IL2CPP_TYPE_STRING)
        {
            return const_cast<Il2CppClass*>(resolve_system_string_class());
        }

        Il2CppClass* klass = class_from_type(type);
        if (klass != nullptr)
        {
            return klass;
        }

        return resolve_class_from_full_name(type_name(type));
    }

    inline Il2CppObject* create_empty_array_from_type(const Il2CppType* array_type)
    {
        const Il2CppType* element_type = get_array_element_type(array_type);
        Il2CppClass* element_class = resolve_class_from_type_for_allocation(element_type);
        if (element_class == nullptr)
        {
            logger::warn("[BackendSession] failed to resolve array element class for '" +
                type_name(array_type) + "' element='" + type_name(element_type) + "'");
            return nullptr;
        }

        return reinterpret_cast<Il2CppObject*>(il2cpp::il2cpp_array_new(element_class, 0));
    }

    inline const Il2CppType* get_local_raid_started_result_type()
    {
        if (local_raid_started_method == nullptr || local_raid_started_method->return_type == nullptr)
        {
            return nullptr;
        }

        const Il2CppType* return_type = local_raid_started_method->return_type;
        if (return_type->type != IL2CPP_TYPE_GENERICINST ||
            return_type->data.generic_class == nullptr ||
            return_type->data.generic_class->context.class_inst == nullptr ||
            return_type->data.generic_class->context.class_inst->type_argc < 1)
        {
            return nullptr;
        }

        return return_type->data.generic_class->context.class_inst->type_argv[0];
    }

    inline Il2CppClass* get_local_raid_started_result_class()
    {
        const Il2CppType* result_type = get_local_raid_started_result_type();
        const std::string result_type_name = type_name(result_type);
        Il2CppClass* result_class = resolve_class_from_full_name(result_type_name);
        if (result_class == nullptr)
        {
            logger::warn("[BackendSession] failed to resolve LocalRaidStarted result class '" +
                result_type_name + "'");
            return nullptr;
        }

        if (!dumped_local_raid_started_result_class)
        {
            dumped_local_raid_started_result_class = true;
            logger::info("[BackendSession] LocalRaidStarted result class=" + class_name(result_class));
            dump_class_layout(result_class, "LocalRaidStartedResult");
        }

        return result_class;
    }

    inline Il2CppClass* get_local_raid_started_return_task_class()
    {
        if (local_raid_started_method == nullptr || local_raid_started_method->return_type == nullptr)
        {
            logger::warn("[BackendSession] LocalRaidStarted method metadata is unavailable");
            return nullptr;
        }

        Il2CppClass* task_class = class_from_type(local_raid_started_method->return_type);
        if (!logged_local_raid_started_return_type)
        {
            logged_local_raid_started_return_type = true;
            logger::info("[BackendSession] LocalRaidStarted return type='" +
                type_name(local_raid_started_method->return_type) +
                "' class=" + class_name(task_class));
        }

        if (task_class != nullptr && !dumped_local_raid_started_return_task_class)
        {
            dumped_local_raid_started_return_task_class = true;
            logger::info("[BackendSession] LocalRaidStarted return task class=" + class_name(task_class));
            dump_class_layout(task_class, "LocalRaidStartedReturnTask");
        }

        return task_class;
    }

    inline const MethodInfo* find_result_task_constructor(Il2CppClass* task_class, std::string& parameter_type_name)
    {
        if (task_class == nullptr)
        {
            return nullptr;
        }

        il2utils::resolve_method(task_class, ".ctor", 1);

        const MethodInfo* fallback_constructor = nullptr;
        std::string fallback_parameter_type_name;
        if (task_class->methods != nullptr)
        {
            for (uint16_t index = 0; index < task_class->method_count; ++index)
            {
                const MethodInfo* method = task_class->methods[index];
                if (method == nullptr || method->methodPointer == nullptr || method->parameters == nullptr ||
                    method->parameters_count != 1 || method->name == nullptr ||
                    std::string(method->name) != ".ctor")
                {
                    continue;
                }

                const std::string candidate_parameter_type_name = type_name(method->parameters[0]);
                if (fallback_constructor == nullptr)
                {
                    fallback_constructor = method;
                    fallback_parameter_type_name = candidate_parameter_type_name;
                }

                if (candidate_parameter_type_name.find("Func") == std::string::npos &&
                    candidate_parameter_type_name.find("Action") == std::string::npos)
                {
                    parameter_type_name = candidate_parameter_type_name;
                    return method;
                }
            }
        }

        parameter_type_name = fallback_parameter_type_name;
        return fallback_constructor;
    }

    inline bool set_reference_field_if_type_contains(
        Il2CppObject* object,
        const char* field_name,
        Il2CppObject* value,
        const std::string& type_fragment)
    {
        if (object == nullptr || object->klass == nullptr || value == nullptr)
        {
            return false;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->type == nullptr || !is_object_reference_type(field->type))
        {
            return false;
        }

        if (type_name(field->type).find(type_fragment) == std::string::npos)
        {
            return false;
        }

        *reinterpret_cast<Il2CppObject**>(reinterpret_cast<UINT64>(object) + field->offset) = value;
        return true;
    }

    inline bool set_reference_field_value(Il2CppObject* object, const FieldInfo& field, Il2CppObject* value)
    {
        if (object == nullptr || object->klass == nullptr || value == nullptr ||
            field.type == nullptr || !is_object_reference_type(field.type))
        {
            return false;
        }

        *reinterpret_cast<Il2CppObject**>(reinterpret_cast<UINT64>(object) + field.offset) = value;
        return true;
    }

    inline Il2CppObject* get_reference_field_value(Il2CppObject* object, const FieldInfo& field)
    {
        if (object == nullptr || object->klass == nullptr ||
            field.type == nullptr || !is_object_reference_type(field.type))
        {
            return nullptr;
        }

        return *reinterpret_cast<Il2CppObject**>(reinterpret_cast<UINT64>(object) + field.offset);
    }

    inline Il2CppObject* invoke_object_method0(Il2CppObject* object, const MethodInfo* method)
    {
        if (object == nullptr || method == nullptr || method->methodPointer == nullptr)
        {
            return nullptr;
        }

        if (method->invoker_method != nullptr)
        {
            Il2CppObject* result = nullptr;
            method->invoker_method(
                reinterpret_cast<Il2CppMethodPointer>(method->methodPointer),
                method,
                object,
                nullptr,
                &result);
            return result;
        }

        using method_sig = Il2CppObject * (*)(Il2CppObject*);
        return reinterpret_cast<method_sig>(method->methodPointer)(object);
    }

    inline int invoke_int_method0(Il2CppObject* object, const MethodInfo* method)
    {
        if (object == nullptr || method == nullptr || method->methodPointer == nullptr)
        {
            return -1;
        }

        if (method->invoker_method != nullptr)
        {
            int result = -1;
            method->invoker_method(
                reinterpret_cast<Il2CppMethodPointer>(method->methodPointer),
                method,
                object,
                nullptr,
                &result);
            return result;
        }

        using method_sig = int(*)(Il2CppObject*);
        return reinterpret_cast<method_sig>(method->methodPointer)(object);
    }

    inline bool dictionary_contains_key(Il2CppObject* dictionary, Il2CppString* key)
    {
        if (dictionary == nullptr || dictionary->klass == nullptr || key == nullptr)
        {
            return false;
        }

        const MethodInfo* contains_key_method = resolve_method_in_hierarchy(dictionary->klass, "ContainsKey", 1);
        if (contains_key_method == nullptr || contains_key_method->methodPointer == nullptr)
        {
            return false;
        }

        bool result = false;
        void* args[] = { key };
        if (contains_key_method->invoker_method != nullptr)
        {
            contains_key_method->invoker_method(
                reinterpret_cast<Il2CppMethodPointer>(contains_key_method->methodPointer),
                contains_key_method,
                dictionary,
                args,
                &result);
            return result;
        }

        using contains_key_sig = bool(*)(Il2CppObject*, Il2CppString*);
        return reinterpret_cast<contains_key_sig>(contains_key_method->methodPointer)(dictionary, key);
    }

    inline int dictionary_find_entry(Il2CppObject* dictionary, Il2CppString* key)
    {
        if (dictionary == nullptr || dictionary->klass == nullptr || key == nullptr)
        {
            return -1;
        }

        const MethodInfo* find_entry_method = resolve_method_in_hierarchy(dictionary->klass, "FindEntry", 1);
        if (find_entry_method == nullptr || find_entry_method->methodPointer == nullptr)
        {
            return -1;
        }

        int result = -1;
        void* args[] = { key };
        if (find_entry_method->invoker_method != nullptr)
        {
            find_entry_method->invoker_method(
                reinterpret_cast<Il2CppMethodPointer>(find_entry_method->methodPointer),
                find_entry_method,
                dictionary,
                args,
                &result);
            return result;
        }

        using find_entry_sig = int(*)(Il2CppObject*, Il2CppString*);
        return reinterpret_cast<find_entry_sig>(find_entry_method->methodPointer)(dictionary, key);
    }

    inline Il2CppObject* dictionary_try_get_value(Il2CppObject* dictionary, Il2CppString* key)
    {
        if (dictionary == nullptr || dictionary->klass == nullptr || key == nullptr)
        {
            return nullptr;
        }

        const MethodInfo* try_get_value_method = resolve_method_in_hierarchy(dictionary->klass, "TryGetValue", 2);
        if (try_get_value_method == nullptr || try_get_value_method->methodPointer == nullptr)
        {
            logger::warn("[DictionaryDump] TryGetValue method missing for key='" + safe_string(key) + "'");
            return nullptr;
        }

        Il2CppObject* result = nullptr;
        bool found = false;
        void* args[] = { key, &result };
        if (try_get_value_method->invoker_method != nullptr)
        {
            try_get_value_method->invoker_method(
                reinterpret_cast<Il2CppMethodPointer>(try_get_value_method->methodPointer),
                try_get_value_method,
                dictionary,
                args,
                &found);
        }
        else
        {
            using try_get_value_sig = bool(*)(Il2CppObject*, Il2CppString*, Il2CppObject**);
            found = reinterpret_cast<try_get_value_sig>(
                try_get_value_method->methodPointer)(dictionary, key, &result);
        }

        logger::info("[DictionaryDump] TryGetValue key='" + safe_string(key) +
            "' found=" + std::to_string(found) +
            " value_class=" + (result != nullptr ? class_name(result->klass) : std::string("null")));
        return found ? result : nullptr;
    }

    inline Il2CppObject* dictionary_get_item(Il2CppObject* dictionary, Il2CppString* key)
    {
        if (dictionary == nullptr || dictionary->klass == nullptr || key == nullptr)
        {
            return nullptr;
        }

        const bool contains_key = dictionary_contains_key(dictionary, key);
        logger::info("[DictionaryDump] ContainsKey key='" + safe_string(key) +
            "' found=" + std::to_string(contains_key));
        if (!contains_key)
        {
            return nullptr;
        }

        const MethodInfo* get_item_method = resolve_method_in_hierarchy(dictionary->klass, "get_Item", 1);
        if (get_item_method == nullptr || get_item_method->methodPointer == nullptr)
        {
            return nullptr;
        }

        Il2CppObject* result = nullptr;
        void* args[] = { key };
        if (get_item_method->invoker_method != nullptr)
        {
            get_item_method->invoker_method(
                reinterpret_cast<Il2CppMethodPointer>(get_item_method->methodPointer),
                get_item_method,
                dictionary,
                args,
                &result);
            return result;
        }

        using get_item_sig = Il2CppObject * (*)(Il2CppObject*, Il2CppString*);
        return reinterpret_cast<get_item_sig>(get_item_method->methodPointer)(dictionary, key);
    }

    inline bool dictionary_set_item(Il2CppObject* dictionary, Il2CppString* key, Il2CppObject* value)
    {
        if (dictionary == nullptr || dictionary->klass == nullptr || key == nullptr || value == nullptr)
        {
            return false;
        }

        const MethodInfo* set_item_method = resolve_method_in_hierarchy(dictionary->klass, "set_Item", 2);
        if (set_item_method == nullptr || set_item_method->methodPointer == nullptr)
        {
            logger::warn("[DictionaryDump] set_Item method missing for key='" + safe_string(key) + "'");
            return false;
        }

        const int before_count = dictionary_count(dictionary);
        void* args[] = { key, value };
        if (set_item_method->invoker_method != nullptr)
        {
            set_item_method->invoker_method(
                reinterpret_cast<Il2CppMethodPointer>(set_item_method->methodPointer),
                set_item_method,
                dictionary,
                args,
                nullptr);
        }
        else
        {
            using set_item_sig = void(*)(Il2CppObject*, Il2CppString*, Il2CppObject*);
            reinterpret_cast<set_item_sig>(set_item_method->methodPointer)(dictionary, key, value);
        }

        const int after_count = dictionary_count(dictionary);
        logger::info("[DictionaryDump] SetItem key='" + safe_string(key) +
            "' value_class=" + class_name(value->klass) +
            " count=" + std::to_string(before_count) + "->" + std::to_string(after_count));
        return dictionary_try_get_value(dictionary, key) == value;
    }

    inline int dictionary_count(Il2CppObject* dictionary)
    {
        if (dictionary == nullptr || dictionary->klass == nullptr)
        {
            return -1;
        }

        const MethodInfo* count_getter = resolve_method_in_hierarchy(dictionary->klass, "get_Count", 0);
        return invoke_int_method0(dictionary, count_getter);
    }

    inline std::vector<std::string> get_string_dictionary_keys(Il2CppObject* dictionary, const std::string& label)
    {
        std::vector<std::string> keys;
        if (dictionary == nullptr || dictionary->klass == nullptr)
        {
            return keys;
        }

        const int count = dictionary_count(dictionary);
        logger::info("[DictionaryDump] " + label + " class=" + class_name(dictionary->klass) +
            " count=" + std::to_string(count));
        dump_object_class_and_values_once(
            dictionary,
            label + "Dictionary",
            dumped_locations_dictionary_class);

        logger::warn("[DictionaryDump] " + label +
            " managed Keys.CopyTo dump disabled because it can crash during Ready");

        return keys;
    }

    inline Il2CppObject* get_location_from_settings_dictionary(
        Il2CppObject* location_settings,
        const std::string& location_id)
    {
        if (location_settings == nullptr || location_settings->klass == nullptr || location_id.empty())
        {
            return nullptr;
        }

        Il2CppObject* locations = get_object_field(location_settings, "locations");
        if (locations == nullptr)
        {
            return nullptr;
        }

        std::vector<std::string> keys;
        if (!dumped_location_dictionary_keys)
        {
            dumped_location_dictionary_keys = true;
            keys = get_string_dictionary_keys(locations, "LocationSettings.locations");
        }

        std::string exact_key = location_id;
        if (!keys.empty())
        {
            const std::string lower_location_id = lower_copy(location_id);
            for (const std::string& key : keys)
            {
                if (lower_copy(key) == lower_location_id)
                {
                    exact_key = key;
                    break;
                }
            }
        }

        Il2CppString* key = il2cpp::il2cpp_string_new(exact_key.c_str());
        const int entry_index = dictionary_find_entry(locations, key);
        logger::info("[DictionaryDump] FindEntry key='" + exact_key +
            "' index=" + std::to_string(entry_index));

        Il2CppObject* location = dictionary_try_get_value(locations, key);
        if (location == nullptr)
        {
            location = dictionary_get_item(locations, key);
        }
        if (location != nullptr)
        {
            logger::info("[BackendSession] found location settings dictionary entry for '" +
                location_id + "' exact_key='" + exact_key + "' class=" + class_name(location->klass));
        }
        else
        {
            logger::warn("[BackendSession] dictionary lookup missed location '" + location_id +
                "' exact_key='" + exact_key + "'");
        }

        return location;
    }

    inline bool selected_location_object_matches_id(
        Il2CppObject* location,
        const std::string& location_id)
    {
        if (location == nullptr || location->klass == nullptr ||
            location_id.empty())
        {
            return false;
        }

        const std::string lower_location_id = lower_copy(location_id);
        return lower_copy(
                safe_string(get_string_field(location, "Id"))) ==
                    lower_location_id ||
            lower_copy(
                safe_string(get_string_field(location, "_Id"))) ==
                    lower_location_id ||
            lower_copy(
                safe_string(get_string_field(location, "Name"))) ==
                    lower_location_id;
    }

    inline Il2CppObject* resolve_selected_location_object(
        Il2CppObject* location_settings)
    {
        if (selected_forced_location_object == nullptr ||
            selected_forced_location_id.empty())
        {
            return nullptr;
        }

        if (lower_copy(selected_forced_location_id) != labs_dark_id ||
            selected_location_object_matches_id(
                selected_forced_location_object,
                selected_forced_location_id))
        {
            return selected_forced_location_object;
        }

        Il2CppObject* dark_location =
            get_location_from_settings_dictionary(
                location_settings,
                labs_dark_id);
        if (dark_location == nullptr ||
            !selected_location_object_matches_id(
                dark_location,
                labs_dark_id))
        {
            logger::warn(
                "[RaidSettings] Offline Labs Blackout could not resolve "
                "the live 'laboratory_dark' location object");
            return nullptr;
        }

        selected_forced_location_object = dark_location;
        selected_forced_location_from_settings = dark_location;
        logger::info(
            "[RaidSettings] Offline Labs Blackout resolved live "
            "location 'laboratory_dark'");
        return dark_location;
    }

    inline Il2CppObject* find_backend_safe_location_in_settings(Il2CppObject* location_settings)
    {
        if (location_settings == nullptr || location_settings->klass == nullptr)
        {
            return nullptr;
        }

        Il2CppObject* locations = get_object_field(location_settings, "locations");
        Il2CppArray* entries = reinterpret_cast<Il2CppArray*>(
            get_object_field(locations, "_entries"));
        const int count = get_int_field_or(locations, "_count", 0);
        if (entries == nullptr || entries->klass == nullptr || count <= 0 ||
            entries->klass->element_size != sizeof(LocationDictionaryEntry))
        {
            logger::warn("[BackendSession] cannot inspect LocationSettings dictionary entries safely");
            return nullptr;
        }

        const size_t entry_count = std::min<size_t>(
            static_cast<size_t>(count),
            static_cast<size_t>(entries->max_length));
        LocationDictionaryEntry* dictionary_entries =
            reinterpret_cast<LocationDictionaryEntry*>(&entries->data);
        const char* preferred_ids[] = {
            "bigmap",
            "Sandbox",
            "TarkovStreets",
            "Woods",
            "Shoreline",
            "Interchange",
            "Lighthouse",
            "RezervBase",
            backend_safe_location_id
        };

        for (const char* preferred_id : preferred_ids)
        {
            const std::string lower_preferred_id = lower_copy(preferred_id);
            for (size_t index = 0; index < entry_count; ++index)
            {
                Il2CppObject* location = dictionary_entries[index].value;
                if (location == nullptr || location->klass == nullptr ||
                    should_force_display_location_object(location))
                {
                    continue;
                }

                const bool matches =
                    lower_copy(safe_string(get_string_field(location, "Id"))) == lower_preferred_id ||
                    lower_copy(safe_string(get_string_field(location, "_Id"))) == lower_preferred_id ||
                    lower_copy(safe_string(get_string_field(location, "Name"))) == lower_preferred_id;
                if (!matches)
                {
                    continue;
                }

                logger::info("[BackendSession] found backend-safe location from dictionary entry key='" +
                    safe_string(dictionary_entries[index].key) + "' id='" +
                    get_location_identifier(location) + "'");
                return location;
            }
        }

        for (size_t index = 0; index < entry_count; ++index)
        {
            Il2CppObject* location = dictionary_entries[index].value;
            if (location == nullptr || location->klass == nullptr ||
                should_force_display_location_object(location) ||
                !get_bool_field_or(location, "Enabled", false) ||
                get_location_identifier(location).empty())
            {
                continue;
            }

            logger::info("[BackendSession] using first enabled backend-safe dictionary entry key='" +
                safe_string(dictionary_entries[index].key) + "' id='" +
                get_location_identifier(location) + "'");
            return location;
        }

        return nullptr;
    }

    inline Il2CppObject* resolve_active_backend_safe_location(Il2CppObject* backend_session)
    {
        if (active_backend_safe_location_object != nullptr &&
            !active_backend_safe_location_id.empty())
        {
            return active_backend_safe_location_object;
        }

        Il2CppObject* backend_location_settings = get_object_field(
            backend_session,
            "<LocationSettings>k__BackingField");
        Il2CppObject* settings_sources[] = {
            selected_forced_location_settings,
            backend_location_settings
        };
        const char* candidate_ids[] = {
            "bigmap",
            "Sandbox",
            "TarkovStreets",
            "Woods",
            "Shoreline",
            "Interchange",
            "Lighthouse",
            "RezervBase",
            backend_safe_location_id
        };

        for (Il2CppObject* location_settings : settings_sources)
        {
            if (location_settings == nullptr)
            {
                continue;
            }

            Il2CppObject* location = find_backend_safe_location_in_settings(location_settings);
            if (location != nullptr)
            {
                active_backend_safe_location_object = location;
                active_backend_safe_location_id = get_location_identifier(location);
                logger::info("[BackendSession] selected available backend-safe location '" +
                    active_backend_safe_location_id + "' for Icebreaker admission");
                return active_backend_safe_location_object;
            }

            for (const char* candidate_id : candidate_ids)
            {
                location = get_location_from_settings_dictionary(
                    location_settings,
                    candidate_id);
                if (location == nullptr || should_force_location_offline_object(location))
                {
                    continue;
                }

                active_backend_safe_location_object = location;
                active_backend_safe_location_id = get_location_identifier(location);
                if (active_backend_safe_location_id.empty())
                {
                    active_backend_safe_location_id = candidate_id;
                }

                logger::info("[BackendSession] selected available backend-safe location '" +
                    active_backend_safe_location_id + "' for Icebreaker admission");
                return active_backend_safe_location_object;
            }
        }

        logger::warn("[BackendSession] no ordinary backend-safe location exists in current LocationSettings");
        return nullptr;
    }

    inline bool insert_selected_location_into_settings(
        Il2CppObject* location_settings,
        const std::string& label)
    {
        if (location_settings == nullptr || location_settings->klass == nullptr ||
            selected_forced_location_object == nullptr || selected_forced_location_id.empty())
        {
            return false;
        }

        Il2CppObject* selected_location =
            resolve_selected_location_object(location_settings);
        if (selected_location == nullptr)
        {
            return false;
        }

        Il2CppObject* locations = get_object_field(location_settings, "locations");
        if (locations == nullptr || locations->klass == nullptr)
        {
            logger::warn("[DictionaryDump] " + label + ".locations missing while inserting '" +
                selected_forced_location_id + "'");
            return false;
        }

        Il2CppString* key = il2cpp::il2cpp_string_new(selected_forced_location_id.c_str());
        const bool inserted =
            dictionary_set_item(locations, key, selected_location);
        logger::info("[DictionaryDump] " + label + ".locations insert selected '" +
            selected_forced_location_id + "' ok=" + std::to_string(inserted));
        if (inserted)
        {
            selected_forced_location_inserted_into_settings = true;
            selected_forced_location_from_settings = selected_location;
        }

        return inserted;
    }

    inline bool object_location_matches_id(Il2CppObject* object, const std::string& location_id)
    {
        if (object == nullptr || object->klass == nullptr || location_id.empty())
        {
            return false;
        }

        const std::string lower_location_id = lower_copy(location_id);
        return lower_copy(safe_string(get_string_field(object, "Id"))) == lower_location_id ||
            lower_copy(safe_string(get_string_field(object, "_Id"))) == lower_location_id ||
            lower_copy(safe_string(get_string_field(object, "Name"))) == lower_location_id;
    }

    inline Il2CppObject* find_location_in_tree(
        Il2CppObject* object,
        const std::string& location_id,
        std::unordered_set<uintptr_t>& visited,
        const int depth = 0)
    {
        if (object == nullptr || object->klass == nullptr || depth > 5)
        {
            return nullptr;
        }

        const uintptr_t address = reinterpret_cast<uintptr_t>(object);
        if (visited.contains(address))
        {
            return nullptr;
        }
        visited.insert(address);

        if (object_location_matches_id(object, location_id))
        {
            return object;
        }

        if (object->klass->fields == nullptr)
        {
            return nullptr;
        }

        for (uint16_t index = 0; index < object->klass->field_count; ++index)
        {
            const FieldInfo& field = object->klass->fields[index];
            if (field.offset <= 0 || field.type == nullptr || field.type->type == IL2CPP_TYPE_STRING ||
                !is_object_reference_type(field.type))
            {
                continue;
            }

            Il2CppObject* child = get_reference_field_value(object, field);
            if (child == nullptr || child->klass == nullptr)
            {
                continue;
            }

            if (field.type->type == IL2CPP_TYPE_ARRAY || field.type->type == IL2CPP_TYPE_SZARRAY)
            {
                const Il2CppType* element_type = get_array_element_type(field.type);
                if (element_type == nullptr || element_type->valuetype ||
                    (element_type->type != IL2CPP_TYPE_CLASS &&
                        element_type->type != IL2CPP_TYPE_OBJECT &&
                        element_type->type != IL2CPP_TYPE_GENERICINST))
                {
                    continue;
                }

                Il2CppArray* array = reinterpret_cast<Il2CppArray*>(child);
                const size_t max_count = std::min<size_t>(static_cast<size_t>(array->max_length), 512);
                Il2CppObject** elements = reinterpret_cast<Il2CppObject**>(&array->data);
                for (size_t element_index = 0; element_index < max_count; ++element_index)
                {
                    Il2CppObject* element = elements[element_index];
                    Il2CppObject* found = find_location_in_tree(
                        element,
                        location_id,
                        visited,
                        depth + 1);
                    if (found != nullptr)
                    {
                        return found;
                    }
                }

                continue;
            }

            Il2CppObject* found = find_location_in_tree(child, location_id, visited, depth + 1);
            if (found != nullptr)
            {
                return found;
            }
        }

        return nullptr;
    }

    inline Il2CppObject* find_cached_location_object(
        Il2CppObject* backend_session,
        Il2CppObject* raid_settings,
        const std::string& location_id)
    {
        if (selected_forced_location_from_settings != nullptr)
        {
            return selected_forced_location_from_settings;
        }

        Il2CppObject* raid_location_settings = get_object_field(raid_settings, "_locationSettings");
        dump_object_class_and_values_once(
            raid_location_settings,
            "RaidLocationSettings",
            dumped_location_settings_class);

        std::unordered_set<uintptr_t> visited;
        Il2CppObject* found = get_location_from_settings_dictionary(raid_location_settings, location_id);
        if (found == nullptr)
        {
            found = find_location_in_tree(raid_location_settings, location_id, visited);
        }
        if (found != nullptr)
        {
            logger::info("[BackendSession] found cached location '" + location_id +
                "' from raid settings class=" + class_name(found->klass));
            return found;
        }

        Il2CppObject* backend_location_settings = get_object_field(backend_session, "<LocationSettings>k__BackingField");
        visited.clear();
        found = get_location_from_settings_dictionary(backend_location_settings, location_id);
        if (found == nullptr)
        {
            found = find_location_in_tree(backend_location_settings, location_id, visited);
        }
        if (found != nullptr)
        {
            logger::info("[BackendSession] found cached location '" + location_id +
                "' from backend settings class=" + class_name(found->klass));
            return found;
        }

        if (selected_forced_location_object != nullptr)
        {
            logger::warn("[BackendSession] using selected menu location fallback for '" +
                location_id + "' class=" + class_name(selected_forced_location_object->klass));
        }

        return selected_forced_location_object;
    }

    inline void cache_forced_location_settings(Il2CppObject* raid_settings)
    {
        if (!selected_location_forces_offline || selected_forced_location_id.empty() ||
            raid_settings == nullptr || raid_settings->klass == nullptr)
        {
            return;
        }

        Il2CppObject* location_settings = get_object_field(raid_settings, "_locationSettings");
        if (location_settings == nullptr)
        {
            return;
        }

        selected_forced_location_settings = location_settings;
        dump_object_class_and_values_once(
            selected_forced_location_settings,
            "RaidLocationSettings",
            dumped_location_settings_class);

        if (resolve_selected_location_object(
                selected_forced_location_settings) == nullptr)
        {
            return;
        }

        insert_selected_location_into_settings(
            selected_forced_location_settings,
            "RaidLocationSettings");

        if (!dumped_forced_location_snapshot)
        {
            dump_location_snapshot(selected_forced_location_object, "SelectedMenuLocation");
        }

        std::unordered_set<uintptr_t> visited;
        selected_forced_location_from_settings = get_location_from_settings_dictionary(
            selected_forced_location_settings,
            selected_forced_location_id);
        if (selected_forced_location_from_settings == nullptr)
        {
            selected_forced_location_from_settings = find_location_in_tree(
                selected_forced_location_settings,
                selected_forced_location_id,
                visited);
        }

        if (selected_forced_location_from_settings != nullptr)
        {
            logger::info("[BackendSession] cached location settings entry for '" +
                selected_forced_location_id + "' class=" +
                class_name(selected_forced_location_from_settings->klass));
            if (!dumped_forced_location_snapshot)
            {
                dumped_forced_location_snapshot = true;
                dump_location_snapshot(selected_forced_location_from_settings, "CachedSettingsLocation");
            }
        }
        else
        {
            logger::warn("[BackendSession] could not find '" + selected_forced_location_id +
                "' inside cached RaidSettings._locationSettings");
            dumped_forced_location_snapshot = true;
        }
    }

    inline Il2CppObject* create_default_reference_value_for_field(const FieldInfo& field)
    {
        if (field.type == nullptr)
        {
            return nullptr;
        }

        const std::string field_name = field.name ? field.name : "";
        const std::string field_type_name = type_name(field.type);

        if (field_name == "locationLoot" && selected_forced_location_object != nullptr)
        {
            return selected_forced_location_object;
        }

        if (field.type->type == IL2CPP_TYPE_ARRAY || field.type->type == IL2CPP_TYPE_SZARRAY)
        {
            return create_empty_array_from_type(field.type);
        }

        Il2CppClass* field_class = resolve_class_from_type_for_allocation(field.type);
        if (field_class == nullptr)
        {
            return nullptr;
        }

        Il2CppObject* value = il2cpp::il2cpp_object_new(field_class);
        if (value == nullptr)
        {
            return nullptr;
        }

        call_parameterless_constructor_if_exists(value);
        return value;
    }

    inline bool initialize_local_settings_reference_fields(Il2CppObject* result, const int depth = 1)
    {
        if (result == nullptr || result->klass == nullptr || result->klass->fields == nullptr)
        {
            return false;
        }

        bool changed = false;
        for (uint16_t index = 0; index < result->klass->field_count; ++index)
        {
            const FieldInfo& field = result->klass->fields[index];
            if (field.type == nullptr || !is_object_reference_type(field.type) ||
                field.type->type == IL2CPP_TYPE_STRING)
            {
                continue;
            }

            if (get_reference_field_value(result, field) != nullptr)
            {
                continue;
            }

            Il2CppObject* value = create_default_reference_value_for_field(field);
            if (value == nullptr)
            {
                logger::warn("[BackendSession] failed to create LocalRaidStarted result field '" +
                    std::string(field.name ? field.name : "<null>") +
                    "' type='" + type_name(field.type) + "'");
                continue;
            }

            if (set_reference_field_value(result, field, value))
            {
                changed = true;
                logger::info("[BackendSession] initialized LocalRaidStarted result field '" +
                    std::string(field.name ? field.name : "<null>") +
                    "' type='" + type_name(field.type) +
                    "' value_class=" + class_name(value->klass));

                if (depth > 0 && value != selected_forced_location_object &&
                    field.type->type != IL2CPP_TYPE_ARRAY && field.type->type != IL2CPP_TYPE_SZARRAY)
                {
                    changed |= initialize_local_settings_reference_fields(value, depth - 1);
                }
            }
        }

        return changed;
    }

    inline void dump_object_summary(Il2CppObject* object, const std::string& label)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            logger::info("[ObjectSummary] " + label + " null");
            return;
        }

        logger::info("[ObjectSummary] " + label + " class=" + class_name(object->klass) +
            " Id='" + safe_string(get_string_field(object, "Id")) +
            "' _Id='" + safe_string(get_string_field(object, "_Id")) +
            "' Name='" + safe_string(get_string_field(object, "Name")) + "'");
    }

    inline void dump_array_summary(Il2CppObject* object, const char* field_name, const std::string& label)
    {
        Il2CppObject* array_object = get_object_field(object, field_name);
        if (array_object == nullptr)
        {
            logger::info("[ArraySummary] " + label + "." + field_name + " null");
            return;
        }

        Il2CppArray* array = reinterpret_cast<Il2CppArray*>(array_object);
        logger::info("[ArraySummary] " + label + "." + field_name +
            " class=" + class_name(array_object->klass) +
            " length=" + std::to_string(static_cast<size_t>(array->max_length)));
    }

    inline void dump_location_snapshot(Il2CppObject* location, const std::string& label)
    {
        dump_object_summary(location, label);
        if (location == nullptr || location->klass == nullptr)
        {
            return;
        }

        logger::info("[LocationSnapshot] " + label +
            " Enabled=" + std::to_string(get_bool_field_or(location, "Enabled", false)) +
            " ForceOnlineRaidInPVE=" + std::to_string(get_bool_field_or(location, "ForceOnlineRaidInPVE", false)) +
            " ForceOfflineRaidInPVE=" + std::to_string(get_bool_field_or(location, "ForceOfflineRaidInPVE", false)) +
            " EscapeTimeLimit=" + std::to_string(get_int_field_or(location, "EscapeTimeLimit", -1)) +
            " MinPlayers=" + std::to_string(get_int_field_or(location, "MinPlayers", -1)) +
            " MaxPlayers=" + std::to_string(get_int_field_or(location, "MaxPlayers", -1)) +
            " BotMax=" + std::to_string(get_int_field_or(location, "BotMax", -1)) +
            " OpenZones='" + safe_string(get_string_field(location, "OpenZones")) + "'");

        dump_object_summary(get_object_field(location, "Scene"), label + ".Scene");
        dump_object_summary(get_object_field(location, "Preview"), label + ".Preview");
        Il2CppObject* loot = get_object_field(location, "Loot");
        dump_object_summary(loot, label + ".Loot");
        if (label.find("LocalRaidStartedLocationLoot") != std::string::npos ||
            label.find("LocalGameCreate") != std::string::npos)
        {
            dump_object_field_values(loot, label + ".Loot");
        }
        dump_object_summary(get_object_field(location, "BotLocationModifier"), label + ".BotLocationModifier");
        dump_array_summary(location, "SpawnPointParams", label);
        dump_array_summary(location, "exits", label);
        dump_array_summary(location, "SecretExits", label);
        dump_array_summary(location, "waves", label);
        dump_array_summary(location, "BossLocationSpawn", label);
        dump_array_summary(location, "transitParameters", label);

        Il2CppObject* containers = get_object_field(location, "containers");
        if (containers != nullptr)
        {
            logger::info("[DictionaryDump] " + label + ".containers class=" +
                class_name(containers->klass) + " count=" +
                std::to_string(dictionary_count(containers)));
        }
    }

    inline void dump_icebreaker_container_registration_schema(
        Il2CppObject* generated_location,
        Il2CppObject* selected_location)
    {
        if (dumped_icebreaker_container_registration_schema ||
            !selected_forced_location_is_icebreaker())
        {
            return;
        }

        Il2CppObject* source_dictionary = get_object_field(
            generated_location,
            "containers");
        Il2CppObject* target_dictionary = get_object_field(
            selected_location,
            "containers");
        if (source_dictionary == nullptr || source_dictionary->klass == nullptr)
        {
            logger::warn("[IcebreakerContainerSchema] backend-safe source dictionary is unavailable");
            return;
        }

        dumped_icebreaker_container_registration_schema = true;
        Il2CppArray* entries = reinterpret_cast<Il2CppArray*>(
            get_object_field(source_dictionary, "_entries"));
        const int source_count = dictionary_count(source_dictionary);
        logger::info("[IcebreakerContainerSchema] source_class='" +
            class_name(source_dictionary->klass) +
            "' source_count=" + std::to_string(source_count) +
            " target=" +
            (target_dictionary != nullptr && target_dictionary->klass != nullptr ?
                "class='" + class_name(target_dictionary->klass) + "' count=" +
                    std::to_string(dictionary_count(target_dictionary)) :
                std::string("null")) +
            " entries_length=" +
            std::to_string(entries != nullptr ? entries->max_length : 0) +
            " entry_size=" +
            std::to_string(entries != nullptr && entries->klass != nullptr ?
                entries->klass->element_size : 0));

        dump_class_layout(
            source_dictionary->klass,
            "IcebreakerContainerDictionarySchema");
        if (entries == nullptr || entries->klass == nullptr || source_count <= 0 ||
            entries->klass->element_size != sizeof(LocationDictionaryEntry))
        {
            logger::warn("[IcebreakerContainerSchema] source entries cannot be inspected safely");
            return;
        }

        LocationDictionaryEntry* source_entries =
            reinterpret_cast<LocationDictionaryEntry*>(&entries->data);
        const size_t entry_count = std::min<size_t>(
            static_cast<size_t>(source_count),
            static_cast<size_t>(entries->max_length));
        for (size_t entry_index = 0; entry_index < entry_count; ++entry_index)
        {
            const LocationDictionaryEntry& entry = source_entries[entry_index];
            if (entry.hash_code < 0 || entry.key == nullptr ||
                entry.value == nullptr || entry.value->klass == nullptr)
            {
                continue;
            }

            logger::info("[IcebreakerContainerSchema] sample_index=" +
                std::to_string(entry_index) +
                " key='" + safe_string(entry.key) +
                "' value_class='" + class_name(entry.value->klass) + "'");
            dump_class_layout(
                entry.value->klass,
                "IcebreakerContainerRegistrationSchema");
            dump_object_field_values(
                entry.value,
                "IcebreakerContainerRegistrationSchema");
            return;
        }

        logger::warn("[IcebreakerContainerSchema] no populated source entry was found");
    }

    inline void populate_local_raid_started_result(
        Il2CppObject* result,
        Il2CppObject* backend_session,
        Il2CppObject* raid_settings)
    {
        if (result == nullptr || result->klass == nullptr)
        {
            return;
        }

        bool changed = false;
        changed |= set_string_field_if_exists(result, "location", selected_forced_location_id);
        changed |= set_string_field_if_exists(result, "Location", selected_forced_location_id);
        changed |= set_string_field_if_exists(result, "locationId", selected_forced_location_id);
        changed |= set_string_field_if_exists(result, "LocationId", selected_forced_location_id);

        Il2CppObject* selected_location = find_cached_location_object(
            backend_session,
            raid_settings,
            selected_forced_location_id);
        if (selected_location == nullptr)
        {
            selected_location = get_object_field(raid_settings, "selectedLocation");
        }
        if (selected_location == nullptr)
        {
            selected_location = get_object_field(raid_settings, "_selectedLocation");
        }

        dump_location_snapshot(selected_location, "LocalRaidStartedSelectedLocation");

        changed |= set_reference_field_if_type_contains(
            result, "location", selected_location, "LocationSettings.Location");
        changed |= set_reference_field_if_type_contains(
            result, "Location", selected_location, "LocationSettings.Location");
        changed |= set_reference_field_if_type_contains(
            result, "selectedLocation", selected_location, "LocationSettings.Location");
        changed |= set_reference_field_if_type_contains(
            result, "SelectedLocation", selected_location, "LocationSettings.Location");
        changed |= set_reference_field_if_type_contains(
            result, "locationLoot", selected_location, "LocationSettings.Location");
        changed |= set_reference_field_if_type_contains(
            result, "LocationLoot", selected_location, "LocationSettings.Location");

        Il2CppObject* transition_settings = get_object_field(raid_settings, "transition");
        changed |= set_reference_field_if_type_contains(
            result, "transitionSettings", transition_settings, "TransitSettings");
        changed |= set_reference_field_if_type_contains(
            result, "TransitionSettings", transition_settings, "TransitSettings");

        Il2CppObject* location_settings = get_object_field(backend_session, "<LocationSettings>k__BackingField");
        Il2CppObject* weather = get_object_field(backend_session, "<Weather>k__BackingField");
        changed |= set_reference_field_if_type_contains(
            result, "LocationSettings", location_settings, "LocationSettings");
        changed |= set_reference_field_if_type_contains(
            result, "locationSettings", location_settings, "LocationSettings");
        changed |= set_reference_field_if_type_contains(result, "Weather", weather, "Weather");
        changed |= set_reference_field_if_type_contains(result, "weather", weather, "Weather");

        changed |= copy_string_field_if_exists(result, backend_session, "<LocationTime>k__BackingField");
        changed |= copy_string_field_if_empty(result, raid_settings, "serverId");
        changed |= initialize_local_settings_reference_fields(result, 1);

        if (changed)
        {
            logger::info("[BackendSession] populated LocalRaidStarted result for '" +
                selected_forced_location_id + "'");
        }
        else
        {
            logger::warn("[BackendSession] LocalRaidStarted result had no recognized fields to populate");
        }

        dump_object_field_values(result, "LocalRaidStartedResult");
        dump_object_class_once(
            get_object_field(result, "settings"),
            "LocalRaidStartedSettings",
            dumped_local_raid_started_settings_class);
        dump_object_class_once(
            get_object_field(result, "profileInsurance"),
            "LocalRaidStartedProfileInsurance",
            dumped_local_raid_started_profile_insurance_class);
        dump_object_class_once(
            get_object_field(result, "locationLoot"),
            "LocalRaidStartedLocationLoot",
            dumped_local_raid_started_location_loot_class);
        dump_object_class_once(
            get_object_field(result, "transitionSettings"),
            "LocalRaidStartedTransitionSettings",
            dumped_local_raid_started_transition_settings_class);
    }

    inline Il2CppObject* create_local_raid_started_result(Il2CppObject* backend_session, Il2CppObject* raid_settings)
    {
        Il2CppClass* result_class = get_local_raid_started_result_class();
        if (result_class == nullptr)
        {
            return nullptr;
        }

        Il2CppObject* result = il2cpp::il2cpp_object_new(result_class);
        if (result == nullptr)
        {
            logger::warn("[BackendSession] failed to allocate LocalRaidStarted result");
            return nullptr;
        }

        call_parameterless_constructor_if_exists(result);
        populate_local_raid_started_result(result, backend_session, raid_settings);
        logger::info("[BackendSession] created LocalRaidStarted result class=" + class_name(result_class));
        return result;
    }

    inline Il2CppObject* create_completed_local_raid_started_task(
        Il2CppObject* backend_session,
        Il2CppObject* raid_settings)
    {
        Il2CppClass* task_class = get_local_raid_started_return_task_class();
        if (task_class == nullptr)
        {
            return nullptr;
        }

        std::string parameter_type_name;
        const MethodInfo* constructor = find_result_task_constructor(task_class, parameter_type_name);
        if (constructor == nullptr || constructor->methodPointer == nullptr)
        {
            logger::warn("[BackendSession] failed to resolve LocalRaidStarted result task constructor");
            return nullptr;
        }

        Il2CppObject* result = create_local_raid_started_result(backend_session, raid_settings);
        if (result == nullptr)
        {
            return nullptr;
        }

        Il2CppObject* task = il2cpp::il2cpp_object_new(task_class);
        if (task == nullptr)
        {
            logger::warn("[BackendSession] failed to allocate LocalRaidStarted result task");
            return nullptr;
        }

        void* args[] = { result };
        if (constructor->invoker_method != nullptr)
        {
            constructor->invoker_method(
                reinterpret_cast<Il2CppMethodPointer>(constructor->methodPointer), constructor, task, args, nullptr);
        }
        else
        {
            using result_constructor_sig = void(*)(Il2CppObject*, Il2CppObject*);
            reinterpret_cast<result_constructor_sig>(constructor->methodPointer)(task, result);
        }

        logger::info("[BackendSession] created LocalRaidStarted completed result task class=" +
            class_name(task_class) + " constructor_param='" + parameter_type_name + "'");
        return task;
    }

    inline bool task_is_completed(Il2CppObject* task)
    {
        if (task == nullptr || task->klass == nullptr)
        {
            return false;
        }

        const MethodInfo* is_completed_getter = resolve_method_in_hierarchy(task->klass, "get_IsCompleted", 0);
        if (is_completed_getter == nullptr || is_completed_getter->methodPointer == nullptr)
        {
            return false;
        }

        using is_completed_sig = bool(*)(Il2CppObject*);
        return reinterpret_cast<is_completed_sig>(is_completed_getter->methodPointer)(task);
    }

    inline bool wait_for_task_completion(
        Il2CppObject* task,
        const int timeout_ms,
        const int poll_interval_ms = 50)
    {
        int elapsed_ms = 0;

        while (elapsed_ms <= timeout_ms)
        {
            if (task_is_completed(task))
            {
                return true;
            }

            Sleep(poll_interval_ms);
            elapsed_ms += poll_interval_ms;
        }

        return false;
    }

    inline Il2CppObject* get_task_result_object(Il2CppObject* task)
    {
        if (task == nullptr || task->klass == nullptr)
        {
            return nullptr;
        }

        const MethodInfo* result_getter = resolve_method_in_hierarchy(task->klass, "get_Result", 0);
        if (result_getter == nullptr || result_getter->methodPointer == nullptr)
        {
            logger::warn("[BackendSession] failed to resolve LocalRaidStarted task result getter");
            return nullptr;
        }

        using result_getter_sig = Il2CppObject * (*)(Il2CppObject*);
        return reinterpret_cast<result_getter_sig>(result_getter->methodPointer)(task);
    }

    inline Il2CppObject* get_task_cached_result_object(Il2CppObject* task)
    {
        if (task == nullptr || task->klass == nullptr)
        {
            return nullptr;
        }

        const FieldInfo* result_field = il2utils::resolve_field(task->klass, "m_result");
        if (result_field == nullptr || result_field->offset <= 0 ||
            result_field->type == nullptr || !is_object_reference_type(result_field->type))
        {
            return nullptr;
        }

        return *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<UINT64>(task) + result_field->offset);
    }

    using local_raid_started_original_sig = Il2CppObject * (*)(Il2CppObject*, Il2CppObject*);

    struct LocalRaidStartedPatchContext
    {
        Il2CppObject* backend_session = nullptr;
        Il2CppObject* raid_settings = nullptr;
        Il2CppObject* backend_task = nullptr;
        std::string location_id;
    };

    inline bool patch_backend_safe_local_raid_started_result(
        Il2CppObject* backend_result,
        Il2CppObject* backend_session,
        Il2CppObject* raid_settings,
        const std::string& location_id,
        const char* patch_mode)
    {
        if (backend_result == nullptr || backend_result->klass == nullptr)
        {
            return false;
        }

        if (!dumped_backend_result_before_patch)
        {
            dumped_backend_result_before_patch = true;
            dump_object_field_values(backend_result, "LocalRaidStartedResultBeforePatch");
            dump_location_snapshot(
                get_object_field(backend_result, "locationLoot"),
                "LocalRaidStartedLocationLootBeforePatch");
        }

        Il2CppObject* generated_location = get_object_field(backend_result, "locationLoot");
        Il2CppObject* generated_loot = get_object_field(generated_location, "Loot");
        const int generated_loot_size = get_il2cpp_list_size(generated_loot);

        dump_icebreaker_container_registration_schema(
            generated_location,
            selected_forced_location_object);

        const std::string previous_location_id = selected_forced_location_id;
        selected_forced_location_id = location_id;
        populate_local_raid_started_result(
            backend_result,
            backend_session,
            raid_settings);
        selected_forced_location_id = previous_location_id;

        if (lower_copy(location_id) == ice_breaker_id &&
            generated_loot != nullptr && generated_loot_size > 0)
        {
            Il2CppObject* icebreaker_location = get_object_field(backend_result, "locationLoot");
            if (set_reference_field_if_exists(icebreaker_location, "Loot", generated_loot))
            {
                logger::info("[BackendSession] preserved " +
                    std::to_string(generated_loot_size) +
                    " generated loot records for Icebreaker remapping");
            }
        }

        if (!dumped_backend_result_after_patch)
        {
            dumped_backend_result_after_patch = true;
            dump_object_field_values(backend_result, "LocalRaidStartedResultAfterPatch");
            dump_location_snapshot(
                get_object_field(backend_result, "locationLoot"),
                "LocalRaidStartedLocationLootAfterPatch");
        }

        logger::info("[BackendSession] " + std::string(patch_mode) +
            " patched backend-safe LocalRaidStarted result for '" + location_id + "'");
        return true;
    }

    inline bool should_patch_local_raid_started_result(Il2CppObject* result)
    {
        if (result == nullptr || result->klass == nullptr)
        {
            return false;
        }

        const std::string result_class_name = class_name(result->klass);
        if (result_class_name.find("JsonType.LocalSettings") != std::string::npos)
        {
            return true;
        }

        return il2utils::resolve_field(result->klass, "locationLoot") != nullptr;
    }

    inline Il2CppObject* patch_local_raid_started_task_result(
        Il2CppObject* task,
        Il2CppObject* result,
        const char* patch_mode)
    {
        if (task == nullptr ||
            task != last_backend_safe_local_raid_task ||
            last_backend_safe_local_raid_location_id.empty())
        {
            return result;
        }

        if (!should_patch_local_raid_started_result(result))
        {
            return result;
        }

        const uintptr_t result_address = reinterpret_cast<uintptr_t>(result);
        if (patched_local_raid_started_results.contains(result_address))
        {
            return result;
        }

        const std::string location_id = last_backend_safe_local_raid_location_id;
        if (location_id.empty() || !should_force_location_offline(location_id))
        {
            return result;
        }

        if (!logged_local_raid_started_task_result_patch)
        {
            logged_local_raid_started_task_result_patch = true;
            logger::info("[BackendSession] patching LocalRaidStarted task result from " +
                std::string(patch_mode) + " for '" + location_id + "'");
        }

        if (patch_backend_safe_local_raid_started_result(
            result,
            last_backend_safe_local_raid_session,
            last_backend_safe_local_raid_settings,
            location_id,
            patch_mode))
        {
            patched_local_raid_started_results.insert(result_address);
        }

        return result;
    }

    inline bool patch_backend_safe_local_raid_started_cached_result(const char* patch_mode)
    {
        if (last_backend_safe_local_raid_task == nullptr)
        {
            return false;
        }

        Il2CppObject* backend_result = get_task_cached_result_object(last_backend_safe_local_raid_task);
        if (backend_result == nullptr)
        {
            if (!logged_backend_safe_local_raid_cached_result_missing)
            {
                logged_backend_safe_local_raid_cached_result_missing = true;
                logger::warn("[BackendSession] cached LocalRaidStarted task result is not readable from " +
                    std::string(patch_mode));
            }

            return false;
        }

        patch_local_raid_started_task_result(
            last_backend_safe_local_raid_task,
            backend_result,
            patch_mode);
        return true;
    }

    inline Il2CppObject* get_real_local_raid_started_result_location()
    {
        if (last_real_local_raid_task == nullptr ||
            last_real_local_raid_location_id.empty())
        {
            return nullptr;
        }

        Il2CppObject* real_result = get_task_cached_result_object(last_real_local_raid_task);
        if (real_result == nullptr && wait_for_task_completion(last_real_local_raid_task, 10, 1))
        {
            real_result = get_task_result_object(last_real_local_raid_task);
        }

        if (real_result == nullptr)
        {
            if (!logged_real_local_raid_cached_result_missing)
            {
                logged_real_local_raid_cached_result_missing = true;
                logger::warn("[BackendSession] real LocalRaidStarted task result is not readable yet");
            }

            return nullptr;
        }

        if (!should_patch_local_raid_started_result(real_result))
        {
            logger::warn("[BackendSession] real LocalRaidStarted task result has unexpected class " +
                class_name(real_result->klass));
            return nullptr;
        }

        Il2CppObject* result_location = get_object_field(real_result, "locationLoot");
        const std::string result_location_id = get_location_identifier(result_location);
        const int loot_size = get_location_loot_size(result_location);

        if (!dumped_real_local_raid_started_result)
        {
            dumped_real_local_raid_started_result = true;
            dump_object_field_values(real_result, "RealLocalRaidStartedResult");
            dump_location_snapshot(result_location, "RealLocalRaidStartedLocationLoot");
        }

        logger::info("[BackendSession] real LocalRaidStarted locationLoot id='" +
            result_location_id + "' loot_size=" + std::to_string(loot_size) +
            " wanted='" + last_real_local_raid_location_id + "'");

        if (result_location == nullptr || result_location->klass == nullptr)
        {
            return nullptr;
        }

        if (!should_force_location_offline(result_location_id))
        {
            return nullptr;
        }

        if (loot_size <= 0)
        {
            return nullptr;
        }

        return result_location;
    }

    using local_settings_task_result_sig = Il2CppObject * (*)(Il2CppObject*);
    using local_settings_task_result_core_sig = Il2CppObject * (*)(Il2CppObject*, bool);
    static inline local_settings_task_result_sig o_local_settings_task_get_result = nullptr;
    static inline local_settings_task_result_sig o_local_settings_task_get_result_on_success = nullptr;
    static inline local_settings_task_result_core_sig o_local_settings_task_get_result_core = nullptr;

    inline Il2CppObject* hk_local_settings_task_get_result(Il2CppObject* task)
    {
        Il2CppObject* result = o_local_settings_task_get_result(task);
        return patch_local_raid_started_task_result(task, result, "task-get_Result");
    }

    inline Il2CppObject* hk_local_settings_task_get_result_on_success(Il2CppObject* task)
    {
        Il2CppObject* result = o_local_settings_task_get_result_on_success(task);
        return patch_local_raid_started_task_result(task, result, "task-get_ResultOnSuccess");
    }

    inline Il2CppObject* hk_local_settings_task_get_result_core(Il2CppObject* task, const bool wait_completion_notification)
    {
        Il2CppObject* result = o_local_settings_task_get_result_core(task, wait_completion_notification);
        return patch_local_raid_started_task_result(task, result, "task-GetResultCore");
    }

    inline void hook_local_raid_started_task_result_methods_for_class(
        Il2CppClass* task_class,
        const char* label)
    {
        if (task_class == nullptr)
        {
            return;
        }

        if (o_local_settings_task_get_result == nullptr)
        {
            hook_method_in_hierarchy(
                task_class,
                "get_Result",
                0,
                reinterpret_cast<void*>(&hk_local_settings_task_get_result),
                reinterpret_cast<void**>(&o_local_settings_task_get_result),
                label);
        }

        if (o_local_settings_task_get_result_on_success == nullptr)
        {
            hook_method_in_hierarchy(
                task_class,
                "get_ResultOnSuccess",
                0,
                reinterpret_cast<void*>(&hk_local_settings_task_get_result_on_success),
                reinterpret_cast<void**>(&o_local_settings_task_get_result_on_success),
                label);
        }

        if (o_local_settings_task_get_result_core == nullptr)
        {
            hook_method_in_hierarchy(
                task_class,
                "GetResultCore",
                1,
                reinterpret_cast<void*>(&hk_local_settings_task_get_result_core),
                reinterpret_cast<void**>(&o_local_settings_task_get_result_core),
                label);
        }
    }

    inline void hook_local_raid_started_task_result_methods()
    {
        hook_local_raid_started_task_result_methods_for_class(
            get_local_raid_started_return_task_class(),
            "LocalRaidStartedReturnTask");
    }

    inline void hook_local_raid_started_task_result_methods_for_task(Il2CppObject* task)
    {
        if (task == nullptr || task->klass == nullptr)
        {
            return;
        }

        if (!dumped_backend_safe_local_raid_task_class)
        {
            dumped_backend_safe_local_raid_task_class = true;
            logger::info("[BackendSession] backend-safe LocalRaidStarted task class=" +
                class_name(task->klass));
            dump_class_layout(task->klass, "LocalRaidStartedBackendTask");
        }

        hook_local_raid_started_task_result_methods_for_class(
            task->klass,
            "LocalRaidStartedBackendTask");
    }

    inline DWORD WINAPI patch_backend_safe_local_raid_started_task(LPVOID parameter)
    {
        LocalRaidStartedPatchContext* context = static_cast<LocalRaidStartedPatchContext*>(parameter);
        if (context == nullptr)
        {
            return 0;
        }

        Il2CppThread* attached_thread = il2cpp::il2cpp_thread_attach(il2cpp::il2cpp_domain_get());
        if (attached_thread == nullptr)
        {
            logger::warn("[BackendSession] failed to attach LocalRaidStarted patch thread to IL2CPP");
        }

        if (!wait_for_task_completion(context->backend_task, 30000, 5))
        {
            logger::warn("[BackendSession] backend-safe LocalRaidStarted task did not complete before async patch timeout");
            if (attached_thread != nullptr)
            {
                il2cpp::il2cpp_thread_detach(attached_thread);
            }

            delete context;
            return 0;
        }

        Il2CppObject* backend_result = get_task_result_object(context->backend_task);
        if (backend_result == nullptr)
        {
            logger::warn("[BackendSession] backend-safe LocalRaidStarted completed without readable async result");
            if (attached_thread != nullptr)
            {
                il2cpp::il2cpp_thread_detach(attached_thread);
            }

            delete context;
            return 0;
        }

        patch_backend_safe_local_raid_started_result(
            backend_result,
            context->backend_session,
            context->raid_settings,
            context->location_id,
            "asynchronously");

        if (attached_thread != nullptr)
        {
            il2cpp::il2cpp_thread_detach(attached_thread);
        }

        delete context;
        return 0;
    }

    inline Il2CppObject* create_backend_safe_local_raid_started_task(
        Il2CppObject* backend_session,
        Il2CppObject* raid_settings,
        const LocalRaidStartedSnapshot& snapshot,
        local_raid_started_original_sig original)
    {
        Il2CppObject* backend_location = resolve_active_backend_safe_location(backend_session);
        if (backend_location == nullptr || active_backend_safe_location_id.empty())
        {
            logger::warn("[BackendSession] no backend-safe Icebreaker alias found; preserving original request path");
            return original(backend_session, raid_settings);
        }

        Il2CppObject* backend_raid_settings = clone_local_raid_started_for_backend(
            raid_settings,
            snapshot,
            backend_location,
            active_backend_safe_location_id);
        if (backend_raid_settings == nullptr)
        {
            logger::warn("[BackendSession] forwarding LocalRaidStarted without backend clone for '" +
                selected_forced_location_id + "'");
            return original(backend_session, raid_settings);
        }

        dump_object_class_and_values_once(
            backend_raid_settings, "LocalRaidStartedBackendParams", dumped_local_raid_started_params_class);

        logger::warn("[BackendSession] forwarding LocalRaidStarted with backend-safe clone for '" +
            selected_forced_location_id + "'");

        last_backend_safe_local_raid_session = backend_session;
        last_backend_safe_local_raid_settings = raid_settings;
        last_backend_safe_local_raid_task = nullptr;
        last_backend_safe_local_raid_location_id = selected_forced_location_id;
        logged_backend_safe_local_raid_cached_result_missing = false;
        patched_local_raid_started_results.clear();

        Il2CppObject* backend_task = original(backend_session, backend_raid_settings);
        if (backend_task == nullptr)
        {
            logger::warn("[BackendSession] backend-safe LocalRaidStarted returned null task");
            return nullptr;
        }
        last_backend_safe_local_raid_task = backend_task;
        hook_local_raid_started_task_result_methods_for_task(backend_task);

        constexpr int local_raid_started_sync_timeout_ms = 250;
        logger::info("[BackendSession] waiting up to " +
            std::to_string(local_raid_started_sync_timeout_ms) +
            "ms for backend-safe LocalRaidStarted task before returning");
        if (wait_for_task_completion(backend_task, local_raid_started_sync_timeout_ms, 5))
        {
            Il2CppObject* backend_result = get_task_result_object(backend_task);
            if (patch_backend_safe_local_raid_started_result(
                backend_result,
                backend_session,
                raid_settings,
                selected_forced_location_id,
                "synchronously"))
            {
                logger::info("[BackendSession] returning synchronously patched LocalRaidStarted task for '" +
                    selected_forced_location_id + "'");
                return backend_task;
            }

            logger::warn("[BackendSession] backend-safe LocalRaidStarted completed but result patch failed for '" +
                selected_forced_location_id + "'");
        }
        else
        {
            logger::warn("[BackendSession] backend-safe LocalRaidStarted did not complete before " +
                std::to_string(local_raid_started_sync_timeout_ms) + "ms synchronous patch timeout");
        }

        logger::warn("[BackendSession] returning unpatched backend-safe LocalRaidStarted task without async patch for '" +
            selected_forced_location_id + "'");
        return backend_task;
    }

    inline void dump_object_field_values(Il2CppObject* object, const std::string& label)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return;
        }

        int logged_index = 0;
        for (Il2CppClass* class_cursor = object->klass; class_cursor != nullptr;
            class_cursor = class_cursor->parent)
        {
            if (class_cursor->fields == nullptr)
            {
                continue;
            }

            for (uint16_t index = 0; index < class_cursor->field_count; ++index)
            {
                const FieldInfo& field = class_cursor->fields[index];
                const int type = field.type ? static_cast<int>(field.type->type) : -1;
                std::string value = "<unread>";

                if (field.offset <= 0)
                {
                    value = "<static/skipped>";
                }
                else if (field.type != nullptr && field.type->type == IL2CPP_TYPE_BOOLEAN)
                {
                    value = std::to_string(*reinterpret_cast<bool*>(reinterpret_cast<UINT64>(object) + field.offset));
                }
                else if (field.type != nullptr && field.type->type == IL2CPP_TYPE_STRING)
                {
                    value = "'" + safe_string(
                        *reinterpret_cast<Il2CppString**>(reinterpret_cast<UINT64>(object) + field.offset)) + "'";
                }
                else if (field.type != nullptr && field.type->type == IL2CPP_TYPE_I4)
                {
                    value = std::to_string(*reinterpret_cast<int*>(reinterpret_cast<UINT64>(object) + field.offset));
                }
                else if (field.type != nullptr && field.type->type == IL2CPP_TYPE_U4)
                {
                    value = std::to_string(*reinterpret_cast<uint32_t*>(reinterpret_cast<UINT64>(object) + field.offset));
                }
                else if (field.type != nullptr && field.type->type == IL2CPP_TYPE_I8)
                {
                    value = std::to_string(*reinterpret_cast<int64_t*>(reinterpret_cast<UINT64>(object) + field.offset));
                }
                else if (field.type != nullptr && field.type->type == IL2CPP_TYPE_U8)
                {
                    value = std::to_string(*reinterpret_cast<uint64_t*>(reinterpret_cast<UINT64>(object) + field.offset));
                }
                else if (field.type != nullptr && field.type->type == IL2CPP_TYPE_I2)
                {
                    value = std::to_string(*reinterpret_cast<int16_t*>(reinterpret_cast<UINT64>(object) + field.offset));
                }
                else if (field.type != nullptr && field.type->type == IL2CPP_TYPE_U2)
                {
                    value = std::to_string(*reinterpret_cast<uint16_t*>(reinterpret_cast<UINT64>(object) + field.offset));
                }
                else if (field.type != nullptr && field.type->type == IL2CPP_TYPE_I1)
                {
                    value = std::to_string(*reinterpret_cast<int8_t*>(reinterpret_cast<UINT64>(object) + field.offset));
                }
                else if (field.type != nullptr && field.type->type == IL2CPP_TYPE_U1)
                {
                    value = std::to_string(*reinterpret_cast<uint8_t*>(reinterpret_cast<UINT64>(object) + field.offset));
                }
                else if (field.type != nullptr && field.type->type == IL2CPP_TYPE_R4)
                {
                    value = std::to_string(*reinterpret_cast<float*>(reinterpret_cast<UINT64>(object) + field.offset));
                }
                else if (field.type != nullptr && field.type->type == IL2CPP_TYPE_R8)
                {
                    value = std::to_string(*reinterpret_cast<double*>(reinterpret_cast<UINT64>(object) + field.offset));
                }
                else if (field.type != nullptr && is_object_reference_type(field.type))
                {
                    Il2CppObject* child = *reinterpret_cast<Il2CppObject**>(
                        reinterpret_cast<UINT64>(object) + field.offset);
                    if (child == nullptr)
                    {
                        value = "null";
                    }
                    else if (field.type->type == IL2CPP_TYPE_ARRAY || field.type->type == IL2CPP_TYPE_SZARRAY)
                    {
                        Il2CppArray* array = reinterpret_cast<Il2CppArray*>(child);
                        value = "class='" + class_name(child->klass) +
                            "' length=" + std::to_string(static_cast<size_t>(array->max_length));
                    }
                    else
                    {
                        value = "class='" + class_name(child->klass) + "'";
                    }
                }

                logger::info("[" + label + "Values] #" + std::to_string(logged_index++) +
                    " declaring_class='" + class_name(class_cursor) +
                    "' name='" + std::string(field.name ? field.name : "<null>") +
                    "' type=" + std::to_string(type) +
                    " value=" + value);
            }
        }
    }

    inline bool should_patch_init_level_location_string_field(
        const std::string& lower_field_name,
        const std::string& value)
    {
        if (!should_rewrite_backend_safe_location_string(value))
        {
            return false;
        }

        return lower_field_name == "location" ||
            lower_field_name == "locationid" ||
            lower_field_name == "location_id" ||
            lower_field_name == "selectedlocation";
    }

    inline bool is_backend_safe_location_object(Il2CppObject* object)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        return should_rewrite_backend_safe_location_string(safe_string(get_string_field(object, "Id"))) ||
            should_rewrite_backend_safe_location_string(safe_string(get_string_field(object, "_Id"))) ||
            lower_copy(safe_string(get_string_field(object, "Name"))) == "factory";
    }

    inline Il2CppObject* selected_location_for_init_level_patch()
    {
        if (selected_forced_location_from_settings != nullptr)
        {
            return selected_forced_location_from_settings;
        }

        return selected_forced_location_object;
    }

    inline bool should_patch_init_level_location_reference_field(
        const FieldInfo& field,
        Il2CppObject* current_value)
    {
        if (field.name == nullptr || field.type == nullptr)
        {
            return false;
        }

        const std::string lower_field_name = lower_copy(field.name);
        const bool field_matches =
            lower_field_name == "locationloot" ||
            lower_field_name == "location" ||
            lower_field_name == "selectedlocation" ||
            lower_field_name == "_selectedlocation" ||
            lower_field_name.find("locationloot") != std::string::npos;
        if (!field_matches)
        {
            return false;
        }

        const std::string field_type_name = type_name(field.type);
        const bool type_matches =
            field_type_name.find("LocationSettings.Location") != std::string::npos ||
            field_type_name.find("JsonType.Location") != std::string::npos ||
            (current_value != nullptr && class_name(current_value->klass).find(".Location") != std::string::npos);
        if (!type_matches)
        {
            return false;
        }

        return current_value == nullptr || is_backend_safe_location_object(current_value);
    }

    inline bool should_recurse_init_level_object(Il2CppObject* object)
    {
        if (!should_recurse_backend_callback_object(object))
        {
            return false;
        }

        const std::string klass_name = class_name(object->klass);
        return klass_name.find("Dictionary") == std::string::npos &&
            klass_name.find("List") == std::string::npos &&
            klass_name.find("HashSet") == std::string::npos;
    }

    inline void dump_init_level_reference_summaries(
        Il2CppObject* object,
        const std::string& label,
        std::unordered_set<uintptr_t>& visited,
        const int depth = 0)
    {
        if (!should_recurse_init_level_object(object) || depth > 1)
        {
            return;
        }

        const uintptr_t address = reinterpret_cast<uintptr_t>(object);
        if (visited.contains(address))
        {
            return;
        }
        visited.insert(address);

        for (uint16_t index = 0; index < object->klass->field_count; ++index)
        {
            const FieldInfo& field = object->klass->fields[index];
            if (field.offset <= 0 || field.type == nullptr || field.name == nullptr ||
                !is_object_reference_type(field.type) ||
                field.type->type == IL2CPP_TYPE_ARRAY ||
                field.type->type == IL2CPP_TYPE_SZARRAY)
            {
                continue;
            }

            Il2CppObject* child = *reinterpret_cast<Il2CppObject**>(
                reinterpret_cast<UINT64>(object) + field.offset);
            if (child == nullptr || child->klass == nullptr)
            {
                continue;
            }

            const std::string field_name = field.name;
            const std::string lower_field_name = lower_copy(field_name);
            const std::string child_class_name = class_name(child->klass);
            const bool interesting =
                lower_field_name.find("location") != std::string::npos ||
                lower_field_name.find("loot") != std::string::npos ||
                lower_field_name.find("settings") != std::string::npos ||
                child_class_name.find("Location") != std::string::npos ||
                child_class_name.find("Local") != std::string::npos;
            if (!interesting)
            {
                continue;
            }

            logger::info("[GameWorldInitLevelRefs] " + label + "." + field_name +
                " type='" + type_name(field.type) +
                "' class=" + child_class_name);
            dump_object_summary(child, label + "." + field_name);

            if (child_class_name.find(".Location") != std::string::npos ||
                lower_field_name.find("locationloot") != std::string::npos)
            {
                dump_location_snapshot(child, label + "." + field_name);
            }

            dump_init_level_reference_summaries(child, label + "." + field_name, visited, depth + 1);
        }
    }

    inline void observe_game_world_init_level(
        Il2CppObject* instance,
        Il2CppObject* item_factory,
        Il2CppObject* config,
        Il2CppObject* resources,
        Il2CppObject* progress,
        Il2CppObject* ct)
    {
        if (!selected_location_forces_offline || selected_forced_location_id.empty())
        {
            return;
        }

        if (!dumped_game_world_init_level_params)
        {
            dumped_game_world_init_level_params = true;
            logger::info("[GameWorldInitLevel] selected_location='" + selected_forced_location_id +
                "' instance=" + (instance != nullptr ? class_name(instance->klass) : std::string("null")) +
                " item_factory=" + (item_factory != nullptr ? class_name(item_factory->klass) : std::string("null")) +
                " config=" + (config != nullptr ? class_name(config->klass) : std::string("null")) +
                " resources=" + (resources != nullptr ? class_name(resources->klass) : std::string("null")) +
                " progress=" + (progress != nullptr ? class_name(progress->klass) : std::string("null")) +
                " ct=" + (ct != nullptr ? class_name(ct->klass) : std::string("null")));

            dump_object_class_and_values_once(
                instance,
                "GameWorldInitLevelWorld",
                dumped_game_world_init_level_world);

            dump_object_class_and_values_once(
                config,
                "GameWorldInitLevelConfig",
                dumped_game_world_init_level_config);

            std::unordered_set<uintptr_t> visited;
            dump_init_level_reference_summaries(instance, "GameWorldInitLevelWorld", visited);
            dump_init_level_reference_summaries(config, "GameWorldInitLevelConfig", visited);
        }

        logger::info("[GameWorldInitLevel] InitLevel config is diagnostics-only; no location patch applied here");
    }

    inline void force_location_unlocked(Il2CppObjectInstance& location_instance)
    {
        location_instance.set_field("Enabled", true);
        location_instance.set_field("EnableCoop", true);
        location_instance.set_field("Locked", false);
        location_instance.set_field("LockedByQuest", false);
        location_instance.set_field("HiddenWhenLockedByQuest", false);
        location_instance.set_field("IsSecret", false);
        location_instance.set_field("DisabledForScav", false);
        location_instance.set_field("RequiredPlayerLevelMin", 0);
        location_instance.set_field("RequiredPlayerLevelMax", 100);
        location_instance.set_field("MinPlayerLvlAccessKeys", 0);
        clear_location_access_requirements(location_instance);
    }

    inline void dump_location_fields(Il2CppObject* location)
    {
        if (dumped_location_fields || location == nullptr || location->klass == nullptr)
        {
            return;
        }

        dumped_location_fields = true;

        Il2CppClass* location_class = location->klass;
        logger::info("[LocationFields] class=" + std::string(location_class->namespaze ? location_class->namespaze : "") +
            "." + std::string(location_class->name ? location_class->name : "<unknown>") +
            " field_count=" + std::to_string(location_class->field_count));

        if (location_class->fields == nullptr)
        {
            logger::warn("[LocationFields] fields pointer is null");
            return;
        }

        for (uint16_t index = 0; index < location_class->field_count; ++index)
        {
            const FieldInfo& field = location_class->fields[index];
            const int type = field.type ? static_cast<int>(field.type->type) : -1;
            logger::info("[LocationFields] #" + std::to_string(index) +
                " name='" + std::string(field.name ? field.name : "<null>") +
                "' offset=" + std::to_string(field.offset) +
                " token=" + std::to_string(field.token) +
                " type=" + std::to_string(type));
        }
    }

    inline void dump_location_access_fields(Il2CppObjectInstance& location_instance)
    {
        if (dumped_location_access_fields)
        {
            return;
        }

        Il2CppObject* access = location_instance.get_field<Il2CppObject*>("access");
        if (access == nullptr || access->klass == nullptr)
        {
            return;
        }

        dumped_location_access_fields = true;
        const Il2CppClass* access_class = access->klass;
        logger::info("[LocationAccess] class=" + class_name(access_class) +
            " fields=" + std::to_string(access_class->field_count) +
            " properties=" + std::to_string(access_class->property_count) +
            " methods=" + std::to_string(access_class->method_count));

        if (access_class->fields == nullptr)
        {
            return;
        }

        for (uint16_t index = 0; index < access_class->field_count; ++index)
        {
            const FieldInfo& field = access_class->fields[index];
            const int type = field.type ? static_cast<int>(field.type->type) : -1;
            logger::info("[LocationAccessFields] #" + std::to_string(index) +
                " name='" + std::string(field.name ? field.name : "<null>") +
                "' offset=" + std::to_string(field.offset) +
                " token=" + std::to_string(field.token) +
                " type=" + std::to_string(type));
        }
    }

    inline void dump_class_layout(const Il2CppClass* klass, const std::string& label)
    {
        if (klass == nullptr)
        {
            logger::warn("[" + label + "] class is null");
            return;
        }

        logger::info("[" + label + "] class=" + class_name(klass) +
            " fields=" + std::to_string(klass->field_count) +
            " properties=" + std::to_string(klass->property_count) +
            " methods=" + std::to_string(klass->method_count));

        if (klass->fields)
        {
            for (uint16_t index = 0; index < klass->field_count; ++index)
            {
                const FieldInfo& field = klass->fields[index];
                const int type = field.type ? static_cast<int>(field.type->type) : -1;
                logger::info("[" + label + "Fields] #" + std::to_string(index) +
                    " name='" + std::string(field.name ? field.name : "<null>") +
                    "' offset=" + std::to_string(field.offset) +
                    " token=" + std::to_string(field.token) +
                    " type=" + std::to_string(type) +
                    " type_name='" + type_name(field.type) + "'");
            }
        }

        if (klass->properties)
        {
            for (uint16_t index = 0; index < klass->property_count; ++index)
            {
                const PropertyInfo& property = klass->properties[index];
                logger::info("[" + label + "Properties] #" + std::to_string(index) +
                    " name='" + std::string(property.name ? property.name : "<null>") +
                    "' get='" + std::string(property.get && property.get->name ? property.get->name : "<null>") +
                    "' set='" + std::string(property.set && property.set->name ? property.set->name : "<null>") + "'");
            }
        }

        if (klass->methods)
        {
            for (uint16_t index = 0; index < klass->method_count; ++index)
            {
                const MethodInfo* method = klass->methods[index];
                if (method == nullptr)
                {
                    continue;
                }

                const int return_type = method->return_type ? static_cast<int>(method->return_type->type) : -1;
                logger::info("[" + label + "Methods] #" + std::to_string(index) +
                    " name='" + std::string(method->name ? method->name : "<null>") +
                    "' params=" + std::to_string(method->parameters_count) +
                    " return_type=" + std::to_string(return_type) +
                    " return_name='" + type_name(method->return_type) + "'");

                if (method->parameters == nullptr)
                {
                    continue;
                }

                for (uint8_t parameter_index = 0; parameter_index < method->parameters_count; ++parameter_index)
                {
                    const Il2CppType* parameter = method->parameters[parameter_index];
                    const int parameter_type = parameter ? static_cast<int>(parameter->type) : -1;
                    logger::info("[" + label + "MethodParams] method='" +
                        std::string(method->name ? method->name : "<null>") +
                        "' #" + std::to_string(parameter_index) +
                        " type=" + std::to_string(parameter_type) +
                        " type_name='" + type_name(parameter) + "'" +
                        " byref=" + std::to_string(parameter ? parameter->byref : 0) +
                        " valuetype=" + std::to_string(parameter ? parameter->valuetype : 0));
                }
            }
        }
    }

    inline void dump_matchmaker_screen_class(Il2CppObject* instance)
    {
        if (dumped_matchmaker_screen_class || instance == nullptr || instance->klass == nullptr)
        {
            return;
        }

        dumped_matchmaker_screen_class = true;

        const Il2CppClass* klass = instance->klass;
        int depth = 0;
        while (klass != nullptr && depth < 8)
        {
            dump_class_layout(klass, "MatchmakerScreen" + std::to_string(depth));
            klass = klass->parent;
            ++depth;
        }
    }

    inline void dump_object_class_once(Il2CppObject* object, const std::string& label, bool& dumped)
    {
        if (dumped || object == nullptr || object->klass == nullptr)
        {
            return;
        }

        dumped = true;
        dump_class_layout(object->klass, label);
    }

    inline void dump_object_class_and_values_once(Il2CppObject* object, const std::string& label, bool& dumped)
    {
        if (dumped || object == nullptr || object->klass == nullptr)
        {
            return;
        }

        dumped = true;
        dump_class_layout(object->klass, label);
        dump_object_field_values(object, label);
    }

    inline void dump_loaded_icebreaker_type(
        const char* namespaze,
        const char* class_name_value,
        const std::string& label)
    {
        const Il2CppClass* klass = il2utils::resolve_class(
            assembly_csharp,
            namespaze,
            class_name_value);
        dump_class_layout(klass, label);
        if (klass == nullptr || unity::find_objects_of_type_internal == nullptr)
        {
            logger::warn("[IcebreakerProbe] unable to enumerate " + label);
            return;
        }

        const Il2CppType* type = il2cpp::il2cpp_class_get_type(
            const_cast<Il2CppClass*>(klass));
        Il2CppObject* system_type = type != nullptr ?
            il2cpp::il2cpp_type_get_object(type) :
            nullptr;
        if (system_type == nullptr)
        {
            logger::warn("[IcebreakerProbe] no System.Type for " + label);
            return;
        }

        Il2CppArray* objects = unity::find_objects_of_type_internal(system_type);
        if (objects == nullptr)
        {
            logger::warn("[IcebreakerProbe] FindObjectsOfType returned null for " + label);
            return;
        }

        const size_t object_count = static_cast<size_t>(objects->max_length);
        logger::info("[IcebreakerProbe] " + label + " loaded_count=" +
            std::to_string(object_count));
        if (object_count == 0)
        {
            return;
        }

        Il2CppObject** entries = reinterpret_cast<Il2CppObject**>(&objects->data);
        Il2CppObject* first_object = entries[0];
        if (first_object == nullptr || first_object->klass == nullptr)
        {
            logger::warn("[IcebreakerProbe] first loaded object is null for " + label);
            return;
        }

        dump_object_field_values(first_object, label + "Instance");
    }

    inline void dump_icebreaker_scene_services()
    {
        if (dumped_icebreaker_scene_services ||
            lower_copy(selected_forced_location_id) != ice_breaker_id)
        {
            return;
        }

        dumped_icebreaker_scene_services = true;
        logger::info("[IcebreakerProbe] begin loaded scene service probe");
        dump_loaded_icebreaker_type(
            "CommonAssets.Scripts.Cutscenes",
            "MapTimelinesBank",
            "IcebreakerMapTimelinesBank");
        dump_loaded_icebreaker_type(
            "CommonAssets.Scripts.Cutscenes",
            "CutscenesClientController",
            "IcebreakerCutscenesClientController");
        dump_loaded_icebreaker_type(
            "CommonAssets.Scripts.Cutscenes",
            "StartCutsceneByStartRaid",
            "IcebreakerStartCutsceneByStartRaid");
        dump_loaded_icebreaker_type(
            "CommonAssets.Scripts.Cutscenes",
            "CutscenePlayableDirector",
            "IcebreakerCutscenePlayableDirector");
        dump_loaded_icebreaker_type(
            "EFT.Interactive",
            "LootPoint",
            "IcebreakerLootPoint");
        dump_loaded_icebreaker_type(
            "",
            "LootPointsGroup",
            "IcebreakerLootPointsGroup");
        dump_loaded_icebreaker_type(
            "EFT.Interactive",
            "LootPointService",
            "IcebreakerLootPointService");
        logger::info("[IcebreakerProbe] end loaded scene service probe");
    }

    inline Il2CppArray* find_loaded_objects_of_type(
        const char* namespaze,
        const char* class_name_value)
    {
        const Il2CppClass* klass = il2utils::resolve_class(
            assembly_csharp,
            namespaze,
            class_name_value);
        if (klass == nullptr || unity::find_objects_of_type_internal == nullptr)
        {
            return nullptr;
        }

        const Il2CppType* type = il2cpp::il2cpp_class_get_type(
            const_cast<Il2CppClass*>(klass));
        Il2CppObject* system_type = type != nullptr ?
            il2cpp::il2cpp_type_get_object(type) :
            nullptr;
        return system_type != nullptr ?
            unity::find_objects_of_type_internal(system_type) :
            nullptr;
    }

    inline std::string summarize_string_array(
        Il2CppArray* values,
        const size_t maximum_count = 24)
    {
        if (values == nullptr)
        {
            return "[]";
        }

        const size_t count = std::min<size_t>(
            static_cast<size_t>(values->max_length),
            maximum_count);
        Il2CppString** strings = reinterpret_cast<Il2CppString**>(&values->data);
        std::string summary = "[";
        for (size_t index = 0; index < count; ++index)
        {
            if (index != 0)
            {
                summary += ",";
            }

            summary += "'" + safe_string(strings[index]) + "'";
        }

        if (static_cast<size_t>(values->max_length) > count)
        {
            summary += ",...";
        }
        summary += "]";
        return summary;
    }

    inline std::string summarize_string_list(
        Il2CppObject* list,
        const size_t maximum_count = 24)
    {
        if (list == nullptr)
        {
            return "[]";
        }

        Il2CppArray* items = reinterpret_cast<Il2CppArray*>(get_object_field(list, "_items"));
        const int list_size = get_il2cpp_list_size(list);
        if (items == nullptr || list_size <= 0)
        {
            return "[]";
        }

        const size_t count = std::min<size_t>(
            std::min<size_t>(
                static_cast<size_t>(list_size),
                static_cast<size_t>(items->max_length)),
            maximum_count);
        Il2CppString** strings = reinterpret_cast<Il2CppString**>(&items->data);
        std::string summary = "[";
        for (size_t index = 0; index < count; ++index)
        {
            if (index != 0)
            {
                summary += ",";
            }

            summary += "'" + safe_string(strings[index]) + "'";
        }

        if (static_cast<size_t>(list_size) > count)
        {
            summary += ",...";
        }
        summary += "]";
        return summary;
    }

    inline void dump_icebreaker_loot_point_configuration(Il2CppArray* loot_points)
    {
        if (dumped_icebreaker_loot_point_configuration || loot_points == nullptr)
        {
            return;
        }

        dumped_icebreaker_loot_point_configuration = true;
        Il2CppObject** points = reinterpret_cast<Il2CppObject**>(&loot_points->data);
        const size_t point_count = std::min<size_t>(
            static_cast<size_t>(loot_points->max_length),
            128);
        size_t enabled_count = 0;
        size_t filter_mode_count = 0;
        size_t loot_set_mode_count = 0;
        size_t group_position_count = 0;

        for (size_t point_index = 0; point_index < point_count; ++point_index)
        {
            Il2CppObject* loot_point = points[point_index];
            if (loot_point == nullptr || loot_point->klass == nullptr)
            {
                logger::info("[IcebreakerLootPointConfig] #" +
                    std::to_string(point_index) + " null");
                continue;
            }

            const int mode = get_int_field_or(loot_point, "Mode", -1);
            const bool enabled = get_bool_field_or(loot_point, "Enabled", false);
            enabled_count += enabled ? 1 : 0;
            loot_set_mode_count += mode == 0 ? 1 : 0;
            filter_mode_count += mode == 1 ? 1 : 0;

            Il2CppArray* filters = reinterpret_cast<Il2CppArray*>(
                get_object_field(loot_point, "_filterInclusive"));
            Il2CppObject* loot_sets = get_object_field(loot_point, "_lootSets");
            Il2CppObject* group_positions = get_object_field(loot_point, "_groupPositions");
            const int group_count = (std::max)(get_il2cpp_list_size(group_positions), 0);
            group_position_count += static_cast<size_t>(group_count);

            Il2CppObject* game_object = unity::component_get_game_object(loot_point);
            Il2CppObject* transform = game_object != nullptr ?
                unity::gameobject_get_transform(game_object) :
                nullptr;
            const unity::vector3 position = transform != nullptr ?
                unity::transform_get_pos(transform) :
                unity::vector3{};

            logger::info("[IcebreakerLootPointConfig] #" +
                std::to_string(point_index) +
                " id='" + safe_string(get_string_field(loot_point, "Id")) +
                "' mode=" + std::to_string(mode) +
                " enabled=" + std::to_string(enabled ? 1 : 0) +
                " rarity=" + std::to_string(get_int_field_or(loot_point, "Rarity", -1)) +
                " chance=" + std::to_string(
                    get_float_field_or(loot_point, "ChanceModifier", 0.0f)) +
                " always=" + std::to_string(
                    get_bool_field_or(loot_point, "IsAlwaysSpawn", false) ? 1 : 0) +
                " gravity=" + std::to_string(
                    get_bool_field_or(loot_point, "UseGravity", false) ? 1 : 0) +
                " random_rotation=" + std::to_string(
                    get_bool_field_or(loot_point, "RandomRotation", false) ? 1 : 0) +
                " groups=" + std::to_string(group_count) +
                " position=(" + std::to_string(position.x) + "," +
                    std::to_string(position.y) + "," + std::to_string(position.z) + ")" +
                " filters=" + summarize_string_array(filters) +
                " loot_sets=" + summarize_string_list(loot_sets));
        }

        logger::info("[IcebreakerLootPointConfig] summary points=" +
            std::to_string(point_count) +
            " enabled=" + std::to_string(enabled_count) +
            " filter_mode=" + std::to_string(filter_mode_count) +
            " loot_set_mode=" + std::to_string(loot_set_mode_count) +
            " group_positions=" + std::to_string(group_position_count));
    }

    inline void dump_icebreaker_container_configuration(Il2CppArray* containers)
    {
        if (dumped_icebreaker_container_configuration || containers == nullptr)
        {
            return;
        }

        dumped_icebreaker_container_configuration = true;
        Il2CppObject** values = reinterpret_cast<Il2CppObject**>(&containers->data);
        const size_t container_count = std::min<size_t>(
            static_cast<size_t>(containers->max_length),
            128);

        for (size_t container_index = 0; container_index < container_count; ++container_index)
        {
            Il2CppObject* container = values[container_index];
            if (container == nullptr || container->klass == nullptr)
            {
                logger::info("[IcebreakerContainerConfig] #" +
                    std::to_string(container_index) + " null");
                continue;
            }

            Il2CppObject* game_object = unity::component_get_game_object(container);
            Il2CppObject* transform = game_object != nullptr ?
                unity::gameobject_get_transform(game_object) :
                nullptr;
            const unity::vector3 position = transform != nullptr ?
                unity::transform_get_pos(transform) :
                unity::vector3{};

            logger::info("[IcebreakerContainerConfig] #" +
                std::to_string(container_index) +
                " id='" + safe_string(get_string_field(container, "Id")) +
                "' template='" + safe_string(get_string_field(container, "Template")) +
                "' spawn_type=" + std::to_string(
                    get_int_field_or(container, "SpawnType", -1)) +
                " spawn_chance=" + std::to_string(
                    get_byte_field_or(container, "SpawnChance", 0)) +
                " chance=" + std::to_string(
                    get_float_field_or(container, "ChanceModifier", 0.0f)) +
                " always_spawn=" + std::to_string(
                    get_bool_field_or(container, "IsAlwaysSpawn", false) ? 1 : 0) +
                " always_lootable=" + std::to_string(
                    get_bool_field_or(container, "IsAlwaysLootable", false) ? 1 : 0) +
                " position=(" + std::to_string(position.x) + "," +
                    std::to_string(position.y) + "," + std::to_string(position.z) + ")");
        }

        logger::info("[IcebreakerContainerConfig] summary containers=" +
            std::to_string(container_count));
    }

    inline bool icebreaker_container_member_is_relevant(const char* raw_name)
    {
        if (raw_name == nullptr)
        {
            return false;
        }

        const std::string name = lower_copy(raw_name);
        static constexpr const char* keywords[] = {
            "id", "template", "item", "owner", "loot", "search",
            "spawn", "state", "door", "lock", "init", "interact"
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

    inline std::string summarize_icebreaker_container_field(
        Il2CppObject* container,
        const FieldInfo& field)
    {
        if (container == nullptr || field.offset <= 0 || field.type == nullptr)
        {
            return "<unread>";
        }

        const uintptr_t address = reinterpret_cast<uintptr_t>(container) + field.offset;
        switch (field.type->type)
        {
        case IL2CPP_TYPE_BOOLEAN:
            return std::to_string(*reinterpret_cast<const bool*>(address) ? 1 : 0);
        case IL2CPP_TYPE_I1:
            return std::to_string(*reinterpret_cast<const int8_t*>(address));
        case IL2CPP_TYPE_U1:
            return std::to_string(*reinterpret_cast<const uint8_t*>(address));
        case IL2CPP_TYPE_I2:
            return std::to_string(*reinterpret_cast<const int16_t*>(address));
        case IL2CPP_TYPE_U2:
            return std::to_string(*reinterpret_cast<const uint16_t*>(address));
        case IL2CPP_TYPE_I4:
            return std::to_string(*reinterpret_cast<const int32_t*>(address));
        case IL2CPP_TYPE_U4:
            return std::to_string(*reinterpret_cast<const uint32_t*>(address));
        case IL2CPP_TYPE_STRING:
            return "'" + safe_string(*reinterpret_cast<Il2CppString* const*>(address)) + "'";
        default:
            break;
        }

        if (!is_object_reference_type(field.type))
        {
            return "<unread>";
        }

        Il2CppObject* value = *reinterpret_cast<Il2CppObject* const*>(address);
        return value != nullptr && value->klass != nullptr ?
            "class='" + class_name(value->klass) + "'" :
            "null";
    }

    inline void dump_icebreaker_live_container_state()
    {
        if (dumped_icebreaker_live_container_state ||
            !selected_forced_location_is_icebreaker() ||
            !game_state::is_in_raid)
        {
            return;
        }

        Il2CppArray* containers = find_loaded_objects_of_type(
            "EFT.Interactive",
            "LootableContainer");
        if (containers == nullptr || containers->max_length == 0)
        {
            return;
        }

        dumped_icebreaker_live_container_state = true;
        Il2CppObject** values = reinterpret_cast<Il2CppObject**>(&containers->data);
        const size_t instance_count = std::min<size_t>(
            static_cast<size_t>(containers->max_length),
            3);
        logger::info("[IcebreakerLiveContainer] begin loaded_count=" +
            std::to_string(containers->max_length) +
            " sampled=" + std::to_string(instance_count));

        size_t logged_members = 0;
        for (Il2CppClass* cursor = values[0] != nullptr ? values[0]->klass : nullptr;
            cursor != nullptr && logged_members < 128;
            cursor = cursor->parent)
        {
            logger::info("[IcebreakerLiveContainerClass] owner='" +
                class_name(cursor) + "' fields=" + std::to_string(cursor->field_count) +
                " methods=" + std::to_string(cursor->method_count));

            if (cursor->fields != nullptr)
            {
                for (uint16_t field_index = 0;
                    field_index < cursor->field_count && logged_members < 128;
                    ++field_index)
                {
                    const FieldInfo& field = cursor->fields[field_index];
                    if (!icebreaker_container_member_is_relevant(field.name))
                    {
                        continue;
                    }

                    std::string values_summary;
                    for (size_t instance_index = 0; instance_index < instance_count; ++instance_index)
                    {
                        if (instance_index != 0)
                        {
                            values_summary += ",";
                        }
                        values_summary += "#" + std::to_string(instance_index) + "=" +
                            summarize_icebreaker_container_field(values[instance_index], field);
                    }

                    logger::info("[IcebreakerLiveContainerField] owner='" +
                        class_name(cursor) + "' name='" +
                        std::string(field.name ? field.name : "<null>") +
                        "' offset=" + std::to_string(field.offset) +
                        " type=" + std::to_string(
                            field.type != nullptr ? static_cast<int>(field.type->type) : -1) +
                        " values=[" + values_summary + "]");
                    ++logged_members;
                }
            }

            if (cursor->methods != nullptr)
            {
                for (uint16_t method_index = 0;
                    method_index < cursor->method_count && logged_members < 128;
                    ++method_index)
                {
                    const MethodInfo* method = cursor->methods[method_index];
                    if (method == nullptr ||
                        !icebreaker_container_member_is_relevant(method->name))
                    {
                        continue;
                    }

                    logger::info("[IcebreakerLiveContainerMethod] owner='" +
                        class_name(cursor) + "' name='" +
                        std::string(method->name ? method->name : "<null>") +
                        "' params=" + std::to_string(method->parameters_count));
                    ++logged_members;
                }
            }
        }

        logger::info("[IcebreakerLiveContainer] end members=" +
            std::to_string(logged_members));
    }

    inline bool item_template_matches_filters(
        Il2CppObject* item_template,
        Il2CppArray* filters)
    {
        if (item_template == nullptr || item_template->klass == nullptr)
        {
            return false;
        }

        if (filters == nullptr || filters->max_length == 0)
        {
            return true;
        }

        Il2CppString** filter_values = reinterpret_cast<Il2CppString**>(&filters->data);
        const size_t filter_count = std::min<size_t>(
            static_cast<size_t>(filters->max_length),
            64);

        Il2CppObject* current_template = item_template;
        for (size_t depth = 0;
            current_template != nullptr && current_template->klass != nullptr && depth < 32;
            ++depth)
        {
            const std::string template_id = get_mongo_id_field_string(
                current_template,
                "<_id>k__BackingField");
            for (size_t filter_index = 0; filter_index < filter_count; ++filter_index)
            {
                if (!template_id.empty() && template_id == safe_string(filter_values[filter_index]))
                {
                    return true;
                }
            }

            Il2CppObject* parent = get_object_field(
                current_template,
                "<Parent>k__BackingField");
            if (parent == current_template)
            {
                break;
            }
            current_template = parent;
        }

        return false;
    }

    inline int get_icebreaker_container_item_count(Il2CppObject* container_item)
    {
        if (container_item == nullptr || container_item->klass == nullptr)
        {
            return 0;
        }

        Il2CppArray* grids = reinterpret_cast<Il2CppArray*>(
            get_object_field(container_item, "Grids"));
        if (grids == nullptr)
        {
            return 0;
        }

        int total_count = 0;
        Il2CppObject** grid_values = reinterpret_cast<Il2CppObject**>(&grids->data);
        const size_t grid_count = std::min<size_t>(
            static_cast<size_t>(grids->max_length),
            16);
        for (size_t grid_index = 0; grid_index < grid_count; ++grid_index)
        {
            if (grid_values[grid_index] == nullptr ||
                grid_values[grid_index]->klass == nullptr)
            {
                continue;
            }

            Il2CppObject* item_collection = get_object_field(
                grid_values[grid_index],
                "<ItemCollection>k__BackingField");
            if (item_collection == nullptr || item_collection->klass == nullptr)
            {
                continue;
            }

            const MethodInfo* count_getter = resolve_method_in_hierarchy(
                item_collection->klass,
                "get_Count",
                0);
            if (count_getter == nullptr || count_getter->methodPointer == nullptr)
            {
                continue;
            }

            using count_getter_sig = int(*)(Il2CppObject*);
            total_count += (std::max)(
                reinterpret_cast<count_getter_sig>(
                    count_getter->methodPointer)(item_collection),
                0);
        }

        return total_count;
    }

    inline size_t find_unused_container_group(
        const std::vector<IcebreakerLootContainerGroup>& groups,
        const std::vector<bool>& used,
        const std::string& template_id)
    {
        if (template_id.empty())
        {
            return std::string::npos;
        }

        size_t best_index = std::string::npos;
        int best_item_count = -1;
        for (size_t index = 0; index < groups.size(); ++index)
        {
            if (!used[index] && groups[index].root_record != nullptr &&
                groups[index].template_id == template_id &&
                groups[index].contained_item_count > best_item_count)
            {
                best_index = index;
                best_item_count = groups[index].contained_item_count;
            }
        }

        return best_index;
    }

    inline size_t find_unused_loose_candidate(
        const std::vector<IcebreakerLootCandidate>& candidates,
        const std::vector<bool>& used,
        Il2CppArray* filters)
    {
        for (size_t index = 0; index < candidates.size(); ++index)
        {
            if (!used[index] &&
                item_template_matches_filters(candidates[index].item_template, filters))
            {
                return index;
            }
        }

        return std::string::npos;
    }

    inline bool retarget_icebreaker_loot_record(
        Il2CppObject* record,
        Il2CppObject* target,
        const std::string& target_id,
        const bool is_container)
    {
        if (record == nullptr || record->klass == nullptr ||
            target == nullptr || target->klass == nullptr)
        {
            return false;
        }

        Il2CppObject* game_object = unity::component_get_game_object(target);
        Il2CppObject* transform = game_object != nullptr ?
            unity::gameobject_get_transform(game_object) :
            nullptr;
        if (transform == nullptr)
        {
            return false;
        }

        if (!target_id.empty())
        {
            set_string_field_if_exists(record, "Id", target_id);
        }

        const unity::vector3 position = unity::transform_get_pos(transform);
        set_vector3_field_if_exists(record, "Position", position);
        set_vector3_field_if_exists(record, "position", position);
        set_bool_field_if_exists(record, "IsContainer", is_container);
        set_bool_field_if_exists(
            record,
            "useGravity",
            get_bool_field_or(target, "UseGravity", false));
        set_bool_field_if_exists(
            record,
            "randomRotation",
            !is_container && get_bool_field_or(target, "RandomRotation", false));
        set_vector3_field_if_exists(record, "Shift", unity::vector3{});
        return true;
    }

    inline void dump_icebreaker_remap_record_once(Il2CppObject* generated_item)
    {
        if (dumped_icebreaker_json_loot_item ||
            generated_item == nullptr || generated_item->klass == nullptr)
        {
            return;
        }

        dumped_icebreaker_json_loot_item = true;
        dump_class_layout(generated_item->klass, "IcebreakerJsonLootItem");
        dump_object_field_values(generated_item, "IcebreakerJsonLootItemBeforeRemap");
        Il2CppObject* selected_item = get_object_field(generated_item, "Item");
        if (selected_item != nullptr && selected_item->klass != nullptr)
        {
            dump_class_layout(selected_item->klass, "IcebreakerSelectedInventoryItem");
            dump_object_field_values(selected_item, "IcebreakerSelectedInventoryItem");
        }
    }

    inline void dump_icebreaker_loot_record_relationships(
        Il2CppObject** generated_items,
        const size_t source_count)
    {
        if (dumped_icebreaker_loot_record_probe || generated_items == nullptr)
        {
            return;
        }

        dumped_icebreaker_loot_record_probe = true;
        size_t root_record_count = 0;
        size_t container_member_count = 0;
        size_t loose_record_count = 0;
        size_t logged_roots = 0;
        size_t logged_members = 0;
        size_t logged_loose = 0;

        for (size_t source_index = 0; source_index < source_count; ++source_index)
        {
            Il2CppObject* record = generated_items[source_index];
            if (record == nullptr || record->klass == nullptr)
            {
                continue;
            }

            Il2CppObject* item = get_object_field(record, "Item");
            if (item == nullptr || item->klass == nullptr)
            {
                continue;
            }

            const std::string item_class_name = lower_copy(class_name(item->klass));
            const bool is_root =
                item_class_name.find("lootcontainer") != std::string::npos;
            const bool is_container = get_bool_field_or(record, "IsContainer", false);
            if (is_root)
            {
                ++root_record_count;
            }
            else if (is_container)
            {
                ++container_member_count;
            }
            else
            {
                ++loose_record_count;
            }

            const bool should_log =
                (is_root && logged_roots < 1) ||
                (!is_root && is_container && logged_members < 6) ||
                (!is_container && logged_loose < 2);
            if (!should_log)
            {
                continue;
            }

            if (is_root)
            {
                ++logged_roots;
            }
            else if (is_container)
            {
                ++logged_members;
            }
            else
            {
                ++logged_loose;
            }

            Il2CppObject* current_address = get_object_field(
                item,
                "<CurrentAddress>k__BackingField");
            Il2CppObject* original_address = get_object_field(item, "OriginalAddress");
            Il2CppArray* grids = reinterpret_cast<Il2CppArray*>(
                get_object_field(item, "Grids"));
            Il2CppArray* slots = reinterpret_cast<Il2CppArray*>(
                get_object_field(item, "Slots"));

            logger::info("[IcebreakerLootRecordProbe] source_index=" +
                std::to_string(source_index) +
                " record_id='" + safe_string(get_string_field(record, "Id")) +
                "' is_container=" + std::to_string(is_container ? 1 : 0) +
                " root=" + std::to_string(is_root ? 1 : 0) +
                " item_class='" + class_name(item->klass) +
                "' item_id='" + safe_string(
                    get_string_field(item, "<Id>k__BackingField")) +
                "' template_id='" + get_inventory_item_template_id(item) +
                "' current_address='" +
                    (current_address != nullptr ? class_name(current_address->klass) : std::string("null")) +
                "' original_address='" +
                    (original_address != nullptr ? class_name(original_address->klass) : std::string("null")) +
                "' grids=" + std::to_string(grids != nullptr ? grids->max_length : 0) +
                " slots=" + std::to_string(slots != nullptr ? slots->max_length : 0));

            if (!is_root && is_container && !dumped_icebreaker_member_item_probe)
            {
                dump_object_class_and_values_once(
                    item,
                    "IcebreakerContainerMemberItemProbe",
                    dumped_icebreaker_member_item_probe);
            }

            Il2CppObject* address = current_address != nullptr ?
                current_address :
                original_address;
            if (address != nullptr && !dumped_icebreaker_item_address_probe)
            {
                dump_object_class_and_values_once(
                    address,
                    "IcebreakerContainerMemberAddressProbe",
                    dumped_icebreaker_item_address_probe);
            }

            if (is_root && grids != nullptr && grids->max_length > 0 &&
                !dumped_icebreaker_root_grid_probe)
            {
                Il2CppObject** grid_values = reinterpret_cast<Il2CppObject**>(&grids->data);
                Il2CppObject* root_grid = grid_values[0];
                dump_object_class_and_values_once(
                    root_grid,
                    "IcebreakerContainerRootGridProbe",
                    dumped_icebreaker_root_grid_probe);

                Il2CppObject* item_collection = get_object_field(
                    root_grid,
                    "<ItemCollection>k__BackingField");
                dump_object_class_and_values_once(
                    item_collection,
                    "IcebreakerContainerRootItemCollectionProbe",
                    dumped_icebreaker_root_item_collection_probe);
            }
        }

        logger::info("[IcebreakerLootRecordProbe] summary source=" +
            std::to_string(source_count) +
            " roots=" + std::to_string(root_record_count) +
            " container_members=" + std::to_string(container_member_count) +
            " loose=" + std::to_string(loose_record_count));
    }

    inline void remap_icebreaker_loot(Il2CppObject* location)
    {
        if (icebreaker_loot_remapped || !selected_forced_location_is_icebreaker() ||
            location == nullptr || location->klass == nullptr)
        {
            return;
        }

        Il2CppObject* loot = get_object_field(location, "Loot");
        Il2CppArray* loot_items = reinterpret_cast<Il2CppArray*>(
            get_object_field(loot, "_items"));
        const int loot_size = get_il2cpp_list_size(loot);
        Il2CppArray* loot_points = find_loaded_objects_of_type(
            "EFT.Interactive",
            "LootPoint");
        Il2CppArray* containers = find_loaded_objects_of_type(
            "EFT.Interactive",
            "LootableContainer");
        if (loot == nullptr || loot_items == nullptr || loot_size <= 0 ||
            ((loot_points == nullptr || loot_points->max_length == 0) &&
                (containers == nullptr || containers->max_length == 0)))
        {
            logger::warn("[IcebreakerLoot] generated loot or scene targets are unavailable");
            return;
        }

        dump_icebreaker_loot_point_configuration(loot_points);
        dump_icebreaker_container_configuration(containers);

        Il2CppObject** generated_items = reinterpret_cast<Il2CppObject**>(&loot_items->data);
        const size_t source_count = std::min<size_t>(
            static_cast<size_t>(loot_size),
            static_cast<size_t>(loot_items->max_length));
        dump_icebreaker_loot_record_relationships(generated_items, source_count);
        std::vector<IcebreakerLootContainerGroup> container_groups;
        std::unordered_map<std::string, size_t> container_group_indices;
        std::vector<IcebreakerLootCandidate> loose_candidates;
        container_groups.reserve(source_count);
        container_group_indices.reserve(source_count);
        loose_candidates.reserve(source_count);
        size_t source_container_records = 0;
        size_t skipped_info = 0;
        size_t skipped_quest = 0;
        size_t skipped_children = 0;

        for (size_t source_index = 0; source_index < source_count; ++source_index)
        {
            Il2CppObject* record = generated_items[source_index];
            Il2CppObject* item = get_object_field(record, "Item");
            if (record == nullptr || record->klass == nullptr ||
                item == nullptr || item->klass == nullptr)
            {
                continue;
            }

            const std::string item_class_name = lower_copy(class_name(item->klass));
            const bool is_container_record =
                get_bool_field_or(record, "IsContainer", false) ||
                item_class_name.find("lootcontainer") != std::string::npos;
            if (is_container_record)
            {
                ++source_container_records;
                const std::string source_id = safe_string(
                    get_string_field(record, "Id"));
                if (source_id.empty())
                {
                    continue;
                }

                const auto [iterator, inserted] = container_group_indices.emplace(
                    source_id,
                    container_groups.size());
                if (inserted)
                {
                    container_groups.push_back(IcebreakerLootContainerGroup{
                        source_id,
                        {},
                        nullptr,
                        0,
                        {}
                    });
                }

                IcebreakerLootContainerGroup& group = container_groups[iterator->second];
                group.records.push_back(record);
                if (item_class_name.find("lootcontainer") != std::string::npos)
                {
                    group.root_record = record;
                    group.template_id = get_inventory_item_template_id(item);
                    group.contained_item_count =
                        get_icebreaker_container_item_count(item);
                }
                continue;
            }

            if (item_class_name.find("inventorylogic.info") != std::string::npos)
            {
                ++skipped_info;
                continue;
            }

            if (get_object_field(item, "<CurrentAddress>k__BackingField") != nullptr)
            {
                ++skipped_children;
                continue;
            }

            const MethodInfo* quest_item_getter = resolve_method_in_hierarchy(
                item->klass,
                "get_QuestItem",
                0);
            if (quest_item_getter != nullptr && quest_item_getter->methodPointer != nullptr)
            {
                using quest_item_getter_sig = bool(*)(Il2CppObject*);
                if (reinterpret_cast<quest_item_getter_sig>(
                    quest_item_getter->methodPointer)(item))
                {
                    ++skipped_quest;
                    continue;
                }
            }

            IcebreakerLootCandidate candidate{
                record,
                item,
                get_inventory_item_template(item),
                get_inventory_item_template_id(item)
            };
            loose_candidates.push_back(std::move(candidate));
        }

        size_t mapped_count = 0;
        size_t mapped_containers = 0;
        size_t mapped_nonempty_containers = 0;
        size_t mapped_contained_items = 0;
        size_t unmatched_containers = 0;
        size_t mapped_loose_items = 0;
        size_t disabled_loot_points = 0;
        std::vector<bool> used_container_groups(container_groups.size(), false);
        std::vector<bool> used_loose_items(loose_candidates.size(), false);

        if (containers != nullptr)
        {
            Il2CppObject** scene_containers = reinterpret_cast<Il2CppObject**>(&containers->data);
            const size_t container_count = static_cast<size_t>(containers->max_length);
            for (size_t container_index = 0; container_index < container_count; ++container_index)
            {
                Il2CppObject* container = scene_containers[container_index];
                if (container == nullptr || container->klass == nullptr)
                {
                    ++unmatched_containers;
                    continue;
                }

                const std::string target_template = safe_string(
                    get_string_field(container, "Template"));
                const size_t group_index = find_unused_container_group(
                    container_groups,
                    used_container_groups,
                    target_template);
                if (group_index == std::string::npos)
                {
                    ++unmatched_containers;
                    continue;
                }

                IcebreakerLootContainerGroup& group = container_groups[group_index];
                Il2CppObject* root_record = group.root_record;
                const std::string target_id = safe_string(
                    get_string_field(container, "Id"));
                dump_icebreaker_remap_record_once(root_record);
                if (!retarget_icebreaker_loot_record(
                    root_record,
                    container,
                    target_id,
                    true))
                {
                    ++unmatched_containers;
                    continue;
                }

                used_container_groups[group_index] = true;
                for (Il2CppObject* record : group.records)
                {
                    if (record == nullptr || mapped_count >= source_count)
                    {
                        continue;
                    }

                    set_string_field_if_exists(record, "Id", target_id);
                    generated_items[mapped_count++] = record;
                }
                ++mapped_containers;
                if (group.contained_item_count > 0)
                {
                    ++mapped_nonempty_containers;
                    mapped_contained_items += static_cast<size_t>(
                        group.contained_item_count);
                }
            }
        }

        if (loot_points != nullptr)
        {
            Il2CppObject** scene_points = reinterpret_cast<Il2CppObject**>(&loot_points->data);
            const size_t point_count = static_cast<size_t>(loot_points->max_length);
            for (size_t point_index = 0; point_index < point_count; ++point_index)
            {
                Il2CppObject* loot_point = scene_points[point_index];
                if (loot_point == nullptr || loot_point->klass == nullptr ||
                    !get_bool_field_or(loot_point, "Enabled", false))
                {
                    ++disabled_loot_points;
                    continue;
                }

                Il2CppArray* filters = reinterpret_cast<Il2CppArray*>(
                    get_object_field(loot_point, "_filterInclusive"));
                const size_t candidate_index = find_unused_loose_candidate(
                    loose_candidates,
                    used_loose_items,
                    filters);
                if (candidate_index == std::string::npos)
                {
                    continue;
                }

                Il2CppObject* record = loose_candidates[candidate_index].record;
                dump_icebreaker_remap_record_once(record);
                if (!retarget_icebreaker_loot_record(
                    record,
                    loot_point,
                    safe_string(get_string_field(loot_point, "Id")),
                    false))
                {
                    continue;
                }

                used_loose_items[candidate_index] = true;
                generated_items[mapped_count++] = record;
                ++mapped_loose_items;
            }
        }

        if (mapped_count == 0)
        {
            logger::warn("[IcebreakerLoot] no template-compatible generated records were mapped; "
                "leaving the backend-safe loot list unchanged");
            return;
        }

        set_int_field_if_exists(loot, "_size", static_cast<int>(mapped_count));
        set_int_field_if_exists(
            loot,
            "_version",
            get_int_field_or(loot, "_version", 0) + 1);
        icebreaker_loot_remapped = true;

        logger::info("[IcebreakerLoot] mapped records=" + std::to_string(mapped_count) +
            " containers=" + std::to_string(mapped_containers) +
            " nonempty_containers=" + std::to_string(mapped_nonempty_containers) +
            " contained_items=" + std::to_string(mapped_contained_items) +
            " unmatched_containers=" + std::to_string(unmatched_containers) +
            " loose=" + std::to_string(mapped_loose_items) +
            " disabled_loot_points=" + std::to_string(disabled_loot_points) +
            " source_container_records=" + std::to_string(source_container_records) +
            " source_container_groups=" + std::to_string(container_groups.size()) +
            " source_loose=" + std::to_string(loose_candidates.size()) +
            " skipped_info=" + std::to_string(skipped_info) +
            " skipped_quest=" + std::to_string(skipped_quest) +
            " skipped_children=" + std::to_string(skipped_children));
        dump_object_field_values(generated_items[0], "IcebreakerJsonLootItemAfterRemap");
    }

    inline std::string find_icebreaker_timeline_id()
    {
        if (!icebreaker_timeline_id.empty())
        {
            return icebreaker_timeline_id;
        }

        Il2CppArray* banks = find_loaded_objects_of_type(
            "CommonAssets.Scripts.Cutscenes",
            "MapTimelinesBank");
        if (banks == nullptr || banks->max_length == 0)
        {
            return "";
        }

        Il2CppObject* bank = reinterpret_cast<Il2CppObject**>(&banks->data)[0];
        Il2CppObject* timelines = get_object_field(bank, "timelines");
        Il2CppArray* items = reinterpret_cast<Il2CppArray*>(
            get_object_field(timelines, "_items"));
        const int timeline_count = get_il2cpp_list_size(timelines);
        if (items == nullptr || items->klass == nullptr ||
            items->klass->element_class == nullptr || timeline_count <= 0 ||
            items->klass->element_size == 0)
        {
            return "";
        }

        Il2CppClass* timeline_class = items->klass->element_class;
        dump_class_layout(timeline_class, "IcebreakerMapTimelineData");
        const size_t count = std::min<size_t>(
            static_cast<size_t>(timeline_count),
            static_cast<size_t>(items->max_length));
        const size_t stride = static_cast<size_t>(items->klass->element_size);
        uint8_t* values = reinterpret_cast<uint8_t*>(&items->data);
        std::string first_value;

        for (size_t value_index = 0; value_index < count; ++value_index)
        {
            uint8_t* value = values + (value_index * stride);
            for (uint16_t field_index = 0;
                field_index < timeline_class->field_count;
                ++field_index)
            {
                const FieldInfo& field = timeline_class->fields[field_index];
                if (field.name == nullptr || field.type == nullptr ||
                    field.type->type != IL2CPP_TYPE_STRING ||
                    field.offset < static_cast<int32_t>(sizeof(Il2CppObject)))
                {
                    continue;
                }

                const size_t relative_offset =
                    static_cast<size_t>(field.offset) - sizeof(Il2CppObject);
                if (relative_offset + sizeof(Il2CppString*) > stride)
                {
                    continue;
                }

                Il2CppString* string_value = *reinterpret_cast<Il2CppString**>(
                    value + relative_offset);
                const std::string text = safe_string(string_value);
                if (text.empty())
                {
                    continue;
                }

                logger::info("[IcebreakerCutscene] timeline[" +
                    std::to_string(value_index) + "]." + field.name +
                    "='" + text + "'");
                if (first_value.empty())
                {
                    first_value = text;
                }

                const std::string lower_text = lower_copy(text);
                if (lower_text.find("icebreaker") != std::string::npos ||
                    lower_text.find("cutscene") != std::string::npos)
                {
                    icebreaker_timeline_id = text;
                    return icebreaker_timeline_id;
                }
            }
        }

        icebreaker_timeline_id = first_value;
        return icebreaker_timeline_id;
    }

    inline void dump_matchmaker_runtime_objects(Il2CppObject* instance)
    {
        if (instance == nullptr || instance->klass == nullptr)
        {
            return;
        }

        dump_object_class_once(get_object_field(instance, "_raidSettings"), "RaidSettings", dumped_raid_settings_class);
        dump_object_class_once(get_object_field(instance, "_matchmaker"), "MatchmakerInstance", dumped_matchmaker_instance_class);
    }

    inline void apply_menu_location_flags(Il2CppObject* instance, Il2CppObject* location, const bool mark_selected = false)
    {
        dump_matchmaker_runtime_objects(instance);

        if (location == nullptr || location->klass == nullptr)
        {
            return;
        }

        if (mark_selected)
        {
            update_selected_forced_location(location);
        }

        Il2CppObjectInstance location_instance(location);
        const bool location_forced_offline = should_force_location_offline_object(location);

        if (game_state::force_offline || location_forced_offline)
        {
            force_location_offline(location_instance);
        }

        if (should_force_display_location_object(location))
        {
            force_location_unlocked(location_instance);
        }

        if (location_forced_offline)
        {
            force_raid_settings_offline(get_object_field(instance, "_raidSettings"));
        }
    }

    using display_location_sig = bool(*)(Il2CppObject*, Il2CppObject*);
    using selected_location_sig = void(*)(Il2CppObject*, Il2CppObject*);
    using should_hide_location_button_sig = bool(*)(Il2CppObject*, Il2CppObject*);
    using can_display_accept_button_sig = bool(*)(Il2CppObject*, Il2CppObject*, int);
    using available_for_players_sig = Il2CppObject * (*)(Il2CppObject*, Il2CppObject*);
    using raid_settings_bool_getter_sig = bool(*)(Il2CppObject*);
    using raid_settings_pve_gate_sig = bool(*)(Il2CppObject*, int);
    using raid_settings_apply_sig = void(*)(Il2CppObject*, Il2CppObject*);
    using raid_settings_params_sig = Il2CppObject * (*)(Il2CppObject*);
    using backend_session_object_task_sig = Il2CppObject * (*)(Il2CppObject*, Il2CppObject*);
    using object_callback_sig = void(*)(Il2CppObject*, Il2CppObject*);
    using cutscenes_on_update_sig = void(*)(Il2CppObject*, float);
    using local_game_create_sig = Il2CppObject * (*)(
        Il2CppObject*,
        Il2CppObject*,
        Il2CppObject*,
        Il2CppObject*,
        Il2CppObject*,
        Il2CppObject*,
        Il2CppObject*,
        uintptr_t,
        uintptr_t,
        int,
        Il2CppObject*,
        float,
        int,
        Il2CppObject*,
        uintptr_t,
        Il2CppObject*,
        Il2CppObject*,
        Il2CppObject*);
    using local_game_create_instance_sig = Il2CppObject * (*)(
        Il2CppObject*,
        Il2CppObject*,
        Il2CppObject*,
        Il2CppObject*,
        Il2CppObject*,
        Il2CppObject*,
        Il2CppObject*,
        Il2CppObject*,
        uintptr_t,
        uintptr_t,
        int,
        Il2CppObject*,
        float,
        int,
        Il2CppObject*,
        uintptr_t,
        Il2CppObject*,
        Il2CppObject*,
        Il2CppObject*);

    static inline display_location_sig o_display_location = nullptr;
    static inline selected_location_sig o_set_selected_location = nullptr;
    static inline selected_location_sig o_on_location_selected = nullptr;
    static inline should_hide_location_button_sig o_should_hide_location_button = nullptr;
    static inline can_display_accept_button_sig o_can_display_accept_button = nullptr;
    static inline available_for_players_sig o_available_for_players = nullptr;
    static inline raid_settings_bool_getter_sig o_raid_settings_get_local = nullptr;
    static inline raid_settings_bool_getter_sig o_raid_settings_get_online_pve_raid = nullptr;
    static inline raid_settings_pve_gate_sig o_raid_settings_need_to_go_online_in_pve = nullptr;
    static inline raid_settings_pve_gate_sig o_raid_settings_need_to_go_offline_in_pve = nullptr;
    static inline selected_location_sig o_raid_settings_set_selected_location = nullptr;
    static inline raid_settings_apply_sig o_raid_settings_apply_from_backend = nullptr;
    static inline raid_settings_apply_sig o_raid_settings_apply = nullptr;
    static inline raid_settings_params_sig o_raid_settings_get_update_status_params = nullptr;
    static inline raid_settings_params_sig o_raid_settings_get_create_raid_group_params = nullptr;
    static inline backend_session_object_task_sig o_backend_send_raid_settings = nullptr;
    static inline backend_session_object_task_sig o_backend_local_raid_started = nullptr;
    static inline object_callback_sig o_matchmaker_receive_raid_settings = nullptr;
    static inline object_callback_sig o_matchmaker_receive_matching_start_notification = nullptr;
    static inline local_game_create_sig o_local_game_create = nullptr;
    static inline local_game_create_instance_sig o_local_game_create_instance = nullptr;
    static inline cutscenes_on_update_sig o_cutscenes_client_controller_on_update = nullptr;

    inline void hk_cutscenes_client_controller_on_update(
        Il2CppObject* instance,
        const float delta_time)
    {
        o_cutscenes_client_controller_on_update(instance, delta_time);

        if (icebreaker_cutscene_started || !selected_forced_location_is_icebreaker() ||
            instance == nullptr || instance->klass == nullptr ||
            get_object_field(instance, "_timelinesMapBank") == nullptr)
        {
            return;
        }

        const MethodInfo* start_show_timeline = il2utils::resolve_method(
            instance->klass,
            "StartShowTimeline",
            2);
        if (start_show_timeline == nullptr || start_show_timeline->methodPointer == nullptr)
        {
            return;
        }

        std::string timeline_id = find_icebreaker_timeline_id();
        if (timeline_id.empty())
        {
            timeline_id = "Icebreaker_cutscene_01";
        }

        set_bool_field_if_exists(instance, "_offlineMode", true);
        set_bool_field_if_exists(instance, "_clientOnlyCutscenePlay", true);
        icebreaker_cutscene_started = true;

        Il2CppString* timeline = il2cpp::il2cpp_string_new(timeline_id.c_str());
        float start_time = 0.0f;
        Il2CppObject* task = nullptr;
        void* args[] = { timeline, &start_time };
        if (start_show_timeline->invoker_method != nullptr)
        {
            start_show_timeline->invoker_method(
                reinterpret_cast<Il2CppMethodPointer>(start_show_timeline->methodPointer),
                start_show_timeline,
                instance,
                args,
                &task);
        }
        else
        {
            using start_show_timeline_sig = Il2CppObject * (*)(
                Il2CppObject*,
                Il2CppString*,
                float);
            task = reinterpret_cast<start_show_timeline_sig>(
                start_show_timeline->methodPointer)(instance, timeline, start_time);
        }

        logger::info("[IcebreakerCutscene] requested timeline '" + timeline_id +
            "' task=" + (task != nullptr ? class_name(task->klass) : std::string("null")));
    }

    inline void patch_local_game_create_location(
        Il2CppObject*& location,
        Il2CppObject* local_raid_settings)
    {
        if (!selected_location_forces_offline || selected_forced_location_id.empty())
        {
            return;
        }

        if (local_raid_settings != nullptr)
        {
            last_backend_safe_local_raid_settings = local_raid_settings;
        }

        patch_backend_safe_local_raid_started_cached_result("local-game-create-cached-result");
        force_local_raid_started_selected_location(local_raid_settings);
        dump_object_class_and_values_once(
            local_raid_settings,
            "LocalGameCreateLocalRaidSettings",
            dumped_local_game_create_raid_settings);

        if (!dumped_local_game_create_location_before)
        {
            dumped_local_game_create_location_before = true;
            dump_location_snapshot(location, "LocalGameCreateLocationBefore");
        }

        Il2CppObject* forced_location = get_real_local_raid_started_result_location();
        if (forced_location != nullptr)
        {
            logger::warn("[LocalGameCreate] using real LocalRaidStarted locationLoot for '" +
                selected_forced_location_id + "' loot_size=" +
                std::to_string(get_location_loot_size(forced_location)));
            set_reference_field_if_exists(local_raid_settings, "selectedLocation", forced_location);
            set_reference_field_if_exists(local_raid_settings, "_selectedLocation", forced_location);
            set_reference_field_if_exists(local_raid_settings, "SelectedLocation", forced_location);
        }
        else
        {
            forced_location = selected_location_for_local_raid_settings();
        }

        if (forced_location == nullptr || forced_location->klass == nullptr)
        {
            logger::warn("[LocalGameCreate] no forced location object available for '" +
                selected_forced_location_id + "'");
            return;
        }

        Il2CppObjectInstance forced_location_instance(forced_location);
        force_location_offline(forced_location_instance);
        force_location_unlocked(forced_location_instance);

        const std::string before_id = get_location_identifier(location);
        const std::string forced_id = get_location_identifier(forced_location);
        if (location != forced_location)
        {
            logger::warn("[LocalGameCreate] replacing Create location '" +
                before_id + "' -> '" + forced_id + "'");
            location = forced_location;
        }
        else
        {
            logger::info("[LocalGameCreate] Create already has forced location '" + forced_id + "'");
        }

        remap_icebreaker_loot(location);

        if (!dumped_local_game_create_location_after)
        {
            dumped_local_game_create_location_after = true;
            dump_location_snapshot(location, "LocalGameCreateLocationAfter");
        }

        dump_icebreaker_scene_services();
    }

    inline bool hk_display_location(Il2CppObject* instance, Il2CppObject* location)
    {
        dump_matchmaker_screen_class(instance);

        bool force_display_location = false;
        if (location != nullptr)
        {
            dump_location_fields(location);

            Il2CppObjectInstance location_instance(location);
            dump_location_access_fields(location_instance);

            apply_menu_location_flags(instance, location);
            if (should_force_display_location_object(location))
            {
                force_display_location = true;
            }
        }

        const bool res = o_display_location(instance, location);
        return force_display_location || res;
    }

    inline void hk_set_selected_location(Il2CppObject* instance, Il2CppObject* location)
    {
        apply_menu_location_flags(instance, location, true);
        o_set_selected_location(instance, location);
        apply_menu_location_flags(instance, location, true);
    }

    inline void hk_on_location_selected(Il2CppObject* instance, Il2CppObject* location)
    {
        apply_menu_location_flags(instance, location, true);
        o_on_location_selected(instance, location);
        apply_menu_location_flags(instance, location, true);
    }

    inline bool hk_should_hide_location_button(Il2CppObject* instance, Il2CppObject* location)
    {
        if (should_force_display_location_object(location))
        {
            return false;
        }

        return o_should_hide_location_button(instance, location);
    }

    inline bool hk_can_display_accept_button(Il2CppObject* instance, Il2CppObject* location, int side)
    {
        apply_menu_location_flags(instance, location);
        if (should_force_display_location_object(location) || has_forced_selected_location(instance))
        {
            return true;
        }

        return o_can_display_accept_button(instance, location, side);
    }

    inline Il2CppObject* hk_available_for_players(Il2CppObject* instance, Il2CppObject* location)
    {
        apply_menu_location_flags(instance, location);
        Il2CppObject* unavailable_players = o_available_for_players(instance, location);
        if (should_force_display_location_object(location))
        {
            clear_il2cpp_list(unavailable_players, "AvailableForPlayers");
        }

        return unavailable_players;
    }

    inline bool hk_raid_settings_get_local(Il2CppObject* instance)
    {
        if (raid_settings_forces_offline(instance))
        {
            force_raid_settings_offline(instance);
            return true;
        }

        return o_raid_settings_get_local(instance);
    }

    inline bool hk_raid_settings_get_online_pve_raid(Il2CppObject* instance)
    {
        if (raid_settings_forces_offline(instance))
        {
            force_raid_settings_offline(instance);
            return false;
        }

        return o_raid_settings_get_online_pve_raid(instance);
    }

    inline bool hk_raid_settings_need_to_go_online_in_pve(Il2CppObject* instance, int raid_mode)
    {
        if (raid_settings_forces_offline(instance))
        {
            force_raid_settings_offline(instance);
            return false;
        }

        return o_raid_settings_need_to_go_online_in_pve(instance, raid_mode);
    }

    inline bool hk_raid_settings_need_to_go_offline_in_pve(Il2CppObject* instance, int raid_mode)
    {
        if (raid_settings_forces_offline(instance))
        {
            force_raid_settings_offline(instance);
            return true;
        }

        return o_raid_settings_need_to_go_offline_in_pve(instance, raid_mode);
    }

    inline void hk_raid_settings_set_selected_location(Il2CppObject* instance, Il2CppObject* location)
    {
        update_selected_forced_location(location);

        Il2CppObject* selected_location = location;
        if (lower_copy(selected_forced_location_id) == labs_dark_id)
        {
            Il2CppObject* resolved_location =
                resolve_selected_location_object(
                    get_object_field(instance, "_locationSettings"));
            if (resolved_location != nullptr)
            {
                selected_location = resolved_location;
            }
            else
            {
                selected_forced_location_id =
                    get_location_identifier(location);
                selected_forced_location_object = location;
                logger::warn(
                    "[RaidSettings] Offline Labs Blackout fell back to "
                    "ordinary Labs");
            }
        }

        if (should_force_location_offline_object(selected_location))
        {
            Il2CppObjectInstance location_instance(selected_location);
            force_location_offline(location_instance);
        }

        o_raid_settings_set_selected_location(instance, selected_location);

        if (raid_settings_forces_offline(instance))
        {
            force_raid_settings_offline(instance);
        }
    }

    inline void hk_raid_settings_apply_from_backend(Il2CppObject* instance, Il2CppObject* other)
    {
        const bool force_after_apply = raid_settings_forces_offline(instance) || raid_settings_forces_offline(other);
        o_raid_settings_apply_from_backend(instance, other);

        if (force_after_apply || raid_settings_forces_offline(instance))
        {
            force_raid_settings_offline(instance);
        }
    }

    inline void hk_raid_settings_apply(Il2CppObject* instance, Il2CppObject* other)
    {
        const bool force_after_apply = raid_settings_forces_offline(instance) || raid_settings_forces_offline(other);
        o_raid_settings_apply(instance, other);

        if (force_after_apply || raid_settings_forces_offline(instance))
        {
            force_raid_settings_offline(instance);
        }
    }

    inline Il2CppObject* hk_raid_settings_get_update_status_params(Il2CppObject* instance)
    {
        if (raid_settings_forces_offline(instance))
        {
            force_raid_settings_offline(instance);
        }

        Il2CppObject* params = o_raid_settings_get_update_status_params(instance);
        if (raid_settings_forces_offline(instance))
        {
            force_backend_params_offline(params, "GetUpdateStatusParams");
            dump_object_class_and_values_once(params, "UpdateStatusParams", dumped_update_status_params_class);
        }

        return params;
    }

    inline Il2CppObject* hk_raid_settings_get_create_raid_group_params(Il2CppObject* instance)
    {
        if (raid_settings_forces_offline(instance))
        {
            force_raid_settings_offline(instance);
        }

        Il2CppObject* params = o_raid_settings_get_create_raid_group_params(instance);
        if (raid_settings_forces_offline(instance))
        {
            force_backend_params_offline(params, "GetCreateRaidGroupParams");
            dump_object_class_and_values_once(params, "CreateRaidGroupParams", dumped_create_raid_group_params_class);
        }

        return params;
    }

    inline Il2CppObject* hk_backend_send_raid_settings(Il2CppObject* instance, Il2CppObject* raid_settings)
    {
        if (selected_location_forces_offline)
        {
            force_raid_settings_offline(raid_settings);
            force_backend_params_offline(raid_settings, "SendRaidSettings");
            cache_forced_location_settings(raid_settings);
            insert_selected_location_into_settings(
                get_object_field(instance, "<LocationSettings>k__BackingField"),
                "BackendSession.LocationSettings");
            dump_object_class_and_values_once(
                raid_settings, "SendRaidSettingsParams", dumped_send_raid_settings_params_class);
            logger::info("[BackendSession] SendRaidSettings forced offline for '" + selected_forced_location_id + "'");
        }

        Il2CppObject* backend_raid_settings = raid_settings;
        if (selected_location_forces_offline &&
            selected_forced_location_is_icebreaker() &&
            raid_settings != nullptr &&
            raid_settings->klass != nullptr &&
            raid_settings->klass->instance_size > sizeof(Il2CppObject))
        {
            Il2CppObject* backend_location = resolve_active_backend_safe_location(instance);
            if (backend_location != nullptr && !active_backend_safe_location_id.empty())
            {
                Il2CppObject* clone = il2cpp::il2cpp_object_new(raid_settings->klass);
                if (clone != nullptr)
                {
                    const size_t field_bytes =
                        static_cast<size_t>(raid_settings->klass->instance_size) - sizeof(Il2CppObject);
                    std::memcpy(
                        reinterpret_cast<uint8_t*>(clone) + sizeof(Il2CppObject),
                        reinterpret_cast<uint8_t*>(raid_settings) + sizeof(Il2CppObject),
                        field_bytes);

                    set_string_field_if_exists(clone, "LocationId", active_backend_safe_location_id);
                    set_string_field_if_exists(clone, "locationId", active_backend_safe_location_id);
                    set_reference_field_if_exists(clone, "_selectedLocation", backend_location);
                    set_reference_field_if_exists(clone, "selectedLocation", backend_location);
                    set_reference_field_if_exists(clone, "SelectedLocation", backend_location);
                    backend_raid_settings = clone;

                    logger::info("[BackendSession] SendRaidSettings backend clone aliased Icebreaker -> '" +
                        active_backend_safe_location_id + "' selectedLocation='" +
                        safe_string(get_string_field(backend_location, "Id")) + "'");
                }
                else
                {
                    logger::warn("[BackendSession] failed to allocate Icebreaker SendRaidSettings backend clone");
                }
            }
            else
            {
                logger::warn("[BackendSession] SendRaidSettings kept Icebreaker because no backend-safe location was available");
            }
        }

        return o_backend_send_raid_settings(instance, backend_raid_settings);
    }

    inline Il2CppObject* hk_backend_local_raid_started(Il2CppObject* instance, Il2CppObject* raid_settings)
    {
        if (selected_location_forces_offline)
        {
            force_raid_settings_offline(raid_settings);
            force_local_raid_started_selected_location(raid_settings);
            cache_forced_location_settings(raid_settings);
            insert_selected_location_into_settings(
                get_object_field(instance, "<LocationSettings>k__BackingField"),
                "BackendSession.LocationSettings");

            const LocalRaidStartedSnapshot snapshot = capture_local_raid_started_snapshot(raid_settings);
            if (snapshot.server_id_text.empty())
            {
                set_string_field_if_exists(raid_settings, "serverId", "local");
            }

            dump_object_class_and_values_once(
                raid_settings, "LocalRaidStartedParams", dumped_local_raid_started_params_class);

            if (selected_forced_location_inserted_into_settings && selected_forced_location_is_labs())
            {
                last_backend_safe_local_raid_session = instance;
                last_backend_safe_local_raid_settings = raid_settings;
                last_backend_safe_local_raid_task = nullptr;
                last_backend_safe_local_raid_location_id.clear();
                last_real_local_raid_task = nullptr;
                last_real_local_raid_location_id = selected_forced_location_id;
                logged_real_local_raid_cached_result_missing = false;
                dumped_real_local_raid_started_result = false;
                patched_local_raid_started_results.clear();

                logger::warn("[BackendSession] forwarding LocalRaidStarted with real forced location '" +
                    selected_forced_location_id + "' after LocationSettings insertion");
                Il2CppObject* real_task = o_backend_local_raid_started(instance, raid_settings);
                if (real_task == nullptr)
                {
                    logger::warn("[BackendSession] real LocalRaidStarted returned null task for '" +
                        selected_forced_location_id + "'");
                    return nullptr;
                }

                last_real_local_raid_task = real_task;
                hook_local_raid_started_task_result_methods_for_task(real_task);
                return real_task;
            }

            last_real_local_raid_task = nullptr;
            last_real_local_raid_location_id.clear();
            return create_backend_safe_local_raid_started_task(
                instance,
                raid_settings,
                snapshot,
                o_backend_local_raid_started);
        }

        return o_backend_local_raid_started(instance, raid_settings);
    }

    inline Il2CppObject* hk_local_game_create(
        Il2CppObject* input_tree,
        Il2CppObject* profile,
        Il2CppObject* game_world,
        Il2CppObject* game_date_time,
        Il2CppObject* insurance_company,
        Il2CppObject* game_ui,
        Il2CppObject* location,
        uintptr_t time_and_weather_settings,
        uintptr_t waves_settings,
        int time_variant,
        Il2CppObject* callback,
        float fixed_delta_time,
        int update_queue,
        Il2CppObject* session,
        uintptr_t session_end_time,
        Il2CppObject* client_metrics_events,
        Il2CppObject* client_metrics_collector,
        Il2CppObject* local_raid_settings)
    {
        patch_local_game_create_location(location, local_raid_settings);
        return o_local_game_create(
            input_tree,
            profile,
            game_world,
            game_date_time,
            insurance_company,
            game_ui,
            location,
            time_and_weather_settings,
            waves_settings,
            time_variant,
            callback,
            fixed_delta_time,
            update_queue,
            session,
            session_end_time,
            client_metrics_events,
            client_metrics_collector,
            local_raid_settings);
    }

    inline Il2CppObject* hk_local_game_create_instance(
        Il2CppObject* instance,
        Il2CppObject* input_tree,
        Il2CppObject* profile,
        Il2CppObject* game_world,
        Il2CppObject* game_date_time,
        Il2CppObject* insurance_company,
        Il2CppObject* game_ui,
        Il2CppObject* location,
        uintptr_t time_and_weather_settings,
        uintptr_t waves_settings,
        int time_variant,
        Il2CppObject* callback,
        float fixed_delta_time,
        int update_queue,
        Il2CppObject* session,
        uintptr_t session_end_time,
        Il2CppObject* client_metrics_events,
        Il2CppObject* client_metrics_collector,
        Il2CppObject* local_raid_settings)
    {
        patch_local_game_create_location(location, local_raid_settings);
        return o_local_game_create_instance(
            instance,
            input_tree,
            profile,
            game_world,
            game_date_time,
            insurance_company,
            game_ui,
            location,
            time_and_weather_settings,
            waves_settings,
            time_variant,
            callback,
            fixed_delta_time,
            update_queue,
            session,
            session_end_time,
            client_metrics_events,
            client_metrics_collector,
            local_raid_settings);
    }

    inline void hk_matchmaker_receive_raid_settings(Il2CppObject* instance, Il2CppObject* payload)
    {
        force_backend_callback_location(
            payload,
            "ReceiveRaidSettingsParams",
            dumped_receive_raid_settings_params_class);
        o_matchmaker_receive_raid_settings(instance, payload);
    }

    inline void hk_matchmaker_receive_matching_start_notification(Il2CppObject* instance, Il2CppObject* payload)
    {
        force_backend_callback_location(
            payload,
            "ReceiveMatchingStartNotificationParams",
            dumped_matching_start_notification_params_class);
        o_matchmaker_receive_matching_start_notification(instance, payload);
    }

    using transit_sig = Il2CppObject * (*)(Il2CppObject*, Il2CppObject*, int, Il2CppString*, Il2CppObject*, Il2CppObject*);
    static inline transit_sig o_transit = nullptr;

    inline Il2CppObject* hk_transit(Il2CppObject* instance, Il2CppObject* point, int player_count, Il2CppString* hash, Il2CppObject* keys, Il2CppObject* player)
    {
        Il2CppObjectInstance point_inst(point);
        Il2CppObjectInstance params(point_inst.get_field<Il2CppObject*>("parameters"));

        const std::string location = il2utils::conv_string(params.get_field<Il2CppString*>("location"));
        const std::string name = il2utils::conv_string(params.get_field<Il2CppString*>("name"));

        if (!game_state::force_offline || should_return_original_transit(location, name)) // if labs
            return o_transit(instance, point, player_count, hash, keys, player);

        // force offline transit
        return o_transit(instance, point, 1, il2cpp::il2cpp_string_new(""), keys, player);
    }

    inline void init()
    {
        // set assembly csharp image
        while (!il2utils::resolve_image("Assembly-CSharp.dll"))
        {
            Sleep(50);
        }
        assembly_csharp = il2utils::resolve_image("Assembly-CSharp.dll");

        const Il2CppClass* raid_settings = il2utils::resolve_class(assembly_csharp, "EFT", "RaidSettings");
        il2utils::hook_method(raid_settings, "get_Local", 0,
            hk_raid_settings_get_local,
            &o_raid_settings_get_local);
        il2utils::hook_method(raid_settings, "get_OnlinePveRaid", 0,
            hk_raid_settings_get_online_pve_raid,
            &o_raid_settings_get_online_pve_raid);
        il2utils::hook_method(raid_settings, "NeedToGoOnlineInPve", 1,
            hk_raid_settings_need_to_go_online_in_pve,
            &o_raid_settings_need_to_go_online_in_pve);
        il2utils::hook_method(raid_settings, "NeedToGoOfflineInPve", 1,
            hk_raid_settings_need_to_go_offline_in_pve,
            &o_raid_settings_need_to_go_offline_in_pve);
        il2utils::hook_method(raid_settings, "set_SelectedLocation", 1,
            hk_raid_settings_set_selected_location,
            &o_raid_settings_set_selected_location);
        il2utils::hook_method(raid_settings, "ApplyFromBackend", 1,
            hk_raid_settings_apply_from_backend,
            &o_raid_settings_apply_from_backend);
        il2utils::hook_method(raid_settings, "Apply", 1,
            hk_raid_settings_apply,
            &o_raid_settings_apply);
        il2utils::hook_method(raid_settings, "GetUpdateStatusParams", 0,
            hk_raid_settings_get_update_status_params,
            &o_raid_settings_get_update_status_params);
        il2utils::hook_method(raid_settings, "GetCreateRaidGroupParams", 0,
            hk_raid_settings_get_create_raid_group_params,
            &o_raid_settings_get_create_raid_group_params);

        const Il2CppClass* eft_client_backend_session = il2utils::resolve_class(
            assembly_csharp, "EFT", "EftClientBackendSession");
        il2utils::hook_method(eft_client_backend_session, "SendRaidSettings", 1,
            hk_backend_send_raid_settings,
            &o_backend_send_raid_settings);
        local_raid_started_method = il2utils::resolve_method(eft_client_backend_session, "LocalRaidStarted", 1);
        il2utils::hook_method(eft_client_backend_session, "LocalRaidStarted", 1,
            hk_backend_local_raid_started,
            &o_backend_local_raid_started);

        if (!dumped_client_backend_session_class)
        {
            dumped_client_backend_session_class = true;
            dump_class_layout(eft_client_backend_session, "EftClientBackendSession");
        }

        const Il2CppClass* local_game = il2utils::resolve_class(assembly_csharp, "EFT", "LocalGame");
        if (local_game != nullptr && !dumped_local_game_class)
        {
            dumped_local_game_class = true;
            dump_class_layout(local_game, "LocalGame");
        }
        const MethodInfo* local_game_create = local_game != nullptr ?
            il2utils::resolve_method(local_game, "Create", 18) :
            nullptr;
        if (local_game_create != nullptr && local_game_create->methodPointer != nullptr)
        {
            constexpr uint16_t method_attribute_static = 0x0010;
            const bool is_static_create = (local_game_create->flags & method_attribute_static) != 0;
            if (!logged_local_game_create_hook)
            {
                logged_local_game_create_hook = true;
                logger::info("[LocalGameCreate] method flags=" +
                    std::to_string(local_game_create->flags) +
                    " static=" + std::to_string(is_static_create ? 1 : 0));
            }

            if (is_static_create)
            {
                il2utils::hook_method(local_game, "Create", 18,
                    hk_local_game_create,
                    &o_local_game_create);
            }
            else
            {
                il2utils::hook_method(local_game, "Create", 18,
                    hk_local_game_create_instance,
                    &o_local_game_create_instance);
            }
        }

        const Il2CppClass* cutscenes_client_controller = il2utils::resolve_class(
            assembly_csharp,
            "CommonAssets.Scripts.Cutscenes",
            "CutscenesClientController");
        il2utils::hook_method(cutscenes_client_controller, "OnUpdate", 1,
            hk_cutscenes_client_controller_on_update,
            &o_cutscenes_client_controller_on_update);

        const Il2CppClass* matchmaker_players_controller = il2utils::resolve_class(
            assembly_csharp, "EFT.UI.Matchmaker", "MatchmakerPlayersController");
        il2utils::hook_method(matchmaker_players_controller, "ReceiveRaidSettings", 1,
            hk_matchmaker_receive_raid_settings,
            &o_matchmaker_receive_raid_settings);
        il2utils::hook_method(matchmaker_players_controller, "ReceiveMatchingStartNotification", 1,
            hk_matchmaker_receive_matching_start_notification,
            &o_matchmaker_receive_matching_start_notification);

        // always offline raid and location unlock
        const Il2CppClass* match_maker_selection_location_screen = il2utils::resolve_class(
            assembly_csharp, "EFT.UI.Matchmaker", "MatchMakerSelectionLocationScreen");
        il2utils::hook_method(match_maker_selection_location_screen, "DisplayLocation", 1,
            hk_display_location,
            &o_display_location);
        il2utils::hook_method(match_maker_selection_location_screen, "set_SelectedLocation", 1,
            hk_set_selected_location,
            &o_set_selected_location);
        il2utils::hook_method(match_maker_selection_location_screen, "OnLocationSelected", 1,
            hk_on_location_selected,
            &o_on_location_selected);
        il2utils::hook_method(match_maker_selection_location_screen, "ShouldHideLocationButton", 1,
            hk_should_hide_location_button,
            &o_should_hide_location_button);
        il2utils::hook_method(match_maker_selection_location_screen, "CanDisplayAcceptButton", 2,
            hk_can_display_accept_button,
            &o_can_display_accept_button);
        il2utils::hook_method(match_maker_selection_location_screen, "AvailableForPlayers", 1,
            hk_available_for_players,
            &o_available_for_players);

        // offline transits
        const Il2CppClass* transit_controller = il2utils::resolve_class(
            assembly_csharp, "EFT", "LocalTransitController");
        il2utils::hook_method(transit_controller, "Transit", 5,
            hk_transit,
            &o_transit);
    }
}
