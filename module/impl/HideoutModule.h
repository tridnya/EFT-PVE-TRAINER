#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <sstream>
#include <string>

#include "../Module.h"
#include "../ModuleCategory.h"
#include "../../config/ActionRowValue.h"
#include "../../config/CheckboxValue.h"
#include "../../config/IntSliderValue.h"
#include "../../config/SelectValue.h"
#include "../../il2cpp/il2utils.h"
#include "../../il2cpp/unity.h"
#include "../../util/logger.h"

class HideoutModule : Module
{
public:
    HideoutModule() : Module("Hideout", Progression)
    {
    }

    ActionRowValue* dump_hideout_state = conf(new ActionRowValue([this]
    {
        dump_requested_.store(true, std::memory_order_release);
        logger::info("[HideoutProbe] queued live Hideout state capture");
    }, "Dump Hideout State"));
    ActionRowValue* dump_hideout_runtime = conf(new ActionRowValue([this]
    {
        runtime_dump_requested_.store(true, std::memory_order_release);
        logger::info("[HideoutRuntimeProbe] queued loaded hideout runtime capture");
    }, "Dump Hideout Runtime"));

    SelectValue* area_selection = conf(new SelectValue(0, {
        "Vents", "Security", "Lavatory", "Stash", "Generator", "Heating",
        "Water Collector", "Medstation", "Nutrition Unit", "Rest Space", "Workbench",
        "Intelligence Center", "Shooting Range", "Library", "Scav Case", "Illumination",
        "Hall of Fame", "Air Filtering Unit", "Solar Power", "Booze Generator",
        "Bitcoin Farm", "Christmas Tree", "Defective Wall", "Gym", "Weapon Stand",
        "Secondary Weapon Stand", "Gear Rack", "Cultist Circle"
    }, "Area"));
    IntSliderValue* area_level = conf(new IntSliderValue(0, 0, 4, "Area Level"));
    CheckboxValue* area_active = conf(new CheckboxValue(true, "Area Active"));
    CheckboxValue* passive_bonuses = conf(new CheckboxValue(true, "Passive Bonuses"));
    ActionRowValue* apply_area_state = conf(new ActionRowValue([this]
    {
        apply_area_state_requested_.store(true, std::memory_order_release);
        logger::info("[HideoutTrainer] queued selected area state update");
    }, "Apply Area State"));
    ActionRowValue* activate_all_areas = conf(new ActionRowValue([this]
    {
        activate_all_requested_.store(true, std::memory_order_release);
        logger::info("[HideoutTrainer] queued activation for all live areas");
    }, "Activate All Areas"));
    ActionRowValue* enable_all_bonuses = conf(new ActionRowValue([this]
    {
        enable_all_bonuses_requested_.store(true, std::memory_order_release);
        logger::info("[HideoutTrainer] queued passive bonuses for all live areas");
    }, "Enable All Bonuses"));

    void draw_overlay(ImDrawList* draw_list) override
    {
        (void)draw_list;
    }

    void application_update() override
    {
        if (apply_area_state_requested_.exchange(false, std::memory_order_acq_rel))
        {
            apply_selected_area_state();
        }
        if (activate_all_requested_.exchange(false, std::memory_order_acq_rel))
        {
            set_all_area_flags(true, false);
        }
        if (enable_all_bonuses_requested_.exchange(false, std::memory_order_acq_rel))
        {
            set_all_area_flags(false, true);
        }
        if (dump_requested_.exchange(false, std::memory_order_acq_rel))
        {
            dump_hideout_state_impl();
        }
        if (runtime_dump_requested_.exchange(false, std::memory_order_acq_rel))
        {
            dump_hideout_runtime_impl();
        }
    }

    void gameworld_update(const Il2CppClass* game_world_class, Il2CppObjectInstance game_world_instance,
                          Il2CppObjectInstance main_player) override
    {
        (void)game_world_class;
        (void)game_world_instance;
        (void)main_player;
    }

    void reset_gameworld_state() override
    {
    }

    void init() override
    {
        active_instance_ = this;

        const Il2CppImage* assembly_csharp = il2utils::resolve_image("Assembly-CSharp.dll");
        assembly_csharp_ = assembly_csharp;
        const Il2CppClass* backend_session_class =
            il2utils::resolve_class(assembly_csharp, "EFT", "EftClientBackendSession");
        const MethodInfo* get_hideout_areas_info = backend_session_class != nullptr ?
            il2utils::resolve_method(backend_session_class, "GetHideoutAreasInfo", 0) :
            nullptr;
        const MethodInfo* get_php_session_id = backend_session_class != nullptr ?
            il2utils::resolve_method(backend_session_class, "GetPhpSessionId", 0) :
            nullptr;
        const MethodInfo* get_session_id = backend_session_class != nullptr ?
            il2utils::resolve_method(backend_session_class, "GetSessionId", 1) :
            nullptr;
        const MethodInfo* get_hideout_settings = backend_session_class != nullptr ?
            il2utils::resolve_method(backend_session_class, "GetHideoutSettings", 0) :
            nullptr;

        if (get_hideout_areas_info == nullptr || get_hideout_areas_info->methodPointer == nullptr)
        {
            logger::warn("[HideoutProbe] EftClientBackendSession.GetHideoutAreasInfo was not resolved");
            return;
        }

        il2utils::hook_method(
            backend_session_class,
            "GetHideoutAreasInfo",
            0,
            hk_get_hideout_areas_info,
            &o_get_hideout_areas_info_);

        std::size_t session_capture_hook_count = 0;
        if (get_php_session_id != nullptr && get_php_session_id->methodPointer != nullptr)
        {
            il2utils::hook_method(
                backend_session_class,
                "GetPhpSessionId",
                0,
                hk_get_php_session_id,
                &o_get_php_session_id_);
            ++session_capture_hook_count;
        }
        if (get_session_id != nullptr && get_session_id->methodPointer != nullptr)
        {
            il2utils::hook_method(
                backend_session_class,
                "GetSessionId",
                1,
                hk_get_session_id,
                &o_get_session_id_);
            ++session_capture_hook_count;
        }
        if (get_hideout_settings != nullptr && get_hideout_settings->methodPointer != nullptr)
        {
            il2utils::hook_method(
                backend_session_class,
                "GetHideoutSettings",
                0,
                hk_get_hideout_settings,
                &o_get_hideout_settings_);
            ++session_capture_hook_count;
        }

        logger::info("[HideoutProbe] ready; backend_session_capture_hooks=" +
            std::to_string(session_capture_hook_count));
        if (session_capture_hook_count == 0)
        {
            logger::warn("[HideoutProbe] no backend-session capture method was resolved");
        }
    }

private:
    using get_hideout_areas_info_sig = Il2CppArray*(*)(Il2CppObject*);
    using get_php_session_id_sig = Il2CppString*(*)(Il2CppObject*);
    using get_session_id_sig = Il2CppString*(*)(Il2CppObject*, bool);
    using get_hideout_settings_sig = Il2CppObject*(*)(Il2CppObject*);
    using image_get_class_count_sig = std::size_t(*)(const Il2CppImage*);
    using image_get_class_sig = Il2CppClass*(*)(const Il2CppImage*, std::size_t);

