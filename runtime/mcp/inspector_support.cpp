#include "inspector_support.h"

#include <Windows.h>

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>

#include "../../il2cpp/exports.h"
#include "../../il2cpp/il2utils.h"
#include "../../il2cpp/runtime_discovery.h"
#include "../../il2cpp/unity.h"

namespace runtime::mcp::inspector_support
{
    std::string json_escape(const std::string& value)
    {
        std::ostringstream output;
        for (const unsigned char character : value)
        {
            switch (character)
            {
            case '"':
                output << "\\\"";
                break;
            case '\\':
                output << "\\\\";
                break;
            case '\b':
                output << "\\b";
                break;
            case '\f':
                output << "\\f";
                break;
            case '\n':
                output << "\\n";
                break;
            case '\r':
                output << "\\r";
                break;
            case '\t':
                output << "\\t";
                break;
            default:
                if (character < 0x20)
                {
                    output << "\\u" << std::hex << std::setw(4)
                        << std::setfill('0') << static_cast<int>(character)
                        << std::dec;
                }
                else
                {
                    output << static_cast<char>(character);
                }
                break;
            }
        }
        return output.str();
    }

    std::string json_string(const std::string& value)
    {
        return "\"" + json_escape(value) + "\"";
    }

    std::string pointer_string(const void* pointer)
    {
        std::ostringstream output;
        output << "0x" << std::hex << std::uppercase
            << reinterpret_cast<std::uintptr_t>(pointer);
        return output.str();
    }

    std::string pointer_json(const void* pointer)
    {
        return json_string(pointer_string(pointer));
    }

    std::string success(const std::string& body)
    {
        return "{\"ok\":true,\"result\":" + body + "}";
    }

    std::string error(const std::string& message)
    {
        return "{\"ok\":false,\"error\":" + json_string(message) + "}";
    }

    bool is_readable_range(const void* pointer, const std::size_t size)
    {
        if (pointer == nullptr || size == 0)
        {
            return false;
        }

        const auto start = reinterpret_cast<std::uintptr_t>(pointer);
        if (start > (std::numeric_limits<std::uintptr_t>::max)() - size)
        {
            return false;
        }

        const std::uintptr_t end = start + size;
        std::uintptr_t cursor = start;
        while (cursor < end)
        {
            MEMORY_BASIC_INFORMATION information{};
            if (VirtualQuery(
                    reinterpret_cast<const void*>(cursor),
                    &information,
                    sizeof(information)) == 0)
            {
                return false;
            }
            if (information.State != MEM_COMMIT ||
                (information.Protect & PAGE_GUARD) != 0 ||
                (information.Protect & PAGE_NOACCESS) != 0)
            {
                return false;
            }

            const auto region_start =
                reinterpret_cast<std::uintptr_t>(information.BaseAddress);
            const auto region_end = region_start + information.RegionSize;
            if (region_end <= cursor)
            {
                return false;
            }
            cursor = region_end;
        }
        return true;
    }

    bool is_writable_range(const void* pointer, const std::size_t size)
    {
        if (!is_readable_range(pointer, size))
        {
            return false;
        }

        MEMORY_BASIC_INFORMATION information{};
        if (VirtualQuery(pointer, &information, sizeof(information)) == 0)
        {
            return false;
        }

        const DWORD protection = information.Protect & 0xff;
        return protection == PAGE_READWRITE ||
            protection == PAGE_WRITECOPY ||
            protection == PAGE_EXECUTE_READWRITE ||
            protection == PAGE_EXECUTE_WRITECOPY;
    }

    bool is_valid_class(const Il2CppClass* klass)
    {
        if (!is_readable_range(klass, sizeof(Il2CppClass)) ||
            klass->name == nullptr ||
            !is_readable_range(klass->name, 1))
        {
            return false;
        }

        return klass->klass == nullptr || klass->klass == klass ||
            is_readable_range(klass->klass, sizeof(void*));
    }

    bool is_valid_object(const Il2CppObject* object)
    {
        return is_readable_range(object, sizeof(Il2CppObject)) &&
            is_valid_class(object->klass);
    }

