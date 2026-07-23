#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <windows.h>

#include "exports.h"
#include "il2utils.h"
#include "unity.h"

namespace runtime_discovery
{
    using image_get_class_count_sig = std::size_t(*)(const Il2CppImage*);
    using image_get_class_sig = Il2CppClass*(*)(const Il2CppImage*, std::size_t);
    using find_objects_of_type_all_sig = Il2CppArray*(*)(Il2CppObject*);

    struct RuntimeApi
    {
        bool initialized = false;
        image_get_class_count_sig image_get_class_count = nullptr;
        image_get_class_sig image_get_class = nullptr;
        find_objects_of_type_all_sig find_objects_of_type_all = nullptr;
    };

    struct RankedClass
    {
        Il2CppClass* klass = nullptr;
        int score = -1;
    };

    struct ClassSearchResult
    {
        std::size_t image_class_count = 0;
        std::size_t total_matches = 0;
        std::vector<RankedClass> classes;
    };

    inline RuntimeApi& api()
    {
        static RuntimeApi runtime_api;
        return runtime_api;
    }

    inline bool initialize()
    {
        RuntimeApi& runtime_api = api();
        if (runtime_api.initialized)
        {
            return runtime_api.image_get_class_count != nullptr &&
                runtime_api.image_get_class != nullptr &&
                runtime_api.find_objects_of_type_all != nullptr;
        }

        runtime_api.initialized = true;
        const HMODULE game_assembly = GetModuleHandleA("GameAssembly.dll");
        if (game_assembly != nullptr)
        {
            runtime_api.image_get_class_count =
                reinterpret_cast<image_get_class_count_sig>(
                    GetProcAddress(game_assembly, "il2cpp_image_get_class_count"));
            runtime_api.image_get_class = reinterpret_cast<image_get_class_sig>(
                GetProcAddress(game_assembly, "il2cpp_image_get_class"));
        }

        const Il2CppImage* core_module =
            il2utils::resolve_image("UnityEngine.CoreModule.dll");
        const Il2CppClass* resources_class =
            il2utils::resolve_class(core_module, "UnityEngine", "Resources");
        const MethodInfo* find_all_method = resources_class != nullptr ?
            il2utils::resolve_method(resources_class, "FindObjectsOfTypeAll", 1) :
            nullptr;
        if (find_all_method != nullptr && find_all_method->methodPointer != nullptr)
        {
            runtime_api.find_objects_of_type_all =
                reinterpret_cast<find_objects_of_type_all_sig>(
                    find_all_method->methodPointer);
        }

        return runtime_api.image_get_class_count != nullptr &&
            runtime_api.image_get_class != nullptr &&
            runtime_api.find_objects_of_type_all != nullptr;
    }

