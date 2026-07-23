#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <string>
#include <unordered_set>
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
#include "../../config/IntSliderValue.h"
#include "../../config/SelectValue.h"
#include "../../gui/media/media_widget.h"
#include "../../gui/menu/imgui_addons.h"
#include "../../il2cpp/Il2CppObjectInstance.h"
#include "../../il2cpp/exports.h"
#include "../../il2cpp/il2utils.h"
#include "../../il2cpp/unity.h"
#include "../../util/logger.h"

namespace entity_visualizer_detail
{
    inline constexpr int cham_shader_count = 6;
    inline constexpr int cham_texture_count = 14;
    inline Il2CppObject* cham_shader_type = nullptr;
    inline std::array<Il2CppObject*, cham_shader_count> cham_shaders{};
    inline std::array<Il2CppObject*, cham_texture_count> cham_pattern_textures{};
    inline std::uint32_t cham_animation_phase = 0;
    inline ULONGLONG cham_animation_update_ms = 0;

    inline void reset_scene_assets()
    {
        cham_shader_type = nullptr;
        cham_shaders.fill(nullptr);
        cham_pattern_textures.fill(nullptr);
        cham_animation_phase = 0;
        cham_animation_update_ms = 0;
    }

    [[nodiscard]] inline int cham_style_shader_index(int style_index)
    {
        switch (style_index)
        {
        case 6:
            return 2;
        case 7:
            return 0;
        case 8:
            return 5;
        case 9:
            return 0;
        case 10:
            return 2;
        case 11:
            return 4;
        case 12:
            return 0;
        case 13:
            return 0;
        case 14:
            return 2;
        case 15:
            return 4;
        default:
            return std::clamp(style_index, 0, cham_shader_count - 1);
        }
    }

    [[nodiscard]] inline int cham_style_default_texture(int style_index)
    {
        switch (style_index)
        {
        case 6:
            return 4;
        case 7:
            return 7;
        case 8:
            return 3;
        case 9:
            return 8;
        case 10:
            return 10;
        case 11:
            return 5;
        case 12:
            return 4;
        case 13:
            return 11;
        case 14:
            return 13;
        case 15:
            return 12;
        default:
            return 0;
        }
    }

    [[nodiscard]] inline bool is_animated_cham_style(int style_index)
    {
        return style_index == 14 || style_index == 15;
    }

    [[nodiscard]] inline const char* cham_style_name(int style_index)
    {
        static constexpr const char* names[] = {
            "Solid", "Wireframe", "Forcefield", "Glass", "Lava", "Outline",
            "Holographic", "Carbon", "Neon Grid", "Digital Camo", "Hex Shield",
            "Molten Noise", "Scanline", "Thermal", "Iridescent", "Animated Noise"
        };
        if (style_index < 0 ||
            static_cast<std::size_t>(style_index) >= std::size(names))
        {
            return "Unknown";
        }
        return names[static_cast<std::size_t>(style_index)];
    }

    [[nodiscard]] inline int resolve_cham_texture_index(int style_index, int texture_index)
    {
        const int selected_texture = std::clamp(texture_index, 0, cham_texture_count - 1);
        return selected_texture != 0 ? selected_texture : cham_style_default_texture(style_index);
    }

    enum class SkeletonBone : std::uint8_t
    {
        Head,
        Neck,
        Chest,
        SpineUpper,
        SpineLower,
        Pelvis,
        LeftShoulder,
        LeftUpperArm,
        LeftForearm,
        LeftHand,
        RightShoulder,
        RightUpperArm,
        RightForearm,
        RightHand,
        LeftThigh,
        LeftCalf,
        LeftFoot,
        RightThigh,
        RightCalf,
        RightFoot,
        Count
    };

    enum class HumanBodyBone : int
    {
        Hips = 0,
        LeftUpperLeg = 1,
        RightUpperLeg = 2,
        LeftLowerLeg = 3,
        RightLowerLeg = 4,
        LeftFoot = 5,
        RightFoot = 6,
        Spine = 7,
        Chest = 8,
        Neck = 9,
        Head = 10,
        LeftShoulder = 11,
        RightShoulder = 12,
        LeftUpperArm = 13,
        RightUpperArm = 14,
        LeftLowerArm = 15,
        RightLowerArm = 16,
        LeftHand = 17,
        RightHand = 18,
        UpperChest = 54
    };

    enum class EftHumanBone : int
    {
        HumanPelvis = 14,
        HumanLThigh1 = 15,
        HumanLThigh2 = 16,
        HumanLCalf = 17,
        HumanLFoot = 18,
        HumanLToe = 19,
        HumanRThigh1 = 20,
        HumanRThigh2 = 21,
        HumanRCalf = 22,
        HumanRFoot = 23,
        HumanRToe = 24,
        HumanSpine1 = 29,
        HumanSpine2 = 36,
        HumanSpine3 = 37,
        HumanRibcage = 66,
        HumanLCollarbone = 89,
        HumanLUpperarm = 90,
        HumanLForearm1 = 91,
        HumanLForearm2 = 92,
        HumanLForearm3 = 93,
        HumanLPalm = 94,
        HumanRCollarbone = 110,
        HumanRUpperarm = 111,
        HumanRForearm1 = 112,
        HumanRForearm2 = 113,
        HumanRForearm3 = 114,
        HumanRPalm = 115,
        HumanNeck = 132,
        HumanHead = 133
    };

    struct SkeletonPoint
    {
        unity::vector3 position{};
        bool valid = false;
    };

    struct SkeletonSegment
    {
        SkeletonBone start;
        SkeletonBone end;
    };

    struct SkeletonBoneMapping
    {
        SkeletonBone skeleton_bone;
        EftHumanBone human_bone;
    };

    using SkeletonPoints = std::array<SkeletonPoint, static_cast<std::size_t>(SkeletonBone::Count)>;

    struct BoneDebugPoint
    {
        std::string label{};
        unity::vector3 position{};
    };

    struct EntityMarker
    {
        std::string name{};
        unity::vector3 position{};
        unity::vector3 head_position{};
        SkeletonPoints skeleton_points{};
        std::vector<BoneDebugPoint> bone_debug_points{};
        bool has_head = false;
        bool is_ai_player = false;
    };

    struct BoxBounds
    {
        ImVec2 min{};
        ImVec2 max{};
    };

    inline constexpr std::array<SkeletonSegment, 19> skeleton_segments{
        SkeletonSegment{ SkeletonBone::Head, SkeletonBone::Neck },
        SkeletonSegment{ SkeletonBone::Neck, SkeletonBone::Chest },
        SkeletonSegment{ SkeletonBone::Chest, SkeletonBone::SpineUpper },
        SkeletonSegment{ SkeletonBone::SpineUpper, SkeletonBone::SpineLower },
        SkeletonSegment{ SkeletonBone::SpineLower, SkeletonBone::Pelvis },
        SkeletonSegment{ SkeletonBone::Chest, SkeletonBone::LeftShoulder },
        SkeletonSegment{ SkeletonBone::LeftShoulder, SkeletonBone::LeftUpperArm },
        SkeletonSegment{ SkeletonBone::LeftUpperArm, SkeletonBone::LeftForearm },
        SkeletonSegment{ SkeletonBone::LeftForearm, SkeletonBone::LeftHand },
        SkeletonSegment{ SkeletonBone::Chest, SkeletonBone::RightShoulder },
        SkeletonSegment{ SkeletonBone::RightShoulder, SkeletonBone::RightUpperArm },
        SkeletonSegment{ SkeletonBone::RightUpperArm, SkeletonBone::RightForearm },
        SkeletonSegment{ SkeletonBone::RightForearm, SkeletonBone::RightHand },
        SkeletonSegment{ SkeletonBone::Pelvis, SkeletonBone::LeftThigh },
        SkeletonSegment{ SkeletonBone::LeftThigh, SkeletonBone::LeftCalf },
        SkeletonSegment{ SkeletonBone::LeftCalf, SkeletonBone::LeftFoot },
        SkeletonSegment{ SkeletonBone::Pelvis, SkeletonBone::RightThigh },
        SkeletonSegment{ SkeletonBone::RightThigh, SkeletonBone::RightCalf },
        SkeletonSegment{ SkeletonBone::RightCalf, SkeletonBone::RightFoot }
    };

    inline constexpr std::array<SkeletonBoneMapping, static_cast<std::size_t>(SkeletonBone::Count)>
        skeleton_bone_mappings{
            SkeletonBoneMapping{ SkeletonBone::Head, EftHumanBone::HumanHead },
            SkeletonBoneMapping{ SkeletonBone::Neck, EftHumanBone::HumanNeck },
            SkeletonBoneMapping{ SkeletonBone::Chest, EftHumanBone::HumanSpine3 },
            SkeletonBoneMapping{ SkeletonBone::SpineUpper, EftHumanBone::HumanSpine2 },
            SkeletonBoneMapping{ SkeletonBone::SpineLower, EftHumanBone::HumanSpine1 },
            SkeletonBoneMapping{ SkeletonBone::Pelvis, EftHumanBone::HumanPelvis },
            SkeletonBoneMapping{ SkeletonBone::LeftShoulder, EftHumanBone::HumanLCollarbone },
            SkeletonBoneMapping{ SkeletonBone::LeftUpperArm, EftHumanBone::HumanLUpperarm },
            SkeletonBoneMapping{ SkeletonBone::LeftForearm, EftHumanBone::HumanLForearm2 },
            SkeletonBoneMapping{ SkeletonBone::LeftHand, EftHumanBone::HumanLPalm },
            SkeletonBoneMapping{ SkeletonBone::RightShoulder, EftHumanBone::HumanRCollarbone },
            SkeletonBoneMapping{ SkeletonBone::RightUpperArm, EftHumanBone::HumanRUpperarm },
            SkeletonBoneMapping{ SkeletonBone::RightForearm, EftHumanBone::HumanRForearm2 },
            SkeletonBoneMapping{ SkeletonBone::RightHand, EftHumanBone::HumanRPalm },
            SkeletonBoneMapping{ SkeletonBone::LeftThigh, EftHumanBone::HumanLThigh1 },
            SkeletonBoneMapping{ SkeletonBone::LeftCalf, EftHumanBone::HumanLCalf },
            SkeletonBoneMapping{ SkeletonBone::LeftFoot, EftHumanBone::HumanLFoot },
            SkeletonBoneMapping{ SkeletonBone::RightThigh, EftHumanBone::HumanRThigh1 },
            SkeletonBoneMapping{ SkeletonBone::RightCalf, EftHumanBone::HumanRCalf },
            SkeletonBoneMapping{ SkeletonBone::RightFoot, EftHumanBone::HumanRFoot }
        };

