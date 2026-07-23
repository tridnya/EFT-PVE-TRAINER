#pragma once

#include <cstdint>
#include <span>
#include <string>

#include "protocol.h"

namespace runtime::mcp
{
    std::string execute_command(
        Command command,
        std::span<const std::uint8_t> payload);
}
