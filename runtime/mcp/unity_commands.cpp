#include "unity_commands.h"

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

namespace runtime::mcp::unity_commands
{
    std::string find_objects(BinaryReader& reader)
    {
        using namespace inspector_support;

        std::string image_name;
        std::string namespaze;
        std::string class_name;
        std::string name_filter;
        std::uint32_t requested_limit = 0;
        if (!reader.read_string(image_name) ||
            !reader.read_string(namespaze) ||
            !reader.read_string(class_name) ||
            !reader.read_string(name_filter) ||
            !reader.read(requested_limit))
        {
            return error("invalid find_objects payload");
        }

        Il2CppImage* image = find_image(image_name);
        if (image == nullptr)
        {
            return error("assembly image not found: " + image_name);
        }
        Il2CppClass* klass = const_cast<Il2CppClass*>(
            il2utils::resolve_class(
                image, namespaze.c_str(), class_name.c_str()));
        if (!is_valid_class(klass))
        {
            return error("class not found: " + namespaze + "." + class_name);
        }
        if (!runtime_discovery::initialize())
        {
            return error("runtime discovery API is unavailable");
        }

        Il2CppArray* objects = runtime_discovery::find_all_objects(klass);
        if (objects == nullptr || !is_readable_range(objects, sizeof(Il2CppArray)))
        {
            return error("Unity returned no live object array for the class");
        }

        const std::size_t object_count =
            static_cast<std::size_t>(objects->max_length);
        const std::string lowered_filter =
            runtime_discovery::to_lower(name_filter);
        const std::uint32_t limit = bounded_limit(requested_limit, 100);
        auto** entries = reinterpret_cast<Il2CppObject**>(&objects->data);
        std::size_t match_count = 0;
        std::string values = "[";
        bool first = true;
        for (std::size_t index = 0; index < object_count; ++index)
        {
            Il2CppObject* object = entries[index];
            if (!is_valid_object(object))
            {
                continue;
            }

            const std::string name = runtime_discovery::object_name(object);
            if (!lowered_filter.empty() &&
                runtime_discovery::to_lower(name).find(lowered_filter) ==
                    std::string::npos)
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
            values += object_summary_json(object);
        }
        values += "]";

        return success("{\"class\":" + class_summary_json(klass) +
            ",\"name_filter\":" + json_string(name_filter) +
            ",\"live_count\":" + std::to_string(object_count) +
            ",\"match_count\":" + std::to_string(match_count) +
            ",\"returned\":" +
            std::to_string((std::min<std::size_t>)(match_count, limit)) +
            ",\"objects\":" + values + "}");
    }

