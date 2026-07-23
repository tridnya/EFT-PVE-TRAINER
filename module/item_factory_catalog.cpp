#include "item_factory_catalog.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "../il2cpp/il2utils.h"
#include "../il2cpp/unity.h"

namespace item_factory_catalog::detail
{
    constexpr std::size_t object_header_size = sizeof(Il2CppObject);
    constexpr std::size_t max_catalog_entries = 20000;

    struct MongoIdValue
    {
        std::uint32_t time_stamp = 0;
        std::uint32_t alignment = 0;
        std::uint64_t counter = 0;
        Il2CppString* string_id = nullptr;
    };

    static_assert(sizeof(MongoIdValue) == 24);

    using ItemTemplateDictionary = unity::dict<MongoIdValue, Il2CppObject*>;

    [[nodiscard]] bool is_readable_range(const void* address, const std::size_t size)
    {
        if (!address || size == 0)
        {
            return false;
        }

        const std::uintptr_t begin = reinterpret_cast<std::uintptr_t>(address);
        if (begin > UINTPTR_MAX - size)
        {
            return false;
        }

        const std::uintptr_t end = begin + size;
        std::uintptr_t cursor = begin;
        while (cursor < end)
        {
            MEMORY_BASIC_INFORMATION memory{};
            if (VirtualQuery(reinterpret_cast<const void*>(cursor), &memory, sizeof(memory)) == 0 ||
                memory.State != MEM_COMMIT ||
                (memory.Protect & (PAGE_GUARD | PAGE_NOACCESS)) != 0)
            {
                return false;
            }

            const std::uintptr_t region_begin = reinterpret_cast<std::uintptr_t>(memory.BaseAddress);
            if (region_begin > UINTPTR_MAX - memory.RegionSize)
            {
                return false;
            }

            const std::uintptr_t region_end = region_begin + memory.RegionSize;
            if (region_end <= cursor)
            {
                return false;
            }
            cursor = (std::min)(region_end, end);
        }

        return true;
    }

    [[nodiscard]] std::string class_name(const Il2CppClass* klass)
    {
        if (!klass)
        {
            return "<null>";
        }

        const std::string namespaze = klass->namespaze ? klass->namespaze : "";
        const std::string name = klass->name ? klass->name : "<unknown>";
        return namespaze.empty() ? name : namespaze + "." + name;
    }

    [[nodiscard]] const FieldInfo* find_field(Il2CppClass* klass, const char* field_name)
    {
        for (Il2CppClass* cursor = klass; cursor; cursor = cursor->parent)
        {
            if (!cursor->fields)
            {
                continue;
            }

            for (std::uint16_t index = 0; index < cursor->field_count; ++index)
            {
                const FieldInfo& field = cursor->fields[index];
                if (field.name && std::strcmp(field.name, field_name) == 0)
                {
                    return &field;
                }
            }
        }
        return nullptr;
    }

    [[nodiscard]] const FieldInfo* item_templates_field(Il2CppClass* klass)
    {
        constexpr std::array<const char*, 2> field_names{
            "ItemTemplates",
            "<ItemTemplates>k__BackingField"
        };
        for (const char* field_name : field_names)
        {
            if (const FieldInfo* field = find_field(klass, field_name))
            {
                return field;
            }
        }
        return nullptr;
    }

    [[nodiscard]] std::string safe_string(Il2CppString* value)
    {
        if (!is_readable_range(value, offsetof(Il2CppString, chars)) ||
            value->length <= 0 || value->length > 4096)
        {
            return {};
        }

        const std::size_t byte_count = offsetof(Il2CppString, chars) +
            static_cast<std::size_t>(value->length) * sizeof(value->chars[0]);
        if (!is_readable_range(value, byte_count))
        {
            return {};
        }

        std::string result;
        result.reserve(static_cast<std::size_t>(value->length));
        for (int index = 0; index < value->length; ++index)
        {
            const wchar_t character = value->chars[index];
            result.push_back(character <= 0x7f ? static_cast<char>(character) : '?');
        }
        return result;
    }

    [[nodiscard]] std::string mongo_id_string(const MongoIdValue& value)
    {
        const std::string cached = safe_string(value.string_id);
        if (!cached.empty())
        {
            return cached;
        }

        std::ostringstream stream;
        stream << std::hex << std::nouppercase << std::setfill('0')
            << std::setw(8) << value.time_stamp
            << std::setw(16) << value.counter;
        return stream.str();
    }

    [[nodiscard]] std::string read_string(Il2CppObject* object, const char* field_name)
    {
        if (!is_readable_range(object, sizeof(Il2CppObject)) || !object->klass)
        {
            return {};
        }

        const FieldInfo* field = find_field(object->klass, field_name);
        if (!field || !field->type || field->type->type != IL2CPP_TYPE_STRING ||
            field->offset < static_cast<std::int32_t>(object_header_size))
        {
            return {};
        }

        auto** value = reinterpret_cast<Il2CppString**>(
            reinterpret_cast<std::uint8_t*>(object) + field->offset);
        if (!is_readable_range(value, sizeof(*value)))
        {
            return {};
        }
        return safe_string(*value);
    }

