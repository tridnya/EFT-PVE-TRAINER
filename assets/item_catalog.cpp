#include "item_catalog.h"

#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <utility>

#include "item_database.h"

namespace item_catalog::state
{
    std::mutex catalog_mutex{};
    Entries catalog_entries{};
    std::uint64_t catalog_generation = 0;
    bool live_game_data = false;
}

namespace item_catalog::detail
{
    [[nodiscard]] Entries embedded_entries()
    {
        Entries entries;
        entries.reserve(ITEM_DB_COUNT);
        for (int index = 0; index < ITEM_DB_COUNT; ++index)
        {
            const ItemDef& item = ITEM_DB[index];
            entries.push_back({
                item.id,
                item.en[0] ? item.en : item.fr,
                {},
                {},
                {},
                {},
                item.fr,
                "embedded"
            });
        }
        return entries;
    }

    void ensure_initialized_locked()
    {
        if (!state::catalog_entries.empty())
        {
            return;
        }

        state::catalog_entries = embedded_entries();
        state::catalog_generation = 1;
    }

    [[nodiscard]] std::string sort_name(const Entry& entry)
    {
        if (!entry.display_name.empty())
        {
            return entry.display_name;
        }
        if (!entry.short_name.empty())
        {
            return entry.short_name;
        }
        if (!entry.internal_name.empty())
        {
            return entry.internal_name;
        }
        return entry.id;
    }
}

namespace item_catalog
{
    Entries snapshot()
    {
        std::scoped_lock lock(state::catalog_mutex);
        detail::ensure_initialized_locked();
        return state::catalog_entries;
    }

    std::uint64_t generation()
    {
        std::scoped_lock lock(state::catalog_mutex);
        detail::ensure_initialized_locked();
        return state::catalog_generation;
    }

    bool uses_live_game_data()
    {
        std::scoped_lock lock(state::catalog_mutex);
        detail::ensure_initialized_locked();
        return state::live_game_data;
    }

    void replace_with_live_entries(Entries entries)
    {
        Entries embedded = detail::embedded_entries();
        std::unordered_map<std::string, const Entry*> embedded_by_id;
        embedded_by_id.reserve(embedded.size());
        for (const Entry& entry : embedded)
        {
            embedded_by_id.emplace(entry.id, &entry);
        }

        std::unordered_map<std::string, std::size_t> live_by_id;
        live_by_id.reserve(entries.size());
        Entries merged;
        merged.reserve(entries.size() + embedded.size());

        for (Entry& entry : entries)
        {
            if (entry.id.empty() || live_by_id.contains(entry.id))
            {
                continue;
            }

            const auto embedded_match = embedded_by_id.find(entry.id);
            if (embedded_match != embedded_by_id.end())
            {
                const Entry& fallback = *embedded_match->second;
                if (entry.display_name.empty() || entry.display_name == entry.internal_name)
                {
                    entry.display_name = fallback.display_name;
                }
                if (entry.french_name.empty())
                {
                    entry.french_name = fallback.french_name;
                }
            }

            entry.source = "game";
            live_by_id.emplace(entry.id, merged.size());
            merged.push_back(std::move(entry));
        }

        for (Entry& entry : embedded)
        {
            if (!live_by_id.contains(entry.id))
            {
                merged.push_back(std::move(entry));
            }
        }

        std::stable_sort(merged.begin(), merged.end(), [](const Entry& left, const Entry& right)
        {
            const std::string left_name = detail::sort_name(left);
            const std::string right_name = detail::sort_name(right);
            if (left_name == right_name)
            {
                return left.id < right.id;
            }
            return left_name < right_name;
        });

        std::scoped_lock lock(state::catalog_mutex);
        state::catalog_entries = std::move(merged);
        state::live_game_data = true;
        ++state::catalog_generation;
    }
}
