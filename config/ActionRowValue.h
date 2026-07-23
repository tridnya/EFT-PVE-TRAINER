#pragma once
#include <functional>

#include "CheckboxValue.h"
#include "ConfigValue.h"
#include "../gui/menu/imgui_addons.h"

#pragma pack(push, 1)
class ActionRowValue : public ConfigValue
{
public:
    ActionRowValue(const std::function<void ()>& action, const std::string& n,
                   const bool sameline = false) : ConfigValue(n)
    {
        this->action_ = action;
        this->sameline_ = sameline;
    }

    void draw() override
    {
        if (this->sameline_) ImGui::SameLine();
        if (imgui_addons::action_button(this->get_imgui_title()))
        {
            this->action_();
        }
        ImGui::Dummy(ImVec2(0, 4.f * menu::get_scale_factor()));
    }

    void set(const std::string& string) override
    {
    }

    std::string get() override
    {
        return "";
    }

    std::string get_default() override
    {
        return "";
    }

private:
    std::function<void ()> action_;
    bool sameline_;
};
#pragma pack(pop)
