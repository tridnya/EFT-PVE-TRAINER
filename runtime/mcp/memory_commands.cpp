#include "memory_commands.h"

#include <Windows.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "inspector_support.h"
#include "../../il2cpp/il2utils.h"
#include "../../il2cpp/runtime_discovery.h"

namespace runtime::mcp::memory_commands
{
    template <typename T>
    bool write_numeric_value(
        const std::uintptr_t address,
        const T value)
    {
        if (!inspector_support::is_writable_range(
                reinterpret_cast<void*>(address), sizeof(T)))
        {
            return false;
        }
        *reinterpret_cast<T*>(address) = value;
        return true;
    }

    std::string write_field(BinaryReader& reader)
    {
        using namespace inspector_support;

        std::uint64_t raw_address = 0;
        std::string field_name;
        ValueKind value_kind = ValueKind::boolean;
        if (!reader.read(raw_address) ||
            !reader.read_string(field_name) ||
            !reader.read(value_kind))
        {
            return error("invalid write_field payload");
        }

        auto* object = reinterpret_cast<Il2CppObject*>(
            static_cast<std::uintptr_t>(raw_address));
        if (!is_valid_object(object))
        {
            return error("object address is not readable IL2CPP data");
        }

        FieldInfo* field = il2utils::resolve_field(
            object->klass, field_name.c_str());
        if (field == nullptr || field->offset <= 0 || field->type == nullptr)
        {
            return error("field is missing, static, or not writable");
        }

        const std::string before =
            runtime_discovery::field_value_summary(object, field);
        const std::uintptr_t field_address =
            reinterpret_cast<std::uintptr_t>(object) + field->offset;
        bool wrote = false;

        if (value_kind == ValueKind::boolean)
        {
            std::uint8_t raw_value = 0;
            if (!reader.read(raw_value) ||
                field->type->type != IL2CPP_TYPE_BOOLEAN)
            {
                return error("boolean value does not match field type");
            }
            wrote = write_numeric_value<bool>(
                field_address, raw_value != 0);
        }
        else if (value_kind == ValueKind::signed_integer)
        {
            std::int64_t value = 0;
            if (!reader.read(value))
            {
                return error("signed integer value is missing");
            }
            switch (field->type->type)
            {
            case IL2CPP_TYPE_I1:
                wrote = write_numeric_value<std::int8_t>(
                    field_address, static_cast<std::int8_t>(value));
                break;
            case IL2CPP_TYPE_I2:
                wrote = write_numeric_value<std::int16_t>(
                    field_address, static_cast<std::int16_t>(value));
                break;
            case IL2CPP_TYPE_I4:
                wrote = write_numeric_value<std::int32_t>(
                    field_address, static_cast<std::int32_t>(value));
                break;
            case IL2CPP_TYPE_I8:
                wrote = write_numeric_value<std::int64_t>(
                    field_address, value);
                break;
            default:
                return error("signed integer value does not match field type");
            }
        }
        else if (value_kind == ValueKind::unsigned_integer)
        {
            std::uint64_t value = 0;
            if (!reader.read(value))
            {
                return error("unsigned integer value is missing");
            }
            switch (field->type->type)
            {
            case IL2CPP_TYPE_U1:
                wrote = write_numeric_value<std::uint8_t>(
                    field_address, static_cast<std::uint8_t>(value));
                break;
            case IL2CPP_TYPE_U2:
                wrote = write_numeric_value<std::uint16_t>(
                    field_address, static_cast<std::uint16_t>(value));
                break;
            case IL2CPP_TYPE_U4:
                wrote = write_numeric_value<std::uint32_t>(
                    field_address, static_cast<std::uint32_t>(value));
                break;
            case IL2CPP_TYPE_U8:
                wrote = write_numeric_value<std::uint64_t>(
                    field_address, value);
                break;
            default:
                return error(
                    "unsigned integer value does not match field type");
            }
        }
        else if (value_kind == ValueKind::floating_point)
        {
            double value = 0;
            if (!reader.read(value))
            {
                return error("floating-point value is missing");
            }
            if (!std::isfinite(value))
            {
                return error("floating-point value must be finite");
            }
            if (field->type->type == IL2CPP_TYPE_R4)
            {
                wrote = write_numeric_value<float>(
                    field_address, static_cast<float>(value));
            }
            else if (field->type->type == IL2CPP_TYPE_R8)
            {
                wrote = write_numeric_value<double>(field_address, value);
            }
            else
            {
                return error(
                    "floating-point value does not match field type");
            }
        }
        else
        {
            return error("unsupported write value kind");
        }

        if (!wrote)
        {
            return error("field memory is not writable");
        }

        const std::string after =
            runtime_discovery::field_value_summary(object, field);
        return success("{\"object\":" + pointer_json(object) +
            ",\"field\":" + json_string(field_name) +
            ",\"before\":" + json_string(before) +
            ",\"after\":" + json_string(after) + "}");
    }

    std::string read_memory(BinaryReader& reader)
    {
        using namespace inspector_support;

        std::uint64_t raw_address = 0;
        std::uint32_t requested_size = 0;
        if (!reader.read(raw_address) || !reader.read(requested_size))
        {
            return error("invalid read_memory payload");
        }

        const std::uint32_t size = (std::min)(
            requested_size, maximum_memory_read);
        if (size == 0)
        {
            return error("read size must be greater than zero");
        }

        const auto* address = reinterpret_cast<const void*>(
            static_cast<std::uintptr_t>(raw_address));
        if (!is_readable_range(address, size))
        {
            return error("requested memory range is not readable");
        }

        std::vector<std::uint8_t> bytes(size);
        SIZE_T bytes_read = 0;
        if (!ReadProcessMemory(
                GetCurrentProcess(),
                address,
                bytes.data(),
                bytes.size(),
                &bytes_read))
        {
            return error("ReadProcessMemory failed");
        }

        std::ostringstream hex;
        hex << std::hex << std::setfill('0');
        for (std::size_t index = 0; index < bytes_read; ++index)
        {
            hex << std::setw(2) << static_cast<unsigned>(bytes[index]);
        }

        return success("{\"address\":" + pointer_json(address) +
            ",\"size\":" + std::to_string(bytes_read) +
            ",\"hex\":" + json_string(hex.str()) + "}");
    }
}