    inline std::string to_lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(),
            [](const unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            });
        return value;
    }

    inline std::string class_name(const Il2CppClass* klass)
    {
        if (klass == nullptr)
        {
            return "<null>";
        }

        const std::string namespaze =
            klass->namespaze != nullptr ? klass->namespaze : "";
        const std::string name = klass->name != nullptr ? klass->name : "<unnamed>";
        return namespaze.empty() ? name : namespaze + "." + name;
    }

    inline std::string type_name(const Il2CppType* type)
    {
        if (type == nullptr)
        {
            return "<null>";
        }

        char* raw_name = il2cpp::il2cpp_type_get_name(type);
        if (raw_name == nullptr)
        {
            return "<unknown>";
        }

        const std::string result(raw_name);
        il2cpp::il2cpp_free(raw_name);
        return result;
    }

    inline std::string safe_string(const Il2CppString* value)
    {
        if (value == nullptr || value->length <= 0)
        {
            return "";
        }

        constexpr int max_length = 256;
        const int length = (std::min)(value->length, max_length);
        std::string result;
        result.reserve(static_cast<std::size_t>(length));
        for (int index = 0; index < length; ++index)
        {
            const wchar_t character = value->chars[index];
            result.push_back(character >= 32 && character <= 126 ?
                static_cast<char>(character) : '?');
        }
        if (value->length > max_length)
        {
            result += "...";
        }
        return result;
    }

    inline bool inherits_from(
        const Il2CppClass* klass,
        const char* expected_namespaze,
        const char* expected_name)
    {
        std::size_t depth = 0;
        for (const Il2CppClass* cursor = klass;
             cursor != nullptr && depth < 64;
             cursor = cursor->parent, ++depth)
        {
            const char* namespaze =
                cursor->namespaze != nullptr ? cursor->namespaze : "";
            const char* name = cursor->name != nullptr ? cursor->name : "";
            if (std::strcmp(namespaze, expected_namespaze) == 0 &&
                std::strcmp(name, expected_name) == 0)
            {
                return true;
            }
        }
        return false;
    }

    inline bool is_reference_type(const Il2CppType* type)
    {
        if (type == nullptr)
        {
            return false;
        }

        switch (type->type)
        {
        case IL2CPP_TYPE_STRING:
        case IL2CPP_TYPE_CLASS:
        case IL2CPP_TYPE_OBJECT:
        case IL2CPP_TYPE_ARRAY:
        case IL2CPP_TYPE_SZARRAY:
        case IL2CPP_TYPE_GENERICINST:
            return true;
        default:
            return false;
        }
    }

    inline Il2CppObject* read_reference_field(
        Il2CppObject* object,
        const FieldInfo* field)
    {
        if (object == nullptr || field == nullptr || field->offset <= 0 ||
            !is_reference_type(field->type))
        {
            return nullptr;
        }

        const auto address = reinterpret_cast<std::uintptr_t>(object) + field->offset;
        return *reinterpret_cast<Il2CppObject**>(address);
    }

    inline bool read_int_field(
        Il2CppObject* object,
        const char* name,
        int& value)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        FieldInfo* field = il2utils::resolve_field(object->klass, name);
        if (field == nullptr || field->offset <= 0 || field->type == nullptr ||
            field->type->type != IL2CPP_TYPE_I4)
        {
            return false;
        }

        value = *reinterpret_cast<int*>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
        return true;
    }

    inline bool collection_count(Il2CppObject* object, std::size_t& count)
    {
        count = 0;
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        const Il2CppTypeEnum type = object->klass->byval_arg.type;
        if (type == IL2CPP_TYPE_ARRAY || type == IL2CPP_TYPE_SZARRAY ||
            (object->klass->name != nullptr &&
                std::strstr(object->klass->name, "[]") != nullptr))
        {
            count = static_cast<std::size_t>(
                reinterpret_cast<Il2CppArray*>(object)->max_length);
            return true;
        }

        int signed_count = 0;
        if (read_int_field(object, "_size", signed_count) ||
            read_int_field(object, "_count", signed_count))
        {
            if (signed_count < 0)
            {
                return false;
            }
            count = static_cast<std::size_t>(signed_count);
            return true;
        }
        return false;
    }

    inline bool collection_elements(
        Il2CppObject* object,
        Il2CppArray*& items,
        std::size_t& count)
    {
        items = nullptr;
        count = 0;
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        const Il2CppTypeEnum type = object->klass->byval_arg.type;
        if (type == IL2CPP_TYPE_ARRAY || type == IL2CPP_TYPE_SZARRAY ||
            (object->klass->name != nullptr &&
                std::strstr(object->klass->name, "[]") != nullptr))
        {
            items = reinterpret_cast<Il2CppArray*>(object);
            count = static_cast<std::size_t>(items->max_length);
            return true;
        }

        FieldInfo* items_field = il2utils::resolve_field(object->klass, "_items");
        int signed_count = 0;
        if (items_field == nullptr || items_field->offset <= 0 ||
            items_field->type == nullptr ||
            (items_field->type->type != IL2CPP_TYPE_ARRAY &&
                items_field->type->type != IL2CPP_TYPE_SZARRAY) ||
            !read_int_field(object, "_size", signed_count) || signed_count < 0)
        {
            return false;
        }

        items = *reinterpret_cast<Il2CppArray**>(
            reinterpret_cast<std::uintptr_t>(object) + items_field->offset);
        if (items == nullptr)
        {
            return false;
        }
        count = (std::min)(
            static_cast<std::size_t>(signed_count),
            static_cast<std::size_t>(items->max_length));
        return true;
    }

    inline std::string field_value_summary(
        Il2CppObject* object,
        const FieldInfo* field)
    {
        if (object == nullptr || field == nullptr || field->offset <= 0 ||
            field->type == nullptr)
        {
            return "<unavailable>";
        }

        const auto address = reinterpret_cast<std::uintptr_t>(object) + field->offset;
        switch (field->type->type)
        {
        case IL2CPP_TYPE_BOOLEAN:
            return *reinterpret_cast<bool*>(address) ? "true" : "false";
        case IL2CPP_TYPE_I4:
            return std::to_string(*reinterpret_cast<int*>(address));
        case IL2CPP_TYPE_U4:
            return std::to_string(*reinterpret_cast<std::uint32_t*>(address));
        case IL2CPP_TYPE_I8:
            return std::to_string(*reinterpret_cast<std::int64_t*>(address));
        case IL2CPP_TYPE_U8:
            return std::to_string(*reinterpret_cast<std::uint64_t*>(address));
        case IL2CPP_TYPE_R4:
            return std::to_string(*reinterpret_cast<float*>(address));
        case IL2CPP_TYPE_R8:
            return std::to_string(*reinterpret_cast<double*>(address));
        case IL2CPP_TYPE_STRING:
        {
            const auto* value = *reinterpret_cast<Il2CppString**>(address);
            return value != nullptr ? "'" + safe_string(value) + "'" : "null";
        }
        default:
            break;
        }

        if (!is_reference_type(field->type))
        {
            return "<unread>";
        }

        Il2CppObject* value = *reinterpret_cast<Il2CppObject**>(address);
        if (value == nullptr)
        {
            return "null";
        }

        std::string summary = "class='" + class_name(value->klass) + "'";
        std::size_t count = 0;
        if (collection_count(value, count))
        {
            summary += " count=" + std::to_string(count);
        }
        return summary;
    }

    inline std::vector<FieldInfo*> collect_fields(
        Il2CppClass* klass,
        const std::size_t max_fields,
        const std::size_t max_depth = 8)
    {
        std::vector<FieldInfo*> fields;
        std::size_t depth = 0;
        for (Il2CppClass* cursor = klass;
             cursor != nullptr && depth < max_depth && fields.size() < max_fields;
             cursor = cursor->parent, ++depth)
        {
            if (cursor->fields == nullptr)
            {
                continue;
            }

            for (std::uint16_t index = 0;
                 index < cursor->field_count && fields.size() < max_fields;
                 ++index)
            {
                fields.push_back(&cursor->fields[index]);
            }
        }
        return fields;
    }

    inline std::vector<const MethodInfo*> collect_methods(
        Il2CppClass* klass,
        const std::size_t max_methods,
        const std::size_t max_depth = 8)
    {
        std::vector<const MethodInfo*> methods;
        std::size_t depth = 0;
        for (Il2CppClass* cursor = klass;
             cursor != nullptr && depth < max_depth && methods.size() < max_methods;
             cursor = cursor->parent, ++depth)
        {
            if (cursor->methods == nullptr)
            {
                continue;
            }

            for (std::uint16_t index = 0;
                 index < cursor->method_count && methods.size() < max_methods;
                 ++index)
            {
                const MethodInfo* method = cursor->methods[index];
                if (method != nullptr)
                {
                    methods.push_back(method);
                }
            }
        }
        return methods;
    }

    inline ClassSearchResult search_image_classes(
        const Il2CppImage* image,
        int(*score_class)(const Il2CppClass*),
        const std::size_t max_results)
    {
        ClassSearchResult result;
        RuntimeApi& runtime_api = api();
        if (image == nullptr || score_class == nullptr ||
            runtime_api.image_get_class_count == nullptr ||
            runtime_api.image_get_class == nullptr)
        {
            return result;
        }

        constexpr std::size_t max_image_classes = 100000;
        result.image_class_count = (std::min)(
            runtime_api.image_get_class_count(image), max_image_classes);
        std::vector<RankedClass> matches;
        matches.reserve((std::min)(result.image_class_count, max_results * 2));
        for (std::size_t index = 0; index < result.image_class_count; ++index)
        {
            Il2CppClass* klass = runtime_api.image_get_class(image, index);
            const int score = score_class(klass);
            if (klass == nullptr || score < 0)
            {
                continue;
            }
            ++result.total_matches;
            matches.push_back({ klass, score });
        }

        std::sort(matches.begin(), matches.end(),
            [](const RankedClass& left, const RankedClass& right)
            {
                if (left.score != right.score)
                {
                    return left.score < right.score;
                }
                return class_name(left.klass) < class_name(right.klass);
            });
        if (matches.size() > max_results)
        {
            matches.resize(max_results);
        }
        result.classes = std::move(matches);
        return result;
    }

    inline Il2CppArray* find_all_objects(Il2CppClass* klass)
    {
        RuntimeApi& runtime_api = api();
        if (klass == nullptr || runtime_api.find_objects_of_type_all == nullptr ||
            !inherits_from(klass, "UnityEngine", "Object"))
        {
            return nullptr;
        }

        const Il2CppType* type = il2cpp::il2cpp_class_get_type(klass);
        Il2CppObject* system_type = type != nullptr ?
            il2cpp::il2cpp_type_get_object(type) : nullptr;
        return system_type != nullptr ?
            runtime_api.find_objects_of_type_all(system_type) : nullptr;
    }

    inline std::string object_name(Il2CppObject* object)
    {
        if (object == nullptr || unity::get_name_internal == nullptr ||
            !inherits_from(object->klass, "UnityEngine", "Object"))
        {
            return "";
        }

        Il2CppString* name = unity::get_name_internal(object);
        return safe_string(name);
    }
}
