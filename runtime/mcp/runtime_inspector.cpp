#include "runtime_inspector.h"

#include "inspector_support.h"
#include "memory_commands.h"
#include "metadata_commands.h"
#include "unity_commands.h"

namespace runtime::mcp
{
    std::string execute_command(
        const Command command,
        const std::span<const std::uint8_t> payload)
    {
        BinaryReader reader(payload);
        switch (command)
        {
        case Command::status:
            return metadata_commands::status();
        case Command::list_assemblies:
            return metadata_commands::list_assemblies();
        case Command::find_classes:
            return metadata_commands::find_classes(reader);
        case Command::inspect_class:
            return metadata_commands::inspect_class(reader);
        case Command::find_objects:
            return unity_commands::find_objects(reader);
        case Command::inspect_object:
            return unity_commands::inspect_object(reader);
        case Command::read_field:
            return metadata_commands::read_field(reader);
        case Command::read_collection:
            return unity_commands::read_collection(reader);
        case Command::get_components:
            return unity_commands::get_components(reader);
        case Command::get_hierarchy:
            return unity_commands::get_hierarchy(reader);
        case Command::write_field:
            return memory_commands::write_field(reader);
        case Command::read_memory:
            return memory_commands::read_memory(reader);
        default:
            return inspector_support::error("unknown runtime MCP command");
        }
    }
}
