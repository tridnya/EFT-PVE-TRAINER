#pragma once
#include <imgui.h>
#include <imgui_stdlib.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "menu.h"

namespace imgui_addons
{
    // ---------- small helpers ----------

    inline ImU32 lerp_col(const ImVec4& a, const ImVec4& b, float t)
    {
        return ImGui::GetColorU32(ImVec4(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t,
            a.w + (b.w - a.w) * t));
    }

    inline float animate(ImGuiID id, float target, float speed = 16.f)
    {
        static std::unordered_map<ImGuiID, float> state;
        float& v = state[id];
        v += (target - v) * (std::min)(1.f, ImGui::GetIO().DeltaTime * speed);
        return v;
    }

    inline std::string label_text(const std::string& label)
    {
        return label.substr(0, label.find("##"));
    }

    // ---------- world-space text ----------

    static void centered_text(ImDrawList* draw_list, ImFont* font, float font_size, const ImVec2& pos, const ImVec4 col,
                              const std::string& text)
    {
        const ImVec2 size = ImGui::CalcTextSize(text.c_str());
        draw_list->AddText(font, font_size, {pos.x - size.x / 2, pos.y - size.y / 2}, ImGui::GetColorU32(col),
                           text.c_str());
    }

    static void centered_text(ImDrawList* draw_list, const ImVec2& pos, const ImVec4 col, const std::string& text)
    {
        const ImVec2 size = ImGui::CalcTextSize(text.c_str());
        draw_list->AddText({pos.x - size.x / 2, pos.y - size.y / 2}, ImGui::GetColorU32(col), text.c_str());
    }

    static void tab(const std::string& label, const bool selected, const std::function<void()>& action)
    {
        if (selected) ImGui::PushStyleColor(ImGuiCol_Tab, ImGui::GetStyle().Colors[ImGuiCol_TabActive]);
        if (ImGui::TabItemButton(label.c_str())) action();
        if (selected) ImGui::PopStyleColor();
    }

    // ---------- aevum-style toggle switch ----------

    static bool checkbox(const std::string& label, bool* v)
    {
        const float sf    = menu::get_scale_factor();
        const float row_h = 34.0f * sf;
        ImGui::PushID(label.c_str());
        const std::string txt = label_text(label);

        const ImVec2 pos   = ImGui::GetCursorScreenPos();
        const float  avail = ImGui::GetContentRegionAvail().x;

        ImGui::InvisibleButton("row", ImVec2(avail, row_h));
        const bool hovered = ImGui::IsItemHovered();
        const bool clicked = ImGui::IsItemClicked();
        if (clicked) *v = !*v;

        const float anim = animate(ImGui::GetID("t"), *v ? 1.f : 0.f);
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // hover tint
        if (hovered)
            dl->AddRectFilled(pos, { pos.x + avail, pos.y + row_h },
                ImGui::GetColorU32(menu::with_alpha(menu::palette::txt, 0.04f)), 4.0f * sf);

        // label (left)
        const ImVec2 ts = ImGui::CalcTextSize(txt.c_str());
        dl->AddText({ pos.x, pos.y + (row_h - ts.y) * 0.5f },
            ImGui::GetColorU32(menu::palette::txt), txt.c_str());

        // pill track dimensions
        const float tw = 26.0f * sf;
        const float th = 13.0f * sf;
        const float tx = pos.x + avail - tw;
        const float ty = pos.y + (row_h - th) * 0.5f;

        // "on" / "off" label just left of the track
        const char* state_text = *v ? "on" : "off";
        const ImVec2 ss = ImGui::CalcTextSize(state_text);
        dl->AddText({ tx - ss.x - 7.0f * sf, pos.y + (row_h - ss.y) * 0.5f },
            *v ? ImGui::GetColorU32(menu::with_alpha(menu::palette::accent, 0.90f))
               : ImGui::GetColorU32(menu::palette::muted),
            state_text);

        // track (aevum: IM_COL32(56,57,60,210) gray → accent)
        const ImU32 track_off = IM_COL32(56, 57, 60, 210);
        const ImU32 track_on  = ImGui::GetColorU32(menu::with_alpha(menu::palette::accent, 0.85f));
        const ImU32 track_col = lerp_col(
            ImGui::ColorConvertU32ToFloat4(track_off),
            ImGui::ColorConvertU32ToFloat4(track_on),
            anim);
        dl->AddRectFilled({ tx, ty }, { tx + tw, ty + th }, track_col, th * 0.5f);

        // knob (aevum: IM_COL32(238,238,236,255))
        const float kr = th * 0.5f - 1.5f * sf;
        const float kx = tx + th * 0.5f + anim * (tw - th);
        dl->AddCircleFilled({ kx, ty + th * 0.5f }, kr, IM_COL32(238, 238, 236, 255), 20);

        // row separator
        dl->AddLine({ pos.x, pos.y + row_h }, { pos.x + avail, pos.y + row_h },
            ImGui::GetColorU32(menu::with_alpha(menu::palette::line, 0.38f)), 1.0f);

        ImGui::PopID();
        return clicked;
    }

