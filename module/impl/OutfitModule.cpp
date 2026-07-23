#include "OutfitModule.h"

#include <algorithm>
#include <array>
#include <cctype>

#include <imgui.h>
#include <imgui_stdlib.h>

#include "../../gui/menu/menu.h"

namespace outfit_module_detail
{
    [[nodiscard]] std::string to_lower(std::string value)
    {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](const unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            });
        return value;
    }

    [[nodiscard]] const char* body_name(
        const game::customization::OutfitBodyRegion region)
    {
        switch (region)
        {
        case game::customization::OutfitBodyRegion::Upper:
            return "upper";
        case game::customization::OutfitBodyRegion::Lower:
            return "lower";
        default:
            return "other";
        }
    }

    [[nodiscard]] bool matches_filter(
        const game::customization::OutfitEntry& entry,
        const int body_filter,
        const std::string& lowered_search)
    {
        if (body_filter == 1 && entry.body_region !=
                game::customization::OutfitBodyRegion::Upper)
        {
            return false;
        }
        if (body_filter == 2 && entry.body_region !=
                game::customization::OutfitBodyRegion::Lower)
        {
            return false;
        }
        if (lowered_search.empty())
        {
            return true;
        }

        return to_lower(entry.name).find(lowered_search) != std::string::npos ||
            to_lower(entry.id).find(lowered_search) != std::string::npos;
    }

    [[nodiscard]] bool icon_button(
        const char* id,
        const ImWchar glyph,
        const char* tooltip,
        const bool enabled = true)
    {
        ImGui::PushID(id);
        if (!enabled)
        {
            ImGui::BeginDisabled();
        }

        const std::array<char, 5> icon_text = menu::icon_utf8(glyph);
        ImGui::PushFont(menu::get_icon_font());
        const bool pressed = ImGui::Button(
            icon_text.data(),
            ImVec2(34.0f, 30.0f));
        ImGui::PopFont();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        {
            ImGui::SetTooltip("%s", tooltip);
        }

        if (!enabled)
        {
            ImGui::EndDisabled();
        }
        ImGui::PopID();
        return pressed && enabled;
    }

    void draw_segment(
        const char* id,
        const char* label,
        const int value,
        int& selected)
    {
        ImGui::PushID(id);
        const bool was_selected = selected == value;
        if (was_selected)
        {
            ImGui::PushStyleColor(
                ImGuiCol_Button,
                ImVec4(0.92f, 0.92f, 0.90f, 1.0f));
            ImGui::PushStyleColor(
                ImGuiCol_Text,
                ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
        }
        if (ImGui::Button(label, ImVec2(78.0f, 28.0f)))
        {
            selected = value;
        }
        if (was_selected)
        {
            ImGui::PopStyleColor(2);
        }
        ImGui::PopID();
    }

    void draw_clipped_text(
        ImDrawList* draw_list,
        const ImVec2 position,
        const float maximum_x,
        const ImU32 color,
        const char* text)
    {
        if (draw_list == nullptr || text == nullptr ||
            maximum_x <= position.x)
        {
            return;
        }

        draw_list->PushClipRect(
            position,
            { maximum_x, position.y + ImGui::GetTextLineHeight() },
            true);
        draw_list->AddText(position, color, text);
        draw_list->PopClipRect();
    }

    void draw_selected_label(const std::string& label)
    {
        const float available_width = ImGui::GetContentRegionAvail().x;
        if (available_width <= 4.0f)
        {
            return;
        }

        const ImVec2 position = ImGui::GetCursorScreenPos();
        const ImVec2 size(available_width, 30.0f);
        ImGui::InvisibleButton("##selected_outfit_label", size);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const float text_y = position.y +
            (size.y - ImGui::GetTextLineHeight()) * 0.5f;
        draw_clipped_text(
            draw_list,
            { position.x, text_y },
            position.x + size.x,
            ImGui::GetColorU32(menu::palette::muted),
            label.c_str());

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) &&
            ImGui::CalcTextSize(label.c_str()).x > available_width)
        {
            ImGui::SetTooltip("%s", label.c_str());
        }
    }
}