    Il2CppImage* find_image(const std::string& image_name)
    {
        Il2CppDomain* domain = il2cpp::il2cpp_domain_get();
        if (domain == nullptr)
        {
            return nullptr;
        }

        std::size_t assembly_count = 0;
        const Il2CppAssembly** assemblies =
            il2cpp::il2cpp_domain_get_assemblies(domain, &assembly_count);
        if (assemblies == nullptr)
        {
            return nullptr;
        }

        for (std::size_t index = 0; index < assembly_count; ++index)
        {
            Il2CppImage* image = assemblies[index] != nullptr ?
                assemblies[index]->image : nullptr;
            if (image != nullptr && image->name != nullptr &&
                image_name == image->name)
            {
                return image;
            }
        }
        return nullptr;
    }

    std::uint32_t bounded_limit(
        const std::uint32_t requested,
        const std::uint32_t fallback)
    {
        return (std::min)(
            requested == 0 ? fallback : requested,
            maximum_result_limit);
    }

    std::string class_summary_json(const Il2CppClass* klass)
    {
        if (!is_valid_class(klass))
        {
            return "null";
        }

        return "{\"address\":" + pointer_json(klass) +
            ",\"namespace\":" +
            json_string(klass->namespaze != nullptr ? klass->namespaze : "") +
            ",\"name\":" +
            json_string(klass->name != nullptr ? klass->name : "") +
            ",\"full_name\":" +
            json_string(runtime_discovery::class_name(klass)) +
            ",\"parent\":" + pointer_json(klass->parent) + "}";
    }

    std::string object_summary_json(Il2CppObject* object)
    {
        if (!is_valid_object(object))
        {
            return "null";
        }

        const bool unity_object = runtime_discovery::inherits_from(
            object->klass, "UnityEngine", "Object");
        const std::string name = unity_object ?
            runtime_discovery::object_name(object) : "";
        return "{\"address\":" + pointer_json(object) +
            ",\"class\":" + class_summary_json(object->klass) +
            ",\"name\":" + json_string(name) + "}";
    }

    std::string method_summary_json(const MethodInfo* method)
    {
        if (method == nullptr || !is_readable_range(method, sizeof(MethodInfo)))
        {
            return "null";
        }

        std::string return_type = "<unknown>";
        if (method->return_type != nullptr)
        {
            return_type = runtime_discovery::type_name(method->return_type);
        }

        return "{\"address\":" + pointer_json(method) +
            ",\"name\":" +
            json_string(method->name != nullptr ? method->name : "<unnamed>") +
            ",\"parameter_count\":" +
            std::to_string(method->parameters_count) +
            ",\"return_type\":" + json_string(return_type) +
            ",\"method_pointer\":" + pointer_json(method->methodPointer) + "}";
    }

    std::string field_summary_json(
        Il2CppObject* object,
        const FieldInfo* field,
        const bool include_value)
    {
        if (field == nullptr || !is_readable_range(field, sizeof(FieldInfo)))
        {
            return "null";
        }

        std::string body = "{\"address\":" + pointer_json(field) +
            ",\"name\":" +
            json_string(field->name != nullptr ? field->name : "<unnamed>") +
            ",\"type\":" +
            json_string(runtime_discovery::type_name(field->type)) +
            ",\"offset\":" + std::to_string(field->offset) +
            ",\"declaring_class\":" + class_summary_json(field->parent);
        if (include_value)
        {
            body += ",\"value\":" +
                json_string(runtime_discovery::field_value_summary(
                    object, field));
            if (runtime_discovery::is_reference_type(field->type) &&
                field->offset > 0)
            {
                body += ",\"reference\":" +
                    pointer_json(runtime_discovery::read_reference_field(
                        object, field));
            }
        }
        return body + "}";
    }

    Il2CppObject* object_to_game_object(Il2CppObject* object)
    {
        if (!is_valid_object(object))
        {
            return nullptr;
        }
        if (runtime_discovery::inherits_from(
                object->klass, "UnityEngine", "GameObject"))
        {
            return object;
        }
        if (runtime_discovery::inherits_from(
                object->klass, "UnityEngine", "Component"))
        {
            return unity::component_get_game_object(object);
        }
        return nullptr;
    }
}
