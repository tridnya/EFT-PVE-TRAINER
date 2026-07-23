#pragma once

#include <array>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_dx12.h"

namespace menu
{
    float get_width();
    float get_height();
    float get_scale_factor();
    float get_hud_font_size();
    ImFont* get_hud_font();
    ImFont* get_icon_font();
    std::array<char, 5> icon_utf8(ImWchar glyph);
    bool is_active();
    void setup();
    void draw();

    constexpr std::array<const char*, 165> key_names = {
        "Unknown",
        "LBUTTON",
        "RBUTTON",
        "CANCEL",
        "MBUTTON",
        "XBUTTON1",
        "XBUTTON2",
        "Unknown",
        "BACK",
        "TAB",
        "Unknown",
        "Unknown",
        "CLEAR",
        "RETURN",
        "Unknown",
        "Unknown",
        "SHIFT",
        "CONTROL",
        "MENU",
        "PAUSE",
        "CAPITAL",
        "KANA",
        "Unknown",
        "JUNJA",
        "FINAL",
        "KANJI",
        "Unknown",
        "ESCAPE",
        "CONVERT",
        "NONCONVERT",
        "ACCEPT",
        "MODECHANGE",
        "SPACE",
        "PRIOR",
        "NEXT",
        "END",
        "HOME",
        "LEFT",
        "UP",
        "RIGHT",
        "DOWN",
        "SELECT",
        "PRINT",
        "EXECUTE",
        "SNAPSHOT",
        "INSERT",
        "DELETE",
        "HELP",
        "0",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "G",
        "H",
        "I",
        "J",
        "K",
        "L",
        "M",
        "N",
        "O",
        "P",
        "Q",
        "R",
        "S",
        "T",
        "U",
        "V",
        "W",
        "X",
        "Y",
        "Z",
        "LWIN",
        "RWIN",
        "APPS",
        "Unknown",
        "SLEEP",
        "NUMPAD0",
        "NUMPAD1",
        "NUMPAD2",
        "NUMPAD3",
        "NUMPAD4",
        "NUMPAD5",
        "NUMPAD6",
        "NUMPAD7",
        "NUMPAD8",
        "NUMPAD9",
        "MULTIPLY",
        "ADD",
        "SEPARATOR",
        "SUBTRACT",
        "DECIMAL",
        "DIVIDE",
        "F1",
        "F2",
        "F3",
        "F4",
        "F5",
        "F6",
        "F7",
        "F8",
        "F9",
        "F10",
        "F11",
        "F12",
        "F13",
        "F14",
        "F15",
        "F16",
        "F17",
        "F18",
        "F19",
        "F20",
        "F21",
        "F22",
        "F23",
        "F24",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "NUMLOCK",
        "SCROLL",
        "OEM_NEC_EQUAL",
        "OEM_FJ_MASSHOU",
        "OEM_FJ_TOUROKU",
        "OEM_FJ_LOYA",
        "OEM_FJ_ROYA",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "Unknown",
        "LSHIFT",
        "RSHIFT",
        "LCONTROL",
        "RCONTROL",
        "LMENU",
        "RMENU"
    };

