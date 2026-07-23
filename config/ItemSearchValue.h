#pragma once
#include <algorithm>
#include <cctype>
#include <functional>
#include <limits>
#include <string>
#include <string.h>
#include <vector>

#include "ConfigValue.h"
#include "../assets/item_catalog.h"
#include "../gui/menu/imgui_addons.h"

/**
 * Search box with autocomplete over the embedded Tarkov item database.
 * Type a (French or English) name, click a result -> the selected BSG id is
 * stored and pushed to a target via the on_select callback.
 */
#pragma pack(push, 1)
class ItemSearchValue : public ConfigValue
{
public:
    explicit ItemSearchValue(const std::string& n,
                             std::function<void(const std::string&)> on_select = nullptr)
        : ConfigValue(n)
    {
        this->on_select_ = std::move(on_select);
    }

    void draw() override
    {
        build_index();

        const std::string title = this->get_imgui_title();
        ImGui::PushID(title.c_str());

        // label
        std::string just_text = title.substr(0, title.find("##"));
        ImGui::Text("%s", just_text.c_str());

        // full-width search input
        ImGui::SetNextItemWidth(-1.f);
        if (ImGui::InputTextWithHint("##itemsearch", "e.g. ledx, red keycard, m995, gpu...", &query_))
        {
            refilter();
        }

        // current selection summary
        if (!selected_id_.empty())
        {
            ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_CheckMark],
                               "> %s  [%s]", selected_name_.c_str(), selected_id_.c_str());
        }

        // results list (only while there is a query)
        if (!query_.empty() && !results_.empty())
        {
            const float row_h = ImGui::GetTextLineHeightWithSpacing();
            const float height = (std::min)(static_cast<int>(results_.size()), 8) * row_h + 8.f;

            ImGui::BeginChild("##itemresults", ImVec2(-1.f, height), true);
            for (int idx : results_)
            {
                const item_catalog::Entry& item = catalog_[idx];
                ImGui::PushID(idx);

                const std::string label = display_name(item);

                if (ImGui::Selectable(label.c_str(), selected_id_ == item.id))
                {
                    selected_id_ = item.id;
                    selected_name_ = label;
                    if (on_select_) on_select_(selected_id_);
                }
                ImGui::PopID();
            }
            ImGui::EndChild();

            ImGui::TextDisabled("%d result%s%s", static_cast<int>(results_.size()),
                                results_.size() > 1 ? "s" : "",
                                results_.size() >= MAX_RESULTS ? " (refine your search)" : "");
        }
        else if (!query_.empty())
        {
            ImGui::TextDisabled("No results");
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y * 2 * menu::get_scale_factor());
        ImGui::Dummy({0, 0});
        ImGui::PopID();
    }

    // persisted value = the selected id
    void set(const std::string& string) override
    {
        selected_id_ = string;
        // try to recover the display name from the db
        build_index();
        selected_name_.clear();
        for (const item_catalog::Entry& item : catalog_)
        {
            if (selected_id_ == item.id)
            {
                selected_name_ = display_name(item);
                break;
            }
        }
    }

    std::string get() override { return selected_id_; }
    std::string get_default() override { return ""; }

    [[nodiscard]] std::string get_selected_id() const { return selected_id_; }

private:
    static constexpr int MAX_RESULTS = 40;

    std::string query_;
    std::string selected_id_;
    std::string selected_name_;
    std::function<void(const std::string&)> on_select_;

    item_catalog::Entries catalog_;
    std::vector<std::string> haystacks_;
    std::vector<int> results_;
    std::uint64_t catalog_generation_ = (std::numeric_limits<std::uint64_t>::max)();

    [[nodiscard]] static std::string display_name(const item_catalog::Entry& item)
    {
        if (!item.display_name.empty()) return item.display_name;
        if (!item.short_name.empty()) return item.short_name;
        if (!item.internal_name.empty()) return item.internal_name;
        return item.id;
    }

    void build_index()
    {
        const std::uint64_t current_generation = item_catalog::generation();
        if (catalog_generation_ == current_generation)
        {
            return;
        }

        catalog_ = item_catalog::snapshot();
        haystacks_.clear();
        haystacks_.reserve(catalog_.size());
        for (const item_catalog::Entry& item : catalog_)
        {
            std::string h = item.id + ' ' + item.display_name + ' ' + item.short_name + ' ' +
                item.internal_name + ' ' + item.description + ' ' + item.type_name + ' ' + item.french_name;
            std::transform(h.begin(), h.end(), h.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            haystacks_.push_back(std::move(h));
        }

        catalog_generation_ = current_generation;
        refilter();
    }

    void refilter()
    {
        results_.clear();
        std::string q = query_;
        std::transform(q.begin(), q.end(), q.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        // split into terms
        std::vector<std::string> terms;
        size_t start = 0;
        while (start < q.size())
        {
            size_t sp = q.find(' ', start);
            if (sp == std::string::npos) { terms.push_back(q.substr(start)); break; }
            if (sp > start) terms.push_back(q.substr(start, sp - start));
            start = sp + 1;
        }
        if (terms.empty()) return;

        for (int i = 0; i < static_cast<int>(catalog_.size()) &&
            static_cast<int>(results_.size()) < MAX_RESULTS; ++i)
        {
            bool ok = true;
            for (const auto& t : terms)
            {
                if (haystacks_[i].find(t) == std::string::npos) { ok = false; break; }
            }
            if (ok) results_.push_back(i);
        }

        const std::string& t0 = terms[0];
        std::stable_sort(results_.begin(), results_.end(), [&](int a, int b)
        {
            const std::string left_name = display_name(catalog_[a]);
            const std::string right_name = display_name(catalog_[b]);
            const bool sa = _strnicmp(left_name.c_str(), t0.c_str(), t0.size()) == 0;
            const bool sb = _strnicmp(right_name.c_str(), t0.c_str(), t0.size()) == 0;
            if (sa != sb) return sa;
            return false;
        });
    }
};
#pragma pack(pop)