    inline constexpr const char* human_bone_names[] = {
        "None",
        "IKController",
        "Mesh",
        "Vest_0",
        "Vest_1",
        "backpack",
        "backpack_0",
        "backpack_1",
        "backpack_2",
        "razgruz",
        "razgruz_0",
        "razgruz_1",
        "razgruz_2",
        "Root_Joint",
        "HumanPelvis",
        "HumanLThigh1",
        "HumanLThigh2",
        "HumanLCalf",
        "HumanLFoot",
        "HumanLToe",
        "HumanRThigh1",
        "HumanRThigh2",
        "HumanRCalf",
        "HumanRFoot",
        "HumanRToe",
        "Bear_Feet",
        "USEC_Feet",
        "BEAR_feet_1",
        "weapon_holster_pistol",
        "HumanSpine1",
        "HumanGear1",
        "HumanGear2",
        "HumanGear3",
        "HumanGear4",
        "HumanGear4_1",
        "HumanGear5",
        "HumanSpine2",
        "HumanSpine3",
        "IK_S_LForearm1",
        "IK_S_LForearm2",
        "IK_S_LForearm3",
        "IK_S_LPalm",
        "IK_S_LDigit11",
        "IK_S_LDigit12",
        "IK_S_LDigit13",
        "IK_S_LDigit21",
        "IK_S_LDigit22",
        "IK_S_LDigit23",
        "IK_S_LDigit31",
        "IK_S_LDigit32",
        "IK_S_LDigit33",
        "IK_S_LDigit41",
        "IK_S_LDigit42",
        "IK_S_LDigit43",
        "IK_S_LDigit51",
        "IK_S_LDigit52",
        "IK_S_LDigit53",
        "XYZ",
        "LCollarbone_anim",
        "RCollarbone_anim",
        "RCollarbone_anim_XYZ",
        "Weapon_root_3rd_anim",
        "XYZ_1",
        "Bend_Goal_Left",
        "Bend_Goal_Right",
        "Bend_Goal_Right_XYZ_1",
        "HumanRibcage",
        "IK_LForearm1",
        "IK_LForearm2",
        "IK_LForearm3",
        "IK_LPalm",
        "IK_LDigit11",
        "IK_LDigit12",
        "IK_LDigit13",
        "IK_LDigit21",
        "IK_LDigit22",
        "IK_LDigit23",
        "IK_LDigit31",
        "IK_LDigit32",
        "IK_LDigit33",
        "IK_LDigit41",
        "IK_LDigit42",
        "IK_LDigit43",
        "IK_LDigit51",
        "IK_LDigit52",
        "IK_LDigit53",
        "Camera_animated",
        "CameraContainer",
        "Cam",
        "HumanLCollarbone",
        "HumanLUpperarm",
        "HumanLForearm1",
        "HumanLForearm2",
        "HumanLForearm3",
        "HumanLPalm",
        "HumanLDigit11",
        "HumanLDigit12",
        "HumanLDigit13",
        "HumanLDigit21",
        "HumanLDigit22",
        "HumanLDigit23",
        "HumanLDigit31",
        "HumanLDigit32",
        "HumanLDigit33",
        "HumanLDigit41",
        "HumanLDigit42",
        "HumanLDigit43",
        "HumanLDigit51",
        "HumanLDigit52",
        "HumanLDigit53",
        "HumanRCollarbone",
        "HumanRUpperarm",
        "HumanRForearm1",
        "HumanRForearm2",
        "HumanRForearm3",
        "HumanRPalm",
        "HumanRDigit11",
        "HumanRDigit12",
        "HumanRDigit13",
        "HumanRDigit21",
        "HumanRDigit22",
        "HumanRDigit23",
        "HumanRDigit31",
        "HumanRDigit32",
        "HumanRDigit33",
        "HumanRDigit41",
        "HumanRDigit42",
        "HumanRDigit43",
        "HumanRDigit51",
        "HumanRDigit52",
        "HumanRDigit53",
        "Weapon_root",
        "HumanNeck",
        "HumanHead",
        "HumanBackpack",
        "weapon_holster",
        "weapon_holster1",
        "Camera_animated_3rd"
    };

    [[nodiscard]] constexpr std::size_t skeleton_bone_index(SkeletonBone bone) noexcept
    {
        return static_cast<std::size_t>(bone);
    }

    [[nodiscard]] constexpr int human_bone_index(EftHumanBone bone) noexcept
    {
        return static_cast<int>(bone);
    }

    [[nodiscard]] inline const char* human_bone_name(int bone_index) noexcept
    {
        if (bone_index < 0 ||
            static_cast<std::size_t>(bone_index) >= sizeof(human_bone_names) / sizeof(human_bone_names[0]))
        {
            return "Unknown";
        }

        return human_bone_names[static_cast<std::size_t>(bone_index)];
    }

    [[nodiscard]] inline Il2CppObject* dictionary_transform_by_element_index(
        Il2CppObject* dictionary_object,
        int bone_index)
    {
        if (!dictionary_object)
        {
            return nullptr;
        }

        auto* bones_dictionary = reinterpret_cast<unity::dict<int, Il2CppObject*>*>(dictionary_object);
        if (!bones_dictionary->m_p_entries || bones_dictionary->m_i_count <= 0)
        {
            return nullptr;
        }

        auto* entries = bones_dictionary->get_entry();
        if (!entries)
        {
            return nullptr;
        }

        const std::size_t entry_count = (std::min)(
            static_cast<std::size_t>(bones_dictionary->m_i_count),
            static_cast<std::size_t>(bones_dictionary->m_p_entries->max_length));
        int occupied_index = 0;
        for (std::size_t index = 0; index < entry_count; ++index)
        {
            const auto& entry = entries[index];
            if (entry.m_i_hash_code < 0)
            {
                continue;
            }

            if (occupied_index == bone_index)
            {
                return entry.m_t_value;
            }

            ++occupied_index;
        }

        return nullptr;
    }

    [[nodiscard]] inline std::string normalized_bone_name(std::string value)
    {
        std::string result{};
        result.reserve(value.size());
        for (unsigned char character : value)
        {
            if (std::isalnum(character))
            {
                result.push_back(static_cast<char>(std::tolower(character)));
            }
        }

        return result;
    }

    [[nodiscard]] inline bool matches_bone_alias(const std::string& normalized_name, const char* alias)
    {
        const std::string normalized_alias = normalized_bone_name(alias);
        if (normalized_name == normalized_alias)
        {
            return true;
        }

        return normalized_alias.size() > 5 && normalized_name.find(normalized_alias) != std::string::npos;
    }

    [[nodiscard]] inline bool matches_skeleton_bone(const std::string& normalized_name, SkeletonBone bone)
    {
        auto matches_any = [&](std::initializer_list<const char*> aliases)
        {
            for (const char* alias : aliases)
            {
                if (matches_bone_alias(normalized_name, alias))
                {
                    return true;
                }
            }

            return false;
        };

        switch (bone)
        {
        case SkeletonBone::Head:
            return matches_any({ "Bip01 Head", "HumanHead", "Head" });
        case SkeletonBone::Neck:
            return matches_any({ "Bip01 Neck", "HumanNeck", "Neck" });
        case SkeletonBone::Chest:
            return matches_any({ "Bip01 Spine2", "HumanSpine3", "Ribcage", "Chest" });
        case SkeletonBone::SpineUpper:
            return matches_any({ "Bip01 Spine1", "HumanSpine2", "Spine3", "Spine2" });
        case SkeletonBone::SpineLower:
            return matches_any({ "Bip01 Spine", "HumanSpine1", "Spine1", "Spine" });
        case SkeletonBone::Pelvis:
            return matches_any({ "Bip01 Pelvis", "HumanPelvis", "Pelvis" });
        case SkeletonBone::LeftShoulder:
            return matches_any({ "Bip01 L Clavicle", "LeftClavicle", "LClavicle", "LeftShoulder" });
        case SkeletonBone::LeftUpperArm:
            return matches_any({ "Bip01 L UpperArm", "LeftUpperArm", "LUpperArm", "LeftArm" });
        case SkeletonBone::LeftForearm:
            return matches_any({ "Bip01 L Forearm", "LeftForeArm", "LeftForearm", "LForearm", "LeftLowerArm" });
        case SkeletonBone::LeftHand:
            return matches_any({ "Bip01 L Hand", "LeftHand", "LHand", "LeftPalm" });
        case SkeletonBone::RightShoulder:
            return matches_any({ "Bip01 R Clavicle", "RightClavicle", "RClavicle", "RightShoulder" });
        case SkeletonBone::RightUpperArm:
            return matches_any({ "Bip01 R UpperArm", "RightUpperArm", "RUpperArm", "RightArm" });
        case SkeletonBone::RightForearm:
            return matches_any({ "Bip01 R Forearm", "RightForeArm", "RightForearm", "RForearm", "RightLowerArm" });
        case SkeletonBone::RightHand:
            return matches_any({ "Bip01 R Hand", "RightHand", "RHand", "RightPalm" });
        case SkeletonBone::LeftThigh:
            return matches_any({ "Bip01 L Thigh", "LeftThigh", "LThigh", "LeftUpperLeg", "LeftUpLeg" });
        case SkeletonBone::LeftCalf:
            return matches_any({ "Bip01 L Calf", "LeftCalf", "LCalf", "LeftLowerLeg" });
        case SkeletonBone::LeftFoot:
            return matches_any({ "Bip01 L Foot", "LeftFoot", "LFoot" });
        case SkeletonBone::RightThigh:
            return matches_any({ "Bip01 R Thigh", "RightThigh", "RThigh", "RightUpperLeg", "RightUpLeg" });
        case SkeletonBone::RightCalf:
            return matches_any({ "Bip01 R Calf", "RightCalf", "RCalf", "RightLowerLeg" });
        case SkeletonBone::RightFoot:
            return matches_any({ "Bip01 R Foot", "RightFoot", "RFoot" });
        case SkeletonBone::Count:
        default:
            return false;
        }
    }

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

    [[nodiscard]] inline bool is_visible(const unity::vector3& screen_point)
    {
        return screen_point.z > 0.01f &&
            screen_point.x > -100.0f &&
            screen_point.y > -100.0f &&
            screen_point.x < menu::get_width() + 100.0f &&
            screen_point.y < menu::get_height() + 100.0f;
    }

