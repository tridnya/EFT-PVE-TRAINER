#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../game_state.h"
#include "../Module.h"
#include "../ModuleCategory.h"
#include "../../config/CheckboxValue.h"
#include "../../config/FloatSliderValue.h"
#include "../../il2cpp/Il2CppObjectInstance.h"
#include "../../il2cpp/il2utils.h"
#include "../../il2cpp/unity.h"
#include "../../util/logger.h"

namespace view_module_state
{
    inline Il2CppObject* fov_camera = nullptr;
    inline float saved_camera_fov = 0.0f;
    inline bool saved_camera_fov_valid = false;

    using get_ambient_light_sig = unity::color(*)();
    using set_ambient_light_sig = void(*)(unity::color);
    using get_ambient_intensity_sig = float(*)();
    using set_ambient_intensity_sig = void(*)(float);
    using get_ambient_mode_sig = int(*)();
    using set_ambient_mode_sig = void(*)(int);
    using get_fog_sig = bool(*)();
    using set_fog_sig = void(*)(bool);
    using get_render_color_sig = unity::color(*)();
    using set_render_color_sig = void(*)(unity::color);
    using get_render_float_sig = float(*)();
    using set_render_float_sig = void(*)(float);
    using get_light_intensity_sig = float(*)(Il2CppObject*);
    using set_light_intensity_sig = void(*)(Il2CppObject*, float);
    using get_light_color_sig = unity::color(*)(Il2CppObject*);
    using set_light_color_sig = void(*)(Il2CppObject*, unity::color);
    using get_light_float_sig = float(*)(Il2CppObject*);
    using set_light_float_sig = void(*)(Il2CppObject*, float);
    using get_light_shadows_sig = int(*)(Il2CppObject*);
    using set_light_shadows_sig = void(*)(Il2CppObject*, int);
    using get_behaviour_enabled_sig = bool(*)(Il2CppObject*);
    using set_behaviour_enabled_sig = void(*)(Il2CppObject*, bool);
    using get_quality_shadows_sig = int(*)();
    using set_quality_shadows_sig = void(*)(int);
    using get_quality_shadow_distance_sig = float(*)();
    using set_quality_shadow_distance_sig = void(*)(float);
    using get_components_sig = Il2CppArray*(*)(Il2CppObject*, Il2CppObject*);

    struct LightState
    {
        Il2CppObject* light = nullptr;
        float intensity = 1.0f;
        unity::color color = unity::color(1.0f, 1.0f, 1.0f, 1.0f);
        float bounce_intensity = 1.0f;
        float shadow_strength = 1.0f;
        int shadows = 0;
        bool enabled = true;
        bool has_intensity = false;
        bool has_color = false;
        bool has_bounce_intensity = false;
        bool has_shadow_strength = false;
        bool has_shadows = false;
        bool has_enabled = false;
    };

    struct CameraEffectState
    {
        Il2CppObject* component = nullptr;
        bool enabled = false;
    };

    struct CameraBooleanFieldState
    {
        Il2CppObject* component = nullptr;
        const FieldInfo* field = nullptr;
        bool value = false;
    };

    inline bool render_settings_resolved = false;
    inline get_ambient_light_sig get_ambient_light = nullptr;
    inline set_ambient_light_sig set_ambient_light = nullptr;
    inline get_ambient_intensity_sig get_ambient_intensity = nullptr;
    inline set_ambient_intensity_sig set_ambient_intensity = nullptr;
    inline get_ambient_mode_sig get_ambient_mode = nullptr;
    inline set_ambient_mode_sig set_ambient_mode = nullptr;
    inline get_fog_sig get_fog = nullptr;
    inline set_fog_sig set_fog = nullptr;
    inline get_render_color_sig get_ambient_sky_color = nullptr;
    inline set_render_color_sig set_ambient_sky_color = nullptr;
    inline get_render_color_sig get_ambient_equator_color = nullptr;
    inline set_render_color_sig set_ambient_equator_color = nullptr;
    inline get_render_color_sig get_ambient_ground_color = nullptr;
    inline set_render_color_sig set_ambient_ground_color = nullptr;
    inline get_render_color_sig get_subtractive_shadow_color = nullptr;
    inline set_render_color_sig set_subtractive_shadow_color = nullptr;
    inline get_render_float_sig get_reflection_intensity = nullptr;
    inline set_render_float_sig set_reflection_intensity = nullptr;
    inline bool light_settings_resolved = false;
    inline Il2CppObject* light_type = nullptr;
    inline get_light_intensity_sig get_light_intensity = nullptr;
    inline set_light_intensity_sig set_light_intensity = nullptr;
    inline get_light_color_sig get_light_color = nullptr;
    inline set_light_color_sig set_light_color = nullptr;
    inline get_light_float_sig get_light_bounce_intensity = nullptr;
    inline set_light_float_sig set_light_bounce_intensity = nullptr;
    inline get_light_float_sig get_light_shadow_strength = nullptr;
    inline set_light_float_sig set_light_shadow_strength = nullptr;
    inline get_light_shadows_sig get_light_shadows = nullptr;
    inline set_light_shadows_sig set_light_shadows = nullptr;
    inline get_behaviour_enabled_sig get_behaviour_enabled = nullptr;
    inline set_behaviour_enabled_sig set_behaviour_enabled = nullptr;
    inline bool quality_settings_resolved = false;
    inline get_quality_shadows_sig get_quality_shadows = nullptr;
    inline set_quality_shadows_sig set_quality_shadows = nullptr;
    inline get_quality_shadow_distance_sig get_quality_shadow_distance = nullptr;
    inline set_quality_shadow_distance_sig set_quality_shadow_distance = nullptr;
    inline bool camera_components_resolved = false;
    inline bool camera_components_dumped = false;
    inline bool weather_components_dumped = false;
    inline Il2CppObject* component_type = nullptr;
    inline get_components_sig get_components = nullptr;

