#pragma once
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include <Windows.h>

#include "../game_state.h"
#include "../Module.h"
#include "../ModuleCategory.h"
#include "../../assets/weapon_finish_assets.h"
#include "../../config/ActionRowValue.h"
#include "../../config/CheckboxValue.h"
#include "../../config/ColorpickerValue.h"
#include "../../config/FloatSliderValue.h"
#include "../../config/SelectValue.h"
#include "../../gui/media/media_widget.h"
#include "../../gui/menu/menu.h"
#include "../../il2cpp/Il2CppObjectInstance.h"
#include "../../il2cpp/il2utils.h"
#include "../../il2cpp/unity.h"

namespace weapon_finish_detail
{
    enum class MaterialKind
    {
        Weapon,
        Optic,
        Arms,
        Skip
    };

    [[nodiscard]] inline std::string lowercase(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        });
        return value;
    }

    [[nodiscard]] inline bool contains_any(const std::string& value, const std::vector<const char*>& needles)
    {
        for (const char* needle : needles)
        {
            if (value.find(needle) != std::string::npos)
            {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] inline MaterialKind classify_material(
        const std::string& material_name,
        bool include_scope,
        bool include_arms)
    {
        const std::string name = lowercase(material_name);
        const bool is_scope = contains_any(name, { "scope", "optic", "sight", "lens" });
        const bool is_arms = contains_any(name, { "watch", "bear", "usec", "arm", "hand", "glove", "sleeve" });
        const bool is_wearable = contains_any(name, {
            "helmet", "glasses", "rig", "belt", "wear", "mask", "balaclava", "armor", "armour", "vest", "head"
        });

        if (is_wearable)
        {
            return MaterialKind::Skip;
        }

        if (is_scope && !include_scope)
        {
            return MaterialKind::Skip;
        }

        if (is_arms && !include_arms)
        {
            return MaterialKind::Skip;
        }

        if (is_scope)
        {
            return MaterialKind::Optic;
        }

        if (is_arms)
        {
            return MaterialKind::Arms;
        }

        return MaterialKind::Weapon;
    }

    [[nodiscard]] inline unity::color to_unity_color(const ImVec4& color, float alpha_scale = 1.0f)
    {
        return unity::color{
            color.x,
            color.y,
            color.z,
            std::clamp(color.w * alpha_scale, 0.0f, 1.0f)
        };
    }

    [[nodiscard]] inline ImVec4 color_from_u32(ImU32 color, float alpha)
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

    [[nodiscard]] inline const char* shader_asset_name(int style_index)
    {
        switch (style_index)
        {
        case 1:
            return "forcefield.shader";
        case 2:
            return "glass.shader";
        case 3:
            return "lava.shader";
        default:
            return "outline.shader";
        }
    }

    [[nodiscard]] inline Il2CppObject* finish_shader(int style_index)
    {
        if (!weapon_finish_assets::asset_bundle)
        {
            return nullptr;
        }

        static Il2CppObject* shader_type = nullptr;
        if (!shader_type)
        {
            const Il2CppImage* unity_image = il2utils::resolve_image("UnityEngine.CoreModule.dll");
            if (!unity_image)
            {
                return nullptr;
            }

            shader_type = il2utils::get_system_type(unity_image, "UnityEngine", "Shader");
            if (!shader_type)
            {
                return nullptr;
            }
        }

        static std::array<Il2CppObject*, 4> shaders{};
        const int resolved_index = std::clamp(style_index, 0, static_cast<int>(shaders.size()) - 1);
        if (!shaders[static_cast<std::size_t>(resolved_index)])
        {
            shaders[static_cast<std::size_t>(resolved_index)] =
                weapon_finish_assets::load_asset(shader_asset_name(resolved_index), shader_type);
        }

        return shaders[static_cast<std::size_t>(resolved_index)];
    }
}

namespace weapon_aim_detail
{
    struct AimTarget
    {
        unity::vector3 screen_position{};
        unity::vector3 world_position{};
        bool valid = false;
    };

    inline AimTarget current_target{};
    inline std::mutex current_target_mutex{};
    inline ULONGLONG last_mouse_input_tick = 0;

    inline void set_current_target(const AimTarget& target)
    {
        std::lock_guard<std::mutex> lock(current_target_mutex);
        current_target = target;
    }

    [[nodiscard]] inline AimTarget get_current_target()
    {
        std::lock_guard<std::mutex> lock(current_target_mutex);
        return current_target;
    }

    inline void clear_current_target()
    {
        set_current_target({});
    }

    [[nodiscard]] inline Il2CppObject* object_field(Il2CppObject* object, const char* field_name)
    {
        if (!object || !object->klass)
        {
            return nullptr;
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (!field || field->offset <= 0)
        {
            return nullptr;
        }

        return *reinterpret_cast<Il2CppObject**>(reinterpret_cast<std::uintptr_t>(object) + field->offset);
    }

    [[nodiscard]] inline Il2CppObject* object_method0(Il2CppObject* object, const char* method_name)
    {
        if (!object || !object->klass)
        {
            return nullptr;
        }

        const MethodInfo* method = il2utils::resolve_method(object->klass, method_name, 0);
        if (!method || !method->methodPointer)
        {
            return nullptr;
        }

        const auto call_method = static_cast<Il2CppObject*(*)(Il2CppObject*)>(method->methodPointer);
        return call_method(object);
    }

    [[nodiscard]] inline bool object_position(Il2CppObject* object, unity::vector3& position)
    {
        if (!object || !object->klass)
        {
            return false;
        }

        const MethodInfo* method = il2utils::resolve_method(object->klass, "get_position", 0);
        if (!method || !method->methodPointer)
        {
            return false;
        }

        const auto get_position = static_cast<unity::vector3(*)(Il2CppObject*)>(method->methodPointer);
        position = get_position(object);
        return true;
    }

    [[nodiscard]] inline bool can_send_mouse_input(ULONGLONG interval_ms)
    {
        const ULONGLONG now = GetTickCount64();
        if (now - last_mouse_input_tick < interval_ms)
        {
            return false;
        }

        last_mouse_input_tick = now;
        return true;
    }

    [[nodiscard]] inline unity::vector3 world_to_screen(Il2CppObject* camera, unity::vector3 world_point)
    {
        unity::vector3 screen_point = unity::world_to_screen_point(camera, world_point);
        const float camera_height = static_cast<float>((std::max)(1, unity::camera_pixel_height(camera)));
        const float scale = menu::get_height() / camera_height;
        screen_point.y = menu::get_height() - (screen_point.y * scale);
        screen_point.x *= scale;

        if (game_state::current_zoom > 1.0f)
        {
            screen_point.x -= menu::get_width() * 0.5f;
            screen_point.y -= menu::get_height() * 0.5f;
            screen_point.x *= game_state::current_zoom;
            screen_point.y *= game_state::current_zoom;
            screen_point.x += menu::get_width() * 0.5f;
            screen_point.y += menu::get_height() * 0.5f;
        }

        return screen_point;
    }

    [[nodiscard]] inline bool is_visible(const unity::vector3& screen_point) noexcept
    {
        return screen_point.z > 0.01f &&
            screen_point.x > -100.0f &&
            screen_point.y > -100.0f &&
            screen_point.x < menu::get_width() + 100.0f &&
            screen_point.y < menu::get_height() + 100.0f;
    }

    [[nodiscard]] inline int aim_key_code(int key_index) noexcept
    {
        switch (key_index)
        {
        case 1:
            return VK_LBUTTON;
        case 2:
            return VK_SHIFT;
        case 3:
            return VK_MENU;
        case 4:
            return VK_XBUTTON1;
        case 5:
            return VK_XBUTTON2;
        case 0:
        default:
            return VK_RBUTTON;
        }
    }

    inline void send_mouse_delta(float delta_x, float delta_y)
    {
        constexpr float max_delta = 60.0f;
        const auto dx = static_cast<LONG>(std::lround(std::clamp(delta_x, -max_delta, max_delta)));
        const auto dy = static_cast<LONG>(std::lround(std::clamp(delta_y, -max_delta, max_delta)));
        if (dx == 0 && dy == 0)
        {
            return;
        }

        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dx = dx;
        input.mi.dy = dy;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        SendInput(1, &input, sizeof(INPUT));
    }
}

namespace weapon_runtime_detail
{
    struct ValueTypePatch
    {
        const char* field_name;
        std::size_t byte_count;
    };

    [[nodiscard]] inline std::string lowercase(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        });
        return value;
    }

    [[nodiscard]] inline std::string class_name(const Il2CppClass* klass)
    {
        if (!klass)
        {
            return "<null>";
        }

        return std::string(klass->namespaze ? klass->namespaze : "") + "." +
            std::string(klass->name ? klass->name : "<unknown>");
    }

    [[nodiscard]] inline std::string safe_string(const Il2CppString* value)
    {
        if (!value)
        {
            return "";
        }

        return il2utils::conv_string(value);
    }

    [[nodiscard]] inline bool field_name_contains_token(const char* field_name, const std::vector<const char*>& tokens)
    {
        if (!field_name)
        {
            return false;
        }

        const std::string name = lowercase(field_name);
        for (const char* token : tokens)
        {
            if (name.find(token) != std::string::npos)
            {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] inline bool is_reference_type(const Il2CppType* type)
    {
        if (!type || type->valuetype)
        {
            return false;
        }

        return type->type == IL2CPP_TYPE_CLASS || type->type == IL2CPP_TYPE_OBJECT ||
            type->type == IL2CPP_TYPE_ARRAY || type->type == IL2CPP_TYPE_SZARRAY ||
            type->type == IL2CPP_TYPE_GENERICINST;
    }

    [[nodiscard]] inline const FieldInfo* field_by_name(Il2CppObject* object, const char* field_name)
    {
        if (!object || !object->klass || !field_name)
        {
            return nullptr;
        }

        return il2utils::resolve_field(object->klass, field_name);
    }

    [[nodiscard]] inline Il2CppObject* reference_field(Il2CppObject* object, const char* field_name)
    {
        const FieldInfo* field = field_by_name(object, field_name);
        if (!field || field->offset <= 0 || !field->type || !is_reference_type(field->type))
        {
            return nullptr;
        }

        return *reinterpret_cast<Il2CppObject**>(reinterpret_cast<std::uintptr_t>(object) + field->offset);
    }

    [[nodiscard]] inline bool set_numeric_field_zero(Il2CppObject* object, const FieldInfo& field)
    {
        if (!object || !object->klass || field.offset <= 0 || !field.type)
        {
            return false;
        }

        const uintptr_t address = reinterpret_cast<uintptr_t>(object) + field.offset;
        switch (field.type->type)
        {
        case IL2CPP_TYPE_R4:
            *reinterpret_cast<float*>(address) = 0.0f;
            return true;
        case IL2CPP_TYPE_R8:
            *reinterpret_cast<double*>(address) = 0.0;
            return true;
        case IL2CPP_TYPE_I4:
            *reinterpret_cast<int32_t*>(address) = 0;
            return true;
        case IL2CPP_TYPE_U4:
            *reinterpret_cast<uint32_t*>(address) = 0;
            return true;
        case IL2CPP_TYPE_I2:
            *reinterpret_cast<int16_t*>(address) = 0;
            return true;
        case IL2CPP_TYPE_U2:
            *reinterpret_cast<uint16_t*>(address) = 0;
            return true;
        case IL2CPP_TYPE_I1:
            *reinterpret_cast<int8_t*>(address) = 0;
            return true;
        case IL2CPP_TYPE_U1:
            *reinterpret_cast<uint8_t*>(address) = 0;
            return true;
        default:
            return false;
        }
    }

    inline int zero_exact_numeric_fields(
        Il2CppObject* object,
        const std::vector<const char*>& field_names)
    {
        if (!object || !object->klass)
        {
            return 0;
        }

        int changed = 0;
        for (const char* field_name : field_names)
        {
            const FieldInfo* field = field_by_name(object, field_name);
            if (field && set_numeric_field_zero(object, *field))
            {
                ++changed;
            }
        }

        return changed;
    }

    inline bool zero_value_type_field_bytes(
        Il2CppObject* object,
        const char* field_name,
        std::size_t byte_count)
    {
        const FieldInfo* field = field_by_name(object, field_name);
        if (!field || field->offset <= 0 || !field->type ||
            field->type->type != IL2CPP_TYPE_VALUETYPE || byte_count == 0 || byte_count > 32)
        {
            return false;
        }

        std::memset(
            reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(object) + field->offset),
            0,
            byte_count);
        return true;
    }

    inline int zero_value_type_fields(
        Il2CppObject* object,
        const std::vector<ValueTypePatch>& patches)
    {
        if (!object || !object->klass)
        {
            return 0;
        }

        int changed = 0;
        for (const ValueTypePatch& patch : patches)
        {
            if (zero_value_type_field_bytes(object, patch.field_name, patch.byte_count))
            {
                ++changed;
            }
        }

        return changed;
    }

    inline int zero_matching_numeric_fields(
        Il2CppObject* object,
        const std::vector<const char*>& tokens)
    {
        if (!object || !object->klass)
        {
            return 0;
        }

        int changed = 0;
        for (Il2CppClass* class_cursor = object->klass; class_cursor != nullptr;
            class_cursor = class_cursor->parent)
        {
            if (!class_cursor->fields)
            {
                continue;
            }

            for (uint16_t index = 0; index < class_cursor->field_count; ++index)
            {
                const FieldInfo& field = class_cursor->fields[index];
                if (field_name_contains_token(field.name, tokens) &&
                    set_numeric_field_zero(object, field))
                {
                    ++changed;
                }
            }
        }

        return changed;
    }

    inline int zero_all_numeric_fields(Il2CppObject* object)
    {
        if (!object || !object->klass)
        {
            return 0;
        }

        int changed = 0;
        for (Il2CppClass* class_cursor = object->klass; class_cursor != nullptr;
            class_cursor = class_cursor->parent)
        {
            if (!class_cursor->fields)
            {
                continue;
            }

            for (uint16_t index = 0; index < class_cursor->field_count; ++index)
            {
                if (set_numeric_field_zero(object, class_cursor->fields[index]))
                {
                    ++changed;
                }
            }
        }

        return changed;
    }

    inline int zero_nested_numeric_fields(
        Il2CppObject* object,
        const std::vector<const char*>& container_tokens,
        const std::vector<const char*>& field_tokens)
    {
        if (!object || !object->klass)
        {
            return 0;
        }

        int changed = 0;
        for (Il2CppClass* class_cursor = object->klass; class_cursor != nullptr;
            class_cursor = class_cursor->parent)
        {
            if (!class_cursor->fields)
            {
                continue;
            }

            for (uint16_t index = 0; index < class_cursor->field_count; ++index)
            {
                const FieldInfo& field = class_cursor->fields[index];
                if (!field.type || !is_reference_type(field.type) || field.offset <= 0 ||
                    !field_name_contains_token(field.name, container_tokens))
                {
                    continue;
                }

                Il2CppObject* child = *reinterpret_cast<Il2CppObject**>(
                    reinterpret_cast<uintptr_t>(object) + field.offset);
                changed += zero_matching_numeric_fields(child, field_tokens);
            }
        }

        return changed;
    }

    inline int zero_nested_all_numeric_fields(
        Il2CppObject* object,
        const std::vector<const char*>& container_tokens)
    {
        if (!object || !object->klass)
        {
            return 0;
        }

        int changed = 0;
        for (Il2CppClass* class_cursor = object->klass; class_cursor != nullptr;
            class_cursor = class_cursor->parent)
        {
            if (!class_cursor->fields)
            {
                continue;
            }

            for (uint16_t index = 0; index < class_cursor->field_count; ++index)
            {
                const FieldInfo& field = class_cursor->fields[index];
                if (!field.type || !is_reference_type(field.type) || field.offset <= 0 ||
                    !field_name_contains_token(field.name, container_tokens))
                {
                    continue;
                }

                Il2CppObject* child = *reinterpret_cast<Il2CppObject**>(
                    reinterpret_cast<uintptr_t>(object) + field.offset);
                changed += zero_all_numeric_fields(child);
            }
        }

        return changed;
    }

    inline void dump_class_fields(Il2CppObject* object, const std::string& label)
    {
        if (!object || !object->klass)
        {
            logger::warn("[WeaponDump] " + label + " null");
            return;
        }

        logger::info("[WeaponDump] " + label + " class=" + class_name(object->klass));
        int logged_index = 0;
        for (Il2CppClass* class_cursor = object->klass; class_cursor != nullptr;
            class_cursor = class_cursor->parent)
        {
            if (!class_cursor->fields)
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
                else if (field.type && field.type->type == IL2CPP_TYPE_BOOLEAN)
                {
                    value = std::to_string(*reinterpret_cast<bool*>(
                        reinterpret_cast<uintptr_t>(object) + field.offset));
                }
                else if (field.type && field.type->type == IL2CPP_TYPE_STRING)
                {
                    value = "'" + safe_string(*reinterpret_cast<Il2CppString**>(
                        reinterpret_cast<uintptr_t>(object) + field.offset)) + "'";
                }
                else if (field.type && field.type->type == IL2CPP_TYPE_R4)
                {
                    value = std::to_string(*reinterpret_cast<float*>(
                        reinterpret_cast<uintptr_t>(object) + field.offset));
                }
                else if (field.type && field.type->type == IL2CPP_TYPE_R8)
                {
                    value = std::to_string(*reinterpret_cast<double*>(
                        reinterpret_cast<uintptr_t>(object) + field.offset));
                }
                else if (field.type && field.type->type == IL2CPP_TYPE_I4)
                {
                    value = std::to_string(*reinterpret_cast<int32_t*>(
                        reinterpret_cast<uintptr_t>(object) + field.offset));
                }
                else if (field.type && field.type->type == IL2CPP_TYPE_U4)
                {
                    value = std::to_string(*reinterpret_cast<uint32_t*>(
                        reinterpret_cast<uintptr_t>(object) + field.offset));
                }
                else if (field.type && is_reference_type(field.type))
                {
                    Il2CppObject* child = *reinterpret_cast<Il2CppObject**>(
                        reinterpret_cast<uintptr_t>(object) + field.offset);
                    value = child ? "class='" + class_name(child->klass) + "'" : "null";
                }

                logger::info("[WeaponDump] " + label + " #" + std::to_string(logged_index++) +
                    " declaring_class='" + class_name(class_cursor) +
                    "' name='" + std::string(field.name ? field.name : "<null>") +
                    "' offset=" + std::to_string(field.offset) +
                    " type=" + std::to_string(type) +
                    " value=" + value);
            }
        }
    }

    inline void dump_matching_reference_fields(
        Il2CppObject* object,
        const std::string& label,
        const std::vector<const char*>& tokens)
    {
        if (!object || !object->klass)
        {
            return;
        }

        for (Il2CppClass* class_cursor = object->klass; class_cursor != nullptr;
            class_cursor = class_cursor->parent)
        {
            if (!class_cursor->fields)
            {
                continue;
            }

            for (uint16_t index = 0; index < class_cursor->field_count; ++index)
            {
                const FieldInfo& field = class_cursor->fields[index];
                if (!field.type || !is_reference_type(field.type) || field.offset <= 0 ||
                    !field_name_contains_token(field.name, tokens))
                {
                    continue;
                }

                Il2CppObject* child = *reinterpret_cast<Il2CppObject**>(
                    reinterpret_cast<uintptr_t>(object) + field.offset);
                dump_class_fields(child, label + "." + std::string(field.name ? field.name : "<null>"));
            }
        }
    }
}