    [[nodiscard]] inline float distance_sq(const unity::vector3& a, const unity::vector3& b)
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        const float dz = a.z - b.z;
        return dx * dx + dy * dy + dz * dz;
    }

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

    [[nodiscard]] inline bool is_gear_material(const std::string& material_name)
    {
        const std::string name = lowercase(material_name);
        return contains_any(name, {
            "armour", "armor", "helmet", "glasses", "cover", "rig", "belt", "wear", "mask", "balaclava", "vest"
        });
    }

    [[nodiscard]] inline unity::color to_unity_color(const ImVec4& color)
    {
        return unity::color{ color.x, color.y, color.z, std::clamp(color.w, 0.0f, 1.0f) };
    }

    [[nodiscard]] inline ImVec4 color_from_u32(ImU32 color, float alpha = 1.0f)
    {
        ImVec4 value = ImGui::ColorConvertU32ToFloat4(color);
        value.w = alpha;
        return value;
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

    [[nodiscard]] inline const char* cham_shader_asset_name(int style_index)
    {
        switch (cham_style_shader_index(style_index))
        {
        case 0:
            return "chams.shader";
        case 1:
            return "wireframe.shader";
        case 2:
            return "forcefield.shader";
        case 3:
            return "glass.shader";
        case 4:
            return "lava.shader";
        case 5:
            return "outline.shader";
        default:
            return "outline.shader";
        }
    }

    [[nodiscard]] inline Il2CppObject* cham_shader(int style_index)
    {
        if (!weapon_finish_assets::asset_bundle)
        {
            return nullptr;
        }

        if (!cham_shader_type)
        {
            const Il2CppImage* unity_image = il2utils::resolve_image("UnityEngine.CoreModule.dll");
            if (!unity_image)
            {
                return nullptr;
            }

            cham_shader_type = il2utils::get_system_type(unity_image, "UnityEngine", "Shader");
            if (!cham_shader_type)
            {
                return nullptr;
            }
        }

        const int resolved_index = cham_style_shader_index(style_index);
        Il2CppObject*& shader = cham_shaders[static_cast<std::size_t>(resolved_index)];
        if (!shader)
        {
            shader = weapon_finish_assets::load_asset(
                cham_shader_asset_name(resolved_index), cham_shader_type);
        }

        return shader;
    }

    [[nodiscard]] inline unity::color hue_color(float hue, float alpha = 1.0f)
    {
        hue -= std::floor(hue);
        const float scaled = hue * 6.0f;
        const int sector = static_cast<int>(scaled) % 6;
        const float fraction = scaled - std::floor(scaled);
        switch (sector)
        {
        case 0:
            return unity::color(1.0f, fraction, 0.0f, alpha);
        case 1:
            return unity::color(1.0f - fraction, 1.0f, 0.0f, alpha);
        case 2:
            return unity::color(0.0f, 1.0f, fraction, alpha);
        case 3:
            return unity::color(0.0f, 1.0f - fraction, 1.0f, alpha);
        case 4:
            return unity::color(fraction, 0.0f, 1.0f, alpha);
        default:
            return unity::color(1.0f, 0.0f, 1.0f - fraction, alpha);
        }
    }

    [[nodiscard]] inline unity::color cham_pattern_pixel(
        int pattern_index,
        int x,
        int y,
        std::uint32_t animation_phase = 0)
    {
        if (pattern_index == 11)
        {
            const float vertical = static_cast<float>(y) / 15.0f;
            return hue_color(0.78f - (vertical * 0.78f), 1.0f);
        }
        if (pattern_index == 13)
        {
            const float phase =
                static_cast<float>(animation_phase % 160u) / 160.0f;
            const float diagonal =
                (static_cast<float>(x) * 0.04f) +
                (static_cast<float>(y) * 0.025f);
            return hue_color(std::fmod(phase + diagonal, 1.0f), 1.0f);
        }

        float value = 1.0f;
        switch (pattern_index)
        {
        case 1:
            value = ((x / 4) + (y / 4)) % 2 == 0 ? 1.0f : 0.12f;
            break;
        case 2:
            value = ((x + y) % 8) < 3 ? 1.0f : 0.08f;
            break;
        case 3:
            value = (x % 4 == 0 || y % 4 == 0) ? 1.0f : 0.08f;
            break;
        case 4:
            value = y % 4 == 0 ? 1.0f : (y % 4 == 1 ? 0.35f : 0.06f);
            break;
        case 5:
        {
            const std::uint32_t noise =
                (static_cast<std::uint32_t>(x) * 374761393u) ^
                (static_cast<std::uint32_t>(y) * 668265263u);
            value = 0.12f + (static_cast<float>((noise ^ (noise >> 13)) & 0xffu) / 255.0f) * 0.88f;
            break;
        }
        case 6:
        {
            const int center_x = x % 8;
            const int center_y = y % 8;
            const int delta_x = center_x - 4;
            const int delta_y = center_y - 4;
            value = (delta_x * delta_x) + (delta_y * delta_y) <= 4 ? 1.0f : 0.06f;
            break;
        }
        case 7:
        {
            const int diagonal = (x + y) % 8;
            const int opposite = (x - y + 32) % 8;
            value = (diagonal == 0 || opposite == 0) ? 1.0f : 0.08f;
            break;
        }
        case 8:
        {
            const std::uint32_t cell_x = static_cast<std::uint32_t>(x / 4);
            const std::uint32_t cell_y = static_cast<std::uint32_t>(y / 4);
            const std::uint32_t cell = (cell_x * 1103515245u) ^ (cell_y * 12345u);
            value = (cell & 3u) == 0u ? 1.0f : ((cell & 1u) == 0u ? 0.42f : 0.08f);
            break;
        }
        case 9:
            value = ((x + y) % 6 == 0 || (x - y + 32) % 6 == 0) ? 1.0f : 0.07f;
            break;
        case 10:
        {
            const int row = y / 4;
            const int shifted_x = (x + ((row & 1) * 4)) % 8;
            const int local_y = y % 4;
            value = (local_y == 0 || shifted_x == 0 || shifted_x == 7) ? 1.0f : 0.08f;
            break;
        }
        case 12:
        {
            std::uint32_t noise =
                (static_cast<std::uint32_t>(x) * 374761393u) ^
                (static_cast<std::uint32_t>(y) * 668265263u) ^
                (animation_phase * 2246822519u);
            noise ^= noise >> 13;
            noise *= 1274126177u;
            noise ^= noise >> 16;
            const float brightness =
                0.08f + (static_cast<float>(noise & 0xffu) / 255.0f) * 0.92f;
            const unity::color tint = hue_color(
                static_cast<float>((noise >> 8) & 0xffu) / 255.0f);
            return unity::color(
                tint.r * brightness,
                tint.g * brightness,
                tint.b * brightness,
                1.0f);
        }
        default:
            break;
        }

        return unity::color(value, value, value, 1.0f);
    }

    [[nodiscard]] inline Il2CppObject* create_cham_pattern_texture(int pattern_index)
    {
        const Il2CppImage* unity_image = il2utils::resolve_image("UnityEngine.CoreModule.dll");
        const Il2CppClass* texture_class =
            il2utils::resolve_class(unity_image, "UnityEngine", "Texture2D");
        if (!texture_class)
        {
            return nullptr;
        }

        const MethodInfo* constructor = il2utils::resolve_method(texture_class, ".ctor", 2);
        const MethodInfo* set_pixel_method = il2utils::resolve_method(texture_class, "SetPixel", 3);
        const MethodInfo* apply_method = il2utils::resolve_method(texture_class, "Apply", 0);
        if (!constructor || !constructor->methodPointer ||
            !set_pixel_method || !set_pixel_method->methodPointer ||
            !apply_method || !apply_method->methodPointer)
        {
            return nullptr;
        }

        Il2CppObject* texture = il2cpp::il2cpp_object_new(texture_class);
        if (!texture)
        {
            return nullptr;
        }

        constexpr int texture_size = 16;
        const auto construct = static_cast<void(*)(Il2CppObject*, int, int)>(constructor->methodPointer);
        const auto set_pixel = static_cast<void(*)(Il2CppObject*, int, int, unity::color)>(
            set_pixel_method->methodPointer);
        const auto apply = static_cast<void(*)(Il2CppObject*)>(apply_method->methodPointer);

        construct(texture, texture_size, texture_size);
        for (int y = 0; y < texture_size; ++y)
        {
            for (int x = 0; x < texture_size; ++x)
            {
                set_pixel(texture, x, y, cham_pattern_pixel(pattern_index, x, y));
            }
        }
        apply(texture);

        const MethodInfo* set_filter_mode = il2utils::resolve_method(texture_class, "set_filterMode", 1);
        if (set_filter_mode && set_filter_mode->methodPointer)
        {
            static_cast<void(*)(Il2CppObject*, int)>(set_filter_mode->methodPointer)(texture, 0);
        }

        const MethodInfo* set_wrap_mode = il2utils::resolve_method(texture_class, "set_wrapMode", 1);
        if (set_wrap_mode && set_wrap_mode->methodPointer)
        {
            static_cast<void(*)(Il2CppObject*, int)>(set_wrap_mode->methodPointer)(texture, 0);
        }

        return texture;
    }

    inline bool refresh_cham_pattern_texture(
        Il2CppObject* texture,
        int pattern_index,
        std::uint32_t animation_phase)
    {
        if (texture == nullptr || texture->klass == nullptr)
        {
            return false;
        }

        const MethodInfo* set_pixel_method =
            il2utils::resolve_method(texture->klass, "SetPixel", 3);
        const MethodInfo* apply_method =
            il2utils::resolve_method(texture->klass, "Apply", 0);
        if (set_pixel_method == nullptr || set_pixel_method->methodPointer == nullptr ||
            apply_method == nullptr || apply_method->methodPointer == nullptr)
        {
            return false;
        }

        const auto set_pixel =
            static_cast<void(*)(Il2CppObject*, int, int, unity::color)>(
                set_pixel_method->methodPointer);
        const auto apply =
            static_cast<void(*)(Il2CppObject*)>(apply_method->methodPointer);
        constexpr int texture_size = 16;
        for (int y = 0; y < texture_size; ++y)
        {
            for (int x = 0; x < texture_size; ++x)
            {
                set_pixel(
                    texture,
                    x,
                    y,
                    cham_pattern_pixel(pattern_index, x, y, animation_phase));
            }
        }
        apply(texture);
        return true;
    }

    [[nodiscard]] inline Il2CppObject* cham_pattern_texture(int pattern_index)
    {
        const int resolved_index = std::clamp(
            pattern_index, 0, static_cast<int>(cham_pattern_textures.size()) - 1);
        if (resolved_index == 0)
        {
            return nullptr;
        }

        Il2CppObject*& texture = cham_pattern_textures[static_cast<std::size_t>(resolved_index)];
        if (!texture)
        {
            texture = create_cham_pattern_texture(resolved_index);
        }
        return texture;
    }

    inline void draw_marker_text(
        ImDrawList* draw_list,
        const unity::vector3& screen_position,
        const ImVec4& color,
        float font_size,
        const std::string& text)
    {
        ImFont* font = menu::get_hud_font();
        const float scaled_font_size = font_size * menu::get_scale_factor();
        const ImVec2 text_size =
            font->CalcTextSizeA(scaled_font_size, (std::numeric_limits<float>::max)(), 0.0f, text.c_str());
        const ImVec2 text_position{
            screen_position.x - (text_size.x * 0.5f),
            screen_position.y - (text_size.y * 0.5f)
        };
        const float outline_offset = (std::max)(1.0f, menu::get_scale_factor());
        const ImU32 outline_color = IM_COL32(0, 0, 0, 225);
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
            draw_list->AddText(
                font,
                scaled_font_size,
                {
                    text_position.x + (offset.x * outline_offset),
                    text_position.y + (offset.y * outline_offset)
                },
                outline_color,
                text.c_str());
        }

        draw_list->AddText(
            font,
            scaled_font_size,
            text_position,
            ImGui::GetColorU32(color),
            text.c_str());
    }

    [[nodiscard]] inline ImVec2 marker_text_size(float font_size, const std::string& text)
    {
        ImFont* font = menu::get_hud_font();
        const float scaled_font_size = font_size * menu::get_scale_factor();
        return font->CalcTextSizeA(scaled_font_size, (std::numeric_limits<float>::max)(), 0.0f, text.c_str());
    }

    [[nodiscard]] inline float distance_text_scale(float distance, float max_distance)
    {
        const float distance_start = 15.0f;
        const float distance_end = (std::max)(distance_start + 1.0f, max_distance);
        const float amount = std::clamp((distance - distance_start) / (distance_end - distance_start), 0.0f, 1.0f);
        return 1.0f - (amount * 0.42f);
    }

    [[nodiscard]] inline bool player_box_bounds(
        const unity::vector3& head_screen,
        const unity::vector3& base_screen,
        BoxBounds& bounds)
    {
        const float top_y = (std::min)(head_screen.y, base_screen.y);
        const float bottom_y = (std::max)(head_screen.y, base_screen.y);
        const float height = bottom_y - top_y;
        if (height < 8.0f)
        {
            return false;
        }

        const float center_x = (head_screen.x + base_screen.x) * 0.5f;
        const float width = height * 0.42f;
        bounds.min = { center_x - (width * 0.5f), top_y };
        bounds.max = { center_x + (width * 0.5f), bottom_y };
        return true;
    }

    inline void draw_player_box(
        ImDrawList* draw_list,
        const BoxBounds& bounds,
        const ImVec4& color,
        float thickness)
    {
        const float scaled_thickness = (std::max)(0.5f, thickness * menu::get_scale_factor());
        const float width = bounds.max.x - bounds.min.x;
        const float height = bounds.max.y - bounds.min.y;
        const float corner_width = (std::max)(5.0f, width * 0.30f);
        const float corner_height = (std::max)(5.0f, height * 0.22f);
        const ImU32 outline_color = IM_COL32(0, 0, 0, 185);
        const ImU32 draw_color = ImGui::GetColorU32(color);

        auto draw_corner_segments = [&](ImU32 line_color, float line_thickness)
        {
            draw_list->AddLine(bounds.min, { bounds.min.x + corner_width, bounds.min.y }, line_color, line_thickness);
            draw_list->AddLine(bounds.min, { bounds.min.x, bounds.min.y + corner_height }, line_color, line_thickness);

            draw_list->AddLine(
                { bounds.max.x, bounds.min.y },
                { bounds.max.x - corner_width, bounds.min.y },
                line_color,
                line_thickness);
            draw_list->AddLine(
                { bounds.max.x, bounds.min.y },
                { bounds.max.x, bounds.min.y + corner_height },
                line_color,
                line_thickness);

            draw_list->AddLine(
                { bounds.min.x, bounds.max.y },
                { bounds.min.x + corner_width, bounds.max.y },
                line_color,
                line_thickness);
            draw_list->AddLine(
                { bounds.min.x, bounds.max.y },
                { bounds.min.x, bounds.max.y - corner_height },
                line_color,
                line_thickness);

            draw_list->AddLine(bounds.max, { bounds.max.x - corner_width, bounds.max.y }, line_color, line_thickness);
            draw_list->AddLine(bounds.max, { bounds.max.x, bounds.max.y - corner_height }, line_color, line_thickness);
        };

        draw_corner_segments(outline_color, scaled_thickness + 2.0f);
        draw_corner_segments(draw_color, scaled_thickness);
    }

    inline void draw_skeleton(
        ImDrawList* draw_list,
        Il2CppObject* camera,
        const SkeletonPoints& skeleton_points,
        const ImVec4& color,
        float thickness)
    {
        std::array<ImVec2, static_cast<std::size_t>(SkeletonBone::Count)> screen_points{};
        std::array<bool, static_cast<std::size_t>(SkeletonBone::Count)> valid_screen_points{};

        for (std::size_t index = 0; index < skeleton_points.size(); ++index)
        {
            if (!skeleton_points[index].valid)
            {
                continue;
            }

            const unity::vector3 screen_position = world_to_screen(camera, skeleton_points[index].position);
            if (!is_visible(screen_position))
            {
                continue;
            }

            screen_points[index] = { screen_position.x, screen_position.y };
            valid_screen_points[index] = true;
        }

        const ImU32 draw_color = ImGui::GetColorU32(color);
        const float scaled_thickness = (std::max)(0.5f, thickness * menu::get_scale_factor());
        for (const SkeletonSegment& segment : skeleton_segments)
        {
            const std::size_t start_index = skeleton_bone_index(segment.start);
            const std::size_t end_index = skeleton_bone_index(segment.end);
            if (!valid_screen_points[start_index] || !valid_screen_points[end_index])
            {
                continue;
            }

            draw_list->AddLine(screen_points[start_index], screen_points[end_index], draw_color, scaled_thickness);
        }
    }

    inline void draw_bone_debug_points(
        ImDrawList* draw_list,
        Il2CppObject* camera,
        const std::vector<BoneDebugPoint>& bone_debug_points,
        const ImVec4& color,
        float radius,
        float label_font_size)
    {
        ImFont* font = menu::get_hud_font();
        const float scaled_radius = (std::max)(1.0f, radius * menu::get_scale_factor());
        const float scaled_font_size = label_font_size * menu::get_scale_factor();
        const ImU32 draw_color = ImGui::GetColorU32(color);

        for (const BoneDebugPoint& point : bone_debug_points)
        {
            const unity::vector3 screen_position = world_to_screen(camera, point.position);
            if (!is_visible(screen_position))
            {
                continue;
            }

            const ImVec2 dot_position{ screen_position.x, screen_position.y };
            const ImVec2 label_position{
                dot_position.x + scaled_radius + (3.0f * menu::get_scale_factor()),
                dot_position.y - (scaled_font_size * 0.5f)
            };

            draw_list->AddCircleFilled(dot_position, scaled_radius + 1.0f, IM_COL32(0, 0, 0, 210), 12);
            draw_list->AddCircleFilled(dot_position, scaled_radius, draw_color, 12);
            draw_list->AddText(
                font,
                scaled_font_size,
                { label_position.x + 1.0f, label_position.y + 1.0f },
                IM_COL32(0, 0, 0, 220),
                point.label.c_str());
            draw_list->AddText(font, scaled_font_size, label_position, draw_color, point.label.c_str());
        }
    }
}

