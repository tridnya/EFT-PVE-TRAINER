#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include "../Module.h"
#include "../ModuleCategory.h"
#include "../../config/ActionRowValue.h"
#include "../../il2cpp/runtime_discovery.h"
#include "../../util/logger.h"

class AssetDiscoveryModule : Module
{
public:
    AssetDiscoveryModule() : Module("Asset Discovery", Visuals)
    {
    }

    ActionRowValue* discover_loaded_assets = conf(new ActionRowValue([this]
    {
        discovery_requested_.store(true, std::memory_order_release);
        logger::info("[AssetDiscovery] queued bounded loaded-asset discovery");
    }, "Discover Loaded Assets"));

    void draw_overlay(ImDrawList* draw_list) override
    {
        (void)draw_list;
    }

    void application_update() override
    {
        if (discovery_requested_.exchange(false, std::memory_order_acq_rel))
        {
            run_discovery();
        }
    }

    void gameworld_update(
        const Il2CppClass* game_world_class,
        Il2CppObjectInstance game_world_instance,
        Il2CppObjectInstance main_player) override
    {
        (void)game_world_class;
        (void)game_world_instance;
        (void)main_player;
    }

    void init() override
    {
        core_module_ = il2utils::resolve_image("UnityEngine.CoreModule.dll");
        asset_bundle_module_ =
            il2utils::resolve_image("UnityEngine.AssetBundleModule.dll");
        shader_class_ = const_cast<Il2CppClass*>(il2utils::resolve_class(
            core_module_, "UnityEngine", "Shader"));
        material_class_ = const_cast<Il2CppClass*>(il2utils::resolve_class(
            core_module_, "UnityEngine", "Material"));
        texture_2d_class_ = const_cast<Il2CppClass*>(il2utils::resolve_class(
            core_module_, "UnityEngine", "Texture2D"));
        asset_bundle_class_ = const_cast<Il2CppClass*>(il2utils::resolve_class(
            asset_bundle_module_, "UnityEngine", "AssetBundle"));

        const bool ready = runtime_discovery::initialize() &&
            shader_class_ != nullptr && material_class_ != nullptr &&
            texture_2d_class_ != nullptr && asset_bundle_class_ != nullptr;
        logger::info("[AssetDiscovery] ready=" + std::to_string(ready ? 1 : 0));
    }

private:
    static constexpr std::size_t max_loaded_bundles_ = 64;
    static constexpr std::size_t max_bundles_with_asset_names_ = 32;
    static constexpr std::size_t max_asset_names_per_bundle_ = 96;
    static constexpr std::size_t max_asset_names_total_ = 768;
    static constexpr std::size_t max_shader_objects_ = 256;
    static constexpr std::size_t max_detailed_shaders_ = 48;
    static constexpr std::size_t max_shader_properties_ = 32;
    static constexpr std::size_t max_material_objects_ = 384;
    static constexpr std::size_t max_texture_objects_ = 128;
    static constexpr std::size_t max_streaming_asset_files_ = 256;

    std::atomic_bool discovery_requested_ = false;
    const Il2CppImage* core_module_ = nullptr;
    const Il2CppImage* asset_bundle_module_ = nullptr;
    Il2CppClass* shader_class_ = nullptr;
    Il2CppClass* material_class_ = nullptr;
    Il2CppClass* texture_2d_class_ = nullptr;
    Il2CppClass* asset_bundle_class_ = nullptr;

    static const MethodInfo* find_method(
        Il2CppClass* klass,
        const char* name,
        const std::uint8_t parameter_count,
        const Il2CppTypeEnum first_parameter_type = IL2CPP_TYPE_END)
    {
        const auto methods = runtime_discovery::collect_methods(klass, 512, 8);
        for (const MethodInfo* method : methods)
        {
            if (method == nullptr || method->name == nullptr ||
                std::strcmp(method->name, name) != 0 ||
                method->parameters_count != parameter_count ||
                method->methodPointer == nullptr)
            {
                continue;
            }
            if (parameter_count != 0 && first_parameter_type != IL2CPP_TYPE_END)
            {
                if (method->parameters == nullptr || method->parameters[0] == nullptr ||
                    method->parameters[0]->type != first_parameter_type)
                {
                    continue;
                }
            }
            return method;
        }
        return nullptr;
    }