OutfitModule::OutfitModule() : Module("Outfits", Progression)
{
}

void OutfitModule::draw_overlay(ImDrawList* draw_list)
{
    (void)draw_list;
}

void OutfitModule::draw_menu()
{
    const game::customization::OutfitSnapshot state = controller_.snapshot();
    const float available_width = ImGui::GetContentRegionAvail().x;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));
    ImGui::SetNextItemWidth((std::max)(180.0f, available_width - 44.0f));
    ImGui::InputTextWithHint(
        "##outfit_search", "Search outfits...", &search_text_);
    ImGui::PopStyleVar();
    ImGui::SameLine();
    if (outfit_module_detail::icon_button(
            "refresh", 0xE72C, "Refresh live outfits"))
    {
        controller_.request_refresh();
    }

    outfit_module_detail::draw_segment("all", "All", 0, body_filter_);
    ImGui::SameLine();
    outfit_module_detail::draw_segment("upper", "Upper", 1, body_filter_);
    ImGui::SameLine();
    outfit_module_detail::draw_segment("lower", "Lower", 2, body_filter_);

    ImGui::Dummy(ImVec2(0.0f, 6.0f));
    ImGui::TextColored(
        menu::palette::muted,
        "%s",
        state.status.empty() ? "Outfit runtime idle" : state.status.c_str());
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    const std::string lowered_search =
        outfit_module_detail::to_lower(search_text_);
    const float list_height = 250.0f * menu::get_scale_factor();
    ImGui::PushStyleColor(
        ImGuiCol_ChildBg,
        ImVec4(0.030f, 0.031f, 0.032f, 0.90f));
    ImGui::BeginChild(
        "##outfit_list",
        ImVec2(available_width, list_height),
        true,
        ImGuiWindowFlags_AlwaysVerticalScrollbar);

    std::size_t visible_count = 0;
    for (const game::customization::OutfitEntry& entry : state.entries)
    {
        if (!outfit_module_detail::matches_filter(
                entry, body_filter_, lowered_search))
        {
            continue;
        }
        ++visible_count;

        ImGui::PushID(entry.id.c_str());
        const ImVec2 row_position = ImGui::GetCursorScreenPos();
        const float row_width = ImGui::GetContentRegionAvail().x;
        const ImVec2 row_size(row_width, 48.0f);
        const bool selected = selected_outfit_id_ == entry.id;
        ImGui::InvisibleButton("##outfit_row", row_size);
        const bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked())
        {
            selected_outfit_id_ = entry.id;
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        if (selected || hovered)
        {
            draw_list->AddRectFilled(
                row_position,
                { row_position.x + row_size.x, row_position.y + row_size.y },
                ImGui::GetColorU32(ImVec4(
                    0.14f, 0.14f, 0.14f, selected ? 0.86f : 0.48f)),
                5.0f);
        }

        const ImU32 title_color = ImGui::GetColorU32(menu::palette::txt);
        const ImU32 detail_color = ImGui::GetColorU32(menu::palette::muted);
        const char* body = outfit_module_detail::body_name(entry.body_region);
        const ImVec2 body_size = ImGui::CalcTextSize(body);
        const std::string state_label = entry.equipped ?
            "equipped" :
            (entry.trainer_eligible && !entry.purchased ?
                (entry.internal_obtain ? "internal" : "unobtainable") :
                entry.state);
        const ImVec2 state_size = ImGui::CalcTextSize(state_label.c_str());
        const float right_text_width = (std::max)(
            body_size.x, state_size.x);
        const float right_text_x = row_position.x + row_size.x -
            right_text_width - 12.0f;
        const float left_text_maximum_x = right_text_x - 14.0f;

        outfit_module_detail::draw_clipped_text(
            draw_list,
            { row_position.x + 10.0f, row_position.y + 7.0f },
            left_text_maximum_x,
            title_color,
            entry.name.c_str());
        outfit_module_detail::draw_clipped_text(
            draw_list,
            { row_position.x + 10.0f, row_position.y + 27.0f },
            left_text_maximum_x,
            detail_color,
            entry.id.c_str());

        draw_list->AddText(
            { row_position.x + row_size.x - body_size.x - 12.0f,
              row_position.y + 7.0f },
            detail_color,
            body);
        draw_list->AddText(
            { row_position.x + row_size.x - state_size.x - 12.0f,
              row_position.y + 27.0f },
            entry.equipped ? title_color : detail_color,
            state_label.c_str());
        draw_list->AddLine(
            { row_position.x + 8.0f, row_position.y + row_size.y },
            { row_position.x + row_size.x - 8.0f,
              row_position.y + row_size.y },
            ImGui::GetColorU32(ImVec4(0.18f, 0.18f, 0.18f, 0.70f)));
        const float left_text_width = (std::max)(
            0.0f, left_text_maximum_x - row_position.x - 10.0f);
        if (hovered &&
            (ImGui::CalcTextSize(entry.name.c_str()).x > left_text_width ||
             ImGui::CalcTextSize(entry.id.c_str()).x > left_text_width))
        {
            ImGui::SetTooltip(
                "%s\n%s", entry.name.c_str(), entry.id.c_str());
        }
        ImGui::PopID();
    }

    if (visible_count == 0)
    {
        ImGui::SetCursorPosY((std::max)(
            ImGui::GetCursorPosY(), list_height * 0.42f));
        const char* empty_text = state.view_ready ?
            "No outfits match" : "Tactical clothing screen not detected";
        const float text_width = ImGui::CalcTextSize(empty_text).x;
        ImGui::SetCursorPosX((std::max)(
            8.0f, (ImGui::GetContentRegionAvail().x - text_width) * 0.5f));
        ImGui::TextColored(menu::palette::muted, "%s", empty_text);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    const game::customization::OutfitEntry* selected_entry = nullptr;
    for (const game::customization::OutfitEntry& entry : state.entries)
    {
        if (entry.id == selected_outfit_id_)
        {
            selected_entry = &entry;
            break;
        }
    }

    const bool has_selection = selected_entry != nullptr;
    //const bool can_equip = selected_entry != nullptr && (selected_entry->purchased || selected_entry->equipped || selected_entry->trainer_eligible);
    const bool can_equip = selected_entry != nullptr && selected_entry->equipped;
    if (outfit_module_detail::icon_button(
            "preview", 0xE7B3, "Preview selected outfit", has_selection))
    {
        controller_.request_preview(selected_outfit_id_);
    }
    ImGui::SameLine();
    if (outfit_module_detail::icon_button(
            "equip", 0xE73E, "Equip selected outfit", can_equip))
    {
        controller_.request_equip(selected_outfit_id_);
    }
    ImGui::SameLine();
    if (outfit_module_detail::icon_button(
            "restore", 0xE7A7, "Restore original outfit",
            state.view_ready &&
                (!state.original_upper_id.empty() ||
                 !state.original_lower_id.empty())))
    {
        controller_.request_restore();
    }

    if (selected_entry != nullptr)
    {
        ImGui::SameLine();
        outfit_module_detail::draw_selected_label(selected_entry->name);
    }
}

void OutfitModule::application_update()
{
    controller_.set_enforced_outfits(
        persisted_upper_outfit_->get(),
        persisted_lower_outfit_->get());
    controller_.update();

    const game::customization::OutfitSnapshot state = controller_.snapshot();
    if (persisted_upper_outfit_->get() != state.enforced_upper_id)
    {
        persisted_upper_outfit_->set(state.enforced_upper_id);
    }
    if (persisted_lower_outfit_->get() != state.enforced_lower_id)
    {
        persisted_lower_outfit_->set(state.enforced_lower_id);
    }
}

void OutfitModule::gameworld_update(
    const Il2CppClass* game_world_class,
    Il2CppObjectInstance game_world_instance,
    Il2CppObjectInstance main_player)
{
    (void)game_world_class;
    (void)game_world_instance;
    (void)main_player;
}

void OutfitModule::reset_gameworld_state()
{
    controller_.request_reapply();
}

void OutfitModule::init()
{
    controller_.initialize();
}