class EntityVisualizerModule : Module
{
public:
    EntityVisualizerModule() : Module("Entities", Visuals)
    {
    }

    CheckboxValue* show_ai_players = conf(new CheckboxValue(false, "AI Players"));
    ColorpickerValue* ai_player_color = conf(new ColorpickerValue({ 1.0f, 0.34f, 0.30f, 1.0f }, "AI Player Text Color"));
    CheckboxValue* show_npcs = conf(new CheckboxValue(false, "NPCs"));
    ColorpickerValue* npc_color = conf(new ColorpickerValue({ 1.0f, 0.86f, 0.30f, 1.0f }, "NPC Text Color"));
    FloatSliderValue* entity_distance = conf(new FloatSliderValue(250.0f, 1.0f, 600.0f, "Entity Distance"));
    IntSliderValue* font_size = conf(new IntSliderValue(13, 8, 30, "Entity Text Size"));

    CheckboxValue* ai_player_boxes = conf(new CheckboxValue(false, "AI Corner Boxes"));
    ColorpickerValue* ai_player_box_color =
        conf(new ColorpickerValue({ 1.0f, 0.34f, 0.30f, 1.0f }, "AI Corner Color"));
    CheckboxValue* npc_boxes = conf(new CheckboxValue(false, "NPC Corner Boxes"));
    ColorpickerValue* npc_box_color =
        conf(new ColorpickerValue({ 1.0f, 0.86f, 0.30f, 1.0f }, "NPC Corner Color"));
    FloatSliderValue* box_thickness = conf(new FloatSliderValue(1.0f, 0.5f, 4.0f, "Corner Thickness"));

    CheckboxValue* ai_player_skeletons = conf(new CheckboxValue(false, "AI Player Skeletons"));
    ColorpickerValue* ai_player_skeleton_color =
        conf(new ColorpickerValue({ 1.0f, 0.34f, 0.30f, 1.0f }, "AI Player Skeleton Color"));
    CheckboxValue* npc_skeletons = conf(new CheckboxValue(false, "NPC Skeletons"));
    ColorpickerValue* npc_skeleton_color =
        conf(new ColorpickerValue({ 1.0f, 0.86f, 0.30f, 1.0f }, "NPC Skeleton Color"));
    FloatSliderValue* skeleton_thickness = conf(new FloatSliderValue(1.0f, 0.5f, 4.0f, "Skeleton Thickness"));
    CheckboxValue* show_bone_debug_points = conf(new CheckboxValue(false, "Bone Debug Points"));
    ColorpickerValue* bone_debug_color =
        conf(new ColorpickerValue({ 0.40f, 1.0f, 0.70f, 1.0f }, "Bone Debug Color"));
    FloatSliderValue* bone_debug_size = conf(new FloatSliderValue(2.0f, 1.0f, 5.0f, "Bone Debug Size"));

    ActionRowValue* dump_player_bones = conf(new ActionRowValue([&]
    {
        dump_player_bones_impl();
    }, "Dump Player Bones"));