    static std::string candidate_style(const std::string& name)
    {
        const std::string value = runtime_discovery::to_lower(name);
        if (value.find("holo") != std::string::npos)
        {
            return "holographic";
        }
        if (value.find("irides") != std::string::npos ||
            value.find("pearles") != std::string::npos)
        {
            return "iridescent";
        }
        if (value.find("metal") != std::string::npos ||
            value.find("chrome") != std::string::npos)
        {
            return "metallic";
        }
        if (value.find("emiss") != std::string::npos ||
            value.find("glow") != std::string::npos ||
            value.find("unlit") != std::string::npos)
        {
            return "emissive";
        }
        if (value.find("thermal") != std::string::npos ||
            value.find("heat") != std::string::npos)
        {
            return "thermal";
        }
        if (value.find("glass") != std::string::npos ||
            value.find("transluc") != std::string::npos ||
            value.find("transparent") != std::string::npos)
        {
            return "translucent_glass";
        }
        if (value.find("wire") != std::string::npos)
        {
            return "wireframe";
        }
        if (value.find("grid") != std::string::npos ||
            value.find("checker") != std::string::npos ||
            value.find("stripe") != std::string::npos)
        {
            return "pattern";
        }
        if (value.find("scan") != std::string::npos)
        {
            return "scanline";
        }
        if (value.find("energy") != std::string::npos ||
            value.find("forcefield") != std::string::npos ||
            value.find("noise") != std::string::npos ||
            value.find("distort") != std::string::npos)
        {
            return "animated_energy_noise";
        }
        return "";
    }

    static std::string asset_type_hint(const std::string& asset_name)
    {
        const std::string value = runtime_discovery::to_lower(asset_name);
        if (value.ends_with(".shader"))
        {
            return "Shader";
        }
        if (value.ends_with(".mat"))
        {
            return "Material";
        }
        if (value.ends_with(".prefab"))
        {
            return "GameObject_prefab";
        }
        if (value.ends_with(".png") || value.ends_with(".tga") ||
            value.ends_with(".dds") || value.ends_with(".exr") ||
            value.ends_with(".texture2d"))
        {
            return "Texture";
        }
        return "unknown_extension_hint";
    }

    static void log_disk_asset_file(
        const std::filesystem::path& path,
        const char* category)
    {
        std::error_code error;
        const std::uintmax_t size = std::filesystem::file_size(path, error);
        logger::info("[AssetDiskFile] category='" + std::string(category) +
            "' path='" + path.string() +
            "' size=" + std::to_string(error ? 0 : size));
    }

