#include "menu.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

#include "resources.h"
#include "../../module/ModuleManager.h"
#include "../../util/globals.h"
#include "../../util/utils.h"

namespace menu
{
    std::array<char, 5> icon_utf8(const ImWchar glyph)
    {
        std::array<char, 5> text{};
        const std::uint32_t codepoint = static_cast<std::uint32_t>(glyph);

        if (codepoint <= 0x7F)
        {
            text[0] = static_cast<char>(codepoint);
        }
        else if (codepoint <= 0x7FF)
        {
            text[0] = static_cast<char>(0xC0 | (codepoint >> 6));
            text[1] = static_cast<char>(0x80 | (codepoint & 0x3F));
        }
        else
        {
            text[0] = static_cast<char>(0xE0 | (codepoint >> 12));
            text[1] = static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
            text[2] = static_cast<char>(0x80 | (codepoint & 0x3F));
        }

        return text;
    }

    namespace menu_state
    {
        bool draw_imgui = false;
        float widthh;
        float heightt;
        float scale_factor;
        float font_size;
        float large_font_size;
        float hud_font_size;
        ImFont* poppins_regular;
        ImFont* poppins_regular_hud;
        ImFont* sfui_light;
        ImFont* sfui_bold;
        ImFont* icons;

        ModuleCategory selected_tab = Trainer;
        ModuleCategory animated_tab = Trainer;
        ImVec2 menu_position{};
        ImVec2 menu_size{ 960.0f, 600.0f };
        bool position_ready = false;
        bool dragging = false;
        float page_alpha = 1.0f;
        float page_slide = 0.0f;
        double page_animation_start = 0.0;

        constexpr float navigation_rail_width = 88.0f;
        constexpr float navigation_item_width = 40.0f;
    }

    namespace icon_detail
    {
        enum class IconKind : uint8_t
        {
            Trainer,
            Inventory,
            Progression,
            Weapon,
            Visuals,
            World,
            Config
        };

        [[nodiscard]] float ease_out_cubic(float amount) noexcept
        {
            const float inverse = 1.0f - std::clamp(amount, 0.0f, 1.0f);
            return 1.0f - (inverse * inverse * inverse);
        }

        [[nodiscard]] float lerp_float(float start, float end, float amount) noexcept
        {
            return start + ((end - start) * amount);
        }

        [[nodiscard]] ImU32 lerp_color(ImU32 start_color, ImU32 end_color, float amount) noexcept
        {
            const ImVec4 start = ImGui::ColorConvertU32ToFloat4(start_color);
            const ImVec4 end = ImGui::ColorConvertU32ToFloat4(end_color);
            return ImGui::ColorConvertFloat4ToU32({
                lerp_float(start.x, end.x, amount),
                lerp_float(start.y, end.y, amount),
                lerp_float(start.z, end.z, amount),
                lerp_float(start.w, end.w, amount)
            });
        }

        [[nodiscard]] ImU32 scale_alpha(ImU32 color, float alpha) noexcept
        {
            const int source_alpha = static_cast<int>((color >> IM_COL32_A_SHIFT) & 0xFF);
            const int scaled_alpha = std::clamp(
                static_cast<int>(source_alpha * std::clamp(alpha, 0.0f, 1.0f) + 0.5f),
                0,
                255);
            return (color & ~IM_COL32_A_MASK) | (static_cast<ImU32>(scaled_alpha) << IM_COL32_A_SHIFT);
        }

        [[nodiscard]] ImU32 with_alpha_u32(ImU32 color, float alpha) noexcept
        {
            ImVec4 value = ImGui::ColorConvertU32ToFloat4(color);
            value.w = std::clamp(alpha, 0.0f, 1.0f);
            return ImGui::ColorConvertFloat4ToU32(value);
        }

        [[nodiscard]] ImU32 text_dim(float alpha) noexcept
        {
            return scale_alpha(IM_COL32(139, 141, 145, 255), alpha);
        }

        [[nodiscard]] ImU32 text_color(float alpha) noexcept
        {
            return scale_alpha(IM_COL32(232, 232, 230, 255), alpha);
        }

        [[nodiscard]] ImU32 panel_color(float alpha) noexcept
        {
            return scale_alpha(IM_COL32(13, 14, 15, 248), alpha);
        }

        [[nodiscard]] ImU32 rail_color(float alpha) noexcept
        {
            return scale_alpha(IM_COL32(10, 11, 12, 250), alpha);
        }