    // ---------- aevum-style slider ----------

    inline bool slider_core(const std::string& label, float* value, float vmin, float vmax, bool is_int,
                            const char* fmt)
    {
        const float sf    = menu::get_scale_factor();
        const float row_h = 48.0f * sf;
        ImGui::PushID(label.c_str());
        const std::string txt = label_text(label);

        const ImVec2 pos   = ImGui::GetCursorScreenPos();
        const float  avail = ImGui::GetContentRegionAvail().x;
        const float knob_radius = 5.5f * sf;
        const float track_inset = knob_radius + 1.0f * sf;
        const float track_width = (std::max)(
            1.0f, avail - track_inset * 2.0f);

        ImGui::InvisibleButton("s", ImVec2(avail, row_h));
        const bool active  = ImGui::IsItemActive();
        const bool hovered = ImGui::IsItemHovered();

        if (active && vmax > vmin)
        {
            const float t = std::clamp(
                (ImGui::GetIO().MousePos.x - pos.x - track_inset) /
                    track_width,
                0.0f,
                1.0f);
            *value = vmin + (vmax - vmin) * t;
            if (is_int) *value = static_cast<float>(lroundf(*value));
        }

        const float t = vmax > vmin
            ? std::clamp((*value - vmin) / (vmax - vmin), 0.0f, 1.0f)
            : 0.0f;
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // hover / active tint
        if (hovered || active)
            dl->AddRectFilled(pos, { pos.x + avail, pos.y + row_h },
                ImGui::GetColorU32(menu::with_alpha(menu::palette::txt, 0.04f)), 4.0f * sf);

        // label (top-left)
        dl->AddText({ pos.x + 2.0f * sf, pos.y + 8.0f * sf },
            ImGui::GetColorU32(menu::palette::txt), txt.c_str());

        // value readout (top-right, muted)
        char vbuf[32];
        if (is_int) snprintf(vbuf, sizeof(vbuf), "%d", static_cast<int>(lroundf(*value)));
        else        snprintf(vbuf, sizeof(vbuf), fmt ? fmt : "%.2f", *value);
        const ImVec2 vs = ImGui::CalcTextSize(vbuf);
        dl->AddText({ pos.x + avail - vs.x - 2.0f * sf, pos.y + 8.0f * sf },
            ImGui::GetColorU32(menu::palette::muted), vbuf);

        // track
        const ImVec2 track_min{
            pos.x + track_inset, pos.y + 34.0f * sf };
        const ImVec2 track_max{
            track_min.x + track_width, pos.y + 38.0f * sf };
        const ImVec2 knob{
            track_min.x + t * track_width,
            (track_min.y + track_max.y) * 0.5f };

        // aevum: IM_COL32(58,59,62,210) for empty rail, accent*0.78 for fill
        dl->AddRectFilled(track_min, track_max, IM_COL32(58, 59, 62, 210), 2.0f * sf);
        dl->AddRectFilled(track_min, { knob.x, track_max.y },
            ImGui::GetColorU32(menu::with_alpha(menu::palette::accent, 0.78f)), 2.0f * sf);
        dl->AddCircleFilled(knob, active ? 5.5f * sf : 4.5f * sf,
            IM_COL32(236, 236, 234, 255), 20);

        // row separator
        dl->AddLine({ pos.x, pos.y + row_h }, { pos.x + avail, pos.y + row_h },
            ImGui::GetColorU32(menu::with_alpha(menu::palette::line, 0.38f)), 1.0f);

        ImGui::PopID();
        return active;
    }