    inline bool fullbright_saved = false;
    inline bool saved_ambient_light_valid = false;
    inline bool saved_ambient_intensity_valid = false;
    inline bool saved_ambient_mode_valid = false;
    inline bool saved_fog_valid = false;
    inline bool saved_ambient_sky_color_valid = false;
    inline bool saved_ambient_equator_color_valid = false;
    inline bool saved_ambient_ground_color_valid = false;
    inline bool saved_subtractive_shadow_color_valid = false;
    inline bool saved_reflection_intensity_valid = false;
    inline bool saved_quality_shadows_valid = false;
    inline bool saved_quality_shadow_distance_valid = false;
    inline unity::color saved_ambient_light = unity::color(0.0f, 0.0f, 0.0f, 1.0f);
    inline unity::color saved_ambient_sky_color = unity::color(0.0f, 0.0f, 0.0f, 1.0f);
    inline unity::color saved_ambient_equator_color = unity::color(0.0f, 0.0f, 0.0f, 1.0f);
    inline unity::color saved_ambient_ground_color = unity::color(0.0f, 0.0f, 0.0f, 1.0f);
    inline unity::color saved_subtractive_shadow_color = unity::color(0.0f, 0.0f, 0.0f, 1.0f);
    inline float saved_ambient_intensity = 1.0f;
    inline float saved_reflection_intensity = 1.0f;
    inline int saved_quality_shadows = 0;
    inline float saved_quality_shadow_distance = 0.0f;
    inline int saved_ambient_mode = 0;
    inline bool saved_fog = false;
    inline std::vector<LightState> saved_lights{};
    inline ULONGLONG last_light_refresh = 0;
    inline bool fullbright_resolution_logged = false;
    inline bool fullbright_readback_logged = false;
    inline std::vector<CameraEffectState> saved_camera_effects{};
    inline std::vector<CameraBooleanFieldState> saved_camera_boolean_fields{};

    template <typename T>
    [[nodiscard]] T resolve_render_settings_method(const Il2CppClass* render_settings_class, const char* method_name)
    {
        const MethodInfo* method = il2utils::resolve_method(render_settings_class, method_name, 0);
        if (!method || !method->methodPointer)
        {
            return nullptr;
        }

        return static_cast<T>(method->methodPointer);
    }

    template <typename T>
    [[nodiscard]] T resolve_render_settings_method(
        const Il2CppClass* render_settings_class,
        const char* method_name,
        int arg_count)
    {
        const MethodInfo* method = il2utils::resolve_method(render_settings_class, method_name, arg_count);
        if (!method || !method->methodPointer)
        {
            return nullptr;
        }

        return static_cast<T>(method->methodPointer);
    }

    [[nodiscard]] inline bool resolve_render_settings()
    {
        if (render_settings_resolved)
        {
            return set_ambient_light || set_ambient_sky_color || set_ambient_equator_color ||
                set_ambient_ground_color || set_ambient_intensity || set_ambient_mode || set_fog;
        }

        render_settings_resolved = true;
        Il2CppImage* unity_image = il2utils::resolve_image("UnityEngine.CoreModule.dll");
        const Il2CppClass* render_settings_class =
            il2utils::resolve_class(unity_image, "UnityEngine", "RenderSettings");
        if (!render_settings_class)
        {
            return false;
        }

        get_ambient_light =
            resolve_render_settings_method<get_ambient_light_sig>(render_settings_class, "get_ambientLight");
        set_ambient_light =
            resolve_render_settings_method<set_ambient_light_sig>(render_settings_class, "set_ambientLight", 1);
        get_ambient_intensity =
            resolve_render_settings_method<get_ambient_intensity_sig>(render_settings_class, "get_ambientIntensity");
        set_ambient_intensity =
            resolve_render_settings_method<set_ambient_intensity_sig>(
                render_settings_class,
                "set_ambientIntensity",
                1);
        get_ambient_mode =
            resolve_render_settings_method<get_ambient_mode_sig>(render_settings_class, "get_ambientMode");
        set_ambient_mode =
            resolve_render_settings_method<set_ambient_mode_sig>(render_settings_class, "set_ambientMode", 1);
        get_fog = resolve_render_settings_method<get_fog_sig>(render_settings_class, "get_fog");
        set_fog = resolve_render_settings_method<set_fog_sig>(render_settings_class, "set_fog", 1);
        get_ambient_sky_color = resolve_render_settings_method<get_render_color_sig>(
            render_settings_class, "get_ambientSkyColor");
        set_ambient_sky_color = resolve_render_settings_method<set_render_color_sig>(
            render_settings_class, "set_ambientSkyColor", 1);
        get_ambient_equator_color = resolve_render_settings_method<get_render_color_sig>(
            render_settings_class, "get_ambientEquatorColor");
        set_ambient_equator_color = resolve_render_settings_method<set_render_color_sig>(
            render_settings_class, "set_ambientEquatorColor", 1);
        get_ambient_ground_color = resolve_render_settings_method<get_render_color_sig>(
            render_settings_class, "get_ambientGroundColor");
        set_ambient_ground_color = resolve_render_settings_method<set_render_color_sig>(
            render_settings_class, "set_ambientGroundColor", 1);
        get_subtractive_shadow_color = resolve_render_settings_method<get_render_color_sig>(
            render_settings_class, "get_subtractiveShadowColor");
        set_subtractive_shadow_color = resolve_render_settings_method<set_render_color_sig>(
            render_settings_class, "set_subtractiveShadowColor", 1);
        get_reflection_intensity = resolve_render_settings_method<get_render_float_sig>(
            render_settings_class, "get_reflectionIntensity");
        set_reflection_intensity = resolve_render_settings_method<set_render_float_sig>(
            render_settings_class, "set_reflectionIntensity", 1);

        return set_ambient_light || set_ambient_sky_color || set_ambient_equator_color ||
            set_ambient_ground_color || set_ambient_intensity || set_ambient_mode || set_fog;
    }

    template <typename T>
    [[nodiscard]] T resolve_instance_method(const Il2CppClass* klass, const char* method_name, int arg_count)
    {
        const MethodInfo* method = il2utils::resolve_method(klass, method_name, arg_count);
        if (!method || !method->methodPointer)
        {
            return nullptr;
        }

        return static_cast<T>(method->methodPointer);
    }

