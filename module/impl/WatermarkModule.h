#pragma once
#include "../Module.h"
#include "../ModuleCategory.h"
#include "../../config/CheckboxValue.h"
#include "../../config/ColorpickerValue.h"
#include "../../config/IntSliderValue.h"
#include "../../config/TextboxValue.h"
#include "../../gui/media/media_widget.h"
#include "../../util/globals.h"

class WatermarkModule : Module
{
public:
    WatermarkModule() : Module("Watermark", Config)
    {
    }

    CheckboxValue* enabled = conf(new CheckboxValue(true, "Enabled"));
    TextboxValue* text = conf(new TextboxValue(globals::get_display_title(), "Text"));
    ColorpickerValue* color = conf(new ColorpickerValue({0, 0.5, 1, 1}, "Color"));
    IntSliderValue* font_size = conf(new IntSliderValue(16, 8, 36, "Font Size"));
    CheckboxValue* media_widget_enabled = conf(new CheckboxValue(true, "Media Widget"));

    void draw_overlay(ImDrawList* draw_list) override
    {
        if (enabled->get_value())
            draw_list->AddText(menu::get_hud_font(),
                               static_cast<float>(font_size->get_value()) * menu::get_scale_factor(), {10, 10},
                               ImGui::GetColorU32(color->get_value()), text->get().c_str());
    }

    void application_update() override
    {
        media_widget::set_enabled(media_widget_enabled->get_value());
    }

    void gameworld_update(const Il2CppClass* game_world_class, Il2CppObjectInstance game_world_instance,
                                  Il2CppObjectInstance main_player) override
    {
    }

    void init() override
    {
        media_widget::set_enabled(media_widget_enabled->get_value());
    }
};