    static constexpr std::size_t max_area_count_ = 64;
    static constexpr std::size_t max_class_depth_ = 8;
    static constexpr std::size_t max_layout_fields_ = 128;
    static constexpr std::size_t max_layout_methods_ = 160;
    static constexpr std::size_t max_slot_count_ = 16;
    static constexpr std::size_t max_value_fields_ = 96;
    static constexpr std::size_t area_type_count_ = 28;
    static constexpr std::size_t max_runtime_candidate_classes_ = 160;
    static constexpr std::size_t max_runtime_live_classes_ = 32;
    static constexpr std::size_t max_runtime_objects_ = 64;
    static constexpr std::size_t max_runtime_objects_per_class_ = 8;
    static constexpr std::size_t max_runtime_fields_per_class_ = 48;
    static constexpr std::size_t max_runtime_methods_per_class_ = 32;

    struct AreaFields
    {
        FieldInfo* active = nullptr;
        FieldInfo* type = nullptr;
        FieldInfo* level = nullptr;
        FieldInfo* passive_bonuses_enabled = nullptr;
    };

    struct AreaOverride
    {
        bool enabled = false;
        int level = 0;
        bool active = true;
        bool passive_bonuses_enabled = true;
    };

    inline static HideoutModule* active_instance_ = nullptr;
    inline static get_hideout_areas_info_sig o_get_hideout_areas_info_ = nullptr;
    inline static get_php_session_id_sig o_get_php_session_id_ = nullptr;
    inline static get_session_id_sig o_get_session_id_ = nullptr;
    inline static get_hideout_settings_sig o_get_hideout_settings_ = nullptr;

    std::atomic_bool dump_requested_ = false;
    std::atomic_bool runtime_dump_requested_ = false;
    std::atomic_bool apply_area_state_requested_ = false;
    std::atomic_bool activate_all_requested_ = false;
    std::atomic_bool enable_all_bonuses_requested_ = false;
    std::atomic_bool force_all_active_ = false;
    std::atomic_bool force_all_bonuses_ = false;
    std::array<AreaOverride, area_type_count_> area_overrides_{};
    std::mutex area_overrides_mutex_;
    const Il2CppImage* assembly_csharp_ = nullptr;
    Il2CppObject* backend_session_ = nullptr;
    Il2CppArray* hideout_areas_ = nullptr;

    static bool is_field_type(const FieldInfo* field, const Il2CppTypeEnum type)
    {
        return field != nullptr && field->offset > 0 && field->type != nullptr &&
            field->type->type == type;
    }

    static bool resolve_area_fields(Il2CppClass* area_class, AreaFields& fields)
    {
        if (area_class == nullptr)
        {
            return false;
        }

        fields.active = il2utils::resolve_field(area_class, "Active");
        fields.type = il2utils::resolve_field(area_class, "Type");
        fields.level = il2utils::resolve_field(area_class, "Level");
        fields.passive_bonuses_enabled =
            il2utils::resolve_field(area_class, "PassiveBonusesEnabled");

        return is_field_type(fields.active, IL2CPP_TYPE_BOOLEAN) &&
            is_field_type(fields.type, IL2CPP_TYPE_I4) &&
            is_field_type(fields.level, IL2CPP_TYPE_I4) &&
            is_field_type(fields.passive_bonuses_enabled, IL2CPP_TYPE_BOOLEAN);
    }

    static bool read_bool_field(Il2CppObject* object, const FieldInfo* field)
    {
        return *reinterpret_cast<const bool*>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
    }

    static int read_int_field(Il2CppObject* object, const FieldInfo* field)
    {
        return *reinterpret_cast<const int*>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
    }