    static void dump_disk_asset_files()
    {
        std::array<char, 32768> executable_path{};
        const DWORD length = GetModuleFileNameA(
            nullptr,
            executable_path.data(),
            static_cast<DWORD>(executable_path.size()));
        if (length == 0 || length >= executable_path.size())
        {
            logger::warn("[AssetDiskDiscovery] executable path is unavailable");
            return;
        }

        const std::filesystem::path executable(executable_path.data());
        const std::filesystem::path data_directory =
            executable.parent_path() /
            (executable.stem().string() + "_Data");
        std::error_code error;
        if (!std::filesystem::is_directory(data_directory, error))
        {
            logger::warn("[AssetDiskDiscovery] data directory is unavailable path='" +
                data_directory.string() + "'");
            return;
        }

        logger::info("[AssetDiskDiscovery] data_directory='" +
            data_directory.string() + "'");
        constexpr std::array<const char*, 3> known_files{
            "resources.assets", "globalgamemanagers", "globalgamemanagers.assets"
        };
        for (const char* file_name : known_files)
        {
            const std::filesystem::path path = data_directory / file_name;
            if (std::filesystem::is_regular_file(path, error))
            {
                log_disk_asset_file(path, "unity_container");
            }
            error.clear();
        }

        for (std::filesystem::directory_iterator iterator(
                 data_directory,
                 std::filesystem::directory_options::skip_permission_denied,
                 error), end;
             !error && iterator != end;
             iterator.increment(error))
        {
            if (!iterator->is_regular_file(error))
            {
                error.clear();
                continue;
            }
            const std::string file_name =
                runtime_discovery::to_lower(iterator->path().filename().string());
            if (file_name.starts_with("sharedassets") &&
                file_name.ends_with(".assets"))
            {
                log_disk_asset_file(iterator->path(), "sharedassets");
            }
            error.clear();
        }

        const std::filesystem::path streaming_assets =
            data_directory / "StreamingAssets";
        if (!std::filesystem::is_directory(streaming_assets, error))
        {
            logger::info("[AssetDiskDiscovery] StreamingAssets directory not present");
            return;
        }

        std::size_t logged_streaming_files = 0;
        for (std::filesystem::recursive_directory_iterator iterator(
                 streaming_assets,
                 std::filesystem::directory_options::skip_permission_denied,
                 error), end;
             !error && iterator != end &&
                logged_streaming_files < max_streaming_asset_files_;
             iterator.increment(error))
        {
            if (!iterator->is_regular_file(error))
            {
                error.clear();
                continue;
            }
            log_disk_asset_file(iterator->path(), "streaming_asset");
            ++logged_streaming_files;
            error.clear();
        }
        logger::info("[AssetDiskDiscovery] streaming_files_sampled=" +
            std::to_string(logged_streaming_files) +
            " cap=" + std::to_string(max_streaming_asset_files_));
    }

    void dump_loading_capabilities() const
    {
        if (asset_bundle_class_ == nullptr)
        {
            return;
        }

        constexpr std::array<const char*, 9> method_names{
            "LoadFromFile", "LoadFromFileAsync", "LoadFromMemory",
            "LoadFromMemoryAsync", "LoadAsset", "LoadAssetAsync",
            "GetAllAssetNames", "GetAllScenePaths", "GetAllLoadedAssetBundles"
        };
        for (const char* method_name : method_names)
        {
            bool resolved = false;
            const auto methods = runtime_discovery::collect_methods(
                asset_bundle_class_, 512, 8);
            for (const MethodInfo* method : methods)
            {
                if (method != nullptr && method->name != nullptr &&
                    std::strcmp(method->name, method_name) == 0 &&
                    method->methodPointer != nullptr)
                {
                    resolved = true;
                    break;
                }
            }
            logger::info("[AssetLoadingCapability] method='" +
                std::string(method_name) +
                "' resolved=" + std::to_string(resolved ? 1 : 0));
        }
    }

    static bool enumerate_with_managed_enumerator(
        Il2CppObject* enumerable,
        std::vector<Il2CppObject*>& values)
    {
        if (enumerable == nullptr || enumerable->klass == nullptr)
        {
            return false;
        }

        Il2CppArray* direct_items = nullptr;
        std::size_t direct_count = 0;
        if (runtime_discovery::collection_elements(
                enumerable, direct_items, direct_count) && direct_items != nullptr)
        {
            const std::size_t sampled_count = (std::min)(
                direct_count, max_loaded_bundles_);
            auto** direct_values = reinterpret_cast<Il2CppObject**>(&direct_items->data);
            for (std::size_t index = 0; index < sampled_count; ++index)
            {
                if (direct_values[index] != nullptr)
                {
                    values.push_back(direct_values[index]);
                }
            }
            return true;
        }

        const MethodInfo* get_enumerator = find_method(
            enumerable->klass, "GetEnumerator", 0);
        if (get_enumerator == nullptr)
        {
            return false;
        }
        const auto call_get_enumerator =
            reinterpret_cast<Il2CppObject*(*)(Il2CppObject*)>(
                get_enumerator->methodPointer);
        Il2CppObject* enumerator = call_get_enumerator(enumerable);
        if (enumerator == nullptr || enumerator->klass == nullptr)
        {
            return false;
        }

        const MethodInfo* move_next = find_method(
            enumerator->klass, "MoveNext", 0);
        const MethodInfo* get_current = find_method(
            enumerator->klass, "get_Current", 0);
        if (move_next == nullptr || get_current == nullptr)
        {
            return false;
        }

        const auto call_move_next =
            reinterpret_cast<bool(*)(Il2CppObject*)>(move_next->methodPointer);
        const auto call_get_current =
            reinterpret_cast<Il2CppObject*(*)(Il2CppObject*)>(
                get_current->methodPointer);
        while (values.size() < max_loaded_bundles_ && call_move_next(enumerator))
        {
            Il2CppObject* value = call_get_current(enumerator);
            if (value != nullptr)
            {
                values.push_back(value);
            }
        }
        return true;
    }