        [[nodiscard]] ImU32 soft_border(float alpha) noexcept
        {
            return scale_alpha(IM_COL32(62, 64, 68, 118), alpha);
        }

        [[nodiscard]] ImU32 interface_accent(ImU32 accent) noexcept
        {
            return lerp_color(IM_COL32(198, 198, 190, 245), accent, 0.10f);
        }

        [[nodiscard]] bool point_in_rect(const ImVec2& point, const ImVec2& min, const ImVec2& max) noexcept
        {
            return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y;
        }

        void clamp_menu_pos(ImVec2& pos, const ImVec2& size, const ImVec2& display_size) noexcept
        {
            pos.x = std::clamp(pos.x, 8.0f, (std::max)(8.0f, display_size.x - size.x - 8.0f));
            pos.y = std::clamp(pos.y, 8.0f, (std::max)(8.0f, display_size.y - size.y - 8.0f));
        }

        [[nodiscard]] ImWchar icon_glyph(const IconKind kind) noexcept
        {
            switch (kind)
            {
            case IconKind::Trainer:
                return 0xE90F;
            case IconKind::Inventory:
                return 0xE719;
            case IconKind::Progression:
                return 0xE716;
            case IconKind::Weapon:
                return 0xF272;
            case IconKind::Visuals:
                return 0xE7B3;
            case IconKind::World:
                return 0xE774;
            case IconKind::Config:
                return 0xE713;
            }
            return 0xE10C;
        }

        void draw_icon(
            ImDrawList* draw_list,
            const IconKind kind,
            const ImVec2 center,
            const ImU32 color,
            const float alpha,
            const float scale) noexcept
        {
            ImFont* icon_font = menu_state::icons != nullptr ?
                menu_state::icons : ImGui::GetFont();
            const std::array<char, 5> icon_text = icon_utf8(icon_glyph(kind));
            const float font_size = 18.0f * menu_state::scale_factor * scale;
            const ImVec2 text_size = icon_font->CalcTextSizeA(
                font_size,
                (std::numeric_limits<float>::max)(),
                0.0f,
                icon_text.data());
            draw_list->AddText(
                icon_font,
                font_size,
                { center.x - text_size.x * 0.5f,
                  center.y - text_size.y * 0.5f },
                scale_alpha(color, alpha),
                icon_text.data());
        }

        [[nodiscard]] bool nav_item(
            const char* id,
            const char* label,
            IconKind kind,
            bool selected,
            const ImVec2& pos,
            ImU32 accent,
            float alpha) noexcept
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            const ImGuiIO& io = ImGui::GetIO();
            const ImVec2 size{ 40.0f, 40.0f };

            ImGui::SetCursorScreenPos(pos);
            ImGui::PushID(id);
            const bool pressed = ImGui::InvisibleButton("##nav_item", size);
            const bool hovered = ImGui::IsItemHovered();
            if (hovered)
            {
                ImGui::SetTooltip("%s", label);
            }
            const ImGuiID active_anim_id = ImGui::GetID("active_anim");
            const ImGuiID hover_anim_id = ImGui::GetID("hover_anim");
            ImGui::PopID();

            float* active_anim = ImGui::GetStateStorage()->GetFloatRef(active_anim_id, selected ? 1.0f : 0.0f);
            float* hover_anim = ImGui::GetStateStorage()->GetFloatRef(hover_anim_id, hovered ? 1.0f : 0.0f);
            const float speed = std::clamp(io.DeltaTime * 16.0f, 0.0f, 1.0f);
            *active_anim = lerp_float(*active_anim, selected ? 1.0f : 0.0f, speed);
            *hover_anim = lerp_float(*hover_anim, hovered ? 1.0f : 0.0f, speed);

            const float active_eased = ease_out_cubic(*active_anim);
            const float hover_eased = ease_out_cubic(*hover_anim);
            const ImVec2 max{ pos.x + size.x, pos.y + size.y };
            const ImU32 base_bg = scale_alpha(IM_COL32(255, 255, 255, 16), alpha * hover_eased);
            const ImU32 active_bg = scale_alpha(IM_COL32(255, 255, 255, 18), alpha * active_eased);
            const ImU32 icon_color = lerp_color(text_dim(alpha), scale_alpha(accent, alpha), active_eased);

