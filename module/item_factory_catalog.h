#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "../assets/item_catalog.h"
#include "../il2cpp/libil2cpp/il2cpp-api-types.h"

namespace item_factory_catalog
{
    struct CaptureResult
    {
        bool success = false;
        item_catalog::Entries entries;
        std::size_t scanned_fields = 0;
        std::size_t scanned_collections = 0;
        std::vector<std::string> diagnostics;
        std::string error;
    };

    [[nodiscard]] CaptureResult capture(Il2CppObject* item_factory);
}