    [[nodiscard]] inline bool resolve_light_settings()
    {
        if (light_settings_resolved)
        {
            return light_type && (set_light_intensity || set_light_color || set_behaviour_enabled);
        }

        light_settings_resolved = true;
        Il2CppImage* unity_image = il2utils::resolve_image("UnityEngine.CoreModule.dll");
        const Il2CppClass* light_class = il2utils::resolve_class(unity_image, "UnityEngine", "Light");
        const Il2CppClass* behaviour_class = il2utils::resolve_class(unity_image, "UnityEngine", "Behaviour");
        if (!light_class)
        {
            return false;
        }

        light_type = il2utils::get_system_type(unity_image, "UnityEngine", "Light");
        get_light_intensity =
            resolve_instance_method<get_light_intensity_sig>(light_class, "get_intensity", 0);
        set_light_intensity =
            resolve_instance_method<set_light_intensity_sig>(light_class, "set_intensity", 1);
        get_light_color = resolve_instance_method<get_light_color_sig>(light_class, "get_color", 0);
        set_light_color = resolve_instance_method<set_light_color_sig>(light_class, "set_color", 1);
        get_light_bounce_intensity = resolve_instance_method<get_light_float_sig>(
            light_class, "get_bounceIntensity", 0);
        set_light_bounce_intensity = resolve_instance_method<set_light_float_sig>(
            light_class, "set_bounceIntensity", 1);
        get_light_shadow_strength = resolve_instance_method<get_light_float_sig>(
            light_class, "get_shadowStrength", 0);
        set_light_shadow_strength = resolve_instance_method<set_light_float_sig>(
            light_class, "set_shadowStrength", 1);
        get_light_shadows = resolve_instance_method<get_light_shadows_sig>(
            light_class, "get_shadows", 0);
        set_light_shadows = resolve_instance_method<set_light_shadows_sig>(
            light_class, "set_shadows", 1);

        if (behaviour_class)
        {
            get_behaviour_enabled =
                resolve_instance_method<get_behaviour_enabled_sig>(behaviour_class, "get_enabled", 0);
            set_behaviour_enabled =
                resolve_instance_method<set_behaviour_enabled_sig>(behaviour_class, "set_enabled", 1);
        }

        return light_type && (set_light_intensity || set_light_color || set_behaviour_enabled);
    }

    [[nodiscard]] inline bool resolve_quality_settings()
    {
        if (quality_settings_resolved)
        {
            return set_quality_shadows || set_quality_shadow_distance;
        }

        quality_settings_resolved = true;
        Il2CppImage* unity_image = il2utils::resolve_image("UnityEngine.CoreModule.dll");
        const Il2CppClass* quality_settings_class =
            il2utils::resolve_class(unity_image, "UnityEngine", "QualitySettings");
        if (!quality_settings_class)
        {
            return false;
        }

        get_quality_shadows = resolve_render_settings_method<get_quality_shadows_sig>(
            quality_settings_class, "get_shadows");
        set_quality_shadows = resolve_render_settings_method<set_quality_shadows_sig>(
            quality_settings_class, "set_shadows", 1);
        get_quality_shadow_distance =
            resolve_render_settings_method<get_quality_shadow_distance_sig>(
                quality_settings_class, "get_shadowDistance");
        set_quality_shadow_distance =
            resolve_render_settings_method<set_quality_shadow_distance_sig>(
                quality_settings_class, "set_shadowDistance", 1);

        return set_quality_shadows || set_quality_shadow_distance;
    }

    [[nodiscard]] inline std::string component_class_name(const Il2CppClass* klass)
    {
        if (klass == nullptr)
        {
            return "<null>";
        }

        const std::string namespaze = klass->namespaze ? klass->namespaze : "";
        const std::string name = klass->name ? klass->name : "<unknown>";
        return namespaze.empty() ? "." + name : namespaze + "." + name;
    }

    [[nodiscard]] inline Il2CppArray* get_current_camera_components()
    {
        if (!camera_components_resolved)
        {
            camera_components_resolved = true;
            Il2CppImage* unity_image = il2utils::resolve_image("UnityEngine.CoreModule.dll");
            const Il2CppClass* game_object_class =
                il2utils::resolve_class(unity_image, "UnityEngine", "GameObject");
            if (game_object_class != nullptr)
            {
                get_components = resolve_instance_method<get_components_sig>(
                    game_object_class, "GetComponents", 1);
            }
            component_type = il2utils::get_system_type(
                unity_image, "UnityEngine", "Component");
        }

        Il2CppObject* camera = unity::get_current_camera();
        Il2CppObject* camera_game_object = camera != nullptr ?
            unity::component_get_game_object(camera) :
            nullptr;
        if (camera_game_object == nullptr || get_components == nullptr || component_type == nullptr)
        {
            return nullptr;
        }

        return get_components(camera_game_object, component_type);
    }

    [[nodiscard]] inline std::string read_diagnostic_field_value(
        Il2CppObject* object,
        const FieldInfo& field)
    {
        if (object == nullptr || field.type == nullptr ||
            field.offset < static_cast<int32_t>(sizeof(Il2CppObject)))
        {
            return field.offset < static_cast<int32_t>(sizeof(Il2CppObject)) ?
                "<static/skipped>" :
                "<unread>";
        }

        const std::uintptr_t address =
            reinterpret_cast<std::uintptr_t>(object) + static_cast<std::uintptr_t>(field.offset);
        switch (field.type->type)
        {
        case IL2CPP_TYPE_BOOLEAN:
            return *reinterpret_cast<const bool*>(address) ? "true" : "false";
        case IL2CPP_TYPE_I1:
            return std::to_string(*reinterpret_cast<const std::int8_t*>(address));
        case IL2CPP_TYPE_U1:
            return std::to_string(*reinterpret_cast<const std::uint8_t*>(address));
        case IL2CPP_TYPE_I2:
            return std::to_string(*reinterpret_cast<const std::int16_t*>(address));
        case IL2CPP_TYPE_U2:
            return std::to_string(*reinterpret_cast<const std::uint16_t*>(address));
        case IL2CPP_TYPE_I4:
            return std::to_string(*reinterpret_cast<const std::int32_t*>(address));
        case IL2CPP_TYPE_U4:
            return std::to_string(*reinterpret_cast<const std::uint32_t*>(address));
        case IL2CPP_TYPE_I8:
            return std::to_string(*reinterpret_cast<const std::int64_t*>(address));
        case IL2CPP_TYPE_U8:
            return std::to_string(*reinterpret_cast<const std::uint64_t*>(address));
        case IL2CPP_TYPE_R4:
            return std::to_string(*reinterpret_cast<const float*>(address));
        case IL2CPP_TYPE_R8:
            return std::to_string(*reinterpret_cast<const double*>(address));
        case IL2CPP_TYPE_STRING:
        {
            const Il2CppString* value = *reinterpret_cast<Il2CppString* const*>(address);
            if (value == nullptr)
            {
                return "null";
            }

            const int length = (std::min)(value->length, 128);
            std::string text;
            text.reserve(static_cast<size_t>((std::max)(length, 0)));
            for (int index = 0; index < length; ++index)
            {
                text.push_back(static_cast<char>(value->chars[index]));
            }
            return "'" + text + "'";
        }
        default:
            return "<unread>";
        }
    }