    CheckboxValue* ai_player_chams = conf(new CheckboxValue(false, "AI Player Chams"));
    CheckboxValue* ai_player_cham_spotify =
        conf(new CheckboxValue(false, "AI Player Visible Spotify"));
    ColorpickerValue* ai_player_cham_visible_color =
        conf(new ColorpickerValue({ 1.0f, 0.34f, 0.30f, 0.90f }, "AI Player Visible Color"));
    ColorpickerValue* ai_player_cham_occluded_color =
        conf(new ColorpickerValue({ 0.22f, 0.42f, 1.0f, 0.72f }, "AI Player Occluded Color"));
    CheckboxValue* npc_chams = conf(new CheckboxValue(false, "NPC Chams"));
    CheckboxValue* npc_cham_spotify =
        conf(new CheckboxValue(false, "NPC Visible Spotify"));
    ColorpickerValue* npc_cham_visible_color =
        conf(new ColorpickerValue({ 1.0f, 0.86f, 0.30f, 0.90f }, "NPC Visible Color"));
    ColorpickerValue* npc_cham_occluded_color =
        conf(new ColorpickerValue({ 0.22f, 0.42f, 1.0f, 0.72f }, "NPC Occluded Color"));
    CheckboxValue* include_gear = conf(new CheckboxValue(true, "Cham Gear"));
    SelectValue* cham_style = conf(new SelectValue(5, {
                                                      "Solid", "Wireframe", "Forcefield", "Glass", "Lava", "Outline",
                                                      "Holographic", "Carbon", "Neon Grid", "Digital Camo",
                                                      "Hex Shield", "Molten Noise", "Scanline", "Thermal",
                                                      "Iridescent", "Animated Noise"
                                                  }, "Cham Style"));
    SelectValue* cham_texture = conf(new SelectValue(0, {
                                                        "None", "Checker", "Diagonal Stripes", "Grid", "Scanlines",
                                                        "Noise", "Dots", "Diamonds", "Digital Camo", "Crosshatch",
                                                        "Hex", "Thermal Gradient", "Animated Noise",
                                                        "Iridescent Cycle"
                                                    }, "Cham Texture"));

    unity::vector3 local_position{};
    std::vector<entity_visualizer_detail::EntityMarker> entity_markers{};
    std::unordered_set<std::uintptr_t> applied_chams{};
    bool queue_dump_player_bones = false;
    bool cham_diagnostics_dumped = false;

    void dump_player_bones_impl()
    {
        if (!game_state::is_in_raid)
        {
            logger::warn("PlayerBones dump requested outside raid");
            return;
        }

        queue_dump_player_bones = true;
    }

    void draw_overlay(ImDrawList* draw_list) override
    {
        if (!game_state::is_in_raid)
        {
            return;
        }

        const bool ai_player_text_enabled = show_ai_players->get_value();
        const bool npc_text_enabled = show_npcs->get_value();
        const bool ai_player_boxes_enabled = ai_player_boxes->get_value();
        const bool npc_boxes_enabled = npc_boxes->get_value();
        const bool ai_player_skeletons_enabled = ai_player_skeletons->get_value();
        const bool npc_skeletons_enabled = npc_skeletons->get_value();
        const bool bone_debug_points_enabled = show_bone_debug_points->get_value();
        if (!ai_player_text_enabled &&
            !npc_text_enabled &&
            !ai_player_boxes_enabled &&
            !npc_boxes_enabled &&
            !ai_player_skeletons_enabled &&
            !npc_skeletons_enabled &&
            !bone_debug_points_enabled)
        {
            return;
        }

        Il2CppObject* camera = unity::get_current_camera();
        if (!camera)
        {
            return;
        }

        const float max_distance = entity_distance->get_value();
        const float max_distance_sq = max_distance * max_distance;

        for (const entity_visualizer_detail::EntityMarker& marker : entity_markers)
        {
            const bool should_draw_text = marker.is_ai_player ? ai_player_text_enabled : npc_text_enabled;
            const bool should_draw_box = marker.is_ai_player ? ai_player_boxes_enabled : npc_boxes_enabled;
            const bool should_draw_skeleton =
                marker.is_ai_player ? ai_player_skeletons_enabled : npc_skeletons_enabled;
            const bool should_draw_bone_debug = bone_debug_points_enabled;
            if (!should_draw_text && !should_draw_box && !should_draw_skeleton && !should_draw_bone_debug)
            {
                continue;
            }

            const float marker_distance_sq = entity_visualizer_detail::distance_sq(local_position, marker.position);
            if (marker_distance_sq > max_distance_sq)
            {
                continue;
            }

            const unity::vector3 base_screen_position =
                entity_visualizer_detail::world_to_screen(camera, marker.position);
            if (!entity_visualizer_detail::is_visible(base_screen_position))
            {
                continue;
            }

            const int distance = static_cast<int>(std::sqrt(marker_distance_sq));
            entity_visualizer_detail::BoxBounds box_bounds{};
            bool has_box_bounds = false;
            if (should_draw_box && marker.has_head)
            {
                unity::vector3 head_position = marker.head_position;
                head_position.y += 0.18f;
                const unity::vector3 head_screen_position =
                    entity_visualizer_detail::world_to_screen(camera, head_position);
                if (entity_visualizer_detail::is_visible(head_screen_position))
                {
                    has_box_bounds =
                        entity_visualizer_detail::player_box_bounds(
                            head_screen_position,
                            base_screen_position,
                            box_bounds);
                    if (has_box_bounds)
                    {
                        const ImVec4 box_color = marker.is_ai_player
                            ? ai_player_box_color->get_value()
                            : npc_box_color->get_value();
                        entity_visualizer_detail::draw_player_box(
                            draw_list,
                            box_bounds,
                            box_color,
                            box_thickness->get_value());
                    }
                }
            }

            if (should_draw_skeleton)
            {
                const ImVec4 skeleton_color = marker.is_ai_player
                    ? ai_player_skeleton_color->get_value()
                    : npc_skeleton_color->get_value();
                entity_visualizer_detail::draw_skeleton(
                    draw_list,
                    camera,
                    marker.skeleton_points,
                    skeleton_color,
                    skeleton_thickness->get_value());
            }

            if (should_draw_bone_debug)
            {
                const float debug_font_size =
                    10.0f * entity_visualizer_detail::distance_text_scale(static_cast<float>(distance), max_distance);
                entity_visualizer_detail::draw_bone_debug_points(
                    draw_list,
                    camera,
                    marker.bone_debug_points,
                    bone_debug_color->get_value(),
                    bone_debug_size->get_value(),
                    debug_font_size);
            }

            if (should_draw_text)
            {
                const char* prefix = marker.is_ai_player ? "ai" : "npc";
                const ImVec4 color = marker.is_ai_player ? ai_player_color->get_value() : npc_color->get_value();
                std::string text = std::string(prefix) + " [" + std::to_string(distance) + "m] " + marker.name;
                const float scaled_font_size =
                    static_cast<float>(font_size->get_value()) *
                    entity_visualizer_detail::distance_text_scale(static_cast<float>(distance), max_distance);
                unity::vector3 text_screen_position = base_screen_position;
                if (has_box_bounds)
                {
                    const ImVec2 text_size = entity_visualizer_detail::marker_text_size(scaled_font_size, text);
                    text_screen_position.x = (box_bounds.min.x + box_bounds.max.x) * 0.5f;
                    text_screen_position.y =
                        box_bounds.min.y - (text_size.y * 0.5f) - (3.0f * menu::get_scale_factor());
                }

                entity_visualizer_detail::draw_marker_text(
                    draw_list,
                    text_screen_position,
                    color,
                    scaled_font_size,
                    text);
            }
        }
    }

    void application_update() override
    {
    }

    void reset_gameworld_state() override
    {
        entity_markers.clear();
        applied_chams.clear();
        entity_visualizer_detail::reset_scene_assets();
    }