    std::string inspect_object(BinaryReader& reader)
    {
        using namespace inspector_support;

        std::uint64_t raw_address = 0;
        std::uint32_t requested_fields = 0;
        std::uint32_t requested_methods = 0;
        if (!reader.read(raw_address) ||
            !reader.read(requested_fields) ||
            !reader.read(requested_methods))
        {
            return error("invalid inspect_object payload");
        }

        auto* object = reinterpret_cast<Il2CppObject*>(
            static_cast<std::uintptr_t>(raw_address));
        if (!is_valid_object(object))
        {
            return error("object address is not readable IL2CPP data");
        }

        const std::uint32_t field_limit =
            bounded_limit(requested_fields, 128);
        const std::uint32_t method_limit =
            bounded_limit(requested_methods, 64);
        const std::vector<FieldInfo*> fields =
            runtime_discovery::collect_fields(object->klass, field_limit);
        const std::vector<const MethodInfo*> methods =
            runtime_discovery::collect_methods(object->klass, method_limit);

        std::string field_values = "[";
        for (std::size_t index = 0; index < fields.size(); ++index)
        {
            if (index != 0)
            {
                field_values += ",";
            }
            field_values += field_summary_json(object, fields[index], true);
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

        Il2CppObject* game_object = object_to_game_object(object);
        std::string spatial = "null";
        if (is_valid_object(game_object))
        {
            Il2CppObject* transform = unity::gameobject_get_transform(game_object);
            if (is_valid_object(transform))
            {
                const unity::vector3 position = unity::transform_get_pos(transform);
                spatial = "{\"game_object\":" + pointer_json(game_object) +
                    ",\"transform\":" + pointer_json(transform) +
                    ",\"position\":{\"x\":" + std::to_string(position.x) +
                    ",\"y\":" + std::to_string(position.y) +
                    ",\"z\":" + std::to_string(position.z) + "}}";
            }
        }

        return success("{\"object\":" + object_summary_json(object) +
            ",\"spatial\":" + spatial +
            ",\"fields\":" + field_values +
            ",\"methods\":" + method_values + "}");
    }

    std::string read_collection(BinaryReader& reader)
    {
        using namespace inspector_support;

        std::uint64_t raw_address = 0;
        std::uint32_t requested_limit = 0;
        if (!reader.read(raw_address) || !reader.read(requested_limit))
        {
            return error("invalid read_collection payload");
        }

        auto* object = reinterpret_cast<Il2CppObject*>(
            static_cast<std::uintptr_t>(raw_address));
        if (!is_valid_object(object))
        {
            return error("collection address is not readable IL2CPP data");
        }

        std::size_t logical_count = 0;
        if (!runtime_discovery::collection_count(object, logical_count))
        {
            return error(
                "object is not a supported array/list collection; inspect its fields for backing storage");
        }

        Il2CppArray* items = nullptr;
        std::size_t item_count = 0;
        if (!runtime_discovery::collection_elements(
                object, items, item_count) ||
            items == nullptr)
        {
            return success("{\"collection\":" +
                object_summary_json(object) +
                ",\"count\":" + std::to_string(logical_count) +
                ",\"elements_supported\":false,\"items\":[]}");
        }

        Il2CppClass* element_class =
            items->klass != nullptr ? items->klass->element_class : nullptr;
        if (!is_valid_class(element_class) ||
            (element_class->byval_arg.type != IL2CPP_TYPE_CLASS &&
                element_class->byval_arg.type != IL2CPP_TYPE_OBJECT &&
                element_class->byval_arg.type != IL2CPP_TYPE_STRING &&
                element_class->byval_arg.type != IL2CPP_TYPE_GENERICINST))
        {
            return success("{\"collection\":" +
                object_summary_json(object) +
                ",\"count\":" + std::to_string(logical_count) +
                ",\"element_class\":" + class_summary_json(element_class) +
                ",\"elements_supported\":false,\"items\":[]}");
        }

        const std::uint32_t limit = bounded_limit(requested_limit, 100);
        const std::size_t returned =
            (std::min<std::size_t>)(item_count, limit);
        auto** entries = reinterpret_cast<Il2CppObject**>(&items->data);
        std::string values = "[";
        for (std::size_t index = 0; index < returned; ++index)
        {
            if (index != 0)
            {
                values += ",";
            }
            values += object_summary_json(entries[index]);
        }
        values += "]";

        return success("{\"collection\":" +
            object_summary_json(object) +
            ",\"count\":" + std::to_string(logical_count) +
            ",\"backing_count\":" + std::to_string(item_count) +
            ",\"element_class\":" + class_summary_json(element_class) +
            ",\"elements_supported\":true,\"returned\":" +
            std::to_string(returned) + ",\"items\":" + values + "}");
    }

    std::string get_components(BinaryReader& reader)
    {
        using namespace inspector_support;

        std::uint64_t raw_address = 0;
        std::uint32_t requested_limit = 0;
        if (!reader.read(raw_address) || !reader.read(requested_limit))
        {
            return error("invalid get_components payload");
        }

        auto* object = reinterpret_cast<Il2CppObject*>(
            static_cast<std::uintptr_t>(raw_address));
        Il2CppObject* game_object = object_to_game_object(object);
        if (!is_valid_object(game_object))
        {
            return error("address is not a Unity GameObject or Component");
        }

        Il2CppImage* core_module = find_image("UnityEngine.CoreModule.dll");
        Il2CppClass* game_object_class = const_cast<Il2CppClass*>(
            il2utils::resolve_class(
                core_module, "UnityEngine", "GameObject"));
        Il2CppClass* component_class = const_cast<Il2CppClass*>(
            il2utils::resolve_class(
                core_module, "UnityEngine", "Component"));
        const MethodInfo* method = il2utils::resolve_method(
            game_object_class, "GetComponents", 1);
        if (method == nullptr || method->methodPointer == nullptr ||
            component_class == nullptr)
        {
            return error("Unity GetComponents(Type) is unavailable");
        }

        Il2CppObject* component_type = il2cpp::il2cpp_type_get_object(
            il2cpp::il2cpp_class_get_type(component_class));
        using GetComponents = Il2CppArray*(*)(Il2CppObject*, Il2CppObject*);
        const auto get_components =
            reinterpret_cast<GetComponents>(method->methodPointer);
        Il2CppArray* components =
            get_components(game_object, component_type);
        if (components == nullptr)
        {
            return success("{\"game_object\":" +
                object_summary_json(game_object) +
                ",\"count\":0,\"components\":[]}");
        }

        const std::size_t count =
            static_cast<std::size_t>(components->max_length);
        const std::uint32_t limit = bounded_limit(requested_limit, 100);
        const std::size_t returned =
            (std::min<std::size_t>)(count, limit);
        auto** entries = reinterpret_cast<Il2CppObject**>(&components->data);
        std::string values = "[";
        for (std::size_t index = 0; index < returned; ++index)
        {
            if (index != 0)
            {
                values += ",";
            }
            values += object_summary_json(entries[index]);
        }
        values += "]";

        return success("{\"game_object\":" +
            object_summary_json(game_object) +
            ",\"count\":" + std::to_string(count) +
            ",\"returned\":" + std::to_string(returned) +
            ",\"components\":" + values + "}");
    }

    void append_hierarchy_node(
        Il2CppObject* transform,
        const std::uint32_t depth,
        const std::uint32_t maximum_depth,
        const std::uint32_t maximum_nodes,
        std::uint32_t& visited,
        std::string& values)
    {
        using namespace inspector_support;

        if (!is_valid_object(transform) ||
            depth > maximum_depth ||
            visited >= maximum_nodes)
        {
            values += "null";
            return;
        }

        ++visited;
        Il2CppObject* game_object =
            unity::transform_get_game_object(transform);
        const unity::vector3 position = unity::transform_get_pos(transform);
        const int child_count = (std::max)(
            0, unity::transform_get_child_count(transform));

        values += "{\"transform\":" + pointer_json(transform) +
            ",\"game_object\":" + object_summary_json(game_object) +
            ",\"position\":{\"x\":" + std::to_string(position.x) +
            ",\"y\":" + std::to_string(position.y) +
            ",\"z\":" + std::to_string(position.z) + "}" +
            ",\"child_count\":" + std::to_string(child_count) +
            ",\"children\":[";

        bool first = true;
        for (int index = 0;
             index < child_count &&
             depth < maximum_depth &&
             visited < maximum_nodes;
             ++index)
        {
            Il2CppObject* child =
                unity::transform_get_child(transform, index);
            if (!first)
            {
                values += ",";
            }
            first = false;
            append_hierarchy_node(
                child,
                depth + 1,
                maximum_depth,
                maximum_nodes,
                visited,
                values);
        }
        values += "]}";
    }

    std::string get_hierarchy(BinaryReader& reader)
    {
        using namespace inspector_support;

        std::uint64_t raw_address = 0;
        std::uint32_t requested_depth = 0;
        std::uint32_t requested_nodes = 0;
        if (!reader.read(raw_address) ||
            !reader.read(requested_depth) ||
            !reader.read(requested_nodes))
        {
            return error("invalid get_hierarchy payload");
        }

        auto* object = reinterpret_cast<Il2CppObject*>(
            static_cast<std::uintptr_t>(raw_address));
        Il2CppObject* game_object = object_to_game_object(object);
        if (!is_valid_object(game_object))
        {
            return error("address is not a Unity GameObject or Component");
        }

        Il2CppObject* transform = unity::gameobject_get_transform(game_object);
        if (!is_valid_object(transform))
        {
            return error("GameObject transform is unavailable");
        }

        const std::uint32_t depth = (std::min)(
            requested_depth == 0 ? 3u : requested_depth,
            maximum_hierarchy_depth);
        const std::uint32_t node_limit =
            bounded_limit(requested_nodes, 200);
        std::uint32_t visited = 0;
        std::string tree;
        append_hierarchy_node(
            transform, 0, depth, node_limit, visited, tree);
        return success("{\"visited\":" + std::to_string(visited) +
            ",\"depth_limit\":" + std::to_string(depth) +
            ",\"node_limit\":" + std::to_string(node_limit) +
            ",\"root\":" + tree + "}");
    }
}