class WeaponModule : Module
{
public:
    WeaponModule() : Module("Weapon", Weapons)
    {
    }

    CheckboxValue* no_jam = conf(new CheckboxValue(false, "No Malfunction"));
    CheckboxValue* infinite_ammo = conf(new CheckboxValue(false, "Infinite Ammo"));
    CheckboxValue* no_recoil = conf(new CheckboxValue(false, "No Recoil"));
    CheckboxValue* no_spread = conf(new CheckboxValue(false, "No Spread"));
    CheckboxValue* no_sway = conf(new CheckboxValue(false, "No Sway"));
    ActionRowValue* dump_weapon_fields = conf(new ActionRowValue([&]
    {
        dump_weapon_fields_impl();
    }, "Dump Weapon Fields"));

    CheckboxValue* aim_lock = conf(new CheckboxValue(false, "Aim Lock"));
    SelectValue* aim_lock_key = conf(new SelectValue(0, {
                                                        "RMB", "LMB", "Shift", "Alt", "Mouse 4", "Mouse 5"
                                                    }, "Aim Key"));
    FloatSliderValue* aim_lock_fov = conf(new FloatSliderValue(120.0f, 15.0f, 500.0f, "Aim FOV"));
    FloatSliderValue* aim_lock_smooth = conf(new FloatSliderValue(8.0f, 1.0f, 30.0f, "Aim Smooth"));
    CheckboxValue* show_aim_fov = conf(new CheckboxValue(true, "Show Aim FOV"));
    ColorpickerValue* aim_lock_color = conf(new ColorpickerValue({0.60f, 0.70f, 1.0f, 0.85f}, "Aim Lock Color"));

