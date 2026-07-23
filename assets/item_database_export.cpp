#include "item_database_export.h"

#include <cstdlib>
#include <fstream>
#include <string_view>
#include <Windows.h>

#include "item_catalog.h"

namespace item_database_export::detail
{
    [[nodiscard]] std::string storage_directory()
    {
        char* appdata_value = nullptr;
        std::size_t length = 0;
        if (_dupenv_s(&appdata_value, &length, "APPDATA") != 0 || appdata_value == nullptr)
        {
            return ".";
        }

        const std::string root_path = std::string(appdata_value) + "\\Imperil";
        free(appdata_value);
        CreateDirectoryA(root_path.c_str(), nullptr);

        const std::string veil_path = root_path + "\\Veil";
        CreateDirectoryA(veil_path.c_str(), nullptr);
        return veil_path;
    }

    [[nodiscard]] std::string csv_field(const std::string_view value)
    {
        if (value.find_first_of(",\"\r\n") == std::string_view::npos)
        {
            return std::string(value);
        }

        std::string escaped;
        escaped.reserve(value.size() + 2);
        escaped.push_back('"');
        for (const char character : value)
        {
            if (character == '"')
            {
                escaped.push_back('"');
            }
            escaped.push_back(character);
        }
        escaped.push_back('"');
        return escaped;
    }
}

namespace item_database_export
{
    ExportResult write_csv()
    {
        ExportResult result{};
        result.path = detail::storage_directory() + "\\item_ids.csv";

        std::ofstream output(result.path, std::ios::binary | std::ios::trunc);
        if (!output.is_open())
        {
            result.error = "unable to open the output file";
            return result;
        }

        const item_catalog::Entries entries = item_catalog::snapshot();
        output << "catalog_index,item_id,display_name,short_name,internal_name,description,type_name,french_name,source\r\n";
        for (std::size_t index = 0; index < entries.size(); ++index)
        {
            const item_catalog::Entry& item = entries[index];
            output << index << ','
                << detail::csv_field(item.id) << ','
                << detail::csv_field(item.display_name) << ','
                << detail::csv_field(item.short_name) << ','
                << detail::csv_field(item.internal_name) << ','
                << detail::csv_field(item.description) << ','
                << detail::csv_field(item.type_name) << ','
                << detail::csv_field(item.french_name) << ','
                << detail::csv_field(item.source) << "\r\n";
        }

        output.flush();
        if (!output.good())
        {
            result.error = "the output stream failed while writing";
            return result;
        }

        result.success = true;
        result.item_count = entries.size();
        result.live_game_data = item_catalog::uses_live_game_data();
        return result;
    }
}
