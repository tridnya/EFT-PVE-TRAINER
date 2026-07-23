#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "../../il2cpp/libil2cpp/il2cpp-api-types.h"

namespace runtime::mcp::inspector_support
{
    constexpr std::uint32_t maximum_result_limit = 500;
    constexpr std::uint32_t maximum_hierarchy_depth = 12;
    constexpr std::uint32_t maximum_memory_read = 4096;

    std::string json_escape(const std::string& value);
    std::string json_string(const std::string& value);
    std::string pointer_string(const void* pointer);
    std::string pointer_json(const void* pointer);
    std::string success(const std::string& body);
    std::string error(const std::string& message);

    bool is_readable_range(const void* pointer, std::size_t size);
    bool is_writable_range(const void* pointer, std::size_t size);
    bool is_valid_class(const Il2CppClass* klass);
    bool is_valid_object(const Il2CppObject* object);

    Il2CppImage* find_image(const std::string& image_name);
    std::uint32_t bounded_limit(
        std::uint32_t requested,
        std::uint32_t fallback);

    std::string class_summary_json(const Il2CppClass* klass);
    std::string object_summary_json(Il2CppObject* object);
    std::string method_summary_json(const MethodInfo* method);
    std::string field_summary_json(
        Il2CppObject* object,
        const FieldInfo* field,
        bool include_value);

    Il2CppObject* object_to_game_object(Il2CppObject* object);
}