    static void write_bool_field(Il2CppObject* object, const FieldInfo* field, const bool value)
    {
        *reinterpret_cast<bool*>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset) = value;
    }

    static void write_int_field(Il2CppObject* object, const FieldInfo* field, const int value)
    {
        *reinterpret_cast<int*>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset) = value;
    }

    static void capture_backend_session(Il2CppObject* instance)
    {
        HideoutModule* module = active_instance_;
        if (module != nullptr && instance != nullptr)
        {
            module->backend_session_ = instance;
        }
    }

    static Il2CppArray* hk_get_hideout_areas_info(Il2CppObject* instance)
    {
        Il2CppArray* areas = o_get_hideout_areas_info_ != nullptr ?
            o_get_hideout_areas_info_(instance) :
            nullptr;

        capture_backend_session(instance);

        HideoutModule* module = active_instance_;
        if (module != nullptr && areas != nullptr)
        {
            module->apply_area_overrides(areas, false);
            module->hideout_areas_ = areas;
        }

        return areas;
    }

    static Il2CppString* hk_get_php_session_id(Il2CppObject* instance)
    {
        Il2CppString* result = o_get_php_session_id_ != nullptr ?
            o_get_php_session_id_(instance) :
            nullptr;
        capture_backend_session(instance);
        return result;
    }

    static Il2CppString* hk_get_session_id(Il2CppObject* instance, const bool include_prefix)
    {
        Il2CppString* result = o_get_session_id_ != nullptr ?
            o_get_session_id_(instance, include_prefix) :
            nullptr;
        capture_backend_session(instance);
        return result;
    }

    static Il2CppObject* hk_get_hideout_settings(Il2CppObject* instance)
    {
        Il2CppObject* result = o_get_hideout_settings_ != nullptr ?
            o_get_hideout_settings_(instance) :
            nullptr;
        capture_backend_session(instance);
        return result;
    }

    static std::string to_lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        });
        return value;
    }

    static std::string class_name(const Il2CppClass* klass)
    {
        if (klass == nullptr)
        {
            return "<null>";
        }

        const std::string namespaze = klass->namespaze != nullptr ? klass->namespaze : "";
        const std::string name = klass->name != nullptr ? klass->name : "<unnamed>";
        return namespaze.empty() ? name : namespaze + "." + name;
    }

    static std::string type_name(const Il2CppType* type)
    {
        if (type == nullptr)
        {
            return "<null>";
        }

        char* raw_name = il2cpp::il2cpp_type_get_name(type);
        if (raw_name == nullptr)
        {
            return "<unknown>";
        }

        const std::string result(raw_name);
        il2cpp::il2cpp_free(raw_name);
        return result;
    }

    static std::string safe_string(const Il2CppString* value)
    {
        if (value == nullptr)
        {
            return "";
        }

        constexpr int max_string_length = 512;
        const int length = (std::min)(value->length, max_string_length);
        std::string result;
        result.reserve(static_cast<std::size_t>(length));
        for (int index = 0; index < length; ++index)
        {
            const wchar_t character = value->chars[index];
            result.push_back(character >= 32 && character <= 126 ? static_cast<char>(character) : '?');
        }
        if (value->length > max_string_length)
        {
            result += "...";
        }
        return result;
    }

    static bool is_reference_type(const Il2CppType* type)
    {
        if (type == nullptr)
        {
            return false;
        }
        if (type->valuetype)
        {
            return false;
        }

        return type->type == IL2CPP_TYPE_CLASS ||
            type->type == IL2CPP_TYPE_OBJECT ||
            type->type == IL2CPP_TYPE_GENERICINST ||
            type->type == IL2CPP_TYPE_ARRAY ||
            type->type == IL2CPP_TYPE_SZARRAY;
    }

    static std::string field_value(Il2CppObject* object, const FieldInfo& field)
    {
        if (object == nullptr || field.offset <= 0 || field.type == nullptr)
        {
            return "<static/skipped>";
        }

        const uintptr_t address = reinterpret_cast<uintptr_t>(object) +
            static_cast<uintptr_t>(field.offset);
        switch (field.type->type)
        {
        case IL2CPP_TYPE_BOOLEAN:
            return *reinterpret_cast<const bool*>(address) ? "true" : "false";
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
        case IL2CPP_TYPE_I8:
            return std::to_string(*reinterpret_cast<const int64_t*>(address));
        case IL2CPP_TYPE_U8:
            return std::to_string(*reinterpret_cast<const uint64_t*>(address));
        case IL2CPP_TYPE_R4:
            return std::to_string(*reinterpret_cast<const float*>(address));
        case IL2CPP_TYPE_R8:
            return std::to_string(*reinterpret_cast<const double*>(address));
        case IL2CPP_TYPE_STRING:
        {
            const auto* value = *reinterpret_cast<Il2CppString* const*>(address);
            return value != nullptr ? "'" + safe_string(value) + "'" : "null";
        }
        case IL2CPP_TYPE_VALUETYPE:
            return "<valuetype>";
        default:
            break;
        }

        if (!is_reference_type(field.type))
        {
            return "<unread>";
        }

        Il2CppObject* child = *reinterpret_cast<Il2CppObject* const*>(address);
        if (child == nullptr)
        {
            return "null";
        }

        std::string value = "class='" + class_name(child->klass) + "'";
        if (field.type->type == IL2CPP_TYPE_ARRAY || field.type->type == IL2CPP_TYPE_SZARRAY)
        {
            const auto* array = reinterpret_cast<const Il2CppArray*>(child);
            value += " length=" + std::to_string(static_cast<std::size_t>(array->max_length));
        }
        return value;
    }

    static bool is_hideout_related(const std::string& field_name, const std::string& field_type)
    {
        const std::string searchable = to_lower(field_name + " " + field_type);
        return searchable.find("hideout") != std::string::npos ||
            searchable.find("production") != std::string::npos ||
            searchable.find("craft") != std::string::npos ||
            searchable.find("improvement") != std::string::npos ||
            searchable.find("generator") != std::string::npos ||
            searchable.find("area") != std::string::npos ||
            searchable.find("profile") != std::string::npos;
    }

    static bool contains_runtime_member_term(const std::string& value)
    {
        const std::string searchable = to_lower(value);
        return searchable.find("hideout") != std::string::npos ||
            searchable.find("area") != std::string::npos ||
            searchable.find("level") != std::string::npos ||
            searchable.find("stage") != std::string::npos ||
            searchable.find("improvement") != std::string::npos ||
            searchable.find("production") != std::string::npos ||
            searchable.find("current") != std::string::npos ||
            searchable.find("selected") != std::string::npos ||
            searchable.find("profile") != std::string::npos ||
            searchable.find("info") != std::string::npos ||
            searchable.find("data") != std::string::npos ||
            searchable.find("status") != std::string::npos;
    }

    static int runtime_candidate_priority(const Il2CppClass* klass)
    {
        if (klass == nullptr)
        {
            return -1;
        }

        const std::string namespaze = to_lower(klass->namespaze != nullptr ? klass->namespaze : "");
        const std::string name = to_lower(klass->name != nullptr ? klass->name : "");
        const std::string full_name = namespaze + "." + name;
        if (full_name.find("hideout") != std::string::npos)
        {
            return 0;
        }

        const bool area_named = name.find("area") != std::string::npos ||
            name.find("stage") != std::string::npos ||
            name.find("improvement") != std::string::npos ||
            name.find("production") != std::string::npos;
        if (namespaze.find("eft.ui") != std::string::npos && area_named)
        {
            return 1;
        }
        if ((namespaze == "eft" || namespaze.find("eft.") == 0) && area_named)
        {
            return 2;
        }
        return -1;
    }

    static bool inherits_from(
        const Il2CppClass* klass,
        const char* expected_namespaze,
        const char* expected_name)
    {
        std::size_t depth = 0;
        for (const Il2CppClass* cursor = klass;
             cursor != nullptr && depth < 64;
             cursor = cursor->parent, ++depth)
        {
            const char* namespaze = cursor->namespaze != nullptr ? cursor->namespaze : "";
            const char* name = cursor->name != nullptr ? cursor->name : "";
            if (std::strcmp(namespaze, expected_namespaze) == 0 &&
                std::strcmp(name, expected_name) == 0)
            {
                return true;
            }
        }
        return false;
    }

    static std::string runtime_game_object_name(Il2CppObject* object)
    {
        if (object == nullptr || object->klass == nullptr ||
            !inherits_from(object->klass, "UnityEngine", "Component") ||
            unity::component_get_game_object_internal == nullptr ||
            unity::get_name_internal == nullptr)
        {
            return "";
        }

        Il2CppObject* game_object = unity::component_get_game_object_internal(object);
        Il2CppString* name = game_object != nullptr ? unity::get_name_internal(game_object) : nullptr;
        return name != nullptr ? safe_string(name) : "";
    }

    static void dump_runtime_class_members(const Il2CppClass* klass, const std::string& label)
    {
        std::size_t logged_fields = 0;
        std::size_t depth = 0;
        for (const Il2CppClass* cursor = klass;
             cursor != nullptr && depth < max_class_depth_ &&
                 logged_fields < max_runtime_fields_per_class_;
             cursor = cursor->parent, ++depth)
        {
            if (cursor->fields == nullptr)
            {
                continue;
            }
            for (uint16_t index = 0;
                 index < cursor->field_count && logged_fields < max_runtime_fields_per_class_;
                 ++index)
            {
                const FieldInfo& field = cursor->fields[index];
                const std::string name = field.name != nullptr ? field.name : "<null>";
                const std::string resolved_type_name = type_name(field.type);
                if (!contains_runtime_member_term(name + " " + resolved_type_name))
                {
                    continue;
                }

                logger::info("[" + label + "Fields] #" + std::to_string(logged_fields++) +
                    " declaring_class='" + class_name(cursor) +
                    "' name='" + name +
                    "' offset=" + std::to_string(field.offset) +
                    " type_name='" + resolved_type_name + "'");
            }
        }

        std::size_t logged_methods = 0;
        depth = 0;
        for (const Il2CppClass* cursor = klass;
             cursor != nullptr && depth < max_class_depth_ &&
                 logged_methods < max_runtime_methods_per_class_;
             cursor = cursor->parent, ++depth)
        {
            if (cursor->methods == nullptr)
            {
                continue;
            }
            for (uint16_t index = 0;
                 index < cursor->method_count && logged_methods < max_runtime_methods_per_class_;
                 ++index)
            {
                const MethodInfo* method = cursor->methods[index];
                if (method == nullptr || method->name == nullptr ||
                    !contains_runtime_member_term(method->name))
                {
                    continue;
                }

                logger::info("[" + label + "Methods] #" + std::to_string(logged_methods++) +
                    " declaring_class='" + class_name(cursor) +
                    "' name='" + method->name +
                    "' params=" + std::to_string(method->parameters_count) +
                    " return_name='" + type_name(method->return_type) + "'");
            }
        }
    }

    static void dump_runtime_object_values(Il2CppObject* object, const std::string& label)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return;
        }

        std::size_t logged_fields = 0;
        std::size_t depth = 0;
        for (Il2CppClass* cursor = object->klass;
             cursor != nullptr && depth < max_class_depth_ &&
                 logged_fields < max_runtime_fields_per_class_;
             cursor = cursor->parent, ++depth)
        {
            if (cursor->fields == nullptr)
            {
                continue;
            }
            for (uint16_t index = 0;
                 index < cursor->field_count && logged_fields < max_runtime_fields_per_class_;
                 ++index)
            {
                const FieldInfo& field = cursor->fields[index];
                const std::string name = field.name != nullptr ? field.name : "<null>";
                const std::string resolved_type_name = type_name(field.type);
                if (!contains_runtime_member_term(name + " " + resolved_type_name))
                {
                    continue;
                }

                logger::info("[" + label + "Values] #" + std::to_string(logged_fields++) +
                    " declaring_class='" + class_name(cursor) +
                    "' name='" + name +
                    "' type_name='" + resolved_type_name +
                    "' value=" + field_value(object, field));
            }
        }
    }

    static void dump_class_layout(const Il2CppClass* klass, const std::string& label)
    {
        if (klass == nullptr)
        {
            logger::warn("[" + label + "] class is null");
            return;
        }

        logger::info("[" + label + "] class='" + class_name(klass) +
            "' fields=" + std::to_string(klass->field_count) +
            " methods=" + std::to_string(klass->method_count));

        std::size_t field_count = 0;
        std::size_t class_depth = 0;
        for (const Il2CppClass* cursor = klass;
             cursor != nullptr && class_depth < max_class_depth_ && field_count < max_layout_fields_;
             cursor = cursor->parent, ++class_depth)
        {
            if (cursor->fields == nullptr)
            {
                continue;
            }

            for (uint16_t index = 0;
                 index < cursor->field_count && field_count < max_layout_fields_;
                 ++index, ++field_count)
            {
                const FieldInfo& field = cursor->fields[index];
                logger::info("[" + label + "Fields] #" + std::to_string(field_count) +
                    " declaring_class='" + class_name(cursor) +
                    "' name='" + std::string(field.name != nullptr ? field.name : "<null>") +
                    "' offset=" + std::to_string(field.offset) +
                    " type=" + std::to_string(field.type != nullptr ? static_cast<int>(field.type->type) : -1) +
                    " type_name='" + type_name(field.type) + "'");
            }
        }

        std::size_t method_count = 0;
        class_depth = 0;
        for (const Il2CppClass* cursor = klass;
             cursor != nullptr && class_depth < max_class_depth_ && method_count < max_layout_methods_;
             cursor = cursor->parent, ++class_depth)
        {
            if (cursor->methods == nullptr)
            {
                continue;
            }

            for (uint16_t index = 0;
                 index < cursor->method_count && method_count < max_layout_methods_;
                 ++index)
            {
                const MethodInfo* method = cursor->methods[index];
                if (method == nullptr)
                {
                    continue;
                }

                logger::info("[" + label + "Methods] #" + std::to_string(method_count++) +
                    " declaring_class='" + class_name(cursor) +
                    "' name='" + std::string(method->name != nullptr ? method->name : "<null>") +
                    "' params=" + std::to_string(method->parameters_count) +
                    " return_name='" + type_name(method->return_type) + "'");
            }
        }
    }

    static void dump_object_values(Il2CppObject* object, const std::string& label, const bool relevant_only)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            logger::warn("[" + label + "] object is null");
            return;
        }

        std::size_t logged_fields = 0;
        std::size_t class_depth = 0;
        for (Il2CppClass* cursor = object->klass;
             cursor != nullptr && class_depth < max_class_depth_ && logged_fields < max_value_fields_;
             cursor = cursor->parent, ++class_depth)
        {
            if (cursor->fields == nullptr)
            {
                continue;
            }

            for (uint16_t index = 0;
                 index < cursor->field_count && logged_fields < max_value_fields_;
                 ++index)
            {
                const FieldInfo& field = cursor->fields[index];
                const std::string name = field.name != nullptr ? field.name : "<null>";
                const std::string resolved_type_name = type_name(field.type);
                if (relevant_only && !is_hideout_related(name, resolved_type_name))
                {
                    continue;
                }

                logger::info("[" + label + "Values] #" + std::to_string(logged_fields++) +
                    " declaring_class='" + class_name(cursor) +
                    "' name='" + name +
                    "' type_name='" + resolved_type_name +
                    "' value=" + field_value(object, field));
            }
        }
    }

    static Il2CppObject* find_reference_by_declared_type(
        Il2CppObject* owner,
        const std::string& expected_type_name)
    {
        if (owner == nullptr || owner->klass == nullptr)
        {
            return nullptr;
        }

        std::size_t class_depth = 0;
        for (Il2CppClass* cursor = owner->klass;
             cursor != nullptr && class_depth < max_class_depth_;
             cursor = cursor->parent, ++class_depth)
        {
            if (cursor->fields == nullptr)
            {
                continue;
            }

            for (uint16_t index = 0; index < cursor->field_count; ++index)
            {
                const FieldInfo& field = cursor->fields[index];
                if (field.offset <= 0 || !is_reference_type(field.type))
                {
                    continue;
                }

                if (type_name(field.type) != expected_type_name)
                {
                    continue;
                }

                const uintptr_t address = reinterpret_cast<uintptr_t>(owner) +
                    static_cast<uintptr_t>(field.offset);
                Il2CppObject* value = *reinterpret_cast<Il2CppObject* const*>(address);
                if (value != nullptr)
                {
                    return value;
                }
            }
        }
        return nullptr;
    }

    static Il2CppArray* get_array_field(
        Il2CppObject* owner,
        const char* field_name,
        const std::string& expected_type_name)
    {
        if (owner == nullptr || owner->klass == nullptr)
        {
            return nullptr;
        }

        const FieldInfo* field = il2utils::resolve_field(owner->klass, field_name);
        if (field == nullptr || field->offset <= 0 || field->type == nullptr ||
            type_name(field->type) != expected_type_name ||
            (field->type->type != IL2CPP_TYPE_ARRAY && field->type->type != IL2CPP_TYPE_SZARRAY))
        {
            return nullptr;
        }

        const uintptr_t address = reinterpret_cast<uintptr_t>(owner) +
            static_cast<uintptr_t>(field->offset);
        return *reinterpret_cast<Il2CppArray* const*>(address);
    }

    Il2CppObject* find_loaded_runtime_area(
        const int requested_type,
        std::size_t& matched_area_index) const
    {
        if (assembly_csharp_ == nullptr || unity::find_objects_of_type_internal == nullptr)
        {
            return nullptr;
        }

        const Il2CppClass* hideout_area_class =
            il2utils::resolve_class(assembly_csharp_, "EFT.Hideout", "HideoutArea");
        if (hideout_area_class == nullptr)
        {
            return nullptr;
        }

        const Il2CppType* hideout_area_type = il2cpp::il2cpp_class_get_type(
            const_cast<Il2CppClass*>(hideout_area_class));
        Il2CppObject* system_type = hideout_area_type != nullptr ?
            il2cpp::il2cpp_type_get_object(hideout_area_type) : nullptr;
        Il2CppArray* area_objects = system_type != nullptr ?
            unity::find_objects_of_type_internal(system_type) : nullptr;
        if (area_objects == nullptr)
        {
            return nullptr;
        }

        const std::size_t area_count = (std::min)(
            static_cast<std::size_t>(area_objects->max_length), max_area_count_);
        auto** area_values = reinterpret_cast<Il2CppObject**>(&area_objects->data);
        for (std::size_t area_index = 0; area_index < area_count; ++area_index)
        {
            Il2CppObject* area = area_values[area_index];
            Il2CppObject* area_template = find_reference_by_declared_type(
                area, "EFT.Hideout.AreaTemplate");
            if (area_template == nullptr || area_template->klass == nullptr)
            {
                continue;
            }

            const FieldInfo* type_field = il2utils::resolve_field(area_template->klass, "Type");
            if (type_field == nullptr || type_field->offset <= 0 || type_field->type == nullptr ||
                type_field->type->type != IL2CPP_TYPE_VALUETYPE ||
                type_name(type_field->type) != "EFT.EAreaType")
            {
                continue;
            }
            if (read_int_field(area_template, type_field) != requested_type)
            {
                continue;
            }

            matched_area_index = area_index;
            return area;
        }

        return nullptr;
    }

    bool apply_runtime_area_level(
        Il2CppObject* area,
        const std::size_t area_index,
        const int requested_type,
        const int requested_level) const
    {
        if (area == nullptr || area->klass == nullptr)
        {
            logger::info("[HideoutTrainerRuntime] no loaded runtime area matched type=" +
                std::to_string(requested_type));
            return false;
        }

        Il2CppObject* area_data = find_reference_by_declared_type(
            area, "EFT.Hideout.AreaData");
        Il2CppArray* area_levels = get_array_field(
            area, "_areaLevels", "EFT.Hideout.HideoutAreaLevel[]");
        if (area_data == nullptr || area_data->klass == nullptr || area_levels == nullptr)
        {
            logger::warn("[HideoutTrainerRuntime] matched area is missing AreaData or levels");
            return false;
        }

        const std::size_t level_count = static_cast<std::size_t>(area_levels->max_length);
        if (requested_level < 0 || static_cast<std::size_t>(requested_level) >= level_count)
        {
            logger::warn("[HideoutTrainerRuntime] rejected type=" +
                std::to_string(requested_type) +
                " level=" + std::to_string(requested_level) +
                " available_levels=" + std::to_string(level_count));
            return false;
        }

        const FieldInfo* current_level_field =
            il2utils::resolve_field(area_data->klass, "_currentLevel");
        const MethodInfo* current_level_setter =
            il2utils::resolve_method(area_data->klass, "set_CurrentLevel", 1);
        const MethodInfo* switch_area_levels =
            il2utils::resolve_method(area->klass, "SwitchAreaLevels", 0);
        const bool valid_current_level_field =
            is_field_type(current_level_field, IL2CPP_TYPE_I4);
        const bool valid_current_level_setter = current_level_setter != nullptr &&
            current_level_setter->methodPointer != nullptr &&
            current_level_setter->parameters_count == 1 &&
            current_level_setter->parameters != nullptr &&
            current_level_setter->parameters[0] != nullptr &&
            current_level_setter->parameters[0]->type == IL2CPP_TYPE_I4 &&
            current_level_setter->return_type != nullptr &&
            current_level_setter->return_type->type == IL2CPP_TYPE_VOID;
        const bool valid_switch_area_levels = switch_area_levels != nullptr &&
            switch_area_levels->methodPointer != nullptr &&
            switch_area_levels->parameters_count == 0 &&
            switch_area_levels->return_type != nullptr &&
            switch_area_levels->return_type->type == IL2CPP_TYPE_VOID;
        if (!valid_current_level_field || !valid_current_level_setter ||
            !valid_switch_area_levels)
        {
            logger::warn(
                "[HideoutTrainerRuntime] runtime level-switch contract did not validate");
            return false;
        }

        const int old_runtime_level = read_int_field(area_data, current_level_field);
        Il2CppObject* old_visual_level = find_reference_by_declared_type(
            area, "EFT.Hideout.HideoutAreaLevel");
        using current_level_setter_sig = void(*)(Il2CppObject*, int);
        using switch_area_levels_sig = void(*)(Il2CppObject*);
        reinterpret_cast<current_level_setter_sig>(
            current_level_setter->methodPointer)(area_data, requested_level);
        reinterpret_cast<switch_area_levels_sig>(
            switch_area_levels->methodPointer)(area);
        const int new_runtime_level = read_int_field(area_data, current_level_field);
        Il2CppObject* new_visual_level = find_reference_by_declared_type(
            area, "EFT.Hideout.HideoutAreaLevel");

        logger::info("[HideoutTrainerRuntime] applied area_index=" +
            std::to_string(area_index) +
            " game_object='" + runtime_game_object_name(area) +
            "' type=" + std::to_string(requested_type) +
            " level=" + std::to_string(old_runtime_level) + "->" +
                std::to_string(new_runtime_level) +
            " visual='" + runtime_game_object_name(old_visual_level) + "'->'" +
                runtime_game_object_name(new_visual_level) + "'");
        return new_runtime_level == requested_level;
    }

    Il2CppArray* get_profile_hideout_areas() const
    {
        Il2CppObject* profile =
            find_reference_by_declared_type(backend_session_, "EFT.Profile");
        Il2CppObject* hideout_info =
            find_reference_by_declared_type(profile, "EFT.HideoutInfo");
        return get_array_field(hideout_info, "Areas", "EFT.AreaInfo[]");
    }

    std::size_t apply_area_overrides(Il2CppArray* areas, const bool log_result)
    {
        if (areas == nullptr)
        {
            return 0;
        }

        std::array<AreaOverride, area_type_count_> overrides;
        {
            const std::lock_guard<std::mutex> lock(area_overrides_mutex_);
            overrides = area_overrides_;
        }

        const bool force_active = force_all_active_.load(std::memory_order_acquire);
        const bool force_bonuses = force_all_bonuses_.load(std::memory_order_acquire);
        const std::size_t area_count = (std::min)(
            static_cast<std::size_t>(areas->max_length), max_area_count_);
        auto** area_values = reinterpret_cast<Il2CppObject**>(&areas->data);
        std::size_t changed_area_count = 0;
        for (std::size_t index = 0; index < area_count; ++index)
        {
            Il2CppObject* area = area_values[index];
            AreaFields fields;
            if (area == nullptr || !resolve_area_fields(area->klass, fields))
            {
                continue;
            }

            const int type = read_int_field(area, fields.type);
            const bool valid_type = type >= 0 &&
                static_cast<std::size_t>(type) < area_type_count_;
            const AreaOverride* override_state = valid_type ?
                &overrides[static_cast<std::size_t>(type)] : nullptr;
            if ((override_state == nullptr || !override_state->enabled) &&
                !force_active && !force_bonuses)
            {
                continue;
            }

            bool changed = false;
            if (override_state != nullptr && override_state->enabled)
            {
                if (read_int_field(area, fields.level) != override_state->level)
                {
                    write_int_field(area, fields.level, override_state->level);
                    changed = true;
                }
                if (read_bool_field(area, fields.active) != override_state->active)
                {
                    write_bool_field(area, fields.active, override_state->active);
                    changed = true;
                }
                if (read_bool_field(area, fields.passive_bonuses_enabled) !=
                    override_state->passive_bonuses_enabled)
                {
                    write_bool_field(
                        area, fields.passive_bonuses_enabled,
                        override_state->passive_bonuses_enabled);
                    changed = true;
                }
            }
            if (force_active && !read_bool_field(area, fields.active))
            {
                write_bool_field(area, fields.active, true);
                changed = true;
            }
            if (force_bonuses && !read_bool_field(area, fields.passive_bonuses_enabled))
            {
                write_bool_field(area, fields.passive_bonuses_enabled, true);
                changed = true;
            }

            if (changed)
            {
                ++changed_area_count;
            }
        }

        if (log_result)
        {
            logger::info("[HideoutTrainer] applied persistent overrides changed_areas=" +
                std::to_string(changed_area_count) +
                " source_areas=" + std::to_string(area_count));
        }
        return changed_area_count;
    }

    Il2CppArray* refresh_hideout_areas(const char* action_name)
    {
        if (backend_session_ == nullptr || o_get_hideout_areas_info_ == nullptr)
        {
            logger::warn(std::string("[HideoutTrainer] ") + action_name +
                " requires a captured backend session; open a backend-backed menu and retry");
            return nullptr;
        }

        Il2CppArray* live_areas = o_get_hideout_areas_info_(backend_session_);
        if (live_areas == nullptr)
        {
            logger::warn(std::string("[HideoutTrainer] ") + action_name +
                " could not refresh the live area array");
            return nullptr;
        }

        hideout_areas_ = live_areas;
        return live_areas;
    }

    void apply_selected_area_state()
    {
        Il2CppArray* live_areas = refresh_hideout_areas("selected area update");
        if (live_areas == nullptr)
        {
            return;
        }

        const int requested_type = area_selection->get_value();
        const int requested_level = area_level->get_value();
        if (requested_type < 0 || static_cast<std::size_t>(requested_type) >= area_type_count_)
        {
            logger::warn("[HideoutTrainer] selected area type is outside the live enum range");
            return;
        }
        if (requested_level < 0)
        {
            logger::warn("[HideoutTrainer] area level must be non-negative");
            return;
        }

        std::size_t runtime_area_index = 0;
        Il2CppObject* runtime_area = find_loaded_runtime_area(
            requested_type, runtime_area_index);
        if (runtime_area != nullptr)
        {
            Il2CppArray* runtime_levels = get_array_field(
                runtime_area, "_areaLevels", "EFT.Hideout.HideoutAreaLevel[]");
            if (runtime_levels == nullptr)
            {
                logger::warn(
                    "[HideoutTrainer] loaded runtime area has no validated level array");
                return;
            }

            const std::size_t runtime_level_count =
                static_cast<std::size_t>(runtime_levels->max_length);
            if (static_cast<std::size_t>(requested_level) >= runtime_level_count)
            {
                logger::warn("[HideoutTrainer] rejected type=" +
                    std::to_string(requested_type) +
                    " level=" + std::to_string(requested_level) +
                    " before persistence; available_levels=" +
                    std::to_string(runtime_level_count));
                return;
            }
        }

        const std::size_t area_count = (std::min)(
            static_cast<std::size_t>(live_areas->max_length), max_area_count_);
        auto** areas = reinterpret_cast<Il2CppObject**>(&live_areas->data);
        for (std::size_t index = 0; index < area_count; ++index)
        {
            Il2CppObject* area = areas[index];
            AreaFields fields;
            if (area == nullptr || !resolve_area_fields(area->klass, fields))
            {
                continue;
            }
            if (read_int_field(area, fields.type) != requested_type)
            {
                continue;
            }

            const int old_level = read_int_field(area, fields.level);
            const bool old_active = read_bool_field(area, fields.active);
            const bool old_passive = read_bool_field(area, fields.passive_bonuses_enabled);

            {
                const std::lock_guard<std::mutex> lock(area_overrides_mutex_);
                AreaOverride& override_state =
                    area_overrides_[static_cast<std::size_t>(requested_type)];
                override_state.enabled = true;
                override_state.level = requested_level;
                override_state.active = area_active->get_value();
                override_state.passive_bonuses_enabled = passive_bonuses->get_value();
            }

            const std::size_t getter_changes = apply_area_overrides(live_areas, false);
            Il2CppArray* profile_areas = get_profile_hideout_areas();
            const bool shared_area_array = profile_areas == live_areas;
            const std::size_t profile_changes =
                profile_areas != nullptr && !shared_area_array ?
                    apply_area_overrides(profile_areas, false) : 0;
            const bool runtime_level_applied = apply_runtime_area_level(
                runtime_area,
                runtime_area_index,
                requested_type,
                requested_level);

            logger::info("[HideoutTrainer] applied area_index=" + std::to_string(index) +
                " type=" + std::to_string(requested_type) +
                " level=" + std::to_string(old_level) + "->" +
                    std::to_string(read_int_field(area, fields.level)) +
                " active=" + std::to_string(old_active ? 1 : 0) + "->" +
                    std::to_string(read_bool_field(area, fields.active) ? 1 : 0) +
                " passive=" + std::to_string(old_passive ? 1 : 0) + "->" +
                    std::to_string(read_bool_field(area, fields.passive_bonuses_enabled) ? 1 : 0) +
                " getter_changes=" + std::to_string(getter_changes) +
                " profile_changes=" + std::to_string(profile_changes) +
                " shared_array=" + std::to_string(shared_area_array ? 1 : 0) +
                " runtime_level=" + std::to_string(runtime_level_applied ? 1 : 0) +
                " persistent=1");
            return;
        }

        logger::warn("[HideoutTrainer] no live area matched type=" +
            std::to_string(requested_type));
    }

    void set_all_area_flags(const bool activate_areas, const bool enable_bonuses)
    {
        const char* action_name = activate_areas ? "activate all areas" : "enable all bonuses";
        if (activate_areas)
        {
            force_all_active_.store(true, std::memory_order_release);
        }
        if (enable_bonuses)
        {
            force_all_bonuses_.store(true, std::memory_order_release);
        }

        Il2CppArray* live_areas = refresh_hideout_areas(action_name);
        if (live_areas == nullptr)
        {
            return;
        }

        const std::size_t getter_changes = apply_area_overrides(live_areas, false);
        Il2CppArray* profile_areas = get_profile_hideout_areas();
        const bool shared_area_array = profile_areas == live_areas;
        const std::size_t profile_changes =
            profile_areas != nullptr && !shared_area_array ?
                apply_area_overrides(profile_areas, false) : 0;

        logger::info(std::string("[HideoutTrainer] ") + action_name +
            " getter_changes=" + std::to_string(getter_changes) +
            " profile_changes=" + std::to_string(profile_changes) +
            " shared_array=" + std::to_string(shared_area_array ? 1 : 0) +
            " persistent=1");
    }

    void dump_hideout_runtime_impl() const
    {
        if (assembly_csharp_ == nullptr)
        {
            logger::warn("[HideoutRuntimeProbe] Assembly-CSharp.dll image is unavailable");
            return;
        }
        if (unity::find_objects_of_type_internal == nullptr)
        {
            logger::warn("[HideoutRuntimeProbe] Unity FindObjectsOfType is unavailable");
            return;
        }

        const Il2CppClass* hideout_area_class =
            il2utils::resolve_class(assembly_csharp_, "EFT.Hideout", "HideoutArea");
        if (hideout_area_class == nullptr)
        {
            logger::warn("[HideoutRuntimeProbe] EFT.Hideout.HideoutArea was not resolved");
            return;
        }

        const Il2CppType* hideout_area_type = il2cpp::il2cpp_class_get_type(
            const_cast<Il2CppClass*>(hideout_area_class));
        Il2CppObject* system_type = hideout_area_type != nullptr ?
            il2cpp::il2cpp_type_get_object(hideout_area_type) : nullptr;
        Il2CppArray* area_objects = system_type != nullptr ?
            unity::find_objects_of_type_internal(system_type) : nullptr;
        if (area_objects == nullptr)
        {
            logger::warn("[HideoutRuntimeProbe] FindObjectsOfType returned null for HideoutArea");
            return;
        }

        const std::size_t area_count = static_cast<std::size_t>(area_objects->max_length);
        const std::size_t sampled_area_count = (std::min)(area_count, max_area_count_);
        logger::info("[HideoutRuntimeProbe] begin class='" + class_name(hideout_area_class) +
            "' areas=" + std::to_string(area_count) +
            " sampled=" + std::to_string(sampled_area_count));
        dump_class_layout(hideout_area_class, "HideoutRuntimeAreaClass");

        constexpr const char* method_names[] = {
            "UpdateAreaStatus",
            "SwitchAreaLevels",
            "AreaUpgradeHandler"
        };
        constexpr int method_parameter_counts[] = { 0, 0, 1 };
        for (std::size_t method_index = 0;
             method_index < std::size(method_names);
             ++method_index)
        {
            const MethodInfo* method = il2utils::resolve_method(
                hideout_area_class,
                method_names[method_index],
                method_parameter_counts[method_index]);
            if (method == nullptr)
            {
                logger::warn("[HideoutRuntimeMethod] name='" +
                    std::string(method_names[method_index]) + "' unresolved");
                continue;
            }

            logger::info("[HideoutRuntimeMethod] name='" +
                std::string(method->name != nullptr ? method->name : "<null>") +
                "' params=" + std::to_string(method->parameters_count) +
                " return_name='" + type_name(method->return_type) + "'");
            for (uint8_t parameter_index = 0;
                 parameter_index < method->parameters_count;
                 ++parameter_index)
            {
                const Il2CppType* parameter_type = method->parameters != nullptr ?
                    method->parameters[parameter_index] : nullptr;
                logger::info("[HideoutRuntimeMethodParam] method='" +
                    std::string(method->name != nullptr ? method->name : "<null>") +
                    "' index=" + std::to_string(parameter_index) +
                    " type_name='" + type_name(parameter_type) +
                    "' byref=" + std::to_string(
                        parameter_type != nullptr && parameter_type->byref ? 1 : 0) +
                    " valuetype=" + std::to_string(
                        parameter_type != nullptr && parameter_type->valuetype ? 1 : 0));
            }
        }

        auto** area_values = reinterpret_cast<Il2CppObject**>(&area_objects->data);
        Il2CppClass* dumped_data_class = nullptr;
        Il2CppClass* dumped_template_class = nullptr;
        Il2CppClass* dumped_level_class = nullptr;
        for (std::size_t area_index = 0; area_index < sampled_area_count; ++area_index)
        {
            Il2CppObject* area = area_values[area_index];
            if (area == nullptr)
            {
                continue;
            }

            Il2CppObject* area_data = find_reference_by_declared_type(
                area, "EFT.Hideout.AreaData");
            Il2CppObject* area_template = find_reference_by_declared_type(
                area, "EFT.Hideout.AreaTemplate");
            Il2CppObject* current_level = find_reference_by_declared_type(
                area, "EFT.Hideout.HideoutAreaLevel");
            Il2CppArray* area_levels = get_array_field(
                area,
                "_areaLevels",
                "EFT.Hideout.HideoutAreaLevel[]");

            logger::info("[HideoutRuntimeArea] #" + std::to_string(area_index) +
                " game_object='" + runtime_game_object_name(area) +
                "' address=" + std::to_string(reinterpret_cast<std::uintptr_t>(area)) +
                " data_address=" +
                    std::to_string(reinterpret_cast<std::uintptr_t>(area_data)) +
                " current_level='" + runtime_game_object_name(current_level) +
                "' levels=" + std::to_string(
                    area_levels != nullptr ?
                        static_cast<std::size_t>(area_levels->max_length) : 0));

            if (area_data != nullptr)
            {
                if (area_data->klass != dumped_data_class)
                {
                    dump_class_layout(area_data->klass, "HideoutRuntimeAreaData");
                    dumped_data_class = area_data->klass;
                }
                dump_object_values(
                    area_data,
                    "HideoutRuntimeAreaData" + std::to_string(area_index),
                    false);
            }
            if (area_template != nullptr)
            {
                if (area_template->klass != dumped_template_class)
                {
                    dump_class_layout(area_template->klass, "HideoutRuntimeAreaTemplate");
                    dumped_template_class = area_template->klass;
                }
                dump_object_values(
                    area_template,
                    "HideoutRuntimeAreaTemplate" + std::to_string(area_index),
                    false);
            }
            if (current_level != nullptr && current_level->klass != dumped_level_class)
            {
                dump_class_layout(current_level->klass, "HideoutRuntimeCurrentLevel");
                dumped_level_class = current_level->klass;
            }

            if (area_levels == nullptr)
            {
                continue;
            }
            const std::size_t level_count = (std::min)(
                static_cast<std::size_t>(area_levels->max_length),
                static_cast<std::size_t>(8));
            auto** level_values = reinterpret_cast<Il2CppObject**>(&area_levels->data);
            for (std::size_t level_index = 0; level_index < level_count; ++level_index)
            {
                Il2CppObject* level = level_values[level_index];
                logger::info("[HideoutRuntimeAreaLevel] area_index=" +
                    std::to_string(area_index) +
                    " level_index=" + std::to_string(level_index) +
                    " game_object='" + runtime_game_object_name(level) +
                    "' selected=" + std::to_string(level == current_level ? 1 : 0));
            }
        }

        logger::info("[HideoutRuntimeProbe] complete areas=" +
            std::to_string(area_count) +
            " sampled=" + std::to_string(sampled_area_count));
    }

    void dump_hideout_state_impl()
    {
        if (backend_session_ == nullptr)
        {
            logger::warn(
                "[HideoutProbe] no live backend session captured; open a backend-backed menu and retry");
            return;
        }
        if (o_get_hideout_areas_info_ == nullptr)
        {
            logger::warn("[HideoutProbe] GetHideoutAreasInfo original is unavailable");
            return;
        }

        Il2CppArray* live_hideout_areas = o_get_hideout_areas_info_(backend_session_);
        if (live_hideout_areas == nullptr)
        {
            logger::warn("[HideoutProbe] GetHideoutAreasInfo returned null for the captured session");
            return;
        }
        hideout_areas_ = live_hideout_areas;

        const std::size_t area_count = static_cast<std::size_t>(hideout_areas_->max_length);
        const std::size_t sampled_area_count = (std::min)(area_count, max_area_count_);
        logger::info("[HideoutProbe] session_class='" + class_name(backend_session_->klass) +
            "' area_count=" + std::to_string(area_count) +
            " sampled=" + std::to_string(sampled_area_count));

        dump_object_values(backend_session_, "HideoutBackendSession", true);

        Il2CppObject* profile = find_reference_by_declared_type(backend_session_, "EFT.Profile");
        if (profile != nullptr)
        {
            logger::info("[HideoutProbe] profile_class='" + class_name(profile->klass) + "'");
            dump_object_values(profile, "HideoutProfile", true);

            Il2CppObject* hideout_info = find_reference_by_declared_type(profile, "EFT.HideoutInfo");
            if (hideout_info != nullptr)
            {
                dump_class_layout(hideout_info->klass, "HideoutInfo");
                dump_object_values(hideout_info, "HideoutInfo", false);
            }
            else
            {
                logger::warn("[HideoutProbe] EFT.Profile.Hideout reference is null");
            }
        }
        else
        {
            logger::warn("[HideoutProbe] profile reference was not found in the backend-session field chain");
        }

        auto** areas = reinterpret_cast<Il2CppObject**>(&hideout_areas_->data);
        Il2CppClass* dumped_area_class = nullptr;
        Il2CppClass* dumped_slot_class = nullptr;
        std::size_t dumped_slot_count = 0;
        for (std::size_t index = 0; index < sampled_area_count; ++index)
        {
            Il2CppObject* area = areas[index];
            if (area == nullptr)
            {
                logger::info("[HideoutArea] #" + std::to_string(index) + " null");
                continue;
            }

            if (area->klass != dumped_area_class)
            {
                dump_class_layout(area->klass, "HideoutAreaInfo");
                dumped_area_class = area->klass;
            }

            logger::info("[HideoutArea] #" + std::to_string(index) +
                " class='" + class_name(area->klass) + "'");
            dump_object_values(area, "HideoutArea" + std::to_string(index), false);

            Il2CppArray* slots = get_array_field(area, "Slots", "EFT.Hideout.HideoutSlot[]");
            if (slots == nullptr || dumped_slot_count >= max_slot_count_)
            {
                continue;
            }

            const std::size_t slot_count = static_cast<std::size_t>(slots->max_length);
            auto** slot_values = reinterpret_cast<Il2CppObject**>(&slots->data);
            for (std::size_t slot_index = 0;
                 slot_index < slot_count && dumped_slot_count < max_slot_count_;
                 ++slot_index)
            {
                Il2CppObject* slot = slot_values[slot_index];
                if (slot == nullptr)
                {
                    continue;
                }

                if (slot->klass != dumped_slot_class)
                {
                    dump_class_layout(slot->klass, "HideoutSlot");
                    dumped_slot_class = slot->klass;
                }

                logger::info("[HideoutSlot] area_index=" + std::to_string(index) +
                    " slot_index=" + std::to_string(slot_index) +
                    " class='" + class_name(slot->klass) + "'");
                dump_object_values(slot, "HideoutSlot" + std::to_string(dumped_slot_count), false);
                ++dumped_slot_count;
            }
        }

        logger::info("[HideoutProbe] capture complete");
    }
};
