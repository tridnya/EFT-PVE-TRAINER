#include "metadata_commands.h"

#include <Windows.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "inspector_support.h"
#include "../../il2cpp/exports.h"
#include "../../il2cpp/il2utils.h"
#include "../../il2cpp/runtime_discovery.h"
#include "../../il2cpp/unity.h"
#include "../../module/game_state.h"

namespace runtime::mcp::metadata_commands
{
    std::string status()
    {
        using namespace inspector_support;

        const HMODULE veil_module = GetModuleHandleW(L"veil.dll");
        const HMODULE game_assembly = GetModuleHandleW(L"GameAssembly.dll");
        const HMODULE unity_player = GetModuleHandleW(L"UnityPlayer.dll");
        Il2CppObject* camera = unity::camera_get_current_internal != nullptr ?
            unity::get_current_camera() : nullptr;

        return success(
            "{\"protocol_version\":" +
            std::to_string(protocol_version) +
            ",\"process_id\":" + std::to_string(GetCurrentProcessId()) +
            ",\"unity_thread_id\":" + std::to_string(GetCurrentThreadId()) +
            ",\"in_raid\":" +
            std::string(game_state::is_in_raid ? "true" : "false") +
            ",\"modules\":{\"veil\":" + pointer_json(veil_module) +
            ",\"game_assembly\":" + pointer_json(game_assembly) +
            ",\"unity_player\":" + pointer_json(unity_player) + "}" +
            ",\"current_camera\":" + object_summary_json(camera) + "}");
    }

    std::string list_assemblies()
    {
        using namespace inspector_support;

        Il2CppDomain* domain = il2cpp::il2cpp_domain_get();
        if (domain == nullptr)
        {
            return error("IL2CPP domain is unavailable");
        }

        std::size_t assembly_count = 0;
        const Il2CppAssembly** assemblies =
            il2cpp::il2cpp_domain_get_assemblies(domain, &assembly_count);
        if (assemblies == nullptr)
        {
            return error("IL2CPP assembly list is unavailable");
        }

        std::string values = "[";
        for (std::size_t index = 0; index < assembly_count; ++index)
        {
            if (index != 0)
            {
                values += ",";
            }

            const Il2CppAssembly* assembly = assemblies[index];
            const Il2CppImage* image =
                assembly != nullptr ? assembly->image : nullptr;
            values += "{\"index\":" + std::to_string(index) +
                ",\"name\":" +
                json_string(image != nullptr && image->name != nullptr ?
                    image->name : "") +
                ",\"image\":" + pointer_json(image) + "}";
        }
        values += "]";
        return success("{\"count\":" + std::to_string(assembly_count) +
            ",\"assemblies\":" + values + "}");
    }

    std::string find_classes(BinaryReader& reader)
    {
        using namespace inspector_support;

        std::string image_name;
        std::string query;
        std::uint32_t requested_limit = 0;
        if (!reader.read_string(image_name) ||
            !reader.read_string(query) ||
            !reader.read(requested_limit))
        {
            return error("invalid find_classes payload");
        }

        Il2CppImage* image = find_image(image_name);
        if (image == nullptr)
        {
            return error("assembly image not found: " + image_name);
        }
        if (!runtime_discovery::initialize())
        {
            return error("runtime discovery API is unavailable");
        }

        runtime_discovery::RuntimeApi& api = runtime_discovery::api();
        const std::size_t class_count = api.image_get_class_count(image);
        const std::string lowered_query = runtime_discovery::to_lower(query);
        const std::uint32_t limit = bounded_limit(requested_limit, 50);
        std::size_t match_count = 0;
        std::string values = "[";
        bool first = true;
        for (std::size_t index = 0; index < class_count; ++index)
        {
            Il2CppClass* klass = api.image_get_class(image, index);
            if (!is_valid_class(klass))
            {
                continue;
            }

            const std::string full_name =
                runtime_discovery::to_lower(runtime_discovery::class_name(klass));
            if (!lowered_query.empty() &&
                full_name.find(lowered_query) == std::string::npos)
            {
                continue;
            }

            ++match_count;
            if (match_count > limit)
            {
                continue;
            }

            if (!first)
            {
                values += ",";
            }
            first = false;
            values += class_summary_json(klass);
        }
        values += "]";

        return success("{\"image\":" + json_string(image_name) +
            ",\"query\":" + json_string(query) +
            ",\"image_class_count\":" + std::to_string(class_count) +
            ",\"match_count\":" + std::to_string(match_count) +
            ",\"returned\":" +
            std::to_string((std::min<std::size_t>)(match_count, limit)) +
            ",\"classes\":" + values + "}");
    }

    std::string inspect_class(BinaryReader& reader)
    {
        using namespace inspector_support;

        std::uint64_t raw_address = 0;
        std::uint32_t requested_fields = 0;
        std::uint32_t requested_methods = 0;
        if (!reader.read(raw_address) ||
            !reader.read(requested_fields) ||
            !reader.read(requested_methods))
        {
            return error("invalid inspect_class payload");
        }

        auto* klass = reinterpret_cast<Il2CppClass*>(
            static_cast<std::uintptr_t>(raw_address));
        if (!is_valid_class(klass))
        {
            return error("class address is not readable IL2CPP metadata");
        }

        const std::uint32_t field_limit =
            bounded_limit(requested_fields, 128);
        const std::uint32_t method_limit =
            bounded_limit(requested_methods, 128);
        const std::vector<FieldInfo*> fields =
            runtime_discovery::collect_fields(klass, field_limit);
        const std::vector<const MethodInfo*> methods =
            runtime_discovery::collect_methods(klass, method_limit);

        std::string field_values = "[";
        for (std::size_t index = 0; index < fields.size(); ++index)
        {
            if (index != 0)
            {
                field_values += ",";
            }
            field_values += field_summary_json(nullptr, fields[index], false);
        }
        field_values += "]";

        std::string method_values = "[";
        for (std::size_t index = 0; index < methods.size(); ++index)
        {
            if (index != 0)
            {
                method_values += ",";
            }
            method_values += method_summary_json(methods[index]);
        }
        method_values += "]";

        return success("{\"class\":" + class_summary_json(klass) +
            ",\"instance_size\":" + std::to_string(klass->instance_size) +
            ",\"field_count\":" + std::to_string(klass->field_count) +
            ",\"method_count\":" + std::to_string(klass->method_count) +
            ",\"fields\":" + field_values +
            ",\"methods\":" + method_values + "}");
    }

    std::string read_field(BinaryReader& reader)
    {
        using namespace inspector_support;

        std::uint64_t raw_address = 0;
        std::string field_name;
        if (!reader.read(raw_address) || !reader.read_string(field_name))
        {
            return error("invalid read_field payload");
        }

        auto* object = reinterpret_cast<Il2CppObject*>(
            static_cast<std::uintptr_t>(raw_address));
        if (!is_valid_object(object))
        {
            return error("object address is not readable IL2CPP data");
        }

        FieldInfo* field = il2utils::resolve_field(
            object->klass, field_name.c_str());
        if (field == nullptr)
        {
            return error("field not found: " + field_name);
        }
        return success(field_summary_json(object, field, true));
    }
}
