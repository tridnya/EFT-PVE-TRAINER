#pragma once
#include <mutex>
#include <set>
#include <string>

#include "ConfigValue.h"
#include "../gui/menu/imgui_addons.h"

/**
 * Stores a set of favorited loot names (by display name). Persisted in the config
 * as a single field, items joined with '|' (config itself uses ';' as separator,
 * so '|' is safe here).
 *
 * Thread-safe: contains() is read from the game Update thread (loot scan) while
 * toggle()/draw() write from the render thread, so all access to items_ is guarded.
 * The mutex is a function-local static (NOT a member) because this class is
 * #pragma pack(1) and packing an OS lock primitive would misalign it.
 */
#pragma pack(push, 1)
class FavoritesValue : public ConfigValue
{
public:
    explicit FavoritesValue(const std::string& n) : ConfigValue(n)
    {
    }

    bool contains(const std::string& name) const
    {
        std::lock_guard<std::mutex> lk(mtx());
        return items_.count(name) > 0;
    }

    void toggle(const std::string& name)
    {
        const std::string s = sanitize(name);
        if (s.empty()) return;
        std::lock_guard<std::mutex> lk(mtx());
        const auto it = items_.find(s);
        if (it != items_.end()) items_.erase(it);
        else items_.insert(s);
    }

    void clear()
    {
        std::lock_guard<std::mutex> lk(mtx());
        items_.clear();
    }

    std::set<std::string> all() const
    {
        std::lock_guard<std::mutex> lk(mtx());
        return items_;
    }

    void draw() override
    {
        const std::string title = this->get_imgui_title();
        ImGui::PushID(title.c_str());

        // snapshot under lock, then render the snapshot (keeps the lock hold short)
        std::set<std::string> snapshot;
        {
            std::lock_guard<std::mutex> lk(mtx());
            snapshot = items_;
        }

        const std::string label = title.substr(0, title.find("##"));
        ImGui::Text("%s (%d)", label.c_str(), static_cast<int>(snapshot.size()));
        ImGui::SameLine(ImGui::GetWindowWidth() - 64 * menu::get_scale_factor());
        if (ImGui::SmallButton("Clear")) clear();

        std::string to_remove;
        for (const auto& n : snapshot)
        {
            ImGui::PushID(n.c_str());
            if (ImGui::SmallButton("x")) to_remove = n;
            ImGui::SameLine();
            ImGui::TextDisabled("%s", n.c_str());
            ImGui::PopID();
        }
        if (!to_remove.empty())
        {
            std::lock_guard<std::mutex> lk(mtx());
            items_.erase(to_remove);
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y * 2 * menu::get_scale_factor());
        ImGui::Dummy({0, 0});
        ImGui::PopID();
    }

    void set(const std::string& string) override
    {
        std::lock_guard<std::mutex> lk(mtx());
        items_.clear();
        size_t start = 0;
        while (start <= string.size())
        {
            const size_t bar = string.find('|', start);
            std::string tok = (bar == std::string::npos) ? string.substr(start) : string.substr(start, bar - start);
            if (!tok.empty()) items_.insert(tok);
            if (bar == std::string::npos) break;
            start = bar + 1;
        }
    }

    std::string get() override
    {
        std::lock_guard<std::mutex> lk(mtx());
        std::string out;
        for (const auto& n : items_)
        {
            if (!out.empty()) out += '|';
            out += n;
        }
        return out;
    }

    std::string get_default() override { return ""; }

private:
    std::set<std::string> items_;

    // function-local static mutex (kept out of the packed object layout)
    static std::mutex& mtx()
    {
        static std::mutex m;
        return m;
    }

    // names must not contain the config separators ';' or '|'
    static std::string sanitize(const std::string& in)
    {
        std::string o;
        o.reserve(in.size());
        for (const char c : in) o += (c == ';' || c == '|') ? ' ' : c;
        return o;
    }
};
#pragma pack(pop)