    CheckboxValue* weapon_finish = conf(new CheckboxValue(false, "Weapon Finish"));
    SelectValue* finish_style = conf(new SelectValue(1, {
                                                         "Tint", "Soft Glow", "Chrome", "Glass"
                                                     }, "Finish Style"));
    CheckboxValue* finish_spotify_mode = conf(new CheckboxValue(false, "Weapon Spotify Mode"));
    ColorpickerValue* finish_color = conf(new ColorpickerValue({0.60f, 0.70f, 1.0f, 0.88f}, "Weapon Color"));
    CheckboxValue* optic_spotify_mode = conf(new CheckboxValue(false, "Optic Spotify Mode"));
    ColorpickerValue* optic_finish_color = conf(new ColorpickerValue({0.96f, 0.45f, 0.69f, 0.88f}, "Optic Color"));
    CheckboxValue* arms_spotify_mode = conf(new CheckboxValue(false, "Arms Spotify Mode"));
    ColorpickerValue* arms_finish_color = conf(new ColorpickerValue({0.72f, 0.86f, 1.0f, 0.88f}, "Arms Color"));
    FloatSliderValue* finish_strength = conf(new FloatSliderValue(0.65f, 0.05f, 1.0f, "Finish Strength"));
    CheckboxValue* finish_include_scope = conf(new CheckboxValue(true, "Include Optics"));
    CheckboxValue* finish_include_arms = conf(new CheckboxValue(false, "Include Hands"));