    static bool slider_float(const std::string& label, float* v, float mn, float mx)
    {
        return slider_core(label, v, mn, mx, false, "%.2f");
    }

    static bool slider_int(const std::string& label, int* v, int mn, int mx)
    {
        float f = static_cast<float>(*v);
        const bool r = slider_core(label, &f, static_cast<float>(mn), static_cast<float>(mx), true, "%.0f");
        *v = static_cast<int>(lroundf(f));
        return r;
    }

    // ---------- right-align helper (combo/textbox) ----------

    inline float right_align(float ctrl_w)
    {
        const float sf = menu::get_scale_factor();
        const float target_x = (std::max)(ImGui::GetCursorPosX(),
            ImGui::GetWindowContentRegionMax().x - ctrl_w - 14.f * sf);
        ImGui::SameLine(target_x);
        ImGui::SetNextItemWidth(ctrl_w);
        return ctrl_w;
    }

    // ---------- aevum-style select / dropdown ----------

    static bool select(const std::string& label, int* v, const std::vector<const char*>& options)
    {
        const float sf    = menu::get_scale_factor();
        const float row_h = 34.0f * sf;
        ImGui::PushID(label.c_str());
        const std::string txt = label_text(label);

        const ImVec2 pos   = ImGui::GetCursorScreenPos();
        const float  avail = ImGui::GetContentRegionAvail().x;
        ImDrawList*  dl    = ImGui::GetWindowDrawList();

        // hover tint (passive rect check — doesn't consume click)
        const bool row_hovered = ImGui::IsMouseHoveringRect(pos, { pos.x + avail, pos.y + row_h });
        if (row_hovered)
            dl->AddRectFilled(pos, { pos.x + avail, pos.y + row_h },
                ImGui::GetColorU32(menu::with_alpha(menu::palette::txt, 0.04f)), 4.0f * sf);

        // label (left)
        const ImVec2 ts = ImGui::CalcTextSize(txt.c_str());
        dl->AddText({ pos.x, pos.y + (row_h - ts.y) * 0.5f },
            ImGui::GetColorU32(menu::palette::txt), txt.c_str());

        // styled combo (right-aligned)
        const float combo_w = (std::min)(180.0f * sf, avail * 0.55f);
        const float combo_h = 22.0f * sf;
        ImGui::SetCursorScreenPos({ pos.x + avail - combo_w, pos.y + (row_h - combo_h) * 0.5f });
        ImGui::SetNextItemWidth(combo_w);

        ImGui::PushStyleColor(ImGuiCol_FrameBg,        ImGui::GetColorU32(menu::with_alpha(menu::palette::surface2, 0.90f)));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImGui::GetColorU32(menu::with_alpha(menu::palette::surface3, 1.0f)));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImGui::GetColorU32(menu::with_alpha(menu::palette::accent,  0.18f)));
        ImGui::PushStyleColor(ImGuiCol_PopupBg,        ImGui::GetColorU32(ImVec4(0.055f, 0.059f, 0.067f, 0.98f)));
        ImGui::PushStyleColor(ImGuiCol_Header,         ImGui::GetColorU32(menu::with_alpha(menu::palette::accent, 0.18f)));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered,  ImGui::GetColorU32(menu::with_alpha(menu::palette::accent, 0.28f)));
        ImGui::PushStyleColor(ImGuiCol_Text,           ImGui::GetColorU32(menu::palette::txt));
        ImGui::PushStyleColor(ImGuiCol_Button,         ImGui::GetColorU32(menu::with_alpha(menu::palette::surface2, 0.90f)));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f * sf);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  ImVec2(8.0f * sf, (combo_h - ImGui::GetTextLineHeight()) * 0.5f));

        const bool r = ImGui::Combo("##c", v, options.data(), static_cast<int32_t>(options.size()));

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(8);

        // row separator + advance cursor past row
        dl->AddLine({ pos.x, pos.y + row_h }, { pos.x + avail, pos.y + row_h },
            ImGui::GetColorU32(menu::with_alpha(menu::palette::line, 0.38f)), 1.0f);
        ImGui::SetCursorScreenPos({ pos.x, pos.y + row_h + 1.0f });

        ImGui::PopID();
        return r;
    }

    // ---------- aevum-style color swatch + picker ----------

    static bool colorpicker(const std::string& label, ImVec4* v)
    {
        const float sf    = menu::get_scale_factor();
        const float row_h = 34.0f * sf;
        ImGui::PushID(label.c_str());
        const std::string txt = label_text(label);

        const ImVec2 pos   = ImGui::GetCursorScreenPos();
        const float  avail = ImGui::GetContentRegionAvail().x;
        ImDrawList*  dl    = ImGui::GetWindowDrawList();

        // passive hover tint
        const bool row_hovered = ImGui::IsMouseHoveringRect(pos, { pos.x + avail, pos.y + row_h });
        if (row_hovered)
            dl->AddRectFilled(pos, { pos.x + avail, pos.y + row_h },
                ImGui::GetColorU32(menu::with_alpha(menu::palette::txt, 0.04f)), 4.0f * sf);

        // label (left)
        const ImVec2 ts = ImGui::CalcTextSize(txt.c_str());
        dl->AddText({ pos.x, pos.y + (row_h - ts.y) * 0.5f },
            ImGui::GetColorU32(menu::palette::txt), txt.c_str());

        // swatch hit rect
        const float sw = 26.0f * sf, sh = 26.0f * sf;
        const ImVec2 swatch_min{ pos.x + avail - sw, pos.y + (row_h - sh) * 0.5f };
        const ImVec2 swatch_max{ swatch_min.x + sw,  swatch_min.y + sh };

        ImGui::SetCursorScreenPos(swatch_min);
        ImGui::InvisibleButton("swatch", { sw, sh });
        const bool swatch_hovered = ImGui::IsItemHovered();
        const bool swatch_clicked = ImGui::IsItemClicked();
        if (swatch_clicked) ImGui::OpenPopup("##cpicker");

        // swatch: dark bg → colour fill → border
        dl->AddRectFilled(swatch_min, swatch_max,
            ImGui::GetColorU32(ImVec4(0.031f, 0.033f, 0.039f, 0.90f)), 6.0f * sf);
        dl->AddRectFilled(
            { swatch_min.x + 4.0f * sf, swatch_min.y + 4.0f * sf },
            { swatch_max.x - 4.0f * sf, swatch_max.y - 4.0f * sf },
            ImGui::GetColorU32(*v), 4.0f * sf);
        dl->AddRect(swatch_min, swatch_max,
            swatch_hovered
                ? ImGui::GetColorU32(menu::with_alpha(menu::palette::accent, 0.85f))
                : ImGui::GetColorU32(menu::with_alpha(menu::palette::line, 0.75f)),
            6.0f * sf, 0, swatch_hovered ? 1.8f : 1.0f);

        // picker popup
        bool changed = false;
        ImGui::SetNextWindowPos({ swatch_min.x - 220.0f * sf, swatch_min.y }, ImGuiCond_Always);
        if (ImGui::BeginPopup("##cpicker"))
        {
            changed = ImGui::ColorPicker4("##p", reinterpret_cast<float*>(v),
                ImGuiColorEditFlags_NoSidePreview |
                ImGuiColorEditFlags_NoSmallPreview |
                ImGuiColorEditFlags_PickerHueWheel);
            ImGui::EndPopup();
        }

        // row separator + advance cursor
        dl->AddLine({ pos.x, pos.y + row_h }, { pos.x + avail, pos.y + row_h },
            ImGui::GetColorU32(menu::with_alpha(menu::palette::line, 0.38f)), 1.0f);
        ImGui::SetCursorScreenPos({ pos.x, pos.y + row_h + 1.0f });

        ImGui::PopID();
        return changed || swatch_clicked;
    }

    // ---------- aevum-style action button ----------

    static bool action_button(const std::string& label, bool danger = false)
    {
        const float sf = menu::get_scale_factor();
        ImGui::PushID(label.c_str());
        const std::string txt = label_text(label);

        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const ImVec2 text_size = ImGui::CalcTextSize(txt.c_str());
        const float desired_width = (std::max)(
            120.0f * sf, text_size.x + 24.0f * sf);
        const ImVec2 size{
            (std::min)(desired_width, ImGui::GetContentRegionAvail().x),
            30.0f * sf };

        const bool pressed = ImGui::InvisibleButton("##btn", size);
        const bool hovered = ImGui::IsItemHovered();
        const bool active  = ImGui::IsItemActive();
        const float anim   = animate(ImGui::GetID("h"), hovered ? 1.f : 0.f);
        ImDrawList* dl     = ImGui::GetWindowDrawList();

        const ImVec4 signal = danger ? ImVec4(0.941f, 0.298f, 0.298f, 0.90f) : menu::palette::accent;
        const ImU32  bg     = active
            ? ImGui::GetColorU32(menu::with_alpha(signal, 0.22f))
            : ImGui::GetColorU32(menu::with_alpha(menu::palette::surface2, 0.90f + 0.10f * anim));
        const ImU32  border = ImGui::GetColorU32(menu::with_alpha(signal, 0.15f + 0.28f * anim));
        const ImU32  text   = danger
            ? lerp_col(menu::palette::txt, ImVec4(1.f, 0.66f, 0.66f, 1.f), anim)
            : ImGui::GetColorU32(menu::palette::txt);

        const ImVec2 max{ pos.x + size.x, pos.y + size.y };
        dl->AddRectFilled(pos, max, bg, 4.0f * sf);
        dl->AddRect(pos, max, border, 4.0f * sf, 0, 1.0f);
        dl->AddText(
            { pos.x + (size.x - text_size.x) * 0.5f,
              pos.y + (size.y - text_size.y) * 0.5f },
            text,
            txt.c_str());

        ImGui::PopID();
        return pressed;
    }

    static bool int_textbot(const std::string& label, int* v)
    {
        const float sf = menu::get_scale_factor();
        ImGui::PushID(label.c_str());
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(menu::palette::txt, "%s", label_text(label).c_str());
        const float w = (std::min)(150.f * sf, ImGui::GetContentRegionAvail().x * 0.5f);
        right_align(w);
        const bool r = ImGui::InputInt("##i", v);
        ImGui::Dummy(ImVec2(0, 3.f * sf));
        ImGui::PopID();
        return r;
    }

    static bool textbox(const std::string& label, std::string* v)
    {
        const float sf = menu::get_scale_factor();
        ImGui::PushID(label.c_str());
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(menu::palette::txt, "%s", label_text(label).c_str());
        const float w = (std::min)(230.f * sf, ImGui::GetContentRegionAvail().x * 0.62f);
        right_align(w);
        const bool r = ImGui::InputText("##t", v);
        ImGui::Dummy(ImVec2(0, 3.f * sf));
        ImGui::PopID();
        return r;
    }
}
