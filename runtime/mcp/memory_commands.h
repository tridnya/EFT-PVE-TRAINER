#pragma once

#include <string>

#include "protocol.h"

namespace runtime::mcp::memory_commands
{
    std::string write_field(BinaryReader& reader);
    std::string read_memory(BinaryReader& reader);
}