    void draw_overlay(ImDrawList* draw_list) override
    {
        if (!game_state::is_in_raid || !aim_lock->get_value())
        {
            return;
        }

        const ImVec4 aim_color = aim_lock_color->get_value();
        const ImU32 draw_color = ImGui::GetColorU32(aim_color);
        const ImVec2 screen_center{ menu::get_width() * 0.5f, menu::get_height() * 0.5f };

        if (show_aim_fov->get_value())
        {
            draw_list->AddCircle(screen_center, aim_lock_fov->get_value(), draw_color, 96, 1.0f);
        }

        const weapon_aim_detail::AimTarget aim_target = weapon_aim_detail::get_current_target();
        if (aim_target.valid)
        {
            draw_list->AddCircleFilled(
                {
                    aim_target.screen_position.x,
                    aim_target.screen_position.y
                },
                3.0f * menu::get_scale_factor(),
                draw_color,
                24);
        }
    }

    void application_update() override
    {
        if (!game_state::is_in_raid || !aim_lock->get_value())
        {
            weapon_aim_detail::clear_current_target();
            return;
        }

        apply_aim_lock();
    }

    void reset_gameworld_state() override
    {
        queue_dump_weapon_fields = false;
        weapon_aim_detail::clear_current_target();
        game_state::current_zoom = 1.0f;
    }

    void gameworld_update(const Il2CppClass* game_world_class, Il2CppObjectInstance game_world_instance,
                          Il2CppObjectInstance main_player) override
    {
        (void)game_world_class;

        const bool needs_weapon_runtime =
            no_jam->get_value() ||
            infinite_ammo->get_value() ||
            no_recoil->get_value() ||
            no_spread->get_value() ||
            no_sway->get_value() ||
            weapon_finish->get_value() ||
            queue_dump_weapon_fields;

        if (!needs_weapon_runtime && !aim_lock->get_value())
        {
            weapon_aim_detail::clear_current_target();
            return;
        }

        if (aim_lock->get_value())
        {
            game_state::current_zoom = get_scope_zoom(main_player);
            update_aim_lock(game_world_instance, main_player);
        }
        else
        {
            weapon_aim_detail::clear_current_target();
        }

        if (!needs_weapon_runtime)
        {
            return;
        }

        Il2CppObject* hands_controller_object = main_player.get_method<Il2CppObject*(
            *)(Il2CppObject*)>("get_HandsController", 0)(main_player.get_instance());

        if (!hands_controller_object || std::string(hands_controller_object->klass->name) != "FirearmController")
        {
            return;
        }

        apply_firearm_controller_ballistics(hands_controller_object);

        Il2CppObjectInstance firearm_controller(hands_controller_object);
        Il2CppObject* weapon_object = firearm_controller.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_Item", 0)(
            firearm_controller.get_instance());
        if (!weapon_object)
        {
            return;
        }

        Il2CppObjectInstance weapon(weapon_object);
        Il2CppObject* template_object = weapon.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_Template", 0)(
            weapon.get_instance());
        if (template_object)
        {
            Il2CppObjectInstance weapon_template(template_object);
            apply_reliability_options(weapon_template);
            apply_weapon_template_ballistics(template_object);
        }

        Il2CppObject* procedural_weapon_animation_object = nullptr;
        if (no_recoil->get_value() || no_spread->get_value() || no_sway->get_value() || queue_dump_weapon_fields)
        {
            procedural_weapon_animation_object =
                main_player.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_ProceduralWeaponAnimation", 0)(
                    main_player.get_instance());
        }

        if (no_recoil->get_value() || no_spread->get_value() || no_sway->get_value())
        {
            apply_procedural_weapon_animation_options(procedural_weapon_animation_object);
        }

        if (queue_dump_weapon_fields)
        {
            dump_weapon_runtime_objects(
                hands_controller_object,
                weapon_object,
                template_object,
                procedural_weapon_animation_object);
            queue_dump_weapon_fields = false;
        }

        if (infinite_ammo->get_value())
        {
            apply_ammo_options(weapon);
        }

        if (weapon_finish->get_value())
        {
            apply_weapon_finish(main_player);
        }
    }

    void init() override
    {
    }