    [[nodiscard]] bool is_item_template(Il2CppObject* object)
    {
        if (!is_readable_range(object, sizeof(Il2CppObject)) || !object->klass)
        {
            return false;
        }

        for (Il2CppClass* cursor = object->klass; cursor; cursor = cursor->parent)
        {
            const std::string namespaze = cursor->namespaze ? cursor->namespaze : "";
            const std::string name = cursor->name ? cursor->name : "";
            if (namespaze == "EFT.InventoryLogic" && name == "ItemTemplate")
            {
                return true;
            }
        }
        return false;
    }

    void add_template(CaptureResult& result, Il2CppObject* item_template, const std::string& id)
    {
        if (!is_item_template(item_template) || id.empty() || id.size() > 256 ||
            result.entries.size() >= max_catalog_entries)
        {
            return;
        }

        const std::string name = read_string(item_template, "Name");
        const std::string short_name = read_string(item_template, "ShortName");
        const std::string internal_name = read_string(item_template, "<_name>k__BackingField");
        result.entries.push_back({
            id,
            !name.empty() ? name : (!short_name.empty() ? short_name : internal_name),
            short_name,
            internal_name,
            read_string(item_template, "Description"),
            class_name(item_template->klass),
            {},
            "game"
        });
    }
}

namespace item_factory_catalog
{
    CaptureResult capture(Il2CppObject* item_factory)
    {
        CaptureResult result{};
        if (!detail::is_readable_range(item_factory, sizeof(Il2CppObject)) ||
            !item_factory->klass)
        {
            result.error = "item factory is unavailable";
            return result;
        }

        const FieldInfo* templates_field = detail::item_templates_field(item_factory->klass);
        if (!templates_field || templates_field->offset < static_cast<std::int32_t>(detail::object_header_size))
        {
            result.error = "EFT.ItemFactory.ItemTemplates field was not found";
            return result;
        }

        auto** templates_address = reinterpret_cast<detail::ItemTemplateDictionary**>(
            reinterpret_cast<std::uint8_t*>(item_factory) + templates_field->offset);
        if (!detail::is_readable_range(templates_address, sizeof(*templates_address)))
        {
            result.error = "EFT.ItemFactory.ItemTemplates field address is unreadable";
            return result;
        }

        auto* templates = *templates_address;
        result.scanned_fields = 1;
        if (!detail::is_readable_range(templates, sizeof(detail::ItemTemplateDictionary)) ||
            !templates->m_p_entries || templates->m_i_count < 0 ||
            templates->m_i_count > static_cast<int>(detail::max_catalog_entries))
        {
            result.error = "EFT.ItemFactory.ItemTemplates dictionary is unavailable";
            return result;
        }

        if (!detail::is_readable_range(templates->m_p_entries, offsetof(Il2CppArray, data)))
        {
            result.error = "EFT.ItemFactory.ItemTemplates entries array is unreadable";
            return result;
        }

        const std::size_t array_length = static_cast<std::size_t>(templates->m_p_entries->max_length);
        if (array_length > detail::max_catalog_entries)
        {
            result.error = "EFT.ItemFactory.ItemTemplates entries capacity is implausible";
            return result;
        }

        const std::size_t count = (std::min)(
            static_cast<std::size_t>(templates->m_i_count),
            (std::min)(array_length, detail::max_catalog_entries));
        const auto* entries = templates->get_entry();
        if (!entries || (count != 0 && !detail::is_readable_range(
            entries,
            count * sizeof(detail::ItemTemplateDictionary::entry))))
        {
            result.error = "EFT.ItemFactory.ItemTemplates entries are unavailable";
            return result;
        }

        result.scanned_collections = 1;
        result.entries.reserve(count);
        for (std::size_t index = 0; index < count; ++index)
        {
            const auto& entry = entries[index];
            if (entry.m_i_hash_code < 0 || !entry.m_t_value)
            {
                continue;
            }

            detail::add_template(result, entry.m_t_value, detail::mongo_id_string(entry.m_t_key));
        }

        if (result.entries.empty())
        {
            result.error = "EFT.ItemFactory.ItemTemplates contained no readable ItemTemplate entries";
            return result;
        }

        std::stable_sort(result.entries.begin(), result.entries.end(),
            [](const item_catalog::Entry& left, const item_catalog::Entry& right)
            {
                return left.id < right.id;
            });
        result.diagnostics.push_back(
            "ItemTemplates class=" + detail::class_name(templates->klass) +
            " count=" + std::to_string(templates->m_i_count) +
            " capacity=" + std::to_string(array_length));
        result.success = true;
        return result;
    }
}
