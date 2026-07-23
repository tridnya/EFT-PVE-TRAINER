#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace item_catalog
{
    struct Entry
    {
        std::string id;
        std::string display_name;
        std::string short_name;
        std::string internal_name;
        std::string description;
        std::string type_name;
        std::string french_name;
        std::string source;
    };

    using Entries = std::vector<Entry>;

    [[nodiscard]] Entries snapshot();
    [[nodiscard]] std::uint64_t generation();
    [[nodiscard]] bool uses_live_game_data();
    void replace_with_live_entries(Entries entries);
}