private:
    bool queue_dump_weapon_fields = false;

    void dump_weapon_fields_impl()
    {
        if (!game_state::is_in_raid)
        {
            logger::warn("[WeaponDump] requested outside raid");
            return;
        }

        queue_dump_weapon_fields = true;
        logger::info("[WeaponDump] queued current weapon dump");
    }

    void apply_firearm_controller_ballistics(Il2CppObject* firearm_controller_object) const
    {
        if (!firearm_controller_object)
        {
            return;
        }

        if (no_recoil->get_value())
        {
            weapon_runtime_detail::zero_exact_numeric_fields(
                firearm_controller_object,
                {
                    "_lastShotRecoil"
                });
        }

        if (no_spread->get_value())
        {
            weapon_runtime_detail::zero_exact_numeric_fields(
                firearm_controller_object,
                {
                    "COI",
                    "ShotgunDispersion",
                    "HipInaccuracy"
                });
        }
    }

    void apply_weapon_template_ballistics(Il2CppObject* weapon_template_object) const
    {
        if (!weapon_template_object)
        {
            return;
        }

        if (no_recoil->get_value())
        {
            weapon_runtime_detail::zero_exact_numeric_fields(
                weapon_template_object,
                {
                    "RecoilCategoryMultiplierHandRotation",
                    "RecoilReturnSpeedHandRotation",
                    "RecoilDampingHandRotation",
                    "RecoilCamera",
                    "RecoilStableIndexShot",
                    "RecoilForceBack",
                    "RecoilForceUp",
                    "RecolDispersion",
                    "RecoilPosZMult",
                    "RecoilReturnPathDampingHandRotation",
                    "RecoilReturnPathOffsetHandRotation",
                    "RecoilAngle",
                    "RecoilStableAngleIncreaseStep",
                    "CameraToWeaponAngleStep",
                    "CameraSnap",
                    "MountVerticalRecoilMultiplier",
                    "MountHorizontalRecoilMultiplier",
                    "MountReturnSpeedHandMultiplier",
                    "MountCameraSnapMultiplier"
                });
            weapon_runtime_detail::zero_value_type_fields(
                weapon_template_object,
                {
                    { "ProgressRecoilAngleOnStable", 12 },
                    { "RecoilCenter", 12 },
                    { "PostRecoilVerticalRangeHandRotation", 8 },
                    { "PostRecoilHorizontalRangeHandRotation", 8 },
                    { "CameraToWeaponAngleSpeedRange", 8 }
                });
            weapon_runtime_detail::zero_matching_numeric_fields(
                weapon_template_object,
                {
                    "recoil",
                    "kick",
                    "camerasnap",
                    "cameratoweapon",
                    "weaponanglestep"
                });

            Il2CppObject* recoil_settings =
                weapon_runtime_detail::reference_field(weapon_template_object, "WeaponRecoilSettings");
            weapon_runtime_detail::zero_all_numeric_fields(recoil_settings);
        }

        if (no_spread->get_value())
        {
            weapon_runtime_detail::zero_exact_numeric_fields(
                weapon_template_object,
                {
                    "ShotgunDispersion",
                    "AimPlane",
                    "CenterOfImpact",
                    "DoubleActionAccuracyPenalty",
                    "DeviationMax",
                    "DeviationCurve",
                    "HipInnaccuracyGain",
                    "HipAccuracyRestorationDelay",
                    "HipAccuracyRestorationSpeed"
                });
            weapon_runtime_detail::zero_matching_numeric_fields(
                weapon_template_object,
                {
                    "spread",
                    "dispersion",
                    "deviation",
                    "centerofimpact",
                    "aimplane",
                    "convergence",
                    "accuracy"
                });
        }

        if (no_sway->get_value())
        {
            weapon_runtime_detail::zero_exact_numeric_fields(
                weapon_template_object,
                {
                    "MountingHorizontalOutOfBreathMultiplier",
                    "MountingVerticalOutOfBreathMultiplier"
                });
        }

        Il2CppObject* aim_settings =
            weapon_runtime_detail::reference_field(weapon_template_object, "WeaponAimSettings");
        if (aim_settings && (no_spread->get_value() || no_sway->get_value()))
        {
            weapon_runtime_detail::zero_matching_numeric_fields(
                aim_settings,
                {
                    "spread",
                    "dispersion",
                    "deviation",
                    "accuracy",
                    "sway",
                    "breath",
                    "shake",
                    "motion"
                });
        }
    }

    void apply_procedural_weapon_animation_options(Il2CppObject* procedural_weapon_animation_object) const
    {
        if (!procedural_weapon_animation_object)
        {
            return;
        }

        if (no_recoil->get_value())
        {
            Il2CppObject* shooting_object =
                weapon_runtime_detail::reference_field(procedural_weapon_animation_object, "Shootingg");
            weapon_runtime_detail::zero_value_type_fields(
                shooting_object,
                {
                    { "RecoilPipeline", 8 }
                });

            weapon_runtime_detail::zero_all_numeric_fields(
                weapon_runtime_detail::reference_field(shooting_object, "OldShotRecoil"));
            weapon_runtime_detail::zero_all_numeric_fields(
                weapon_runtime_detail::reference_field(shooting_object, "NewShotRecoil"));
            weapon_runtime_detail::zero_all_numeric_fields(
                weapon_runtime_detail::reference_field(shooting_object, "_currentRecoilEffect"));
            apply_firearm_controller_ballistics(
                weapon_runtime_detail::reference_field(shooting_object, "_firearmController"));
            apply_firearm_controller_ballistics(
                weapon_runtime_detail::reference_field(procedural_weapon_animation_object, "_firearmController"));
            apply_firearm_controller_ballistics(
                weapon_runtime_detail::reference_field(procedural_weapon_animation_object, "_firearmAnimationData"));

            weapon_runtime_detail::zero_matching_numeric_fields(
                procedural_weapon_animation_object,
                {
                    "recoil",
                    "kick",
                    "shot",
                    "shoot",
                    "camerasnap"
                });
        }

        if (no_spread->get_value() || no_sway->get_value())
        {
            Il2CppObject* breath_object =
                weapon_runtime_detail::reference_field(procedural_weapon_animation_object, "Breath");
            weapon_runtime_detail::zero_exact_numeric_fields(
                breath_object,
                {
                    "HipPenalty",
                    "_cameraSensetivity",
                    "AmplitudeGainPerShot",
                    "Delay",
                    "MountingVerticalOutOfBreathMultiplier",
                    "MountingHorizontalOutOfBreathMultiplier",
                    "BipodOutOfBreathMultiplier",
                    "_shotHardness",
                    "<Overweight>k__BackingField"
                });
        }

        if (no_sway->get_value())
        {
            Il2CppObject* breath_object =
                weapon_runtime_detail::reference_field(procedural_weapon_animation_object, "Breath");
            weapon_runtime_detail::zero_exact_numeric_fields(
                breath_object,
                {
                    "Intensity",
                    "_shakeIntensity",
                    "_breathIntensity",
                    "_breathFrequency",
                    "EnergyLowerLimit",
                    "EnergyFractureLimit"
                });
            weapon_runtime_detail::zero_value_type_fields(
                breath_object,
                {
                    { "_baseHipRandomAmplitudes", 8 },
                    { "_randomBetween", 8 }
                });

            Il2CppObject* walk_object =
                weapon_runtime_detail::reference_field(procedural_weapon_animation_object, "Walk");
            weapon_runtime_detail::zero_exact_numeric_fields(
                walk_object,
                {
                    "StepFrequency",
                    "Intensity",
                    "SideSpeedMultyplyer",
                    "BackSpeedMultyplyer",
                    "Treshold",
                    "Overweight",
                    "_speed"
                });
            weapon_runtime_detail::zero_value_type_fields(
                walk_object,
                {
                    { "_lastPosition", 12 }
                });

            Il2CppObject* motion_react_object =
                weapon_runtime_detail::reference_field(procedural_weapon_animation_object, "MotionReact");
            weapon_runtime_detail::zero_exact_numeric_fields(
                motion_react_object,
                {
                    "RotationInputClamp",
                    "Intensity",
                    "BipodModifier"
                });
            weapon_runtime_detail::zero_value_type_fields(
                motion_react_object,
                {
                    { "Motion", 12 },
                    { "Velocity", 12 },
                    { "_lastPosition", 12 },
                    { "_lastForward", 12 },
                    { "PositionVelocity", 12 },
                    { "RotationVelocity", 8 },
                    { "PositionAcceleration", 12 },
                    { "RotationAcceleration", 8 },
                    { "SwayFactors", 12 },
                    { "_lastPositionVelocity", 12 },
                    { "_lastRotationVelocity", 8 },
                    { "_rotVelSum", 8 },
                    { "_rotAccSum", 8 },
                    { "lastRotation", 12 },
                    { "_platformMovement", 12 },
                    { "v", 8 },
                    { "v4", 12 },
                    { "v2", 12 },
                    { "v3", 12 }
                });

            Il2CppObject* force_react_object =
                weapon_runtime_detail::reference_field(procedural_weapon_animation_object, "ForceReact");
            weapon_runtime_detail::zero_exact_numeric_fields(
                force_react_object,
                {
                    "Intensity",
                    "_eulersNoiseXYMaxValue",
                    "_eulersNoiseZMaxValue",
                    "_deltaRandom",
                    "WiggleMagnitude"
                });
            weapon_runtime_detail::zero_value_type_fields(
                force_react_object,
                {
                    { "_force", 12 },
                    { "_forceSum", 12 }
                });
            weapon_runtime_detail::zero_all_numeric_fields(
                weapon_runtime_detail::reference_field(force_react_object, "CameraRotationSpring"));
            weapon_runtime_detail::zero_all_numeric_fields(
                weapon_runtime_detail::reference_field(force_react_object, "CameraPositionSpring"));
            weapon_runtime_detail::zero_all_numeric_fields(
                weapon_runtime_detail::reference_field(force_react_object, "HandsRotationSpring"));

            weapon_runtime_detail::zero_exact_numeric_fields(
                weapon_runtime_detail::reference_field(procedural_weapon_animation_object, "_aimSwayBlender"),
                {
                    "_target",
                    "_startValue",
                    "Speed"
                });
            weapon_runtime_detail::zero_value_type_fields(
                procedural_weapon_animation_object,
                {
                    { "_aimSwayDirection", 16 }
                });
            weapon_runtime_detail::zero_matching_numeric_fields(
                procedural_weapon_animation_object,
                {
                    "sway",
                    "breath",
                    "tremor",
                    "walk",
                    "bob",
                    "motion",
                    "force"
                });
        }
    }

    void dump_weapon_runtime_objects(
        Il2CppObject* firearm_controller_object,
        Il2CppObject* weapon_object,
        Il2CppObject* weapon_template_object,
        Il2CppObject* procedural_weapon_animation_object) const
    {
        logger::info("[WeaponDump] begin");
        weapon_runtime_detail::dump_class_fields(firearm_controller_object, "FirearmController");
        weapon_runtime_detail::dump_class_fields(weapon_object, "Weapon");
        weapon_runtime_detail::dump_class_fields(weapon_template_object, "WeaponTemplate");
        weapon_runtime_detail::dump_class_fields(procedural_weapon_animation_object, "ProceduralWeaponAnimation");
        weapon_runtime_detail::dump_matching_reference_fields(
            weapon_template_object,
            "WeaponTemplate",
            {
                "recoil",
                "aim",
                "shot",
                "spread",
                "dispersion"
            });
        weapon_runtime_detail::dump_matching_reference_fields(
            procedural_weapon_animation_object,
            "ProceduralWeaponAnimation",
            {
                "sway",
                "breath",
                "tremor",
                "walk",
                "motion",
                "force",
                "recoil",
                "shot",
                "shoot",
                "fire"
            });
        Il2CppObject* shooting_object =
            weapon_runtime_detail::reference_field(procedural_weapon_animation_object, "Shootingg");
        weapon_runtime_detail::dump_matching_reference_fields(
            shooting_object,
            "ProceduralWeaponAnimation.Shootingg",
            {
                "recoil",
                "aim",
                "buff"
            });
        logger::info("[WeaponDump] end");
    }

    [[nodiscard]] float get_scope_zoom(Il2CppObjectInstance& player) const
    {
        Il2CppObject* hands_controller_object =
            player.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_HandsController", 0)(player.get_instance());
        if (!hands_controller_object || std::string(hands_controller_object->klass->name) != "FirearmController")
        {
            return 1.0f;
        }

        Il2CppObjectInstance firearm_controller(hands_controller_object);
        const auto is_aiming = firearm_controller.get_method<bool(*)(Il2CppObject*)>("get_IsAiming", 0);
        if (!is_aiming || !is_aiming(firearm_controller.get_instance()))
        {
            return 1.0f;
        }

        Il2CppObject* procedural_weapon_animation_object =
            player.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_ProceduralWeaponAnimation", 0)(
                player.get_instance());
        if (!procedural_weapon_animation_object)
        {
            return 1.0f;
        }

        Il2CppObjectInstance procedural_weapon_animation(procedural_weapon_animation_object);
        Il2CppObject* sight_component_object =
            procedural_weapon_animation.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_CurrentAimingMod", 0)(
                procedural_weapon_animation.get_instance());
        if (!sight_component_object)
        {
            return 1.0f;
        }

        Il2CppObjectInstance sight_component(sight_component_object);
        const float zoom = sight_component.get_field<float>("ScopeZoomValue");
        if (zoom == 0.0f)
        {
            return 1.0f;
        }

        Il2CppObject* adjustable_data_object =
            sight_component.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_AdjustableOpticData", 0)(
                sight_component.get_instance());
        if (!adjustable_data_object)
        {
            return 1.0f;
        }

        Il2CppObjectInstance adjustable_data(adjustable_data_object);
        const unity::vector3 min_max_fov = adjustable_data.get_field<unity::vector3>("MinMaxFov");
        return min_max_fov.x / zoom;
    }

    void update_aim_lock(Il2CppObjectInstance& game_world_instance, Il2CppObjectInstance& main_player)
    {
        Il2CppObject* camera = unity::get_current_camera();
        if (!camera)
        {
            weapon_aim_detail::clear_current_target();
            return;
        }

        unity::list* alive_players_list = game_world_instance.get_field<unity::list*>("AllAlivePlayersList");
        if (!alive_players_list || !alive_players_list->m_p_list_array)
        {
            weapon_aim_detail::clear_current_target();
            return;
        }

        const unity::vector3 screen_center{ menu::get_width() * 0.5f, menu::get_height() * 0.5f, 0.0f };
        const float max_fov = aim_lock_fov->get_value();
        float closest_distance = max_fov;
        weapon_aim_detail::AimTarget best_target{};

        Il2CppArray* alive_players = alive_players_list->m_p_list_array;
        for (std::size_t index = 0; index < alive_players->max_length; ++index)
        {
            Il2CppObject* player_object = reinterpret_cast<Il2CppObject**>(&alive_players->data)[index];
            if (!player_object || player_object == main_player.get_instance())
            {
                continue;
            }

            Il2CppObject* player_body_object = weapon_aim_detail::object_field(player_object, "_playerBody");
            if (!player_body_object)
            {
                player_body_object = weapon_aim_detail::object_method0(player_object, "get_PlayerBody");
            }
            if (!player_body_object)
            {
                continue;
            }

            Il2CppObject* player_bones_object = weapon_aim_detail::object_field(player_body_object, "PlayerBones");
            if (!player_bones_object)
            {
                continue;
            }

            Il2CppObject* head_transform = weapon_aim_detail::object_field(player_bones_object, "Head");
            if (!head_transform)
            {
                continue;
            }

            unity::vector3 head_position{};
            if (!weapon_aim_detail::object_position(head_transform, head_position))
            {
                continue;
            }

            unity::vector3 screen_position = weapon_aim_detail::world_to_screen(camera, head_position);
            if (!weapon_aim_detail::is_visible(screen_position))
            {
                continue;
            }

            screen_position.z = 0.0f;
            const float distance = screen_position.distance(screen_center);
            if (distance < closest_distance)
            {
                closest_distance = distance;
                best_target.screen_position = screen_position;
                best_target.world_position = head_position;
                best_target.valid = true;
            }
        }

        weapon_aim_detail::set_current_target(best_target);
    }

    void apply_aim_lock()
    {
        const weapon_aim_detail::AimTarget aim_target = weapon_aim_detail::get_current_target();
        if (!aim_target.valid || menu::is_active())
        {
            return;
        }

        const int key_code = weapon_aim_detail::aim_key_code(aim_lock_key->get_value());
        if ((GetAsyncKeyState(key_code) & 0x8000) == 0)
        {
            return;
        }

        if (!weapon_aim_detail::can_send_mouse_input(6))
        {
            return;
        }

        const float smooth = (std::max)(1.0f, aim_lock_smooth->get_value());
        const float target_x = aim_target.screen_position.x;
        const float target_y = aim_target.screen_position.y;
        const float center_x = menu::get_width() * 0.5f;
        const float center_y = menu::get_height() * 0.5f;
        const float delta_x = (target_x - center_x) / smooth;
        const float delta_y = (target_y - center_y) / smooth;

        weapon_aim_detail::send_mouse_delta(delta_x, delta_y);
    }

    void apply_reliability_options(Il2CppObjectInstance& weapon_template)
    {
        if (!no_jam->get_value())
        {
            return;
        }

        weapon_template.set_field("AllowOverheat", false);
        weapon_template.set_field("AllowJam", false);
        weapon_template.set_field("AllowMisfire", false);
        weapon_template.set_field("AllowSlide", false);
        weapon_template.set_field("AllowFeed", false);
        weapon_template.set_field("DurabilityBurnRatio", 0.0f);
    }

    void apply_ammo_options(Il2CppObjectInstance& weapon)
    {
        Il2CppObject* mag_object = weapon.get_method<Il2CppObject*(*)(Il2CppObject*)>(
            "GetCurrentMagazine", 0)(
            weapon.get_instance());
        if (!mag_object)
        {
            return;
        }

        Il2CppObjectInstance magazine(mag_object);
        const int current_count = weapon.get_method<int(*)(Il2CppObject*)>("GetCurrentMagazineCount", 0)(
            weapon.get_instance());
        const int max_count = weapon.get_method<int(*)(Il2CppObject*)>("GetMaxMagazineCount", 0)(
            weapon.get_instance());

        if (current_count == 0)
        {
            return;
        }

        Il2CppObject* ammo_object = magazine.get_method<Il2CppObject*(*)(Il2CppObject*)>(
            "FirstRealAmmo", 0)(magazine.get_instance());
        if (!ammo_object)
        {
            return;
        }

        Il2CppObjectInstance first_real_ammo(ammo_object);
        const int max_stack_size = first_real_ammo.get_method<int(*)(Il2CppObject*)>(
            "get_StackMaxSize", 0)(first_real_ammo.get_instance());

        first_real_ammo.set_field("StackObjectsCount", (std::min)(max_count, max_stack_size));
    }

    void apply_weapon_finish(Il2CppObjectInstance& main_player)
    {
        const Il2CppImage* unity_image = il2utils::resolve_image("UnityEngine.CoreModule.dll");
        if (!unity_image)
        {
            return;
        }

        Il2CppObject* renderer_type = il2utils::get_system_type(unity_image, "UnityEngine", "Renderer");
        if (!renderer_type)
        {
            return;
        }

        Il2CppObject* main_player_game_object = unity::component_get_game_object(main_player.get_instance());
        Il2CppObject* main_player_transform = main_player_game_object ? unity::gameobject_get_transform(main_player_game_object) : nullptr;
        if (!main_player_transform)
        {
            return;
        }

        std::vector<Il2CppObject*> renderers{};
        unity::transform_apply_to_all_children(main_player_transform, [&renderer_type, &renderers](Il2CppObject* transform)
        {
            Il2CppObject* game_object = unity::transform_get_game_object(transform);
            if (!game_object)
            {
                return;
            }

            Il2CppObject* renderer = unity::get_component_internal(game_object, renderer_type);
            if (renderer)
            {
                renderers.push_back(renderer);
            }
        });

        for (Il2CppObject* renderer : renderers)
        {
            if (!renderer)
            {
                continue;
            }

            Il2CppObjectInstance renderer_instance(renderer);
            const auto get_materials =
                renderer_instance.get_method<Il2CppArray*(*)(Il2CppObject*)>("get_materials", 0);
            Il2CppArray* materials = get_materials ? get_materials(renderer_instance.get_instance()) : nullptr;
            if (materials)
            {
                for (std::size_t index = 0; index < materials->max_length; ++index)
                {
                    Il2CppObject* material_object = reinterpret_cast<Il2CppObject**>(&materials->data)[index];
                    if (!material_object)
                    {
                        continue;
                    }

                    maybe_style_material(material_object);
                }
            }
            else
            {
                Il2CppObject* material_object =
                    renderer_instance.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_material", 0)(
                        renderer_instance.get_instance());
                maybe_style_material(material_object);
            }
        }
    }

    void maybe_style_material(Il2CppObject* material_object)
    {
        if (!material_object)
        {
            return;
        }

        Il2CppObjectInstance material_instance(material_object);
        const std::string material_name = unity::get_name(material_instance.get_instance());
        const weapon_finish_detail::MaterialKind material_kind = weapon_finish_detail::classify_material(
            material_name,
            finish_include_scope->get_value(),
            finish_include_arms->get_value());
        if (material_kind == weapon_finish_detail::MaterialKind::Skip)
        {
            return;
        }

        style_material(material_instance, material_kind);
    }

    [[nodiscard]] ImVec4 resolve_finish_color(weapon_finish_detail::MaterialKind material_kind) const
    {
        switch (material_kind)
        {
        case weapon_finish_detail::MaterialKind::Optic:
        {
            const ImVec4 configured_color = optic_finish_color->get_value();
            if (!optic_spotify_mode->get_value())
            {
                return configured_color;
            }
            return weapon_finish_detail::color_from_u32(media_widget::get_secondary_color(), configured_color.w);
        }
        case weapon_finish_detail::MaterialKind::Arms:
        {
            const ImVec4 configured_color = arms_finish_color->get_value();
            if (!arms_spotify_mode->get_value())
            {
                return configured_color;
            }
            const ImVec4 primary = weapon_finish_detail::color_from_u32(media_widget::get_accent_color(), configured_color.w);
            const ImVec4 secondary = weapon_finish_detail::color_from_u32(media_widget::get_secondary_color(), configured_color.w);
            return weapon_finish_detail::blend_color(primary, secondary, 0.55f);
        }
        case weapon_finish_detail::MaterialKind::Weapon:
        default:
        {
            const ImVec4 configured_color = finish_color->get_value();
            if (!finish_spotify_mode->get_value())
            {
                return configured_color;
            }
            return weapon_finish_detail::color_from_u32(media_widget::get_accent_color(), configured_color.w);
        }
        }
    }

    void style_material(
        Il2CppObjectInstance& material_instance,
        weapon_finish_detail::MaterialKind material_kind)
    {
        const ImVec4 selected_color = resolve_finish_color(material_kind);
        const float strength = finish_strength->get_value();
        const unity::color base_color = weapon_finish_detail::to_unity_color(selected_color, strength);
        const unity::color glow_color{
            selected_color.x * (0.8f + strength),
            selected_color.y * (0.8f + strength),
            selected_color.z * (0.8f + strength),
            selected_color.w
        };

        Il2CppString* color_property = il2cpp::il2cpp_string_new("_Color");
        Il2CppString* base_color_property = il2cpp::il2cpp_string_new("_BaseColor");
        Il2CppString* emission_property = il2cpp::il2cpp_string_new("_EmissionColor");
        Il2CppString* emission_keyword = il2cpp::il2cpp_string_new("_EMISSION");
        Il2CppString* metallic_property = il2cpp::il2cpp_string_new("_Metallic");
        Il2CppString* gloss_property = il2cpp::il2cpp_string_new("_Glossiness");
        Il2CppString* smoothness_property = il2cpp::il2cpp_string_new("_Smoothness");

        const auto set_color = material_instance.get_method<void(*)(Il2CppObject*, Il2CppString*, unity::color)>(
            "SetColor",
            2
        );
        const auto set_float = material_instance.get_method<void(*)(Il2CppObject*, Il2CppString*, float)>(
            "SetFloat",
            2
        );
        const auto enable_keyword = material_instance.get_method<void(*)(Il2CppObject*, Il2CppString*)>(
            "EnableKeyword",
            1
        );
        const auto set_shader = material_instance.get_method<void(*)(Il2CppObject*, Il2CppObject*)>(
            "set_shader",
            1
        );
        const auto set_main_texture = material_instance.get_method<void(*)(Il2CppObject*, Il2CppObject*)>(
            "set_mainTexture",
            1
        );

        Il2CppObject* visual_shader = weapon_finish_detail::finish_shader(finish_style->get_value());
        if (visual_shader && set_shader)
        {
            set_shader(material_instance.get_instance(), visual_shader);

            Il2CppString* visible_color_property = il2cpp::il2cpp_string_new("_Color_Visible");
            Il2CppString* occluded_color_property = il2cpp::il2cpp_string_new("_Color_Occluded");
            set_color(material_instance.get_instance(), visible_color_property, base_color);
            set_color(material_instance.get_instance(), occluded_color_property, base_color);

            if (set_main_texture)
            {
                set_main_texture(material_instance.get_instance(), nullptr);
            }
        }

        set_color(material_instance.get_instance(), color_property, base_color);
        set_color(material_instance.get_instance(), base_color_property, base_color);

        switch (finish_style->get_value())
        {
        case 1:
            enable_keyword(material_instance.get_instance(), emission_keyword);
            set_color(material_instance.get_instance(), emission_property, glow_color);
            set_float(material_instance.get_instance(), gloss_property, 0.45f + (strength * 0.35f));
            set_float(material_instance.get_instance(), smoothness_property, 0.45f + (strength * 0.35f));
            break;
        case 2:
            set_float(material_instance.get_instance(), metallic_property, 0.35f + (strength * 0.65f));
            set_float(material_instance.get_instance(), gloss_property, 0.72f + (strength * 0.25f));
            set_float(material_instance.get_instance(), smoothness_property, 0.72f + (strength * 0.25f));
            break;
        case 3:
            set_color(
                material_instance.get_instance(),
                color_property,
                weapon_finish_detail::to_unity_color(selected_color, 0.38f + (strength * 0.42f)));
            set_color(
                material_instance.get_instance(),
                base_color_property,
                weapon_finish_detail::to_unity_color(selected_color, 0.38f + (strength * 0.42f)));
            set_float(material_instance.get_instance(), gloss_property, 0.85f);
            set_float(material_instance.get_instance(), smoothness_property, 0.85f);
            break;
        default:
            set_float(material_instance.get_instance(), gloss_property, 0.35f + (strength * 0.30f));
            set_float(material_instance.get_instance(), smoothness_property, 0.35f + (strength * 0.30f));
            break;
        }
    }
};