    inline void dump_component_members_bounded(
        Il2CppObject* component,
        const std::string& prefix)
    {
        if (component == nullptr || component->klass == nullptr)
        {
            return;
        }

        constexpr size_t member_limit = 96;
        constexpr size_t inheritance_limit = 6;
        size_t logged_members = 0;
        size_t depth = 0;
        for (const Il2CppClass* cursor = component->klass;
            cursor != nullptr && depth < inheritance_limit && logged_members < member_limit;
            cursor = cursor->parent, ++depth)
        {
            logger::info("[" + prefix + "Class] class='" +
                component_class_name(cursor) +
                "' depth=" + std::to_string(depth) +
                " fields=" + std::to_string(cursor->field_count) +
                " methods=" + std::to_string(cursor->method_count));

            if (cursor->fields != nullptr)
            {
                for (std::uint16_t field_index = 0;
                    field_index < cursor->field_count && logged_members < member_limit;
                    ++field_index)
                {
                    const FieldInfo& field = cursor->fields[field_index];
                    logger::info("[" + prefix + "Field] owner='" +
                        component_class_name(cursor) +
                        "' name='" + std::string(field.name ? field.name : "<null>") +
                        "' offset=" + std::to_string(field.offset) +
                        " type=" + std::to_string(
                            field.type ? static_cast<int>(field.type->type) : -1) +
                        " value=" + read_diagnostic_field_value(component, field));
                    ++logged_members;
                }
            }

            if (cursor->methods != nullptr)
            {
                for (std::uint16_t method_index = 0;
                    method_index < cursor->method_count && logged_members < member_limit;
                    ++method_index)
                {
                    const MethodInfo* method = cursor->methods[method_index];
                    if (method == nullptr)
                    {
                        continue;
                    }

                    logger::info("[" + prefix + "Method] owner='" +
                        component_class_name(cursor) +
                        "' name='" + std::string(method->name ? method->name : "<null>") +
                        "' params=" + std::to_string(method->parameters_count));
                    ++logged_members;
                }
            }
        }
    }