    void dump_loaded_bundles() const
    {
        const MethodInfo* get_all_loaded = find_method(
            asset_bundle_class_, "GetAllLoadedAssetBundles", 0);
        if (get_all_loaded == nullptr)
        {
            logger::warn("[AssetBundleDiscovery] GetAllLoadedAssetBundles unresolved");
            return;
        }

        const auto call_get_all_loaded =
            reinterpret_cast<Il2CppObject*(*)()>(get_all_loaded->methodPointer);
        Il2CppObject* enumerable = call_get_all_loaded();
        std::vector<Il2CppObject*> bundles;
        const bool enumerated = enumerate_with_managed_enumerator(enumerable, bundles);
        logger::info("[AssetBundleDiscovery] enumerable_class='" +
            runtime_discovery::class_name(
                enumerable != nullptr ? enumerable->klass : nullptr) +
            "' enumerated=" + std::to_string(enumerated ? 1 : 0) +
            " retained=" + std::to_string(bundles.size()));

        std::size_t logged_asset_names = 0;
        const std::size_t bundle_count = (std::min)(
            bundles.size(), max_bundles_with_asset_names_);
        for (std::size_t bundle_index = 0;
             bundle_index < bundle_count &&
                logged_asset_names < max_asset_names_total_;
             ++bundle_index)
        {
            Il2CppObject* bundle = bundles[bundle_index];
            const std::string bundle_name = runtime_discovery::object_name(bundle);
            const MethodInfo* get_asset_names = find_method(
                bundle->klass, "GetAllAssetNames", 0);
            Il2CppArray* asset_names = get_asset_names != nullptr ?
                reinterpret_cast<Il2CppArray*(*)(Il2CppObject*)>(
                    get_asset_names->methodPointer)(bundle) :
                nullptr;
            const std::size_t asset_count = asset_names != nullptr ?
                static_cast<std::size_t>(asset_names->max_length) : 0;
            logger::info("[AssetBundle] #" + std::to_string(bundle_index) +
                " name='" + bundle_name +
                "' class='" + runtime_discovery::class_name(bundle->klass) +
                "' asset_names=" + std::to_string(asset_count));
            if (asset_names == nullptr)
            {
                continue;
            }

            const std::size_t sampled_count = (std::min)(
                asset_count, max_asset_names_per_bundle_);
            auto** names = reinterpret_cast<Il2CppString**>(&asset_names->data);
            for (std::size_t asset_index = 0;
                 asset_index < sampled_count &&
                    logged_asset_names < max_asset_names_total_;
                 ++asset_index, ++logged_asset_names)
            {
                const std::string asset_name =
                    runtime_discovery::safe_string(names[asset_index]);
                logger::info("[AssetBundleAsset] bundle_index=" +
                    std::to_string(bundle_index) +
                    " asset_index=" + std::to_string(asset_index) +
                    " bundle='" + bundle_name +
                    "' name='" + asset_name +
                    "' unity_type_hint='" + asset_type_hint(asset_name) +
                    "' path_id='<runtime_api_unavailable>'");
            }
        }
    }