    void gameworld_update(const Il2CppClass* game_world_class, Il2CppObjectInstance game_world_instance,
                          Il2CppObjectInstance main_player) override
    {
        const bool text_enabled = show_ai_players->get_value() || show_npcs->get_value();
        const bool boxes_enabled = ai_player_boxes->get_value() || npc_boxes->get_value();
        const bool skeletons_enabled = ai_player_skeletons->get_value() || npc_skeletons->get_value();
        const bool chams_enabled = ai_player_chams->get_value() || npc_chams->get_value();
        const bool bone_debug_enabled = show_bone_debug_points->get_value();
        if (!text_enabled &&
            !boxes_enabled &&
            !skeletons_enabled &&
            !chams_enabled &&
            !bone_debug_enabled &&
            !queue_dump_player_bones)
        {
            entity_markers.clear();
            applied_chams.clear();
            return;
        }

        Il2CppObject* main_player_game_object = unity::component_get_game_object(main_player.get_instance());
        Il2CppObject* main_player_transform =
            main_player_game_object ? unity::gameobject_get_transform(main_player_game_object) : nullptr;
        if (!main_player_transform)
        {
            return;
        }

        local_position = unity::transform_get_pos(main_player_transform);
        game_state::current_zoom = get_scope_zoom(main_player);

        if (queue_dump_player_bones)
        {
            if (!dump_player_bones_for_player(main_player))
            {
                logger::warn("PlayerBones dump failed: no PlayerBones object found");
            }
            queue_dump_player_bones = false;
        }

        if (!text_enabled && !boxes_enabled && !skeletons_enabled && !chams_enabled && !bone_debug_enabled)
        {
            entity_markers.clear();
            return;
        }

        const auto get_profile = main_player.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_Profile", 0);
        Il2CppObject* main_profile_object = get_profile(main_player.get_instance());
        if (!main_profile_object)
        {
            return;
        }

        Il2CppObjectInstance main_profile(main_profile_object);
        const std::string main_nickname = profile_nickname(main_profile);

        unity::list* alive_players_list = game_world_instance.get_field<unity::list*>("AllAlivePlayersList");
        if (!alive_players_list || !alive_players_list->m_p_list_array)
        {
            entity_markers.clear();
            return;
        }

        if (!chams_enabled)
        {
            applied_chams.clear();
        }
        else if (menu::is_active() || has_spotify_chams())
        {
            applied_chams.clear();
        }

        const int selected_cham_style = cham_style->get_value();
        if (chams_enabled &&
            entity_visualizer_detail::is_animated_cham_style(selected_cham_style))
        {
            const ULONGLONG now = GetTickCount64();
            if (now - entity_visualizer_detail::cham_animation_update_ms >= 100)
            {
                entity_visualizer_detail::cham_animation_update_ms = now;
                ++entity_visualizer_detail::cham_animation_phase;
                const int animated_texture_index =
                    selected_cham_style == 14 ? 13 : 12;
                Il2CppObject* texture =
                    entity_visualizer_detail::cham_pattern_texture(
                        animated_texture_index);
                entity_visualizer_detail::refresh_cham_pattern_texture(
                    texture,
                    animated_texture_index,
                    entity_visualizer_detail::cham_animation_phase);
            }
        }

        Il2CppArray* alive_players = alive_players_list->m_p_list_array;
        std::vector<entity_visualizer_detail::EntityMarker> next_markers{};
        next_markers.reserve(alive_players->max_length);

        for (std::size_t index = 0; index < alive_players->max_length; ++index)
        {
            Il2CppObject* player_object = reinterpret_cast<Il2CppObject**>(&alive_players->data)[index];
            if (!player_object || player_object == main_player.get_instance())
            {
                continue;
            }

            Il2CppObjectInstance player_instance(player_object);
            Il2CppObject* profile_object = get_profile(player_instance.get_instance());
            if (!profile_object)
            {
                continue;
            }

            Il2CppObjectInstance profile_instance(profile_object);
            const std::string nickname = profile_nickname(profile_instance);
            if (!main_nickname.empty() && nickname == main_nickname)
            {
                continue;
            }

            Il2CppObject* player_game_object = unity::component_get_game_object(player_instance.get_instance());
            Il2CppObject* player_transform =
                player_game_object ? unity::gameobject_get_transform(player_game_object) : nullptr;
            if (!player_transform)
            {
                continue;
            }

            const bool is_ai_player = has_dogtag(player_instance);
            const unity::vector3 position = unity::transform_get_pos(player_transform);
            const bool should_box_entity = is_ai_player ? ai_player_boxes->get_value() : npc_boxes->get_value();
            const bool should_skeleton_entity =
                is_ai_player ? ai_player_skeletons->get_value() : npc_skeletons->get_value();
            const bool should_debug_bones = bone_debug_enabled;
            unity::vector3 head_position{};
            entity_visualizer_detail::SkeletonPoints skeleton_points{};
            std::vector<entity_visualizer_detail::BoneDebugPoint> bone_debug_points{};
            bool has_head = false;

            if (should_box_entity || should_skeleton_entity || should_debug_bones)
            {
                Il2CppObject* player_body_object = get_player_body(player_instance);
                Il2CppObject* skeleton_root_joint = get_skeleton_root_joint(player_body_object);
                if (skeleton_root_joint)
                {
                    if (should_skeleton_entity)
                    {
                        fill_skeleton_points(skeleton_root_joint, skeleton_points);
                    }

                    if (should_debug_bones)
                    {
                        fill_bone_debug_points(skeleton_root_joint, bone_debug_points);
                    }

                    const std::size_t head_index =
                        entity_visualizer_detail::skeleton_bone_index(entity_visualizer_detail::SkeletonBone::Head);
                    if (skeleton_points[head_index].valid)
                    {
                        head_position = skeleton_points[head_index].position;
                        has_head = true;
                    }
                    else if (read_skeleton_bone_position(
                        skeleton_root_joint,
                        entity_visualizer_detail::EftHumanBone::HumanHead,
                        head_position))
                    {
                        has_head = true;
                    }
                }

                if (!has_head && should_box_entity)
                {
                    Il2CppObject* player_bones_object = get_player_bones_from_body(player_body_object);
                    if (read_bone_position(player_bones_object, "Head", head_position))
                    {
                        has_head = true;
                    }
                }
            }

            if (text_enabled || boxes_enabled || skeletons_enabled || bone_debug_enabled)
            {
                next_markers.push_back({
                    nickname.empty() ? (is_ai_player ? "AI Player" : "NPC") : nickname,
                    position,
                    head_position,
                    skeleton_points,
                    bone_debug_points,
                    has_head,
                    is_ai_player
                });
            }

            if (chams_enabled && should_cham_entity(is_ai_player))
            {
                const std::uintptr_t entity_key = reinterpret_cast<std::uintptr_t>(player_object);
                if (!applied_chams.contains(entity_key))
                {
                    apply_player_chams(game_world_class, player_instance, is_ai_player);
                    applied_chams.insert(entity_key);
                }
            }
        }

        entity_markers.swap(next_markers);
    }

    void init() override
    {
    }

private:
    [[nodiscard]] Il2CppObject* get_player_body(Il2CppObjectInstance& player_instance) const
    {
        Il2CppObject* player_body_object =
            entity_visualizer_detail::object_field(player_instance.get_instance(), "_playerBody");
        if (!player_body_object)
        {
            player_body_object =
                entity_visualizer_detail::object_method0(player_instance.get_instance(), "get_PlayerBody");
        }

        return player_body_object;
    }

    [[nodiscard]] Il2CppObject* get_player_bones_from_body(Il2CppObject* player_body_object) const
    {
        if (!player_body_object)
        {
            return nullptr;
        }

        return entity_visualizer_detail::object_field(player_body_object, "PlayerBones");
    }

    [[nodiscard]] Il2CppObject* get_player_bones(Il2CppObjectInstance& player_instance) const
    {
        return get_player_bones_from_body(get_player_body(player_instance));
    }

    [[nodiscard]] Il2CppObject* get_skeleton_root_joint(Il2CppObject* player_body_object) const
    {
        if (!player_body_object)
        {
            return nullptr;
        }

        Il2CppObject* skeleton_root_joint =
            entity_visualizer_detail::object_field(player_body_object, "SkeletonRootJoint");
        if (!skeleton_root_joint)
        {
            skeleton_root_joint =
                entity_visualizer_detail::object_method0(player_body_object, "get_SkeletonRootJoint");
        }

        return skeleton_root_joint;
    }

    [[nodiscard]] Il2CppObject* get_skeleton_bones_dictionary(Il2CppObject* skeleton_root_joint) const
    {
        if (!skeleton_root_joint)
        {
            return nullptr;
        }

        Il2CppObject* bones_dictionary =
            entity_visualizer_detail::object_field(skeleton_root_joint, "Bones");
        if (!bones_dictionary)
        {
            bones_dictionary =
                entity_visualizer_detail::object_method0(skeleton_root_joint, "get_Bones");
        }

        return bones_dictionary;
    }

    [[nodiscard]] Il2CppObject* get_skeleton_bone_transform(
        Il2CppObject* skeleton_root_joint,
        entity_visualizer_detail::EftHumanBone human_bone) const
    {
        Il2CppObject* bones_dictionary = get_skeleton_bones_dictionary(skeleton_root_joint);
        return entity_visualizer_detail::dictionary_transform_by_element_index(
            bones_dictionary,
            entity_visualizer_detail::human_bone_index(human_bone));
    }

    [[nodiscard]] Il2CppObject* get_bone_transform(Il2CppObject* player_bones_object, const char* bone_name) const
    {
        return entity_visualizer_detail::object_field(player_bones_object, bone_name);
    }

    [[nodiscard]] bool read_bone_position(
        Il2CppObject* player_bones_object,
        const char* bone_name,
        unity::vector3& position) const
    {
        Il2CppObject* bone_transform = get_bone_transform(player_bones_object, bone_name);
        return bone_transform && entity_visualizer_detail::object_position(bone_transform, position);
    }

    [[nodiscard]] bool read_skeleton_bone_position(
        Il2CppObject* skeleton_root_joint,
        entity_visualizer_detail::EftHumanBone human_bone,
        unity::vector3& position) const
    {
        Il2CppObject* bone_transform = get_skeleton_bone_transform(skeleton_root_joint, human_bone);
        return bone_transform && entity_visualizer_detail::object_position(bone_transform, position);
    }

    [[nodiscard]] Il2CppObject* get_animator_from_player_bones(Il2CppObject* player_bones_object) const
    {
        return entity_visualizer_detail::object_field(player_bones_object, "PlayableAnimator");
    }

    [[nodiscard]] Il2CppObject* get_human_bone_transform(
        Il2CppObject* animator,
        entity_visualizer_detail::HumanBodyBone human_bone) const
    {
        if (!animator || !animator->klass)
        {
            return nullptr;
        }

        const MethodInfo* method = il2utils::resolve_method(animator->klass, "GetBoneTransform", 1);
        if (!method || !method->methodPointer)
        {
            return nullptr;
        }

        const auto get_bone_transform =
            static_cast<Il2CppObject*(*)(Il2CppObject*, int)>(method->methodPointer);
        return get_bone_transform(animator, static_cast<int>(human_bone));
    }

    void fill_skeleton_point_from_animator(
        entity_visualizer_detail::SkeletonPoints& skeleton_points,
        Il2CppObject* animator,
        entity_visualizer_detail::SkeletonBone bone,
        std::initializer_list<entity_visualizer_detail::HumanBodyBone> human_bones) const
    {
        const std::size_t index = entity_visualizer_detail::skeleton_bone_index(bone);
        if (skeleton_points[index].valid)
        {
            return;
        }

        for (entity_visualizer_detail::HumanBodyBone human_bone : human_bones)
        {
            Il2CppObject* bone_transform = get_human_bone_transform(animator, human_bone);
            unity::vector3 bone_position{};
            if (bone_transform && entity_visualizer_detail::object_position(bone_transform, bone_position))
            {
                set_skeleton_point(skeleton_points, bone, bone_position);
                return;
            }
        }
    }

