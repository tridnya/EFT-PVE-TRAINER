#pragma once

#include <string>

#include "../Module.h"
#include "../../config/TextboxValue.h"
#include "../../game/customization/outfit_controller.h"

class OutfitModule final : public Module
{
public:
    OutfitModule();

    void draw_overlay(ImDrawList* draw_list) override;
    void draw_menu() override;
    void application_update() override;
    void gameworld_update(
        const Il2CppClass* game_world_class,
        Il2CppObjectInstance game_world_instance,
        Il2CppObjectInstance main_player) override;
    void reset_gameworld_state() override;
    void init() override;

private:
    TextboxValue* persisted_upper_outfit_ =
        conf(new TextboxValue("", "Applied Upper Outfit"));
    TextboxValue* persisted_lower_outfit_ =
        conf(new TextboxValue("", "Applied Lower Outfit"));
    game::customization::OutfitController controller_;
    std::string search_text_;
    std::string selected_outfit_id_;
    int body_filter_ = 0;
};
