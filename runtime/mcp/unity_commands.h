#pragma once

#include <string>

#include "protocol.h"

namespace runtime::mcp::unity_commands
{
    std::string find_objects(BinaryReader& reader);
    std::string inspect_object(BinaryReader& reader);
    std::string read_collection(BinaryReader& reader);
    std::string get_components(BinaryReader& reader);
    std::string get_hierarchy(BinaryReader& reader);
}