    void fill_skeleton_points_from_animator(
        Il2CppObject* player_bones_object,
        entity_visualizer_detail::SkeletonPoints& skeleton_points) const
    {
        Il2CppObject* animator = get_animator_from_player_bones(player_bones_object);
        if (!animator)
        {
            return;
        }

        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::Head, { entity_visualizer_detail::HumanBodyBone::Head });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::Neck, { entity_visualizer_detail::HumanBodyBone::Neck });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::Chest, { entity_visualizer_detail::HumanBodyBone::UpperChest, entity_visualizer_detail::HumanBodyBone::Chest });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::SpineUpper, { entity_visualizer_detail::HumanBodyBone::Spine });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::SpineLower, { entity_visualizer_detail::HumanBodyBone::Spine });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::Pelvis, { entity_visualizer_detail::HumanBodyBone::Hips });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::LeftShoulder, { entity_visualizer_detail::HumanBodyBone::LeftShoulder });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::LeftUpperArm, { entity_visualizer_detail::HumanBodyBone::LeftUpperArm });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::LeftForearm, { entity_visualizer_detail::HumanBodyBone::LeftLowerArm });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::LeftHand, { entity_visualizer_detail::HumanBodyBone::LeftHand });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::RightShoulder, { entity_visualizer_detail::HumanBodyBone::RightShoulder });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::RightUpperArm, { entity_visualizer_detail::HumanBodyBone::RightUpperArm });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::RightForearm, { entity_visualizer_detail::HumanBodyBone::RightLowerArm });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::RightHand, { entity_visualizer_detail::HumanBodyBone::RightHand });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::LeftThigh, { entity_visualizer_detail::HumanBodyBone::LeftUpperLeg });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::LeftCalf, { entity_visualizer_detail::HumanBodyBone::LeftLowerLeg });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::LeftFoot, { entity_visualizer_detail::HumanBodyBone::LeftFoot });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::RightThigh, { entity_visualizer_detail::HumanBodyBone::RightUpperLeg });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::RightCalf, { entity_visualizer_detail::HumanBodyBone::RightLowerLeg });
        fill_skeleton_point_from_animator(skeleton_points, animator, entity_visualizer_detail::SkeletonBone::RightFoot, { entity_visualizer_detail::HumanBodyBone::RightFoot });
    }

    void set_skeleton_point(
        entity_visualizer_detail::SkeletonPoints& skeleton_points,
        entity_visualizer_detail::SkeletonBone bone,
        const unity::vector3& position) const
    {
        const std::size_t index = entity_visualizer_detail::skeleton_bone_index(bone);
        if (skeleton_points[index].valid)
        {
            return;
        }

        skeleton_points[index].position = position;
        skeleton_points[index].valid = true;
    }

    void fill_skeleton_points_from_transform(
        Il2CppObject* player_transform,
        entity_visualizer_detail::SkeletonPoints& skeleton_points) const
    {
        if (!player_transform)
        {
            return;
        }

        auto match_transform = [&](Il2CppObject* transform)
        {
            if (!transform)
            {
                return;
            }

            const std::string normalized_name = entity_visualizer_detail::normalized_bone_name(unity::get_name(transform));
            if (normalized_name.empty())
            {
                return;
            }

            for (std::size_t index = 0; index < skeleton_points.size(); ++index)
            {
                if (skeleton_points[index].valid)
                {
                    continue;
                }

                const auto bone = static_cast<entity_visualizer_detail::SkeletonBone>(index);
                if (!entity_visualizer_detail::matches_skeleton_bone(normalized_name, bone))
                {
                    continue;
                }

                unity::vector3 bone_position{};
                if (entity_visualizer_detail::object_position(transform, bone_position))
                {
                    set_skeleton_point(skeleton_points, bone, bone_position);
                }
                break;
            }
        };

        match_transform(player_transform);
        unity::transform_apply_to_all_children(player_transform, match_transform);
    }

    void fill_skeleton_point_from_player_bones(
        entity_visualizer_detail::SkeletonPoints& skeleton_points,
        Il2CppObject* player_bones_object,
        entity_visualizer_detail::SkeletonBone bone,
        const char* field_name) const
    {
        const std::size_t index = entity_visualizer_detail::skeleton_bone_index(bone);
        if (skeleton_points[index].valid)
        {
            return;
        }

        unity::vector3 bone_position{};
        if (read_bone_position(player_bones_object, field_name, bone_position))
        {
            set_skeleton_point(skeleton_points, bone, bone_position);
        }
    }

    void fill_skeleton_points_from_player_bones(
        Il2CppObject* player_bones_object,
        entity_visualizer_detail::SkeletonPoints& skeleton_points) const
    {
        if (!player_bones_object)
        {
            return;
        }

        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::Head, "Head");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::Neck, "Neck");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::Chest, "Ribcage");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::SpineUpper, "Spine3");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::SpineLower, "Spine1");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::Pelvis, "Pelvis");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::LeftShoulder, "LeftShoulder");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::LeftUpperArm, "LeftShoulder");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::LeftForearm, "LeftPalm");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::LeftHand, "LeftPalm");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::RightShoulder, "RightShoulder");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::RightUpperArm, "RightShoulder");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::RightForearm, "RightPalm");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::RightHand, "RightPalm");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::LeftThigh, "LeftThigh1");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::LeftCalf, "LeftThigh2");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::RightThigh, "RightThigh1");
        fill_skeleton_point_from_player_bones(skeleton_points, player_bones_object, entity_visualizer_detail::SkeletonBone::RightCalf, "RightThigh2");
    }

    void fill_skeleton_point_from_skeleton(
        entity_visualizer_detail::SkeletonPoints& skeleton_points,
        Il2CppObject* skeleton_root_joint,
        entity_visualizer_detail::SkeletonBone skeleton_bone,
        entity_visualizer_detail::EftHumanBone human_bone) const
    {
        const std::size_t index = entity_visualizer_detail::skeleton_bone_index(skeleton_bone);
        if (skeleton_points[index].valid)
        {
            return;
        }

        unity::vector3 bone_position{};
        if (read_skeleton_bone_position(skeleton_root_joint, human_bone, bone_position))
        {
            set_skeleton_point(skeleton_points, skeleton_bone, bone_position);
        }
    }

    void fill_skeleton_points_from_skeleton(
        Il2CppObject* skeleton_root_joint,
        entity_visualizer_detail::SkeletonPoints& skeleton_points) const
    {
        if (!skeleton_root_joint)
        {
            return;
        }

        for (const entity_visualizer_detail::SkeletonBoneMapping& mapping :
             entity_visualizer_detail::skeleton_bone_mappings)
        {
            fill_skeleton_point_from_skeleton(
                skeleton_points,
                skeleton_root_joint,
                mapping.skeleton_bone,
                mapping.human_bone);
        }
    }

    void fill_skeleton_points(
        Il2CppObject* skeleton_root_joint,
        entity_visualizer_detail::SkeletonPoints& skeleton_points) const
    {
        fill_skeleton_points_from_skeleton(skeleton_root_joint, skeleton_points);
    }

    void fill_bone_debug_points(
        Il2CppObject* skeleton_root_joint,
        std::vector<entity_visualizer_detail::BoneDebugPoint>& bone_debug_points) const
    {
        Il2CppObject* bones_dictionary_object = get_skeleton_bones_dictionary(skeleton_root_joint);
        if (!bones_dictionary_object)
        {
            return;
        }

        auto* bones_dictionary = reinterpret_cast<unity::dict<int, Il2CppObject*>*>(bones_dictionary_object);
        if (!bones_dictionary->m_p_entries || bones_dictionary->m_i_count <= 0)
        {
            return;
        }

        auto* entries = bones_dictionary->get_entry();
        if (!entries)
        {
            return;
        }

        const std::size_t entry_count = (std::min)(
            static_cast<std::size_t>(bones_dictionary->m_i_count),
            static_cast<std::size_t>(bones_dictionary->m_p_entries->max_length));
        bone_debug_points.reserve(entry_count);

        int occupied_index = 0;
        for (std::size_t index = 0; index < entry_count; ++index)
        {
            const auto& entry = entries[index];
            if (entry.m_i_hash_code < 0)
            {
                continue;
            }

            const int bone_index = occupied_index;
            ++occupied_index;

            if (!entry.m_t_value)
            {
                continue;
            }

            unity::vector3 bone_position{};
            if (!entity_visualizer_detail::object_position(entry.m_t_value, bone_position))
            {
                continue;
            }

            bone_debug_points.push_back({
                std::to_string(bone_index) + " " + entity_visualizer_detail::human_bone_name(bone_index),
                bone_position
            });
        }
    }

    bool dump_player_bones_for_player(Il2CppObjectInstance& player_instance) const
    {
        Il2CppObject* player_bones_object = get_player_bones(player_instance);
        if (!player_bones_object || !player_bones_object->klass)
        {
            return false;
        }

        Il2CppClass* bones_class = player_bones_object->klass;
        (void)il2utils::resolve_field(bones_class, "Head");

        logger::info(
            std::string("[PlayerBones] class=") +
            (bones_class->name ? bones_class->name : "?") +
            " fields=" + std::to_string(bones_class->field_count));

        if (!bones_class->fields || bones_class->field_count == 0)
        {
            logger::warn("[PlayerBones] field array is empty");
            return true;
        }

        for (std::uint16_t field_index = 0; field_index < bones_class->field_count; ++field_index)
        {
            const FieldInfo& field = bones_class->fields[field_index];
            logger::info(
                "[PlayerBones] " +
                std::to_string(field_index) +
                ": " +
                (field.name ? field.name : "?") +
                " offset=" +
                std::to_string(field.offset));
        }

        return true;
    }

    [[nodiscard]] std::string profile_nickname(Il2CppObjectInstance& profile_instance) const
    {
        const auto get_nickname = profile_instance.get_method<Il2CppString*(*)(Il2CppObject*)>("get_Nickname", 0);
        Il2CppString* nickname = get_nickname(profile_instance.get_instance());
        if (!nickname)
        {
            return {};
        }

        return entity_visualizer_detail::wide_to_utf8(il2utils::conv_wstring(nickname));
    }

    [[nodiscard]] bool has_dogtag(Il2CppObjectInstance& player_instance) const
    {
        Il2CppObject* inventory_controller_object =
            player_instance.get_field<Il2CppObject*>("_inventoryController");
        if (!inventory_controller_object)
        {
            return false;
        }

        Il2CppObjectInstance inventory_controller(inventory_controller_object);
        Il2CppObject* inventory_object =
            inventory_controller.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_Inventory", 0)(
                inventory_controller.get_instance());
        if (!inventory_object)
        {
            return false;
        }

        Il2CppObjectInstance inventory(inventory_object);
        Il2CppObject* equipment_object = inventory.get_field<Il2CppObject*>("Equipment");
        if (!equipment_object)
        {
            return false;
        }

        Il2CppObjectInstance equipment(equipment_object);
        Il2CppObject* equipment_slot_object =
            equipment.get_method<Il2CppObject*(*)(Il2CppObject*, int)>("GetSlot", 1)(
                equipment.get_instance(),
                13);
        if (!equipment_slot_object)
        {
            return false;
        }

        Il2CppObjectInstance equipment_slot(equipment_slot_object);
        Il2CppObject* contained_item =
            equipment_slot.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_ContainedItem", 0)(
                equipment_slot.get_instance());
        return contained_item != nullptr;
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

    [[nodiscard]] bool should_cham_entity(bool is_ai_player) const
    {
        return is_ai_player ? ai_player_chams->get_value() : npc_chams->get_value();
    }

    [[nodiscard]] bool has_spotify_chams() const
    {
        return (ai_player_chams->get_value() && ai_player_cham_spotify->get_value()) ||
            (npc_chams->get_value() && npc_cham_spotify->get_value());
    }

    [[nodiscard]] ImVec4 resolve_visible_cham_color(bool is_ai_player) const
    {
        const ImVec4 configured_color = is_ai_player
            ? ai_player_cham_visible_color->get_value()
            : npc_cham_visible_color->get_value();
        const bool use_spotify = is_ai_player
            ? ai_player_cham_spotify->get_value()
            : npc_cham_spotify->get_value();
        if (!use_spotify)
        {
            if (cham_style->get_value() == 13)
            {
                return { 1.0f, 0.85f, 0.15f, configured_color.w };
            }
            if (cham_style->get_value() == 14)
            {
                return { 1.0f, 1.0f, 1.0f, configured_color.w };
            }
            return configured_color;
        }

        const ImU32 media_color = is_ai_player
            ? media_widget::get_accent_color()
            : media_widget::get_secondary_color();
        return entity_visualizer_detail::color_from_u32(media_color, configured_color.w);
    }

    [[nodiscard]] ImVec4 resolve_occluded_cham_color(bool is_ai_player) const
    {
        const ImVec4 configured_color = is_ai_player
            ? ai_player_cham_occluded_color->get_value()
            : npc_cham_occluded_color->get_value();
        if (cham_style->get_value() == 13)
        {
            return { 0.52f, 0.02f, 0.82f, configured_color.w };
        }
        if (cham_style->get_value() == 14)
        {
            return { 1.0f, 1.0f, 1.0f, configured_color.w };
        }
        return is_ai_player
            ? ai_player_cham_occluded_color->get_value()
            : npc_cham_occluded_color->get_value();
    }

    void apply_player_chams(
        const Il2CppClass* game_world_class,
        Il2CppObjectInstance& player_instance,
        bool is_ai_player)
    {
        if (!game_world_class || !game_world_class->image)
        {
            return;
        }

        Il2CppObject* player_body_object =
            player_instance.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_PlayerBody", 0)(
                player_instance.get_instance());
        if (!player_body_object)
        {
            return;
        }

        Il2CppObjectInstance player_body(game_world_class->image, "EFT", "PlayerBody", player_body_object);
        auto* body_skins = player_body.get_field<unity::dict<int, Il2CppObject*>*>("BodySkins");
        if (!body_skins || !body_skins->m_p_entries)
        {
            return;
        }

        for (int skin_index = 0; skin_index < body_skins->m_i_count; ++skin_index)
        {
            Il2CppObject* lodded_skin_object = body_skins->get_value_by_index(skin_index);
            if (!lodded_skin_object)
            {
                continue;
            }

            Il2CppObjectInstance lodded_skin(lodded_skin_object);
            Il2CppArray* lods_array = lodded_skin.get_field<Il2CppArray*>("_lods");
            if (!lods_array)
            {
                continue;
            }

            for (std::size_t lod_index = 0; lod_index < lods_array->max_length; ++lod_index)
            {
                Il2CppObject* skin_object = reinterpret_cast<Il2CppObject**>(&lods_array->data)[lod_index];
                if (!skin_object)
                {
                    continue;
                }

                Il2CppObjectInstance abstract_skin(skin_object);
                Il2CppObject* renderer_object =
                    abstract_skin.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_SkinnedMeshRenderer", 0)(
                        abstract_skin.get_instance());
                if (!renderer_object)
                {
                    continue;
                }

                apply_renderer_chams(renderer_object, is_ai_player);
            }
        }
    }

    void apply_renderer_chams(Il2CppObject* renderer_object, bool is_ai_player)
    {
        Il2CppObjectInstance renderer_instance(renderer_object);
        const auto get_materials = renderer_instance.get_method<Il2CppArray*(*)(Il2CppObject*)>("get_materials", 0);
        Il2CppArray* materials = get_materials ? get_materials(renderer_instance.get_instance()) : nullptr;
        if (materials)
        {
            dump_cham_diagnostics_once(renderer_object, materials);
            for (std::size_t index = 0; index < materials->max_length; ++index)
            {
                Il2CppObject* material_object = reinterpret_cast<Il2CppObject**>(&materials->data)[index];
                apply_material_chams(material_object, is_ai_player);
            }

            return;
        }

        Il2CppObject* material_object =
            renderer_instance.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_material", 0)(
                renderer_instance.get_instance());
        apply_material_chams(material_object, is_ai_player);
    }

    void dump_cham_diagnostics_once(
        Il2CppObject* renderer_object,
        Il2CppArray* materials)
    {
        if (cham_diagnostics_dumped || renderer_object == nullptr ||
            renderer_object->klass == nullptr || materials == nullptr ||
            materials->max_length == 0 || !weapon_finish_assets::asset_bundle)
        {
            return;
        }

        cham_diagnostics_dumped = true;
        logger::info("[ChamProbe] renderer_class='" +
            std::string(renderer_object->klass->namespaze ?
                renderer_object->klass->namespaze : "") +
            "." + std::string(renderer_object->klass->name ?
                renderer_object->klass->name : "<unknown>") +
            "' materials=" + std::to_string(materials->max_length) +
            " selected_style=" + std::to_string(cham_style->get_value()) +
            " style_name='" +
                std::string(entity_visualizer_detail::cham_style_name(
                    cham_style->get_value())) +
            " selected_texture=" + std::to_string(cham_texture->get_value()));

        for (int style_index = 0; style_index < entity_visualizer_detail::cham_shader_count; ++style_index)
        {
            Il2CppObject* shader = entity_visualizer_detail::cham_shader(style_index);
            logger::info("[ChamProbeShader] #" + std::to_string(style_index) +
                " asset='" +
                std::string(entity_visualizer_detail::cham_shader_asset_name(style_index)) +
                "' resolved=" + std::to_string(shader != nullptr ? 1 : 0) +
                " name='" + (shader != nullptr ? unity::get_name(shader) : std::string()) + "'");
        }

        const int resolved_texture_index = entity_visualizer_detail::resolve_cham_texture_index(
            cham_style->get_value(),
            cham_texture->get_value());
        Il2CppObject* pattern_texture =
            entity_visualizer_detail::cham_pattern_texture(resolved_texture_index);
        logger::info("[ChamProbe] pattern_texture=" +
            std::to_string(pattern_texture != nullptr ? 1 : 0) +
            " resolved_texture=" + std::to_string(resolved_texture_index));

        Il2CppObject** material_values =
            reinterpret_cast<Il2CppObject**>(&materials->data);
        const size_t material_count = std::min<size_t>(
            static_cast<size_t>(materials->max_length),
            8);
        for (size_t material_index = 0; material_index < material_count; ++material_index)
        {
            Il2CppObject* material = material_values[material_index];
            if (material == nullptr || material->klass == nullptr)
            {
                continue;
            }

            Il2CppObjectInstance material_instance(material);
            const auto get_shader =
                material_instance.get_method<Il2CppObject*(*)(Il2CppObject*)>(
                    "get_shader", 0);
            const auto set_shader =
                material_instance.get_method<void(*)(Il2CppObject*, Il2CppObject*)>(
                    "set_shader", 1);
            const auto set_color =
                material_instance.get_method<void(*)(Il2CppObject*, Il2CppString*, unity::color)>(
                    "SetColor", 2);
            const auto set_main_texture =
                material_instance.get_method<void(*)(Il2CppObject*, Il2CppObject*)>(
                    "set_mainTexture", 1);
            const auto set_texture =
                material_instance.get_method<void(*)(Il2CppObject*, Il2CppString*, Il2CppObject*)>(
                    "SetTexture", 2);
            Il2CppObject* current_shader = get_shader ? get_shader(material) : nullptr;

            logger::info("[ChamProbeMaterial] #" +
                std::to_string(material_index) +
                " name='" + unity::get_name(material) +
                "' shader='" +
                    (current_shader != nullptr ?
                        unity::get_name(current_shader) :
                        std::string()) +
                "' get_shader=" + std::to_string(get_shader != nullptr ? 1 : 0) +
                " set_shader=" + std::to_string(set_shader != nullptr ? 1 : 0) +
                " set_color=" + std::to_string(set_color != nullptr ? 1 : 0) +
                " set_main_texture=" +
                    std::to_string(set_main_texture != nullptr ? 1 : 0) +
                " set_texture=" + std::to_string(set_texture != nullptr ? 1 : 0));
        }
    }

    void apply_material_chams(Il2CppObject* material_object, bool is_ai_player)
    {
        if (!material_object)
        {
            return;
        }

        Il2CppObjectInstance material_instance(material_object);
        if (!include_gear->get_value() &&
            entity_visualizer_detail::is_gear_material(unity::get_name(material_instance.get_instance())))
        {
            return;
        }

        Il2CppObject* shader = entity_visualizer_detail::cham_shader(cham_style->get_value());
        if (!shader)
        {
            return;
        }

        const ImVec4 visible_color = resolve_visible_cham_color(is_ai_player);
        const ImVec4 invisible_color = resolve_occluded_cham_color(is_ai_player);
        const unity::color visible = entity_visualizer_detail::to_unity_color(visible_color);
        const unity::color invisible = entity_visualizer_detail::to_unity_color(invisible_color);

        const auto set_shader = material_instance.get_method<void(*)(Il2CppObject*, Il2CppObject*)>(
            "set_shader",
            1
        );
        const auto set_color = material_instance.get_method<void(*)(Il2CppObject*, Il2CppString*, unity::color)>(
            "SetColor",
            2
        );
        const auto set_main_texture = material_instance.get_method<void(*)(Il2CppObject*, Il2CppObject*)>(
            "set_mainTexture",
            1
        );
        const auto set_texture = material_instance.get_method<void(*)(Il2CppObject*, Il2CppString*, Il2CppObject*)>(
            "SetTexture",
            2
        );

        Il2CppString* visible_property = il2cpp::il2cpp_string_new("_Color_Visible");
        Il2CppString* occluded_property = il2cpp::il2cpp_string_new("_Color_Occluded");
        Il2CppString* color_property = il2cpp::il2cpp_string_new("_Color");
        Il2CppString* base_color_property = il2cpp::il2cpp_string_new("_BaseColor");
        Il2CppString* main_texture_property = il2cpp::il2cpp_string_new("_MainTex");
        Il2CppString* base_map_property = il2cpp::il2cpp_string_new("_BaseMap");
        Il2CppString* texture_property = il2cpp::il2cpp_string_new("_Texture");
        const int resolved_texture_index = entity_visualizer_detail::resolve_cham_texture_index(
            cham_style->get_value(),
            cham_texture->get_value());
        Il2CppObject* pattern_texture =
            entity_visualizer_detail::cham_pattern_texture(resolved_texture_index);

        set_shader(material_instance.get_instance(), shader);
        set_color(material_instance.get_instance(), visible_property, visible);
        set_color(material_instance.get_instance(), occluded_property, invisible);
        set_color(material_instance.get_instance(), color_property, visible);
        set_color(material_instance.get_instance(), base_color_property, visible);

        if (set_main_texture)
        {
            set_main_texture(material_instance.get_instance(), pattern_texture);
        }

        if (set_texture)
        {
            set_texture(material_instance.get_instance(), main_texture_property, pattern_texture);
            set_texture(material_instance.get_instance(), base_map_property, pattern_texture);
            set_texture(material_instance.get_instance(), texture_property, pattern_texture);
        }
    }
};