            draw_list->AddRectFilled(pos, max, base_bg, 6.0f);
            draw_list->AddRectFilled(pos, max, active_bg, 6.0f);
            draw_list->AddRect(pos, max,
                scale_alpha(IM_COL32(255, 255, 255, 20),
                    alpha * active_eased), 6.0f);
            draw_icon(
                draw_list,
                kind,
                { pos.x + size.x * 0.5f, pos.y + size.y * 0.5f },
                icon_color,
                alpha * 0.92f,
                1.0f);

            return pressed;
        }

        void draw_section_header(
            ImDrawList* draw_list,
            const ImVec2& pos,
            const char* title,
            ImU32 accent,
            float alpha) noexcept
        {
            draw_list->AddText(pos, text_color(alpha), title);
            draw_list->AddLine(
                { pos.x, pos.y + 45.0f },
                { pos.x + 54.0f, pos.y + 45.0f },
                scale_alpha(accent, alpha * 0.56f),
                1.0f);
        }

        void draw_module_panel(
            ImDrawList* draw_list,
            const ImVec2& pos,
            float width,
            const char* title,
            ImU32 accent,
            float alpha) noexcept
        {
            draw_list->AddText(pos, text_color(alpha), title);
            const float line_y = pos.y + 28.0f;
            draw_list->AddLine({ pos.x, line_y }, { pos.x + width, line_y }, soft_border(alpha), 1.0f);
            draw_list->AddLine(
                { pos.x, line_y },
                { pos.x + 28.0f, line_y },
                scale_alpha(accent, alpha * 0.62f),
                1.0f);
        }
    }

    float get_hud_font_size()
    {
        return menu_state::hud_font_size;
    }

    ImFont* get_hud_font()
    {
        return menu_state::poppins_regular_hud;
    }

    ImFont* get_icon_font()
    {
        return menu_state::icons != nullptr ?
            menu_state::icons : ImGui::GetFont();
    }

    float get_scale_factor()
    {
        return menu_state::scale_factor;
    }

    float get_width()
    {
        return menu_state::widthh;
    }

    float get_height()
    {
        return menu_state::heightt;
    }

    bool is_active()
    {
        return menu_state::draw_imgui == true;
    }

    void setup()
    {
        ImGuiIO& io = ImGui::GetIO();
        int horizontal = 0;
        int vertical = 0;
        utils::get_desktop_resolution(horizontal, vertical);
        menu_state::widthh = static_cast<float>(horizontal);
        menu_state::heightt = static_cast<float>(vertical);
        const float raw_scale = (static_cast<float>(horizontal) / 1920.0f + static_cast<float>(vertical) / 1080.0f) / 2.0f;
        menu_state::scale_factor = std::clamp(raw_scale, 0.78f, 1.0f);
        menu_state::font_size = 16 * menu_state::scale_factor;
        menu_state::large_font_size = 18 * menu_state::scale_factor;
        menu_state::hud_font_size = 256 * menu_state::scale_factor;
        io.Fonts->Clear();

        static ImVector<ImWchar> glyph_ranges{};
        glyph_ranges.clear();
        ImFontGlyphRangesBuilder glyph_builder{};
        glyph_builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
        glyph_builder.AddRanges(io.Fonts->GetGlyphRangesGreek());
        glyph_builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
        glyph_builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
        glyph_builder.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
        glyph_builder.AddRanges(io.Fonts->GetGlyphRangesKorean());
        glyph_builder.AddRanges(io.Fonts->GetGlyphRangesThai());
        glyph_builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
        glyph_builder.BuildRanges(&glyph_ranges);

        ImFontConfig base_config{};
        base_config.OversampleH = 2;
        base_config.OversampleV = 1;
        base_config.PixelSnapH = true;

        ImFont* regular_font = io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\segoeui.ttf",
            menu_state::font_size,
            &base_config,
            glyph_ranges.Data);
        if (!regular_font)
        {
            regular_font = io.Fonts->AddFontFromFileTTF(
                "C:\\Windows\\Fonts\\verdana.ttf",
                menu_state::font_size,
                &base_config,
                glyph_ranges.Data);
        }
        if (!regular_font)
        {
            regular_font = io.Fonts->AddFontDefault();
        }

        ImFontConfig merge_config = base_config;
        merge_config.MergeMode = true;
        merge_config.GlyphMinAdvanceX = 0.0f;
        (void)io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\seguisym.ttf",
            menu_state::font_size,
            &merge_config,
            glyph_ranges.Data);
        (void)io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\msyh.ttc",
            menu_state::font_size,
            &merge_config,
            glyph_ranges.Data);
        (void)io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\msgothic.ttc",
            menu_state::font_size,
            &merge_config,
            glyph_ranges.Data);
        (void)io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\malgun.ttf",
            menu_state::font_size,
            &merge_config,
            glyph_ranges.Data);

        ImFont* media_font = io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\consola.ttf",
            14.0f * menu_state::scale_factor,
            &base_config,
            glyph_ranges.Data);
        if (!media_font)
        {
            media_font = regular_font;
        }
        else if (media_font != regular_font)
        {
            (void)io.Fonts->AddFontFromFileTTF(
                "C:\\Windows\\Fonts\\seguisym.ttf",
                14.0f * menu_state::scale_factor,
                &merge_config,
                glyph_ranges.Data);
            (void)io.Fonts->AddFontFromFileTTF(
                "C:\\Windows\\Fonts\\msyh.ttc",
                14.0f * menu_state::scale_factor,
                &merge_config,
                glyph_ranges.Data);
            (void)io.Fonts->AddFontFromFileTTF(
                "C:\\Windows\\Fonts\\msgothic.ttc",
                14.0f * menu_state::scale_factor,
                &merge_config,
                glyph_ranges.Data);
            (void)io.Fonts->AddFontFromFileTTF(
                "C:\\Windows\\Fonts\\malgun.ttf",
                14.0f * menu_state::scale_factor,
                &merge_config,
                glyph_ranges.Data);
        }

        static constexpr ImWchar mdl2_icon_ranges[] = { 0xE700, 0xF8FF, 0 };
        ImFontConfig icon_config = base_config;
        icon_config.GlyphMinAdvanceX = 18.0f * menu_state::scale_factor;
        ImFont* icon_font = io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\segmdl2.ttf",
            20.0f * menu_state::scale_factor,
            &icon_config,
            mdl2_icon_ranges);
        if (!icon_font)
        {
            icon_font = media_font;
        }

        ImFont* bold_font = io.Fonts->AddFontFromFileTTF(
            "C:\\Windows\\Fonts\\segoeuib.ttf",
            menu_state::large_font_size,
            &base_config,
            glyph_ranges.Data);
        if (!bold_font)
        {
            bold_font = regular_font;
        }

        io.FontDefault = regular_font;
        menu_state::poppins_regular = regular_font;
        menu_state::poppins_regular_hud = regular_font;
        menu_state::sfui_light = regular_font;
        menu_state::sfui_bold = bold_font;
        menu_state::icons = icon_font;
    }

    void draw()
    {
        theme();
        apply_modern_style();
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        io.WantCaptureMouse = is_active();

        static float menu_anim = 0.0f;
        const float dt = io.DeltaTime;
        const float anim_target = menu_state::draw_imgui ? 1.0f : 0.0f;
        menu_anim += (anim_target - menu_anim) * (std::min)(1.0f, dt * 14.0f);
        const float ease = menu_anim * menu_anim * (3.0f - (2.0f * menu_anim));

        if (menu_anim > 0.004f)
        {
            const ImVec2 window_size(
                (std::min)(menu_state::menu_size.x, (std::max)(320.0f, io.DisplaySize.x - 16.0f)),
                (std::min)(menu_state::menu_size.y, (std::max)(320.0f, io.DisplaySize.y - 16.0f))
            );

            if (!menu_state::position_ready)
            {
                menu_state::menu_position = {
                    (io.DisplaySize.x - window_size.x) * 0.5f,
                    (io.DisplaySize.y - window_size.y) * 0.5f
                };
                menu_state::position_ready = true;
            }

            icon_detail::clamp_menu_pos(menu_state::menu_position, window_size, io.DisplaySize);

            ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
            ImGui::SetNextWindowPos(menu_state::menu_position, ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ease);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

            ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove;
            if (!menu_state::draw_imgui)
            {
                window_flags |= ImGuiWindowFlags_NoInputs;
            }

            if (ImGui::Begin("aevum##veil_menu", &menu_state::draw_imgui, window_flags))
            {
                using IconKind = icon_detail::IconKind;

                struct NavDef
                {
                    const char* id;
                    const char* label;
                    ModuleCategory category;
                    IconKind icon;
                };

                auto category_name = [](ModuleCategory category) -> const char*
                {
                    switch (category)
                    {
                    case Trainer: return "trainer";
                    case Inventory: return "inventory";
                    case Progression: return "progress";
                    case Weapons: return "weapons";
                    case Visuals: return "visuals";
                    case World: return "world";
                    case Config: return "config";
                    default: return "";
                    }
                };

                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                const ImVec2 window_min = ImGui::GetWindowPos();
                const ImVec2 window_max{ window_min.x + window_size.x, window_min.y + window_size.y };
                const ImVec2 rail_max{
                    window_min.x + menu_state::navigation_rail_width,
                    window_max.y };
                const ImU32 accent_color = ImGui::ColorConvertFloat4ToU32(menu::palette::accent);

                menu_state::menu_position = window_min;

                draw_list->AddRectFilled(
                    { window_min.x + 7.0f, window_min.y + 9.0f },
                    { window_max.x + 7.0f, window_max.y + 9.0f },
                    icon_detail::scale_alpha(IM_COL32(0, 0, 0, 96), ease),
                    4.0f);
                draw_list->AddRectFilled(window_min, window_max, icon_detail::panel_color(ease), 4.0f);
                draw_list->AddRectFilled(
                    window_min,
                    rail_max,
                    icon_detail::rail_color(ease),
                    4.0f,
                    ImDrawFlags_RoundCornersLeft);
                draw_list->AddRect(window_min, window_max, icon_detail::soft_border(ease), 4.0f, 0, 1.0f);
                draw_list->AddLine(
                    { rail_max.x, window_min.y + 1.0f },
                    { rail_max.x, window_max.y - 1.0f },
                    icon_detail::scale_alpha(IM_COL32(255, 255, 255, 20), ease),
                    1.0f);
                draw_list->AddLine(
                    { window_min.x + 1.0f, window_min.y + 38.0f },
                    { window_max.x - 1.0f, window_min.y + 38.0f },
                    icon_detail::scale_alpha(IM_COL32(255, 255, 255, 18), ease),
                    1.0f);
                draw_list->AddText(
                    { window_min.x + 14.0f, window_min.y + 11.0f },
                    icon_detail::text_color(ease),
                    "veil");
                draw_list->AddText(
                    { window_min.x + 43.0f, window_min.y + 12.0f },
                    icon_detail::text_dim(ease),
                    "v0.1");

                auto module_manager = ModuleManager::get_instance();
                std::array<NavDef, 7> navs{};
                int nav_count = 0;
                if (module_manager->has_module("Player"))
                    navs[nav_count++] = { "trainer", "trainer", Trainer, IconKind::Trainer };
                if (module_manager->has_module("Item"))
                    navs[nav_count++] = { "inventory", "inventory", Inventory, IconKind::Inventory };
                if (module_manager->has_module("Quests"))
                    navs[nav_count++] = { "progress", "progress", Progression, IconKind::Progression };
                if (module_manager->has_module("Weapon"))
                    navs[nav_count++] = { "weapons", "weapons", Weapons, IconKind::Weapon };
                if (module_manager->has_module("Entities") || module_manager->has_module("View"))
                    navs[nav_count++] = { "visuals", "visuals", Visuals, IconKind::Visuals };
                if (module_manager->has_module("World"))
                    navs[nav_count++] = { "world", "world", World, IconKind::World };
                navs[nav_count++] = { "config", "config", Config, IconKind::Config };

                for (int index = 0; index < nav_count; ++index)
                {
                    const NavDef& nav = navs[static_cast<std::size_t>(index)];
                    const ImVec2 nav_pos{
                        window_min.x +
                            (menu_state::navigation_rail_width -
                             menu_state::navigation_item_width) * 0.5f,
                        window_min.y + 54.0f + (48.0f * index)
                    };
                    const bool selected = menu_state::selected_tab == nav.category;
                    if (icon_detail::nav_item(nav.id, nav.label, nav.icon, selected, nav_pos, accent_color, ease))
                    {
                        menu_state::selected_tab = nav.category;
                    }
                }

                if (menu_state::selected_tab != menu_state::animated_tab)
                {
                    menu_state::animated_tab = menu_state::selected_tab;
                    menu_state::page_alpha = 0.0f;
                    menu_state::page_slide = 18.0f;
                    menu_state::page_animation_start = ImGui::GetTime();
                }

                const float page_progress = std::clamp(
                    static_cast<float>((ImGui::GetTime() - menu_state::page_animation_start) / 0.22),
                    0.0f,
                    1.0f);
                menu_state::page_alpha = icon_detail::ease_out_cubic(page_progress);
                menu_state::page_slide = icon_detail::lerp_float(18.0f, 0.0f, menu_state::page_alpha);

                const ImVec2 content_base_pos{
                    rail_max.x + 24.0f,
                    window_min.y + 58.0f
                };
                const ImVec2 content_pos{ content_base_pos.x + menu_state::page_slide, content_base_pos.y };
                const ImVec2 content_size{
                    window_max.x - content_base_pos.x - 24.0f,
                    window_max.y - content_base_pos.y - 20.0f
                };
                const float content_alpha = ease * menu_state::page_alpha;

                icon_detail::draw_section_header(
                    draw_list,
                    content_pos,
                    category_name(menu_state::animated_tab),
                    accent_color,
                    content_alpha);

                const char* top_title = category_name(menu_state::animated_tab);
                const ImVec2 top_title_size = ImGui::CalcTextSize(top_title);
                draw_list->AddText(
                    { window_min.x + (window_size.x - top_title_size.x) * 0.5f,
                      window_min.y + 11.0f },
                    icon_detail::text_dim(ease),
                    top_title);

                std::vector<Module*> visible_modules;
                module_manager->apply([&](Module* module)
                {
                    if (module->category == menu_state::animated_tab)
                    {
                        visible_modules.push_back(module);
                    }
                });

                auto draw_module_panel = [&](Module* module)
                {
                    ImDrawList* card_draw_list = ImGui::GetWindowDrawList();
                    const ImVec2 header_pos = ImGui::GetCursorScreenPos();
                    const float panel_width = ImGui::GetContentRegionAvail().x;

                    icon_detail::draw_module_panel(
                        card_draw_list,
                        header_pos,
                        panel_width,
                        module->name.c_str(),
                        accent_color,
                        content_alpha);

                    ImGui::Dummy(ImVec2(panel_width, 38.0f));

                    // widgets — no indent, let the table cell padding handle spacing
                    module->draw_menu();

                    ImGui::Dummy(ImVec2(0, 18.0f));
                };

                const ImVec2 scroll_pos{ content_pos.x, content_pos.y + 58.0f };
                const ImVec2 scroll_size{
                    content_size.x, content_size.y - 58.0f };
                ImGui::SetCursorScreenPos(scroll_pos);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, menu_state::page_alpha);
                ImGui::BeginChild("##veil_content", scroll_size, false, ImGuiWindowFlags_NoBackground);

                if (visible_modules.empty())
                {
                    ImGui::TextColored(menu::with_alpha(menu::palette::muted, content_alpha), "nothing here yet");
                }
                else
                {
                    const int column_count = 1;
                    ImGui::PushStyleVar(
                        ImGuiStyleVar_CellPadding,
                        ImVec2(18.0f, 4.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                    if (ImGui::BeginTable("##veil_module_grid", column_count,
                        ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoPadOuterX))
                    {
                        for (Module* module : visible_modules)
                        {
                            ImGui::TableNextColumn();
                            ImGui::PushID(module);
                            draw_module_panel(module);
                            ImGui::PopID();
                        }
                        ImGui::EndTable();
                    }
                    ImGui::PopStyleVar(2);
                }

                ImGui::EndChild();
                ImGui::PopStyleVar();

                const bool drag_region = icon_detail::point_in_rect(
                    io.MousePos,
                    window_min,
                    { window_max.x, window_min.y + 64.0f });
                if (drag_region && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    menu_state::dragging = true;
                }
                if (menu_state::dragging)
                {
                    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    {
                        menu_state::menu_position.x += io.MouseDelta.x;
                        menu_state::menu_position.y += io.MouseDelta.y;
                        icon_detail::clamp_menu_pos(menu_state::menu_position, window_size, io.DisplaySize);
                    }
                    else
                    {
                        menu_state::dragging = false;
                    }
                }
            }
            ImGui::End();
            ImGui::PopStyleVar(2);
        }

        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            menu_state::draw_imgui = !menu_state::draw_imgui;
        }

        auto draw_list = ImGui::GetBackgroundDrawList();
        ModuleManager::get_instance()->apply([draw_list](Module* module)
        {
            module->draw_overlay(draw_list);
        });
    }
}
