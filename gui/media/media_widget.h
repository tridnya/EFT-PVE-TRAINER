#pragma once

#include <array>
#include <memory>
#include <string>

#include <imgui.h>

struct ID3D11Device;

namespace media_widget
{
    struct MediaDisplayInfo
    {
        std::array<char, 96> title{};
        std::array<char, 96> artist{};
        std::array<char, 16> status{};
        std::array<char, 16> elapsed{};
        std::array<char, 16> remaining{};
        std::array<char, 192> cover_key{};
        std::shared_ptr<const std::string> cover_bytes{};
        float progress = 0.0f;
        bool playing = false;
        bool has_timeline = false;
    };

    struct MediaSnapshot
    {
        MediaDisplayInfo media{};
        bool has_media = false;
        bool initialized = false;
    };

    struct MediaPalette
    {
        ImU32 panel_top = IM_COL32(13, 13, 14, 228);
        ImU32 panel_top_right = IM_COL32(17, 17, 18, 228);
        ImU32 panel_bottom = IM_COL32(10, 10, 11, 228);
        ImU32 panel_bottom_left = IM_COL32(19, 19, 20, 228);
        ImU32 border = IM_COL32(72, 72, 76, 150);
        ImU32 accent = IM_COL32(150, 166, 255, 245);
        ImU32 accent_secondary = IM_COL32(244, 114, 182, 245);
        ImU32 accent_dim = IM_COL32(58, 58, 62, 165);
        ImU32 cover_border = IM_COL32(178, 178, 184, 145);
        ImU32 secondary_text = IM_COL32(205, 205, 210, 255);
        ImU32 timer_text = IM_COL32(184, 184, 190, 255);
        ImU32 control = IM_COL32(232, 232, 235, 235);
    };

    bool start();
    void stop() noexcept;
    void release() noexcept;
    void set_enabled(bool enabled) noexcept;
    [[nodiscard]] bool is_enabled() noexcept;
    void render(ID3D11Device* device);
    [[nodiscard]] std::shared_ptr<const MediaSnapshot> get_snapshot() noexcept;
    [[nodiscard]] ImU32 get_accent_color() noexcept;
    [[nodiscard]] ImU32 get_secondary_color() noexcept;
    [[nodiscard]] ImU32 get_accent_color_faded() noexcept;
}