    [[nodiscard]] inline bool is_fullbright_camera_effect(const std::string& class_name)
    {
        static constexpr const char* effect_classes[] = {
            "UnityStandardAssets.CinematicEffects.AmbientOcclusion",
            "UnityStandardAssets.ImageEffects.ColorCorrectionCurves",
            ".DesaturateEffect"
        };
        for (const char* effect_class : effect_classes)
        {
            if (class_name == effect_class)
            {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] inline CameraEffectState* find_saved_camera_effect(Il2CppObject* component)
    {
        for (CameraEffectState& state : saved_camera_effects)
        {
            if (state.component == component)
            {
                return &state;
            }
        }

        return nullptr;
    }

    [[nodiscard]] inline CameraBooleanFieldState* find_saved_camera_boolean_field(
        Il2CppObject* component,
        const FieldInfo* field)
    {
        for (CameraBooleanFieldState& state : saved_camera_boolean_fields)
        {
            if (state.component == component && state.field == field)
            {
                return &state;
            }
        }

        return nullptr;
    }

    inline void apply_camera_boolean_override(
        Il2CppObject* component,
        const char* field_name,
        bool value)
    {
        if (component == nullptr || component->klass == nullptr)
        {
            return;
        }

        const FieldInfo* field = il2utils::resolve_field(component->klass, field_name);
        if (field == nullptr || field->type == nullptr ||
            field->type->type != IL2CPP_TYPE_BOOLEAN ||
            field->offset < static_cast<int32_t>(sizeof(Il2CppObject)))
        {
            return;
        }

        bool* field_value = reinterpret_cast<bool*>(
            reinterpret_cast<std::uintptr_t>(component) +
            static_cast<std::uintptr_t>(field->offset));
        CameraBooleanFieldState* state = find_saved_camera_boolean_field(component, field);
        if (state == nullptr)
        {
            saved_camera_boolean_fields.push_back({ component, field, *field_value });
            state = &saved_camera_boolean_fields.back();
            logger::info("[FullbrightEffectField] class='" +
                component_class_name(component->klass) +
                "' field='" + field_name +
                "' saved=" + std::to_string(state->value ? 1 : 0) +
                " applied=" + std::to_string(value ? 1 : 0));
        }

        *field_value = value;
    }

    inline void apply_camera_effect_fullbright()
    {
        if (get_behaviour_enabled == nullptr || set_behaviour_enabled == nullptr)
        {
            return;
        }

        Il2CppArray* components = get_current_camera_components();
        if (components == nullptr)
        {
            return;
        }

        Il2CppObject** component_values = reinterpret_cast<Il2CppObject**>(&components->data);
        const size_t component_count = (std::min)(
            static_cast<size_t>(components->max_length),
            static_cast<size_t>(64));
        for (size_t component_index = 0; component_index < component_count; ++component_index)
        {
            Il2CppObject* component = component_values[component_index];
            if (component == nullptr || component->klass == nullptr)
            {
                continue;
            }

            const std::string class_name = component_class_name(component->klass);
            if (!is_fullbright_camera_effect(class_name))
            {
                if (class_name == ".PrismEffects")
                {
                    apply_camera_boolean_override(component, "useExposure", false);
                    apply_camera_boolean_override(component, "useGammaCorrection", false);
                }
                else if (class_name ==
                    "UnityEngine.Rendering.PostProcessing.PostProcessLayer")
                {
                    apply_camera_boolean_override(component, "computeAutoExposure", false);
                }
                continue;
            }

            CameraEffectState* state = find_saved_camera_effect(component);
            if (state == nullptr)
            {
                saved_camera_effects.push_back({ component, get_behaviour_enabled(component) });
                state = &saved_camera_effects.back();
                logger::info("[FullbrightEffect] class='" + class_name +
                    "' saved_enabled=" + std::to_string(state->enabled ? 1 : 0));
            }

            set_behaviour_enabled(component, false);
        }
    }

    inline void restore_camera_effects(bool restore_values)
    {
        if (restore_values && set_behaviour_enabled != nullptr)
        {
            Il2CppArray* components = get_current_camera_components();
            Il2CppObject** component_values = components != nullptr ?
                reinterpret_cast<Il2CppObject**>(&components->data) :
                nullptr;
            const size_t component_count = components != nullptr ?
                (std::min)(static_cast<size_t>(components->max_length), static_cast<size_t>(64)) :
                0;
            for (const CameraBooleanFieldState& state : saved_camera_boolean_fields)
            {
                bool is_current_component = false;
                for (size_t component_index = 0;
                    component_index < component_count;
                    ++component_index)
                {
                    if (component_values[component_index] == state.component)
                    {
                        is_current_component = true;
                        break;
                    }
                }

                if (is_current_component && state.field != nullptr)
                {
                    bool* field_value = reinterpret_cast<bool*>(
                        reinterpret_cast<std::uintptr_t>(state.component) +
                        static_cast<std::uintptr_t>(state.field->offset));
                    *field_value = state.value;
                }
            }

            for (const CameraEffectState& state : saved_camera_effects)
            {
                bool is_current_component = false;
                for (size_t component_index = 0;
                    component_index < component_count;
                    ++component_index)
                {
                    if (component_values[component_index] == state.component)
                    {
                        is_current_component = true;
                        break;
                    }
                }

                if (is_current_component)
                {
                    set_behaviour_enabled(state.component, state.enabled);
                }
            }
        }

        saved_camera_boolean_fields.clear();
        saved_camera_effects.clear();
    }

    inline void dump_camera_components_once()
    {
        if (camera_components_dumped)
        {
            return;
        }

        Il2CppArray* components = get_current_camera_components();
        if (components == nullptr)
        {
            return;
        }

        camera_components_dumped = true;
        const size_t component_count = std::min<size_t>(
            static_cast<size_t>(components->max_length),
            64);
        logger::info("[FullbrightCamera] component_count=" +
            std::to_string(components->max_length) +
            " sampled=" + std::to_string(component_count));
        Il2CppObject** component_values =
            reinterpret_cast<Il2CppObject**>(&components->data);
        for (size_t component_index = 0; component_index < component_count; ++component_index)
        {
            Il2CppObject* component = component_values[component_index];
            if (component == nullptr || component->klass == nullptr)
            {
                continue;
            }

            const std::string class_name = component_class_name(component->klass);
            logger::info("[FullbrightCameraComponent] #" +
                std::to_string(component_index) +
                " class='" + class_name + "'");
        }
    }

    [[nodiscard]] inline bool is_weather_component_name(std::string name)
    {
        std::transform(name.begin(), name.end(), name.begin(),
            [](const unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            });
        static constexpr const char* keywords[] = {
            "weather", "cloud", "rain", "fog", "wind", "sky",
            "atmos", "timeofday", "time_of_day", "tod_", "sun",
            "lightning", "storm", "season", "temperature"
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

    inline void dump_weather_components_once()
    {
        if (weather_components_dumped || component_type == nullptr ||
            unity::find_objects_of_type_internal == nullptr)
        {
            return;
        }

        Il2CppArray* components = unity::find_objects_of_type_internal(component_type);
        if (components == nullptr)
        {
            return;
        }

        weather_components_dumped = true;
        Il2CppObject** component_values =
            reinterpret_cast<Il2CppObject**>(&components->data);
        constexpr size_t scan_limit = 1500000;
        constexpr size_t candidate_limit = 16;
        const size_t scan_count = (std::min)(
            static_cast<size_t>(components->max_length),
            scan_limit);
        std::vector<std::pair<std::string, Il2CppObject*>> candidates;
        candidates.reserve(candidate_limit);
        for (size_t component_index = 0;
            component_index < scan_count && candidates.size() < candidate_limit;
            ++component_index)
        {
            Il2CppObject* component = component_values[component_index];
            if (component == nullptr || component->klass == nullptr)
            {
                continue;
            }

            const std::string full_name = component_class_name(component->klass);
            if (!is_weather_component_name(full_name))
            {
                continue;
            }

            bool already_recorded = false;
            for (const auto& candidate : candidates)
            {
                if (candidate.first == full_name)
                {
                    already_recorded = true;
                    break;
                }
            }
            if (!already_recorded)
            {
                candidates.emplace_back(full_name, component);
            }
        }

        logger::info("[WeatherProbe] component_count=" +
            std::to_string(components->max_length) +
            " scanned=" + std::to_string(scan_count) +
            " candidates=" + std::to_string(candidates.size()));
        size_t candidate_index = 0;
        for (const auto& candidate : candidates)
        {
            logger::info("[WeatherProbeComponent] #" +
                std::to_string(candidate_index++) +
                " class='" + candidate.first + "'");
        }
    }

    [[nodiscard]] inline LightState* find_saved_light(Il2CppObject* light)
    {
        for (LightState& state : saved_lights)
        {
            if (state.light == light)
            {
                return &state;
            }
        }

        return nullptr;
    }

    inline void save_light_state(Il2CppObject* light)
    {
        if (!light || find_saved_light(light))
        {
            return;
        }

        LightState state{};
        state.light = light;

        if (get_light_intensity && set_light_intensity)
        {
            state.intensity = get_light_intensity(light);
            state.has_intensity = true;
        }

        if (get_light_color && set_light_color)
        {
            state.color = get_light_color(light);
            state.has_color = true;
        }

        if (get_light_bounce_intensity && set_light_bounce_intensity)
        {
            state.bounce_intensity = get_light_bounce_intensity(light);
            state.has_bounce_intensity = true;
        }

        if (get_light_shadow_strength && set_light_shadow_strength)
        {
            state.shadow_strength = get_light_shadow_strength(light);
            state.has_shadow_strength = true;
        }

        if (get_light_shadows && set_light_shadows)
        {
            state.shadows = get_light_shadows(light);
            state.has_shadows = true;
        }

        if (get_behaviour_enabled && set_behaviour_enabled)
        {
            state.enabled = get_behaviour_enabled(light);
            state.has_enabled = true;
        }

        saved_lights.push_back(state);
    }
}

class ViewModule : Module
{
public:
    ViewModule() : Module("View", Visuals)
    {
    }

    CheckboxValue* fov_changer = conf(new CheckboxValue(false, "FOV Changer"));
    FloatSliderValue* camera_fov = conf(new FloatSliderValue(75.0f, 35.0f, 120.0f, "Camera FOV"));
    CheckboxValue* fullbright = conf(new CheckboxValue(false, "Fullbright"));
    FloatSliderValue* fullbright_strength =
        conf(new FloatSliderValue(2.50f, 0.25f, 5.00f, "Fullbright Strength"));
    CheckboxValue* fullbright_disable_fog = conf(new CheckboxValue(true, "Disable Fog"));
    CheckboxValue* fullbright_boost_lights = conf(new CheckboxValue(true, "Boost Scene Lights"));

    void draw_overlay(ImDrawList* draw_list) override
    {
        (void)draw_list;
    }

    void application_update() override
    {
        if (!game_state::is_in_raid)
        {
            restore_fullbright(false);
        }
    }

    void reset_gameworld_state() override
    {
        view_module_state::fov_camera = nullptr;
        view_module_state::saved_camera_fov_valid = false;
        view_module_state::fullbright_saved = false;
        view_module_state::saved_lights.clear();
        view_module_state::last_light_refresh = 0;
        view_module_state::saved_camera_effects.clear();
        view_module_state::saved_camera_boolean_fields.clear();
        view_module_state::fullbright_readback_logged = false;
    }

    void gameworld_update(const Il2CppClass* game_world_class, Il2CppObjectInstance game_world_instance,
                          Il2CppObjectInstance main_player) override
    {
        (void)game_world_class;
        (void)game_world_instance;
        (void)main_player;

        apply_fov_changer();
        apply_fullbright();
    }

    void init() override
    {
    }

private:
    void apply_fov_changer() const
    {
        Il2CppObject* camera = unity::get_current_camera();
        if (!camera)
        {
            return;
        }

        if (fov_changer->get_value())
        {
            if (camera != view_module_state::fov_camera)
            {
                view_module_state::fov_camera = camera;
                view_module_state::saved_camera_fov = unity::camera_fov(camera);
                view_module_state::saved_camera_fov_valid = true;
            }

            unity::camera_set_fov(camera, camera_fov->get_value());
            return;
        }

        if (view_module_state::saved_camera_fov_valid)
        {
            if (view_module_state::fov_camera == camera)
            {
                unity::camera_set_fov(camera, view_module_state::saved_camera_fov);
            }

            view_module_state::saved_camera_fov_valid = false;
            view_module_state::fov_camera = nullptr;
        }
    }

    void save_fullbright_state() const
    {
        if (view_module_state::fullbright_saved)
        {
            return;
        }

        view_module_state::saved_ambient_light_valid = false;
        view_module_state::saved_ambient_intensity_valid = false;
        view_module_state::saved_ambient_mode_valid = false;
        view_module_state::saved_fog_valid = false;
        view_module_state::saved_ambient_sky_color_valid = false;
        view_module_state::saved_ambient_equator_color_valid = false;
        view_module_state::saved_ambient_ground_color_valid = false;
        view_module_state::saved_subtractive_shadow_color_valid = false;
        view_module_state::saved_reflection_intensity_valid = false;
        view_module_state::saved_quality_shadows_valid = false;
        view_module_state::saved_quality_shadow_distance_valid = false;

        if (view_module_state::get_ambient_light && view_module_state::set_ambient_light)
        {
            view_module_state::saved_ambient_light = view_module_state::get_ambient_light();
            view_module_state::saved_ambient_light_valid = true;
        }

        if (view_module_state::get_ambient_intensity && view_module_state::set_ambient_intensity)
        {
            view_module_state::saved_ambient_intensity = view_module_state::get_ambient_intensity();
            view_module_state::saved_ambient_intensity_valid = true;
        }

        if (view_module_state::get_ambient_mode && view_module_state::set_ambient_mode)
        {
            view_module_state::saved_ambient_mode = view_module_state::get_ambient_mode();
            view_module_state::saved_ambient_mode_valid = true;
        }

        if (view_module_state::get_fog && view_module_state::set_fog)
        {
            view_module_state::saved_fog = view_module_state::get_fog();
            view_module_state::saved_fog_valid = true;
        }

        if (view_module_state::get_ambient_sky_color && view_module_state::set_ambient_sky_color)
        {
            view_module_state::saved_ambient_sky_color =
                view_module_state::get_ambient_sky_color();
            view_module_state::saved_ambient_sky_color_valid = true;
        }

        if (view_module_state::get_ambient_equator_color &&
            view_module_state::set_ambient_equator_color)
        {
            view_module_state::saved_ambient_equator_color =
                view_module_state::get_ambient_equator_color();
            view_module_state::saved_ambient_equator_color_valid = true;
        }

        if (view_module_state::get_ambient_ground_color &&
            view_module_state::set_ambient_ground_color)
        {
            view_module_state::saved_ambient_ground_color =
                view_module_state::get_ambient_ground_color();
            view_module_state::saved_ambient_ground_color_valid = true;
        }

        if (view_module_state::get_subtractive_shadow_color &&
            view_module_state::set_subtractive_shadow_color)
        {
            view_module_state::saved_subtractive_shadow_color =
                view_module_state::get_subtractive_shadow_color();
            view_module_state::saved_subtractive_shadow_color_valid = true;
        }

        if (view_module_state::get_reflection_intensity &&
            view_module_state::set_reflection_intensity)
        {
            view_module_state::saved_reflection_intensity =
                view_module_state::get_reflection_intensity();
            view_module_state::saved_reflection_intensity_valid = true;
        }

        if (view_module_state::get_quality_shadows && view_module_state::set_quality_shadows)
        {
            view_module_state::saved_quality_shadows =
                view_module_state::get_quality_shadows();
            view_module_state::saved_quality_shadows_valid = true;
        }

        if (view_module_state::get_quality_shadow_distance &&
            view_module_state::set_quality_shadow_distance)
        {
            view_module_state::saved_quality_shadow_distance =
                view_module_state::get_quality_shadow_distance();
            view_module_state::saved_quality_shadow_distance_valid = true;
        }

        view_module_state::fullbright_saved = true;
    }

    void restore_light_states() const
    {
        if (!view_module_state::resolve_light_settings())
        {
            view_module_state::saved_lights.clear();
            return;
        }

        for (const view_module_state::LightState& state : view_module_state::saved_lights)
        {
            if (!state.light)
            {
                continue;
            }

            if (state.has_enabled && view_module_state::set_behaviour_enabled)
            {
                view_module_state::set_behaviour_enabled(state.light, state.enabled);
            }

            if (state.has_color && view_module_state::set_light_color)
            {
                view_module_state::set_light_color(state.light, state.color);
            }

            if (state.has_intensity && view_module_state::set_light_intensity)
            {
                view_module_state::set_light_intensity(state.light, state.intensity);
            }

            if (state.has_bounce_intensity && view_module_state::set_light_bounce_intensity)
            {
                view_module_state::set_light_bounce_intensity(
                    state.light,
                    state.bounce_intensity);
            }

            if (state.has_shadow_strength && view_module_state::set_light_shadow_strength)
            {
                view_module_state::set_light_shadow_strength(
                    state.light,
                    state.shadow_strength);
            }

            if (state.has_shadows && view_module_state::set_light_shadows)
            {
                view_module_state::set_light_shadows(state.light, state.shadows);
            }
        }

        view_module_state::saved_lights.clear();
        view_module_state::last_light_refresh = 0;
    }

    void restore_fullbright(bool restore_lights = true) const
    {
        const bool had_fullbright_settings = view_module_state::fullbright_saved;
        const bool has_render_settings = view_module_state::resolve_render_settings();
        if (had_fullbright_settings && has_render_settings)
        {
            if (view_module_state::saved_ambient_mode_valid && view_module_state::set_ambient_mode)
            {
                view_module_state::set_ambient_mode(view_module_state::saved_ambient_mode);
            }

            if (view_module_state::saved_ambient_light_valid && view_module_state::set_ambient_light)
            {
                view_module_state::set_ambient_light(view_module_state::saved_ambient_light);
            }

            if (view_module_state::saved_ambient_intensity_valid && view_module_state::set_ambient_intensity)
            {
                view_module_state::set_ambient_intensity(view_module_state::saved_ambient_intensity);
            }

            if (view_module_state::saved_fog_valid && view_module_state::set_fog)
            {
                view_module_state::set_fog(view_module_state::saved_fog);
            }

            if (view_module_state::saved_ambient_sky_color_valid &&
                view_module_state::set_ambient_sky_color)
            {
                view_module_state::set_ambient_sky_color(
                    view_module_state::saved_ambient_sky_color);
            }

            if (view_module_state::saved_ambient_equator_color_valid &&
                view_module_state::set_ambient_equator_color)
            {
                view_module_state::set_ambient_equator_color(
                    view_module_state::saved_ambient_equator_color);
            }

            if (view_module_state::saved_ambient_ground_color_valid &&
                view_module_state::set_ambient_ground_color)
            {
                view_module_state::set_ambient_ground_color(
                    view_module_state::saved_ambient_ground_color);
            }

            if (view_module_state::saved_subtractive_shadow_color_valid &&
                view_module_state::set_subtractive_shadow_color)
            {
                view_module_state::set_subtractive_shadow_color(
                    view_module_state::saved_subtractive_shadow_color);
            }

            if (view_module_state::saved_reflection_intensity_valid &&
                view_module_state::set_reflection_intensity)
            {
                view_module_state::set_reflection_intensity(
                    view_module_state::saved_reflection_intensity);
            }

            if (view_module_state::saved_quality_shadows_valid &&
                view_module_state::set_quality_shadows)
            {
                view_module_state::set_quality_shadows(
                    view_module_state::saved_quality_shadows);
            }

            if (view_module_state::saved_quality_shadow_distance_valid &&
                view_module_state::set_quality_shadow_distance)
            {
                view_module_state::set_quality_shadow_distance(
                    view_module_state::saved_quality_shadow_distance);
            }
        }

        view_module_state::fullbright_saved = false;
        view_module_state::fullbright_readback_logged = false;
        view_module_state::restore_camera_effects(restore_lights);

        if (restore_lights)
        {
            restore_light_states();
        }
        else
        {
            view_module_state::saved_lights.clear();
            view_module_state::last_light_refresh = 0;
        }
    }

    void refresh_light_cache(float brightness) const
    {
        if (!view_module_state::resolve_light_settings())
        {
            return;
        }

        const ULONGLONG now = GetTickCount64();
        if (now - view_module_state::last_light_refresh < 750)
        {
            return;
        }
        view_module_state::last_light_refresh = now;

        std::size_t light_count = 0;
        Il2CppObject** lights = unity::find_objects_of_type(view_module_state::light_type, &light_count);
        if (!lights)
        {
            return;
        }

        for (std::size_t index = 0; index < light_count; ++index)
        {
            Il2CppObject* light = lights[index];
            if (!light)
            {
                continue;
            }

            view_module_state::save_light_state(light);
            apply_light_fullbright(light, brightness);
        }
    }

    void apply_saved_light_fullbright(float brightness) const
    {
        for (const view_module_state::LightState& state : view_module_state::saved_lights)
        {
            if (state.light)
            {
                apply_light_fullbright(state.light, brightness);
            }
        }
    }

    void apply_light_fullbright(Il2CppObject* light, float brightness) const
    {
        if (!light)
        {
            return;
        }

        if (view_module_state::set_behaviour_enabled)
        {
            view_module_state::set_behaviour_enabled(light, true);
        }

        if (view_module_state::set_light_color)
        {
            view_module_state::set_light_color(light, unity::color(1.0f, 1.0f, 1.0f, 1.0f));
        }

        if (view_module_state::set_light_intensity)
        {
            float light_intensity = (std::clamp)(brightness, 0.25f, 5.0f);
            if (view_module_state::get_light_intensity)
            {
                light_intensity = (std::max)(
                    light_intensity,
                    view_module_state::get_light_intensity(light));
            }
            view_module_state::set_light_intensity(light, light_intensity);
        }

        if (view_module_state::set_light_bounce_intensity)
        {
            view_module_state::set_light_bounce_intensity(light, brightness);
        }

        if (view_module_state::set_light_shadow_strength)
        {
            view_module_state::set_light_shadow_strength(light, 0.0f);
        }

        if (view_module_state::set_light_shadows)
        {
            constexpr int light_shadows_none = 0;
            view_module_state::set_light_shadows(light, light_shadows_none);
        }
    }

    [[nodiscard]] float get_fullbright_brightness() const
    {
        const float raw_brightness = fullbright_strength->get_value();
        const float brightness = (std::clamp)(raw_brightness, 0.25f, 5.00f);
        if (raw_brightness != brightness)
        {
            fullbright_strength->set(std::to_string(brightness));
        }

        return brightness;
    }

    void apply_fullbright() const
    {
        if (!fullbright->get_value())
        {
            restore_fullbright();
            return;
        }

        const bool has_render_settings = view_module_state::resolve_render_settings();
        const bool boost_scene_lights = fullbright_boost_lights->get_value();
        const bool light_settings_available = view_module_state::resolve_light_settings();
        const bool has_light_settings = boost_scene_lights && light_settings_available;
        const bool has_quality_settings = view_module_state::resolve_quality_settings();
        if (!has_render_settings && !has_light_settings && !has_quality_settings)
        {
            return;
        }

        save_fullbright_state();
        view_module_state::dump_camera_components_once();
        view_module_state::dump_weather_components_once();
        view_module_state::apply_camera_effect_fullbright();

        constexpr int ambient_mode_flat = 3;
        const float brightness = get_fullbright_brightness();
        const unity::color fullbright_color(
            brightness,
            brightness,
            brightness,
            1.0f);
        if (!view_module_state::fullbright_resolution_logged)
        {
            view_module_state::fullbright_resolution_logged = true;
            logger::info("[Fullbright] resolved ambient=" +
                std::to_string(view_module_state::set_ambient_light != nullptr ? 1 : 0) +
                " sky=" + std::to_string(
                    view_module_state::set_ambient_sky_color != nullptr ? 1 : 0) +
                " equator=" + std::to_string(
                    view_module_state::set_ambient_equator_color != nullptr ? 1 : 0) +
                " ground=" + std::to_string(
                    view_module_state::set_ambient_ground_color != nullptr ? 1 : 0) +
                " reflection=" + std::to_string(
                    view_module_state::set_reflection_intensity != nullptr ? 1 : 0) +
                " shadow_color=" + std::to_string(
                    view_module_state::set_subtractive_shadow_color != nullptr ? 1 : 0) +
                " fog=" + std::to_string(view_module_state::set_fog != nullptr ? 1 : 0) +
                " light_type=" + std::to_string(
                    view_module_state::light_type != nullptr ? 1 : 0) +
                " light_intensity=" + std::to_string(
                    view_module_state::set_light_intensity != nullptr ? 1 : 0) +
                " light_shadows=" + std::to_string(
                    view_module_state::set_light_shadows != nullptr ? 1 : 0) +
                " quality_shadows=" + std::to_string(
                    view_module_state::set_quality_shadows != nullptr ? 1 : 0) +
                " quality_distance=" + std::to_string(
                    view_module_state::set_quality_shadow_distance != nullptr ? 1 : 0) +
                " disable_fog=" + std::to_string(
                    fullbright_disable_fog->get_value() ? 1 : 0) +
                " boost_lights=" + std::to_string(boost_scene_lights ? 1 : 0));
        }

        if (view_module_state::set_ambient_mode)
        {
            view_module_state::set_ambient_mode(ambient_mode_flat);
        }

        if (view_module_state::set_ambient_light)
        {
            view_module_state::set_ambient_light(fullbright_color);
        }

        if (view_module_state::set_ambient_sky_color)
        {
            view_module_state::set_ambient_sky_color(fullbright_color);
        }

        if (view_module_state::set_ambient_equator_color)
        {
            view_module_state::set_ambient_equator_color(fullbright_color);
        }

        if (view_module_state::set_ambient_ground_color)
        {
            view_module_state::set_ambient_ground_color(fullbright_color);
        }

        if (view_module_state::set_subtractive_shadow_color)
        {
            view_module_state::set_subtractive_shadow_color(fullbright_color);
        }

        if (view_module_state::set_ambient_intensity)
        {
            view_module_state::set_ambient_intensity(brightness);
        }

        if (view_module_state::set_reflection_intensity)
        {
            view_module_state::set_reflection_intensity(brightness);
        }

        if (view_module_state::set_quality_shadows)
        {
            constexpr int shadow_quality_disable = 0;
            view_module_state::set_quality_shadows(shadow_quality_disable);
        }

        if (view_module_state::set_quality_shadow_distance)
        {
            view_module_state::set_quality_shadow_distance(0.0f);
        }

        if (fullbright_disable_fog->get_value() && view_module_state::set_fog)
        {
            view_module_state::set_fog(false);
        }

        if (boost_scene_lights)
        {
            apply_saved_light_fullbright(brightness);
            refresh_light_cache(brightness);
        }
        else if (!view_module_state::saved_lights.empty())
        {
            restore_light_states();
        }

        if (!view_module_state::fullbright_readback_logged &&
            view_module_state::get_ambient_light)
        {
            view_module_state::fullbright_readback_logged = true;
            const unity::color applied_color = view_module_state::get_ambient_light();
            const int applied_mode = view_module_state::get_ambient_mode ?
                view_module_state::get_ambient_mode() :
                -1;
            const float applied_reflection = view_module_state::get_reflection_intensity ?
                view_module_state::get_reflection_intensity() :
                -1.0f;
            const int applied_quality_shadows = view_module_state::get_quality_shadows ?
                view_module_state::get_quality_shadows() :
                -1;
            const float applied_shadow_distance =
                view_module_state::get_quality_shadow_distance ?
                    view_module_state::get_quality_shadow_distance() :
                    -1.0f;
            logger::info("[Fullbright] applied strength=" + std::to_string(brightness) +
                " mode=" + std::to_string(applied_mode) +
                " ambient=(" + std::to_string(applied_color.r) + "," +
                    std::to_string(applied_color.g) + "," +
                    std::to_string(applied_color.b) + ")" +
                " reflection=" + std::to_string(applied_reflection) +
                " fog=" + std::to_string(
                    view_module_state::get_fog ?
                        (view_module_state::get_fog() ? 1 : 0) :
                        -1) +
                " quality_shadows=" + std::to_string(applied_quality_shadows) +
                " shadow_distance=" + std::to_string(applied_shadow_distance) +
                " disabled_effects=" +
                    std::to_string(view_module_state::saved_camera_effects.size()));
        }
    }
};
