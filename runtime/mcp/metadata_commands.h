#pragma once

#include <string>

#include "protocol.h"

namespace runtime::mcp::metadata_commands
{
    std::string status();
    std::string list_assemblies();
    std::string find_classes(BinaryReader& reader);
    std::string inspect_class(BinaryReader& reader);
    std::string read_field(BinaryReader& reader);
}
