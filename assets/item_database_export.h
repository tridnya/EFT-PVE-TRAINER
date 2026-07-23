#pragma once

#include <cstddef>
#include <string>

namespace item_database_export
{
    struct ExportResult
    {
        bool success = false;
        std::string path;
        std::size_t item_count = 0;
        bool live_game_data = false;
        std::string error;
    };

    [[nodiscard]] ExportResult write_csv();
}