    inline void theme()
    {
        ImGuiStyle& style = ImGui::GetStyle();

        style.Alpha = 1.0f;
        style.DisabledAlpha = 0.6000000238418579f;
        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.WindowRounding = 10.0f;
        style.WindowBorderSize = 0.0f;
        style.WindowMinSize = ImVec2(32.0f, 32.0f);
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
        style.WindowMenuButtonPosition = ImGuiDir_Left;
        style.ChildRounding = 5.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupRounding = 10.0f;
        style.PopupBorderSize = 0.0f;
        style.FramePadding = ImVec2(4.0f, 3.0f);
        style.FrameRounding = 5.0f;
        style.FrameBorderSize = 0.0f;
        style.ItemSpacing = ImVec2(8.0f, 4.0f);
        style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
        style.CellPadding = ImVec2(4.0f, 2.0f);
        style.IndentSpacing = 21.0f;
        style.ColumnsMinSpacing = 6.0f;
        style.ScrollbarSize = 14.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabMinSize = 10.0f;
        style.GrabRounding = 5.0f;
        style.TabRounding = 5.0f;
        style.TabBorderSize = 0.0f;
        style.TabCloseButtonMinWidthSelected = 0.0f;
        style.TabCloseButtonMinWidthUnselected = 0.0f;
        style.ColorButtonPosition = ImGuiDir_Right;
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

        style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(125 / 255.f, 125 / 255.f, 125 / 255.f, 255 / 255.f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.03921568766236305f, 0.03921568766236305f, 0.03921568766236305f,
                                                 1.0f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.05490196123719215f, 0.05490196123719215f, 0.05490196123719215f, 1.0f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f,
                                                0.8583691120147705f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.6995707750320435f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.05490196123719215f, 0.05490196123719215f, 0.05490196123719215f, 1.0f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.06666667014360428f, 0.06666667014360428f, 0.06666667014360428f,
                                                       1.0f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.05490196123719215f, 0.05490196123719215f, 0.05490196123719215f,
                                                      0.05490196123719215f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1843137294054031f, 0.1921568661928177f, 0.2117647081613541f, 1.0f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.03921568766236305f, 0.03921568766236305f, 0.03921568766236305f,
                                                      1.0f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05490196123719215f, 0.05490196123719215f,
                                                         0.05490196123719215f, 1.0f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f,
                                                  0.9399999976158142f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f,
                                                    0.5299999713897705f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f,
                                                      1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f,
                                                             1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5098039507865906f, 0.5098039507865906f,
                                                            0.5098039507865906f, 1.0f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.6470588445663452f, 0.2313725501298904f, 0.2313725501298904f, 1.0f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.05490196123719215f, 0.05490196123719215f, 0.05490196123719215f, 1.0f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f, 1.0f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f,
                                                     1.0f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.05490196123719215f, 0.05490196123719215f, 0.05490196123719215f, 1.0f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.06666667014360428f, 0.06666667014360428f, 0.06666667014360428f,
                                                      1.0f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f,
                                                     1.0f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f,
                                                  0.501960813999176f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f,
                                                         0.6695278882980347f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f,
                                                        0.9570815563201904f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1019607856869698f, 0.1137254908680916f, 0.1294117718935013f,
                                                   0.2000000029802322f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2039215713739395f, 0.2078431397676468f, 0.2156862765550613f,
                                                          0.2000000029802322f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.3019607961177826f, 0.3019607961177826f, 0.3019607961177826f,
                                                         0.2000000029802322f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.1843137294054031f, 0.1921568661928177f, 0.2117647081613541f, 1.0f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2352941185235977f, 0.2470588237047195f, 0.2705882489681244f, 1.0f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.2588235437870026f, 0.2745098173618317f, 0.3019607961177826f, 1.0f);
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667014360428f, 0.06666667014360428f, 0.06666667014360428f,
                                                     0.9725490212440491f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.06666667014360428f, 0.06666667014360428f,
                                                           0.06666667014360428f, 1.0f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.9490196108818054f, 0.3450980484485626f, 0.3450980484485626f,
                                                         1.0f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.9490196108818054f, 0.3450980484485626f, 0.3450980484485626f,
                                                      1.0f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.4274509847164154f, 0.3607843220233917f,
                                                             0.3607843220233917f, 1.0f);
        style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f,
                                                      1.0f);
        style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f,
                                                          1.0f);
        style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f,
                                                         1.0f);
        style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.0f, 0.8784313797950745f, 0.8784313797950745f, 1.0f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.2588235437870026f, 0.2705882489681244f, 0.3803921639919281f,
                                                       1.0f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.1803921610116959f, 0.2274509817361832f, 0.2784313857555389f,
                                                     1.0f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f,
                                                          0.2000000029802322f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f,
                                                         0.3499999940395355f);
    }

    // Imperil neutral theme. Applied after theme() so it owns the final ImGui colors.
    namespace palette
    {
        constexpr ImVec4 accent   = ImVec4(0.900f, 0.900f, 0.885f, 1.0f);
        constexpr ImVec4 accent2  = ImVec4(0.690f, 0.690f, 0.675f, 1.0f);
        constexpr ImVec4 bg       = ImVec4(0.024f, 0.025f, 0.026f, 0.990f);
        constexpr ImVec4 surface  = ImVec4(0.047f, 0.048f, 0.050f, 1.0f);
        constexpr ImVec4 surface2 = ImVec4(0.075f, 0.076f, 0.079f, 1.0f);
        constexpr ImVec4 surface3 = ImVec4(0.112f, 0.113f, 0.116f, 1.0f);
        constexpr ImVec4 line     = ImVec4(0.180f, 0.182f, 0.187f, 1.0f);
        constexpr ImVec4 txt      = ImVec4(0.925f, 0.925f, 0.910f, 1.0f);
        constexpr ImVec4 muted    = ImVec4(0.530f, 0.535f, 0.545f, 1.0f);
    }

    inline ImVec4 with_alpha(ImVec4 c, float a) { c.w = a; return c; }

    inline void apply_modern_style()
    {
        using namespace palette;
        ImGuiStyle& s = ImGui::GetStyle();

        // NOTE: do NOT change padding/spacing/indent here — the widget helpers in
        // imgui_addons.h hard-code a 200px control width tuned to theme()'s metrics.
        // Larger padding overflows color pickers and triggers a scrollbar that
        // overlaps the ENABLED/DISABLED labels. Only rounding/borders/colors here.
        s.WindowRounding  = 6.0f;
        s.ChildRounding   = 5.0f;
        s.FrameRounding   = 4.0f;
        s.PopupRounding   = 5.0f;
        s.GrabRounding    = 4.0f;
        s.TabRounding     = 4.0f;
        s.ScrollbarRounding = 5.0f;
        s.WindowBorderSize = 1.0f;
        s.ChildBorderSize  = 1.0f;
        s.FrameBorderSize  = 0.0f;
        s.PopupBorderSize  = 1.0f;

        ImVec4* c = s.Colors;
        c[ImGuiCol_Text]                 = txt;
        c[ImGuiCol_TextDisabled]         = muted;
        c[ImGuiCol_WindowBg]             = bg;
        c[ImGuiCol_ChildBg]              = with_alpha(surface, 0.44f);
        c[ImGuiCol_PopupBg]              = ImVec4(0.055f, 0.059f, 0.067f, 0.98f);
        c[ImGuiCol_Border]               = with_alpha(line, 0.58f);
        c[ImGuiCol_BorderShadow]         = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_FrameBg]              = surface;
        c[ImGuiCol_FrameBgHovered]       = surface2;
        c[ImGuiCol_FrameBgActive]        = with_alpha(accent, 0.18f);
        c[ImGuiCol_TitleBg]              = bg;
        c[ImGuiCol_TitleBgActive]        = surface;
        c[ImGuiCol_TitleBgCollapsed]     = bg;
        c[ImGuiCol_MenuBarBg]            = surface;
        c[ImGuiCol_ScrollbarBg]          = ImVec4(0, 0, 0, 0.18f);
        c[ImGuiCol_ScrollbarGrab]        = with_alpha(line, 0.72f);
        c[ImGuiCol_ScrollbarGrabHovered] = with_alpha(accent, 0.45f);
        c[ImGuiCol_ScrollbarGrabActive]  = accent;
        c[ImGuiCol_CheckMark]            = accent;
        c[ImGuiCol_SliderGrab]           = accent;
        c[ImGuiCol_SliderGrabActive]     = accent;
        c[ImGuiCol_Button]               = surface2;
        c[ImGuiCol_ButtonHovered]        = with_alpha(accent, 0.28f);
        c[ImGuiCol_ButtonActive]         = with_alpha(accent, 0.42f);
        c[ImGuiCol_Header]               = with_alpha(accent, 0.12f);
        c[ImGuiCol_HeaderHovered]        = with_alpha(accent, 0.22f);
        c[ImGuiCol_HeaderActive]         = with_alpha(accent, 0.32f);
        c[ImGuiCol_Separator]            = with_alpha(line, 0.50f);
        c[ImGuiCol_SeparatorHovered]     = with_alpha(accent, 0.45f);
        c[ImGuiCol_SeparatorActive]      = accent;
        c[ImGuiCol_Tab]                  = with_alpha(surface, 0.0f);
        c[ImGuiCol_TabHovered]           = with_alpha(accent, 0.16f);
        c[ImGuiCol_TabActive]            = with_alpha(accent, 0.22f);
        c[ImGuiCol_TabUnfocused]         = with_alpha(surface, 0.0f);
        c[ImGuiCol_TabUnfocusedActive]   = with_alpha(accent, 0.16f);
        c[ImGuiCol_TextSelectedBg]       = with_alpha(accent, 0.30f);
        c[ImGuiCol_NavHighlight]         = with_alpha(accent, 0.44f);
        c[ImGuiCol_ResizeGrip]           = with_alpha(line, 0.30f);
        c[ImGuiCol_ResizeGripHovered]    = with_alpha(accent, 0.45f);
        c[ImGuiCol_ResizeGripActive]     = accent;
    }
}