    static void dump_shader_properties(
        Il2CppObject* shader,
        const std::size_t shader_index)
    {
        const MethodInfo* get_property_count = find_method(
            shader->klass, "GetPropertyCount", 0);
        const MethodInfo* get_property_name = find_method(
            shader->klass, "GetPropertyName", 1, IL2CPP_TYPE_I4);
        const MethodInfo* get_property_type = find_method(
            shader->klass, "GetPropertyType", 1, IL2CPP_TYPE_I4);
        const MethodInfo* get_property_flags = find_method(
            shader->klass, "GetPropertyFlags", 1, IL2CPP_TYPE_I4);
        if (get_property_count == nullptr || get_property_name == nullptr)
        {
            logger::info("[AssetShaderProperties] shader_index=" +
                std::to_string(shader_index) + " unavailable=1");
            return;
        }

        const int property_count = reinterpret_cast<int(*)(Il2CppObject*)>(
            get_property_count->methodPointer)(shader);
        const int sampled_count = (std::min)(
            (std::max)(property_count, 0),
            static_cast<int>(max_shader_properties_));
        bool transparency_evidence = false;
        bool depth_evidence = false;
        bool emission_evidence = false;
        for (int property_index = 0; property_index < sampled_count; ++property_index)
        {
            Il2CppString* property_name_value =
                reinterpret_cast<Il2CppString*(*)(Il2CppObject*, int)>(
                    get_property_name->methodPointer)(shader, property_index);
            const std::string property_name =
                runtime_discovery::safe_string(property_name_value);
            const std::string searchable =
                runtime_discovery::to_lower(property_name);
            transparency_evidence = transparency_evidence ||
                searchable.find("alpha") != std::string::npos ||
                searchable.find("surface") != std::string::npos ||
                searchable.find("blend") != std::string::npos ||
                searchable.find("cutoff") != std::string::npos;
            depth_evidence = depth_evidence ||
                searchable.find("ztest") != std::string::npos ||
                searchable.find("zwrite") != std::string::npos ||
                searchable.find("depth") != std::string::npos;
            emission_evidence = emission_evidence ||
                searchable.find("emission") != std::string::npos ||
                searchable.find("emissive") != std::string::npos ||
                searchable.find("glow") != std::string::npos;

            int property_type = -1;
            int property_flags = -1;
            if (get_property_type != nullptr)
            {
                property_type = reinterpret_cast<int(*)(Il2CppObject*, int)>(
                    get_property_type->methodPointer)(shader, property_index);
            }
            if (get_property_flags != nullptr)
            {
                property_flags = reinterpret_cast<int(*)(Il2CppObject*, int)>(
                    get_property_flags->methodPointer)(shader, property_index);
            }
            logger::info("[AssetShaderProperty] shader_index=" +
                std::to_string(shader_index) +
                " property_index=" + std::to_string(property_index) +
                " name='" + property_name +
                "' type=" + std::to_string(property_type) +
                " flags=" + std::to_string(property_flags));
        }
        logger::info("[AssetShaderCapability] shader_index=" +
            std::to_string(shader_index) +
            " properties=" + std::to_string(property_count) +
            " sampled=" + std::to_string(sampled_count) +
            " transparency_property_evidence=" +
                std::to_string(transparency_evidence ? 1 : 0) +
            " depth_property_evidence=" +
                std::to_string(depth_evidence ? 1 : 0) +
            " emission_property_evidence=" +
                std::to_string(emission_evidence ? 1 : 0));
    }

    void dump_loaded_shaders() const
    {
        Il2CppArray* shaders = runtime_discovery::find_all_objects(shader_class_);
        const std::size_t shader_count = shaders != nullptr ?
            static_cast<std::size_t>(shaders->max_length) : 0;
        const std::size_t sampled_count = (std::min)(
            shader_count, max_shader_objects_);
        logger::info("[AssetShaderDiscovery] total=" +
            std::to_string(shader_count) +
            " sampled=" + std::to_string(sampled_count));
        if (shaders == nullptr)
        {
            return;
        }

        const MethodInfo* get_render_queue = find_method(
            shader_class_, "get_renderQueue", 0);
        auto** values = reinterpret_cast<Il2CppObject**>(&shaders->data);
        std::size_t detailed_count = 0;
        for (std::size_t shader_index = 0;
             shader_index < sampled_count; ++shader_index)
        {
            Il2CppObject* shader = values[shader_index];
            if (shader == nullptr)
            {
                continue;
            }
            const std::string name = runtime_discovery::object_name(shader);
            const std::string style_hint = candidate_style(name);
            int render_queue = -1;
            if (get_render_queue != nullptr)
            {
                render_queue = reinterpret_cast<int(*)(Il2CppObject*)>(
                    get_render_queue->methodPointer)(shader);
            }
            logger::info("[AssetShader] #" + std::to_string(shader_index) +
                " name='" + name +
                "' render_queue=" + std::to_string(render_queue) +
                " candidate_style='" + style_hint +
                "' source_file='<runtime_api_unavailable>'" +
                " path_id='<runtime_api_unavailable>'");

            const bool should_detail = !style_hint.empty() || detailed_count < 8;
            if (should_detail && detailed_count < max_detailed_shaders_)
            {
                dump_shader_properties(shader, shader_index);
                ++detailed_count;
            }
        }
    }

    static bool material_has_property(
        Il2CppObject* material,
        const MethodInfo* has_property,
        const char* property_name)
    {
        if (material == nullptr || has_property == nullptr)
        {
            return false;
        }
        Il2CppString* name = il2cpp::il2cpp_string_new(property_name);
        return reinterpret_cast<bool(*)(Il2CppObject*, Il2CppString*)>(
            has_property->methodPointer)(material, name);
    }

    static float material_float_property(
        Il2CppObject* material,
        const MethodInfo* get_float,
        const char* property_name,
        const bool available)
    {
        if (!available || material == nullptr || get_float == nullptr)
        {
            return -1.0f;
        }
        Il2CppString* name = il2cpp::il2cpp_string_new(property_name);
        return reinterpret_cast<float(*)(Il2CppObject*, Il2CppString*)>(
            get_float->methodPointer)(material, name);
    }

    void dump_loaded_materials() const
    {
        Il2CppArray* materials = runtime_discovery::find_all_objects(material_class_);
        const std::size_t material_count = materials != nullptr ?
            static_cast<std::size_t>(materials->max_length) : 0;
        const std::size_t sampled_count = (std::min)(
            material_count, max_material_objects_);
        logger::info("[AssetMaterialDiscovery] total=" +
            std::to_string(material_count) +
            " sampled=" + std::to_string(sampled_count));
        if (materials == nullptr)
        {
            return;
        }

        const MethodInfo* get_shader = find_method(
            material_class_, "get_shader", 0);
        const MethodInfo* get_render_queue = find_method(
            material_class_, "get_renderQueue", 0);
        const MethodInfo* has_property = find_method(
            material_class_, "HasProperty", 1, IL2CPP_TYPE_STRING);
        const MethodInfo* get_float = find_method(
            material_class_, "GetFloat", 1, IL2CPP_TYPE_STRING);
        auto** values = reinterpret_cast<Il2CppObject**>(&materials->data);
        for (std::size_t material_index = 0;
             material_index < sampled_count; ++material_index)
        {
            Il2CppObject* material = values[material_index];
            if (material == nullptr)
            {
                continue;
            }
            Il2CppObject* shader = get_shader != nullptr ?
                reinterpret_cast<Il2CppObject*(*)(Il2CppObject*)>(
                    get_shader->methodPointer)(material) :
                nullptr;
            const std::string shader_name =
                runtime_discovery::object_name(shader);
            int render_queue = -1;
            if (get_render_queue != nullptr)
            {
                render_queue = reinterpret_cast<int(*)(Il2CppObject*)>(
                    get_render_queue->methodPointer)(material);
            }

            const bool has_surface = material_has_property(
                material, has_property, "_Surface");
            const bool has_mode = material_has_property(
                material, has_property, "_Mode");
            const bool has_alpha = material_has_property(
                material, has_property, "_Alpha");
            const bool has_cutoff = material_has_property(
                material, has_property, "_Cutoff");
            const bool has_z_test = material_has_property(
                material, has_property, "_ZTest");
            const bool has_z_write = material_has_property(
                material, has_property, "_ZWrite");
            const bool has_emission = material_has_property(
                material, has_property, "_EmissionColor") ||
                material_has_property(material, has_property, "_EmissiveColor") ||
                material_has_property(material, has_property, "_EmissionMap");
            const bool transparency_evidence = render_queue >= 2500 ||
                has_surface || has_mode || has_alpha || has_cutoff;
            const float z_test = material_float_property(
                material, get_float, "_ZTest", has_z_test);
            const float z_write = material_float_property(
                material, get_float, "_ZWrite", has_z_write);

            logger::info("[AssetMaterial] #" +
                std::to_string(material_index) +
                " name='" + runtime_discovery::object_name(material) +
                "' shader='" + shader_name +
                "' render_queue=" + std::to_string(render_queue) +
                " candidate_style='" + candidate_style(
                    runtime_discovery::object_name(material) + " " + shader_name) +
                "' transparency_evidence=" +
                    std::to_string(transparency_evidence ? 1 : 0) +
                " depth_controls=" +
                    std::to_string(has_z_test || has_z_write ? 1 : 0) +
                " z_test=" + std::to_string(z_test) +
                " z_write=" + std::to_string(z_write) +
                " emission_support=" + std::to_string(has_emission ? 1 : 0) +
                " source_file='<runtime_api_unavailable>'" +
                " path_id='<runtime_api_unavailable>'");
        }
    }

    void dump_loaded_textures() const
    {
        Il2CppArray* textures = runtime_discovery::find_all_objects(texture_2d_class_);
        const std::size_t texture_count = textures != nullptr ?
            static_cast<std::size_t>(textures->max_length) : 0;
        const std::size_t sampled_count = (std::min)(
            texture_count, max_texture_objects_);
        logger::info("[AssetTextureDiscovery] total=" +
            std::to_string(texture_count) +
            " sampled=" + std::to_string(sampled_count));
        if (textures == nullptr)
        {
            return;
        }

        const MethodInfo* get_width = find_method(
            texture_2d_class_, "get_width", 0);
        const MethodInfo* get_height = find_method(
            texture_2d_class_, "get_height", 0);
        const MethodInfo* get_format = find_method(
            texture_2d_class_, "get_format", 0);
        const MethodInfo* get_is_readable = find_method(
            texture_2d_class_, "get_isReadable", 0);
        auto** values = reinterpret_cast<Il2CppObject**>(&textures->data);
        for (std::size_t texture_index = 0;
             texture_index < sampled_count; ++texture_index)
        {
            Il2CppObject* texture = values[texture_index];
            if (texture == nullptr)
            {
                continue;
            }
            const int width = get_width != nullptr ?
                reinterpret_cast<int(*)(Il2CppObject*)>(
                    get_width->methodPointer)(texture) : -1;
            const int height = get_height != nullptr ?
                reinterpret_cast<int(*)(Il2CppObject*)>(
                    get_height->methodPointer)(texture) : -1;
            const int format = get_format != nullptr ?
                reinterpret_cast<int(*)(Il2CppObject*)>(
                    get_format->methodPointer)(texture) : -1;
            const bool readable = get_is_readable != nullptr &&
                reinterpret_cast<bool(*)(Il2CppObject*)>(
                    get_is_readable->methodPointer)(texture);
            logger::info("[AssetTexture] #" +
                std::to_string(texture_index) +
                " name='" + runtime_discovery::object_name(texture) +
                "' width=" + std::to_string(width) +
                " height=" + std::to_string(height) +
                " format=" + std::to_string(format) +
                " readable=" + std::to_string(readable ? 1 : 0) +
                " source_file='<runtime_api_unavailable>'" +
                " path_id='<runtime_api_unavailable>'");
        }
    }

    void run_discovery() const
    {
        if (!runtime_discovery::initialize() || shader_class_ == nullptr ||
            material_class_ == nullptr || texture_2d_class_ == nullptr ||
            asset_bundle_class_ == nullptr)
        {
            logger::warn("[AssetDiscovery] required runtime APIs are unavailable");
            return;
        }

        logger::info(
            "[AssetDiscovery] begin mutation=none serialized_path_id=unavailable "
            "runtime_source_file=unavailable");
        dump_loading_capabilities();
        dump_disk_asset_files();
        dump_loaded_bundles();
        dump_loaded_shaders();
        dump_loaded_materials();
        dump_loaded_textures();
        logger::info(
            "[AssetDiscovery] complete note='no bundle, shader, material, texture, or prefab was loaded or created'");
    }
};
