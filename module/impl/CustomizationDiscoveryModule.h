#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>

#include "../Module.h"
#include "../ModuleCategory.h"
#include "../../config/ActionRowValue.h"
#include "../../il2cpp/runtime_discovery.h"
#include "../../util/logger.h"

class CustomizationDiscoveryModule : Module
{
public:
    CustomizationDiscoveryModule() : Module("Customization Discovery", Progression)
    {
    }

    ActionRowValue* discover_customization = conf(new ActionRowValue([this]
    {
        discovery_requested_.store(true, std::memory_order_release);
        logger::info("[CustomizationProbe] queued bounded runtime discovery");
    }, "Discover Customization"));

    ActionRowValue* discover_outfit_authority = conf(new ActionRowValue([this]
    {
        outfit_authority_requested_.store(true, std::memory_order_release);
        logger::info("[OutfitAuthority] queued bounded ownership discovery");
    }, "Discover Outfit Authority"));

    ActionRowValue* discover_full_outfit_catalog = conf(new ActionRowValue([this]
    {
        full_outfit_catalog_requested_.store(true, std::memory_order_release);
        logger::info("[OutfitFullCatalog] queued live solver catalog capture");
    }, "Discover Full Outfit Catalog"));

    ActionRowValue* discover_outfit_ui_state = conf(new ActionRowValue([this]
    {
        outfit_ui_state_requested_.store(true, std::memory_order_release);
        logger::info("[OutfitUiState] queued availability and live card capture");
    }, "Discover Outfit UI State"));

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
        if (outfit_authority_requested_.exchange(
                false, std::memory_order_acq_rel))
        {
            run_outfit_authority_discovery();
        }
        if (full_outfit_catalog_requested_.exchange(
                false, std::memory_order_acq_rel))
        {
            run_full_outfit_catalog_discovery();
        }
        if (outfit_ui_state_requested_.exchange(
                false, std::memory_order_acq_rel))
        {
            run_outfit_ui_state_discovery();
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
        assembly_csharp_ = il2utils::resolve_image("Assembly-CSharp.dll");
        const bool ready = runtime_discovery::initialize();
        logger::info("[CustomizationProbe] ready=" +
            std::to_string(ready && assembly_csharp_ != nullptr ? 1 : 0));
    }

private:
    static constexpr std::size_t max_candidate_classes_ = 96;
    static constexpr std::size_t max_detailed_classes_ = 32;
    static constexpr std::size_t max_fields_per_class_ = 96;
    static constexpr std::size_t max_methods_per_class_ = 128;
    static constexpr std::size_t max_logged_fields_ = 20;
    static constexpr std::size_t max_logged_methods_ = 20;
    static constexpr std::size_t max_live_candidate_classes_ = 32;
    static constexpr std::size_t max_live_objects_total_ = 32;
    static constexpr std::size_t max_live_objects_per_class_ = 4;
    static constexpr std::size_t max_live_fields_ = 24;
    static constexpr std::size_t max_collections_per_object_ = 4;
    static constexpr std::size_t max_collection_entries_ = 6;
    static constexpr std::size_t max_entry_fields_ = 12;
    static constexpr std::size_t max_outfit_selectors_ = 12;
    static constexpr std::size_t max_outfit_buttons_ = 64;
    static constexpr std::size_t max_outfit_suite_fields_ = 48;
    static constexpr std::size_t max_outfit_class_fields_ = 96;
    static constexpr std::size_t max_outfit_methods_per_class_ = 96;
    static constexpr std::size_t max_solver_owner_classes_ = 24;
    static constexpr std::size_t max_solver_owner_objects_per_class_ = 4;
    static constexpr std::size_t max_full_catalog_entries_ = 2048;
    static constexpr std::size_t max_availability_fields_ = 32;
    static constexpr std::size_t max_availability_records_ = 256;
    static constexpr std::size_t max_live_clothing_cards_ = 128;
    static constexpr std::size_t max_full_offer_fields_ = 64;
    static constexpr std::size_t max_full_offer_methods_ = 64;
    static constexpr std::size_t max_full_offer_samples_ = 8;

    struct MongoIdSnapshot
    {
        std::uint32_t time_stamp;
        std::uint32_t alignment;
        std::uint64_t counter;
        Il2CppString* string_id;
    };

    struct OutfitClassDescriptor
    {
        const char* namespaze;
        const char* name;
    };

    struct OutfitDictionaryEntry
    {
        std::int32_t hash_code;
        std::int32_t next;
        MongoIdSnapshot key;
        Il2CppObject* value;
    };

    struct OutfitCatalogSnapshot
    {
        std::string kind;
        std::string class_name;
        std::string id;
        std::string dictionary_id;
        std::string name;
        std::string parent;
        std::string main_body_part;
        std::string body;
        std::string hands;
        std::string feet;
        std::size_t side_count = 0;
        std::size_t clothing_count = 0;
        bool hidden_by_default = false;
    };

    struct OutfitAvailabilitySlot
    {
        std::int32_t hash_code;
        std::int32_t next;
        Il2CppObject* value;
    };

    struct OutfitAvailabilitySnapshot
    {
        std::size_t slot_index = 0;
        std::string id;
        std::int32_t raw_type = 0;
        std::int32_t raw_source = 0;
    };

    static_assert(sizeof(MongoIdSnapshot) == 24);
    static_assert(sizeof(OutfitDictionaryEntry) == 40);
    static_assert(sizeof(OutfitAvailabilitySlot) == 16);

    std::atomic_bool discovery_requested_ = false;
    std::atomic_bool outfit_authority_requested_ = false;
    std::atomic_bool full_outfit_catalog_requested_ = false;
    std::atomic_bool outfit_ui_state_requested_ = false;
    const Il2CppImage* assembly_csharp_ = nullptr;

    static bool contains_any(
        const std::string& value,
        const std::array<const char*, 17>& terms)
    {
        const std::string searchable = runtime_discovery::to_lower(value);
        for (const char* term : terms)
        {
            if (searchable.find(term) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    static int score_customization_class(const Il2CppClass* klass)
    {
        if (klass == nullptr)
        {
            return -1;
        }

        const std::string namespaze = runtime_discovery::to_lower(
            klass->namespaze != nullptr ? klass->namespaze : "");
        const std::string name = runtime_discovery::to_lower(
            klass->name != nullptr ? klass->name : "");
        const std::string full_name = namespaze + "." + name;
        if (full_name.find("customization") != std::string::npos ||
            full_name.find("outfit") != std::string::npos ||
            full_name.find("clothing") != std::string::npos ||
            full_name.find("wardrobe") != std::string::npos)
        {
            return 0;
        }
        if (name.find("suit") != std::string::npos ||
            full_name.find("appearance") != std::string::npos ||
            full_name.find("ragman") != std::string::npos)
        {
            return 1;
        }
        if (namespaze.find("eft.ui") != std::string::npos &&
            (name.find("body") != std::string::npos ||
                name.find("character") != std::string::npos ||
                name.find("preview") != std::string::npos))
        {
            return 2;
        }
        return -1;
    }

    static bool is_relevant_member(const std::string& value)
    {
        static constexpr std::array<const char*, 17> terms{
            "custom", "outfit", "suit", "cloth", "wardrobe", "appearance",
            "body", "upper", "lower", "preview", "equip", "available",
            "lock", "profile", "service", "collection", "ragman"
        };
        return contains_any(value, terms);
    }

    static bool is_relevant_entry_member(const std::string& value)
    {
        const std::string searchable = runtime_discovery::to_lower(value);
        return searchable.find("id") != std::string::npos ||
            searchable.find("name") != std::string::npos ||
            searchable.find("side") != std::string::npos ||
            searchable.find("body") != std::string::npos ||
            searchable.find("upper") != std::string::npos ||
            searchable.find("lower") != std::string::npos ||
            searchable.find("equip") != std::string::npos ||
            searchable.find("lock") != std::string::npos ||
            searchable.find("available") != std::string::npos;
    }

    static void dump_candidate_members(
        const runtime_discovery::RankedClass& candidate,
        const std::size_t candidate_index)
    {
        std::size_t logged_fields = 0;
        const auto fields = runtime_discovery::collect_fields(
            candidate.klass, max_fields_per_class_);
        for (FieldInfo* field : fields)
        {
            if (field == nullptr || field->name == nullptr ||
                !is_relevant_member(std::string(field->name) + " " +
                    runtime_discovery::type_name(field->type)))
            {
                continue;
            }

            logger::info("[CustomizationCandidateField] class_index=" +
                std::to_string(candidate_index) +
                " declaring_class='" +
                runtime_discovery::class_name(field->parent) +
                "' name='" + field->name +
                "' offset=" + std::to_string(field->offset) +
                " type='" + runtime_discovery::type_name(field->type) + "'");
            if (++logged_fields >= max_logged_fields_)
            {
                break;
            }
        }

        std::size_t logged_methods = 0;
        const auto methods = runtime_discovery::collect_methods(
            candidate.klass, max_methods_per_class_);
        for (const MethodInfo* method : methods)
        {
            if (method == nullptr || method->name == nullptr ||
                !is_relevant_member(method->name))
            {
                continue;
            }

            logger::info("[CustomizationCandidateMethod] class_index=" +
                std::to_string(candidate_index) +
                " declaring_class='" +
                runtime_discovery::class_name(method->klass) +
                "' name='" + method->name +
                "' params=" + std::to_string(method->parameters_count) +
                " return='" +
                runtime_discovery::type_name(method->return_type) + "'");
            if (++logged_methods >= max_logged_methods_)
            {
                break;
            }
        }
    }

    static void dump_collection_entries(
        Il2CppObject* collection,
        const std::size_t object_index,
        const std::string& field_name)
    {
        Il2CppArray* items = nullptr;
        std::size_t count = 0;
        if (!runtime_discovery::collection_elements(collection, items, count) ||
            items == nullptr || count == 0)
        {
            return;
        }

        Il2CppClass* element_class = items->klass != nullptr ?
            items->klass->element_class : nullptr;
        if (element_class == nullptr || element_class->byval_arg.valuetype)
        {
            logger::info("[CustomizationCollection] object_index=" +
                std::to_string(object_index) +
                " field='" + field_name +
                "' count=" + std::to_string(count) +
                " sampling=skipped_non_reference_elements");
            return;
        }

        const std::size_t sampled_count = (std::min)(count, max_collection_entries_);
        auto** values = reinterpret_cast<Il2CppObject**>(&items->data);
        logger::info("[CustomizationCollection] object_index=" +
            std::to_string(object_index) +
            " field='" + field_name +
            "' count=" + std::to_string(count) +
            " sampled=" + std::to_string(sampled_count) +
            " element_class='" + runtime_discovery::class_name(element_class) + "'");
        for (std::size_t entry_index = 0; entry_index < sampled_count; ++entry_index)
        {
            Il2CppObject* entry = values[entry_index];
            if (entry == nullptr)
            {
                logger::info("[CustomizationEntry] object_index=" +
                    std::to_string(object_index) +
                    " field='" + field_name +
                    "' entry_index=" + std::to_string(entry_index) + " null");
                continue;
            }

            if (runtime_discovery::class_name(entry->klass) == "System.String")
            {
                logger::info("[CustomizationEntry] object_index=" +
                    std::to_string(object_index) +
                    " field='" + field_name +
                    "' entry_index=" + std::to_string(entry_index) +
                    " string='" + runtime_discovery::safe_string(
                        reinterpret_cast<Il2CppString*>(entry)) + "'");
                continue;
            }

            logger::info("[CustomizationEntry] object_index=" +
                std::to_string(object_index) +
                " field='" + field_name +
                "' entry_index=" + std::to_string(entry_index) +
                " class='" + runtime_discovery::class_name(entry->klass) + "'");
            std::size_t logged_entry_fields = 0;
            const auto entry_fields = runtime_discovery::collect_fields(
                entry->klass, 64);
            for (FieldInfo* entry_field : entry_fields)
            {
                if (entry_field == nullptr || entry_field->name == nullptr ||
                    !is_relevant_entry_member(std::string(entry_field->name) + " " +
                        runtime_discovery::type_name(entry_field->type)))
                {
                    continue;
                }

                logger::info("[CustomizationEntryValue] object_index=" +
                    std::to_string(object_index) +
                    " field='" + field_name +
                    "' entry_index=" + std::to_string(entry_index) +
                    " name='" + entry_field->name +
                    "' type='" + runtime_discovery::type_name(entry_field->type) +
                    "' value=" +
                    runtime_discovery::field_value_summary(entry, entry_field));
                if (++logged_entry_fields >= max_entry_fields_)
                {
                    break;
                }
            }
        }
    }

    static void dump_live_object(
        Il2CppObject* object,
        const std::size_t object_index,
        const std::size_t class_index)
    {
        logger::info("[CustomizationLiveObject] object_index=" +
            std::to_string(object_index) +
            " class_index=" + std::to_string(class_index) +
            " class='" + runtime_discovery::class_name(object->klass) +
            "' name='" + runtime_discovery::object_name(object) +
            "' address=" +
            std::to_string(reinterpret_cast<std::uintptr_t>(object)));

        std::size_t logged_fields = 0;
        std::size_t sampled_collections = 0;
        const auto fields = runtime_discovery::collect_fields(
            object->klass, max_fields_per_class_);
        for (FieldInfo* field : fields)
        {
            if (field == nullptr || field->name == nullptr || field->offset <= 0 ||
                !is_relevant_member(std::string(field->name) + " " +
                    runtime_discovery::type_name(field->type)))
            {
                continue;
            }

            logger::info("[CustomizationLiveValue] object_index=" +
                std::to_string(object_index) +
                " declaring_class='" + runtime_discovery::class_name(field->parent) +
                "' name='" + field->name +
                "' type='" + runtime_discovery::type_name(field->type) +
                "' value=" + runtime_discovery::field_value_summary(object, field));
            ++logged_fields;

            Il2CppObject* reference =
                runtime_discovery::read_reference_field(object, field);
            std::size_t collection_count = 0;
            if (sampled_collections < max_collections_per_object_ &&
                reference != nullptr &&
                runtime_discovery::collection_count(reference, collection_count))
            {
                dump_collection_entries(reference, object_index, field->name);
                ++sampled_collections;
            }
            if (logged_fields >= max_live_fields_)
            {
                break;
            }
        }
    }

    static Il2CppObject* read_reference_member(
        Il2CppObject* object,
        const char* field_name)
    {
        if (object == nullptr || object->klass == nullptr || field_name == nullptr)
        {
            return nullptr;
        }

        FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        return runtime_discovery::read_reference_field(object, field);
    }

    static std::string mongo_id_text(const MongoIdSnapshot* value)
    {
        return value != nullptr && value->string_id != nullptr ?
            runtime_discovery::safe_string(value->string_id) : "";
    }

    static std::string read_mongo_id_member(
        Il2CppObject* object,
        const char* field_name)
    {
        if (object == nullptr || object->klass == nullptr || field_name == nullptr)
        {
            return "";
        }

        FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->offset <= 0 || field->type == nullptr ||
            runtime_discovery::type_name(field->type) != "EFT.MongoID")
        {
            return "";
        }

        const auto* value = reinterpret_cast<const MongoIdSnapshot*>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
        return mongo_id_text(value);
    }

    static std::string read_string_member(
        Il2CppObject* object,
        const char* field_name)
    {
        if (object == nullptr || object->klass == nullptr || field_name == nullptr)
        {
            return "";
        }

        FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->offset <= 0 || field->type == nullptr ||
            field->type->type != IL2CPP_TYPE_STRING)
        {
            return "";
        }

        const auto* value = *reinterpret_cast<Il2CppString**>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
        return runtime_discovery::safe_string(value);
    }

    static bool read_bool_member(
        Il2CppObject* object,
        const char* field_name,
        const bool fallback)
    {
        if (object == nullptr || object->klass == nullptr || field_name == nullptr)
        {
            return fallback;
        }

        FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->offset <= 0 || field->type == nullptr ||
            field->type->type != IL2CPP_TYPE_BOOLEAN)
        {
            return fallback;
        }

        return *reinterpret_cast<bool*>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
    }

    static std::size_t read_collection_member_count(
        Il2CppObject* object,
        const char* field_name)
    {
        Il2CppObject* collection = read_reference_member(object, field_name);
        std::size_t count = 0;
        return runtime_discovery::collection_count(collection, count) ? count : 0;
    }

    static OutfitCatalogSnapshot snapshot_outfit_item(
        Il2CppObject* item,
        const std::string& kind,
        const std::string& dictionary_id)
    {
        OutfitCatalogSnapshot snapshot;
        snapshot.kind = kind;
        snapshot.dictionary_id = dictionary_id;
        if (item == nullptr || item->klass == nullptr)
        {
            return snapshot;
        }

        snapshot.class_name = runtime_discovery::class_name(item->klass);
        snapshot.id = read_mongo_id_member(item, "Id");
        snapshot.name = read_string_member(item, "Name");
        snapshot.parent = read_string_member(item, "Parent");
        snapshot.main_body_part = read_mongo_id_member(
            item, "<MainBodyPartItem>k__BackingField");
        snapshot.body = read_mongo_id_member(item, "Body");
        snapshot.hands = read_mongo_id_member(item, "Hands");
        snapshot.feet = read_mongo_id_member(item, "Feet");
        snapshot.side_count = read_collection_member_count(item, "Side");
        snapshot.clothing_count = read_collection_member_count(item, "_clothings");
        snapshot.hidden_by_default = read_bool_member(
            item, "HiddenByDefault", false);
        return snapshot;
    }

    static bool append_outfit_dictionary(
        Il2CppObject* dictionary,
        const std::string& kind,
        const char* expected_base_namespaze,
        const char* expected_base_name,
        std::vector<OutfitCatalogSnapshot>& snapshots)
    {
        if (dictionary == nullptr || dictionary->klass == nullptr)
        {
            logger::warn("[OutfitFullCatalogDictionary] kind='" + kind +
                "' unavailable");
            return false;
        }

        int dictionary_count = 0;
        Il2CppObject* entries_object = read_reference_member(dictionary, "_entries");
        auto* entries = reinterpret_cast<Il2CppArray*>(entries_object);
        if (!runtime_discovery::read_int_field(
                dictionary, "_count", dictionary_count) ||
            dictionary_count < 0 || entries == nullptr ||
            entries->klass == nullptr ||
            entries->klass->element_size != sizeof(OutfitDictionaryEntry))
        {
            logger::warn("[OutfitFullCatalogDictionary] kind='" + kind +
                "' rejected class='" +
                runtime_discovery::class_name(dictionary->klass) +
                "' count=" + std::to_string(dictionary_count) +
                " element_size=" + std::to_string(
                    entries != nullptr && entries->klass != nullptr ?
                        entries->klass->element_size : 0) +
                " expected_element_size=" +
                std::to_string(sizeof(OutfitDictionaryEntry)));
            return false;
        }

        const std::size_t source_count = (std::min)(
            static_cast<std::size_t>(dictionary_count),
            static_cast<std::size_t>(entries->max_length));
        const std::size_t remaining_capacity =
            max_full_catalog_entries_ > snapshots.size() ?
                max_full_catalog_entries_ - snapshots.size() : 0;
        const std::size_t sampled_count = (std::min)(
            source_count, remaining_capacity);
        const auto* values = reinterpret_cast<const OutfitDictionaryEntry*>(
            &entries->data);
        std::size_t retained = 0;
        std::size_t skipped = 0;
        std::size_t id_mismatches = 0;
        for (std::size_t index = 0; index < sampled_count; ++index)
        {
            const OutfitDictionaryEntry& entry = values[index];
            if (entry.hash_code < 0 || entry.value == nullptr ||
                entry.value->klass == nullptr ||
                !runtime_discovery::inherits_from(
                    entry.value->klass,
                    expected_base_namespaze,
                    expected_base_name))
            {
                ++skipped;
                continue;
            }

            const std::string dictionary_id = mongo_id_text(&entry.key);
            OutfitCatalogSnapshot snapshot = snapshot_outfit_item(
                entry.value, kind, dictionary_id);
            if (!snapshot.id.empty() && !dictionary_id.empty() &&
                snapshot.id != dictionary_id)
            {
                ++id_mismatches;
            }
            snapshots.push_back(std::move(snapshot));
            ++retained;
        }

        logger::info("[OutfitFullCatalogDictionary] kind='" + kind +
            "' class='" + runtime_discovery::class_name(dictionary->klass) +
            "' count=" + std::to_string(dictionary_count) +
            " capacity=" + std::to_string(entries->max_length) +
            " element_size=" + std::to_string(entries->klass->element_size) +
            " sampled=" + std::to_string(sampled_count) +
            " retained=" + std::to_string(retained) +
            " skipped=" + std::to_string(skipped) +
            " id_mismatches=" + std::to_string(id_mismatches) +
            " truncated=" + std::to_string(source_count > sampled_count ? 1 : 0));
        return true;
    }

    static void log_outfit_snapshot(
        const OutfitCatalogSnapshot& snapshot,
        const std::size_t index)
    {
        logger::info("[OutfitFullCatalogEntry] index=" + std::to_string(index) +
            " kind='" + snapshot.kind +
            "' class='" + snapshot.class_name +
            "' id='" + snapshot.id +
            "' dictionary_id='" + snapshot.dictionary_id +
            "' name='" + snapshot.name +
            "' parent='" + snapshot.parent +
            "' hidden=" + std::to_string(snapshot.hidden_by_default ? 1 : 0) +
            " side_count=" + std::to_string(snapshot.side_count) +
            " clothing_count=" + std::to_string(snapshot.clothing_count) +
            " main_body_part='" + snapshot.main_body_part +
            "' body='" + snapshot.body +
            "' hands='" + snapshot.hands +
            "' feet='" + snapshot.feet + "'");
    }

    static std::string outfit_field_value_summary(
        Il2CppObject* object,
        const FieldInfo* field)
    {
        if (object == nullptr || field == nullptr || field->offset <= 0 ||
            field->type == nullptr)
        {
            return "<unavailable>";
        }

        const std::string field_type = runtime_discovery::type_name(field->type);
        if (field_type == "EFT.MongoID")
        {
            const auto* value = reinterpret_cast<const MongoIdSnapshot*>(
                reinterpret_cast<std::uintptr_t>(object) + field->offset);
            return value->string_id != nullptr ?
                "mongo_id='" + runtime_discovery::safe_string(value->string_id) +
                    "'" :
                "mongo_id=null";
        }

        return runtime_discovery::field_value_summary(object, field);
    }

    static void dump_outfit_suite(
        Il2CppObject* suite,
        const std::size_t suite_index)
    {
        if (suite == nullptr || suite->klass == nullptr)
        {
            return;
        }

        logger::info("[OutfitSuite] suite_index=" + std::to_string(suite_index) +
            " class='" + runtime_discovery::class_name(suite->klass) +
            "' address=" +
            std::to_string(reinterpret_cast<std::uintptr_t>(suite)));

        std::size_t logged_fields = 0;
        const auto fields = runtime_discovery::collect_fields(
            suite->klass, max_outfit_suite_fields_);
        for (FieldInfo* field : fields)
        {
            if (field == nullptr || field->name == nullptr || field->offset <= 0)
            {
                continue;
            }

            logger::info("[OutfitSuiteValue] suite_index=" +
                std::to_string(suite_index) +
                " declaring_class='" +
                runtime_discovery::class_name(field->parent) +
                "' name='" + field->name +
                "' offset=" + std::to_string(field->offset) +
                " type='" + runtime_discovery::type_name(field->type) +
                "' value=" + outfit_field_value_summary(suite, field));
            if (++logged_fields >= max_outfit_suite_fields_)
            {
                break;
            }
        }
    }

    void dump_outfit_method_surfaces() const
    {
        static constexpr std::array<OutfitClassDescriptor, 15> classes{
            OutfitClassDescriptor{ "EFT.UI", "InventoryClothingSelectionPanel" },
            OutfitClassDescriptor{ "EFT.UI", "InventoryCustomizationSelector" },
            OutfitClassDescriptor{ "EFT.UI", "InventoryCustomizationSelectorButton" },
            OutfitClassDescriptor{ "EFT.UI", "TacticalClothingView" },
            OutfitClassDescriptor{ "EFT.UI", "ClothingItem" },
            OutfitClassDescriptor{ "EFT.Customization", "BaseCustomizationItem" },
            OutfitClassDescriptor{ "EFT.Customization", "CustomizationClothing" },
            OutfitClassDescriptor{ "EFT.Customization", "CustomizationSuite" },
            OutfitClassDescriptor{ "EFT.Customization", "UpperBodySuit" },
            OutfitClassDescriptor{ "EFT.Customization", "LowerBodySuit" },
            OutfitClassDescriptor{ "EFT", "CustomizationSolver" },
            OutfitClassDescriptor{ "EFT", "AvailableCustomization" },
            OutfitClassDescriptor{ "EFT", "CustomizationLoadOperation" },
            OutfitClassDescriptor{ "", "CustomizationDataLoader" },
            OutfitClassDescriptor{ "", "Customization" }
        };

        for (const OutfitClassDescriptor& descriptor : classes)
        {
            Il2CppClass* klass = const_cast<Il2CppClass*>(il2utils::resolve_class(
                assembly_csharp_, descriptor.namespaze, descriptor.name));
            if (klass == nullptr)
            {
                logger::warn("[OutfitMethodSurface] unresolved class='" +
                    std::string(descriptor.namespaze) + "." + descriptor.name +
                    "'");
                continue;
            }

            const auto fields = runtime_discovery::collect_fields(
                klass, max_outfit_class_fields_, 1);
            logger::info("[OutfitClassSurface] class='" +
                runtime_discovery::class_name(klass) +
                "' direct_fields=" + std::to_string(fields.size()));
            for (std::size_t field_index = 0;
                 field_index < fields.size(); ++field_index)
            {
                const FieldInfo* field = fields[field_index];
                if (field == nullptr || field->name == nullptr)
                {
                    continue;
                }

                logger::info("[OutfitClassField] class='" +
                    runtime_discovery::class_name(klass) +
                    "' field_index=" + std::to_string(field_index) +
                    " name='" + field->name +
                    "' offset=" + std::to_string(field->offset) +
                    " type='" + runtime_discovery::type_name(field->type) +
                    "'");
            }

            const auto methods = runtime_discovery::collect_methods(
                klass, max_outfit_methods_per_class_, 1);
            logger::info("[OutfitMethodSurface] class='" +
                runtime_discovery::class_name(klass) +
                "' direct_methods=" + std::to_string(methods.size()));
            for (std::size_t method_index = 0;
                 method_index < methods.size(); ++method_index)
            {
                const MethodInfo* method = methods[method_index];
                if (method == nullptr || method->name == nullptr)
                {
                    continue;
                }

                logger::info("[OutfitMethod] class='" +
                    runtime_discovery::class_name(klass) +
                    "' method_index=" + std::to_string(method_index) +
                    " name='" + method->name +
                    "' params=" + std::to_string(method->parameters_count) +
                    " return='" +
                    runtime_discovery::type_name(method->return_type) + "'");

                for (std::uint8_t parameter_index = 0;
                     parameter_index < method->parameters_count;
                     ++parameter_index)
                {
                    const Il2CppType* parameter_type =
                        method->parameters != nullptr ?
                        method->parameters[parameter_index] : nullptr;
                    logger::info("[OutfitMethodParam] class='" +
                        runtime_discovery::class_name(klass) +
                        "' method='" + method->name +
                        "' parameter_index=" +
                        std::to_string(parameter_index) +
                        " type='" +
                        runtime_discovery::type_name(parameter_type) +
                        "' byref=" + std::to_string(
                            parameter_type != nullptr && parameter_type->byref ?
                                1 : 0) +
                        " valuetype=" + std::to_string(
                            parameter_type != nullptr &&
                                parameter_type->valuetype ? 1 : 0));
                }
            }
        }
    }

    void dump_outfit_solver_owners() const
    {
        runtime_discovery::RuntimeApi& runtime_api = runtime_discovery::api();
        if (assembly_csharp_ == nullptr ||
            runtime_api.image_get_class_count == nullptr ||
            runtime_api.image_get_class == nullptr)
        {
            logger::warn("[OutfitSolverOwners] image metadata API is unavailable");
            return;
        }

        constexpr std::size_t max_image_classes = 100000;
        const std::size_t image_class_count = (std::min)(
            runtime_api.image_get_class_count(assembly_csharp_),
            max_image_classes);
        std::size_t owner_class_count = 0;
        std::size_t live_object_count = 0;
        logger::info("[OutfitSolverOwners] begin image_classes=" +
            std::to_string(image_class_count));
        for (std::size_t class_index = 0;
             class_index < image_class_count &&
                owner_class_count < max_solver_owner_classes_;
             ++class_index)
        {
            Il2CppClass* klass = runtime_api.image_get_class(
                assembly_csharp_, class_index);
            if (klass == nullptr || klass->fields == nullptr)
            {
                continue;
            }

            for (std::uint16_t field_index = 0;
                 field_index < klass->field_count;
                 ++field_index)
            {
                FieldInfo* field = &klass->fields[field_index];
                if (field->name == nullptr || field->type == nullptr)
                {
                    continue;
                }

                const std::string lowered_name = runtime_discovery::to_lower(
                    field->name);
                if (lowered_name.find("solver") == std::string::npos)
                {
                    continue;
                }

                const std::string field_type =
                    runtime_discovery::type_name(field->type);
                if (field_type.find("CustomizationSolver") ==
                    std::string::npos)
                {
                    continue;
                }

                const bool unity_object = runtime_discovery::inherits_from(
                    klass, "UnityEngine", "Object");
                ++owner_class_count;
                logger::info("[OutfitSolverOwner] class_index=" +
                    std::to_string(class_index) +
                    " class='" + runtime_discovery::class_name(klass) +
                    "' field='" + field->name +
                    "' offset=" + std::to_string(field->offset) +
                    " type='" + field_type +
                    "' unity_object=" + std::to_string(unity_object ? 1 : 0));

                if (!unity_object || field->offset <= 0 ||
                    !runtime_discovery::is_reference_type(field->type))
                {
                    continue;
                }

                Il2CppArray* objects = runtime_discovery::find_all_objects(klass);
                const std::size_t object_count = objects != nullptr ?
                    static_cast<std::size_t>(objects->max_length) : 0;
                logger::info("[OutfitSolverOwnerObjects] class='" +
                    runtime_discovery::class_name(klass) +
                    "' objects=" + std::to_string(object_count));
                if (objects == nullptr)
                {
                    continue;
                }

                auto** object_values = reinterpret_cast<Il2CppObject**>(
                    &objects->data);
                const std::size_t sampled_objects = (std::min)(
                    object_count, max_solver_owner_objects_per_class_);
                for (std::size_t object_index = 0;
                     object_index < sampled_objects;
                     ++object_index)
                {
                    Il2CppObject* owner = object_values[object_index];
                    if (owner == nullptr)
                    {
                        continue;
                    }

                    Il2CppObject* solver =
                        runtime_discovery::read_reference_field(owner, field);
                    logger::info("[OutfitSolverOwnerObject] class='" +
                        runtime_discovery::class_name(klass) +
                        "' object_index=" + std::to_string(object_index) +
                        " object_name='" +
                        runtime_discovery::object_name(owner) +
                        "' owner_address=" + std::to_string(
                            reinterpret_cast<std::uintptr_t>(owner)) +
                        " solver_address=" + std::to_string(
                            reinterpret_cast<std::uintptr_t>(solver)));
                    ++live_object_count;
                }
            }
        }

        logger::info("[OutfitSolverOwners] complete owner_classes=" +
            std::to_string(owner_class_count) +
            " live_objects=" + std::to_string(live_object_count));
    }

    void dump_outfit_catalog() const
    {
        Il2CppClass* selector_class = const_cast<Il2CppClass*>(
            il2utils::resolve_class(
                assembly_csharp_, "EFT.UI", "InventoryCustomizationSelector"));
        if (selector_class == nullptr)
        {
            logger::warn("[OutfitCatalog] selector class is unavailable");
            return;
        }

        Il2CppArray* selectors = runtime_discovery::find_all_objects(selector_class);
        const std::size_t selector_count = selectors != nullptr ?
            static_cast<std::size_t>(selectors->max_length) : 0;
        logger::info("[OutfitCatalog] begin selectors=" +
            std::to_string(selector_count));
        if (selectors == nullptr)
        {
            logger::info("[OutfitCatalog] complete populated_selectors=0 entries=0 "
                "unique_suites=0");
            return;
        }

        std::vector<Il2CppObject*> unique_suites;
        unique_suites.reserve(max_outfit_buttons_);
        std::size_t populated_selectors = 0;
        std::size_t logged_buttons = 0;
        auto** selector_values = reinterpret_cast<Il2CppObject**>(&selectors->data);
        const std::size_t sampled_selectors = (std::min)(
            selector_count, max_outfit_selectors_);
        for (std::size_t selector_index = 0;
             selector_index < sampled_selectors &&
                logged_buttons < max_outfit_buttons_;
             ++selector_index)
        {
            Il2CppObject* selector = selector_values[selector_index];
            if (selector == nullptr)
            {
                continue;
            }

            Il2CppObject* buttons = read_reference_member(selector, "_buttons");
            Il2CppArray* button_items = nullptr;
            std::size_t button_count = 0;
            if (!runtime_discovery::collection_elements(
                    buttons, button_items, button_count) ||
                button_items == nullptr || button_count == 0)
            {
                continue;
            }

            Il2CppObject* selected_item = read_reference_member(
                selector, "_selectedItem");
            Il2CppObject* active_element = read_reference_member(
                selector, "_activeElement");
            ++populated_selectors;
            logger::info("[OutfitSelector] selector_index=" +
                std::to_string(selector_index) +
                " name='" + runtime_discovery::object_name(selector) +
                "' buttons=" + std::to_string(button_count) +
                " selected_address=" + std::to_string(
                    reinterpret_cast<std::uintptr_t>(selected_item)) +
                " active_address=" + std::to_string(
                    reinterpret_cast<std::uintptr_t>(active_element)));

            auto** button_values = reinterpret_cast<Il2CppObject**>(
                &button_items->data);
            for (std::size_t button_index = 0;
                 button_index < button_count &&
                    logged_buttons < max_outfit_buttons_;
                 ++button_index)
            {
                Il2CppObject* button = button_values[button_index];
                if (button == nullptr)
                {
                    continue;
                }

                Il2CppObject* suite = read_reference_member(
                    button, "<CustomizationSuite>k__BackingField");
                logger::info("[OutfitEntry] selector_index=" +
                    std::to_string(selector_index) +
                    " button_index=" + std::to_string(button_index) +
                    " button_name='" + runtime_discovery::object_name(button) +
                    "' suite_class='" +
                    runtime_discovery::class_name(
                        suite != nullptr ? suite->klass : nullptr) +
                    "' selected=" +
                    std::to_string(button == selected_item ? 1 : 0) +
                    " active=" +
                    std::to_string(button == active_element ? 1 : 0) +
                    " suite_address=" + std::to_string(
                        reinterpret_cast<std::uintptr_t>(suite)));
                ++logged_buttons;

                if (suite == nullptr ||
                    std::find(unique_suites.begin(), unique_suites.end(), suite) !=
                        unique_suites.end())
                {
                    continue;
                }

                unique_suites.push_back(suite);
                dump_outfit_suite(suite, unique_suites.size() - 1);
            }
        }

        logger::info("[OutfitCatalog] complete populated_selectors=" +
            std::to_string(populated_selectors) +
            " entries=" + std::to_string(logged_buttons) +
            " unique_suites=" + std::to_string(unique_suites.size()) +
            " note='read-only; no preview, equip, unlock, or profile mutation'");
    }

    static void dump_outfit_availability_schema(Il2CppObject* solver)
    {
        Il2CppObject* availability = read_reference_member(
            solver, "_availableCustomizations");
        std::size_t availability_count = 0;
        const bool has_count = runtime_discovery::collection_count(
            availability, availability_count);
        logger::info("[OutfitAvailabilitySchema] class='" +
            runtime_discovery::class_name(
                availability != nullptr ? availability->klass : nullptr) +
            "' count=" + std::to_string(availability_count) +
            " count_read=" + std::to_string(has_count ? 1 : 0));
        if (availability == nullptr || availability->klass == nullptr)
        {
            return;
        }

        const auto fields = runtime_discovery::collect_fields(
            availability->klass, max_availability_fields_, 1);
        for (std::size_t field_index = 0;
             field_index < fields.size(); ++field_index)
        {
            FieldInfo* field = fields[field_index];
            if (field == nullptr || field->name == nullptr)
            {
                continue;
            }

            logger::info("[OutfitAvailabilityField] field_index=" +
                std::to_string(field_index) +
                " name='" + field->name +
                "' offset=" + std::to_string(field->offset) +
                " type='" + runtime_discovery::type_name(field->type) +
                "' value=" +
                runtime_discovery::field_value_summary(availability, field));

            if (field->type == nullptr ||
                (field->type->type != IL2CPP_TYPE_ARRAY &&
                    field->type->type != IL2CPP_TYPE_SZARRAY))
            {
                continue;
            }

            auto* array = reinterpret_cast<Il2CppArray*>(
                runtime_discovery::read_reference_field(availability, field));
            Il2CppClass* element_class = array != nullptr &&
                array->klass != nullptr ? array->klass->element_class : nullptr;
            logger::info("[OutfitAvailabilityArray] field='" +
                std::string(field->name) +
                "' array_class='" + runtime_discovery::class_name(
                    array != nullptr ? array->klass : nullptr) +
                "' length=" + std::to_string(
                    array != nullptr ? array->max_length : 0) +
                " element_class='" +
                runtime_discovery::class_name(element_class) +
                "' element_size=" + std::to_string(
                    array != nullptr && array->klass != nullptr ?
                        array->klass->element_size : 0));
            if (element_class == nullptr)
            {
                continue;
            }

            const auto element_fields = runtime_discovery::collect_fields(
                element_class, max_availability_fields_, 1);
            for (std::size_t element_field_index = 0;
                 element_field_index < element_fields.size();
                 ++element_field_index)
            {
                const FieldInfo* element_field =
                    element_fields[element_field_index];
                if (element_field == nullptr || element_field->name == nullptr)
                {
                    continue;
                }

                logger::info("[OutfitAvailabilityElementField] array_field='" +
                    std::string(field->name) +
                    "' field_index=" +
                    std::to_string(element_field_index) +
                    " name='" + element_field->name +
                    "' offset=" + std::to_string(element_field->offset) +
                    " type='" +
                    runtime_discovery::type_name(element_field->type) + "'");
            }
        }
    }

    static FieldInfo* find_direct_field(
        Il2CppClass* klass,
        const char* field_name)
    {
        if (klass == nullptr || klass->fields == nullptr || field_name == nullptr)
        {
            return nullptr;
        }

        for (std::uint16_t field_index = 0;
             field_index < klass->field_count;
             ++field_index)
        {
            FieldInfo* field = &klass->fields[field_index];
            if (field->name != nullptr &&
                std::strcmp(field->name, field_name) == 0)
            {
                return field;
            }
        }
        return nullptr;
    }

    static bool direct_field_matches(
        Il2CppClass* klass,
        const char* field_name,
        const std::int32_t expected_offset,
        const char* expected_type_name,
        FieldInfo*& field)
    {
        field = find_direct_field(klass, field_name);
        if (field == nullptr || field->offset != expected_offset ||
            field->type == nullptr)
        {
            return false;
        }
        return expected_type_name == nullptr ||
            runtime_discovery::type_name(field->type) == expected_type_name;
    }

    static bool is_expected_value_field(
        const FieldInfo* field,
        const char* expected_type_name)
    {
        if (field == nullptr || field->type == nullptr ||
            runtime_discovery::type_name(field->type) != expected_type_name)
        {
            return false;
        }

        return field->type->valuetype;
    }

    static bool is_readable_memory(
        const void* address,
        const std::size_t size)
    {
        if (address == nullptr || size == 0)
        {
            return false;
        }

        std::uintptr_t cursor = reinterpret_cast<std::uintptr_t>(address);
        const std::uintptr_t end = cursor + size;
        if (end < cursor)
        {
            return false;
        }

        while (cursor < end)
        {
            MEMORY_BASIC_INFORMATION memory{};
            if (VirtualQuery(
                    reinterpret_cast<const void*>(cursor),
                    &memory,
                    sizeof(memory)) == 0 ||
                memory.State != MEM_COMMIT ||
                (memory.Protect & (PAGE_GUARD | PAGE_NOACCESS)) != 0)
            {
                return false;
            }

            const DWORD protection = memory.Protect & 0xff;
            if (protection != PAGE_READONLY &&
                protection != PAGE_READWRITE &&
                protection != PAGE_WRITECOPY &&
                protection != PAGE_EXECUTE_READ &&
                protection != PAGE_EXECUTE_READWRITE &&
                protection != PAGE_EXECUTE_WRITECOPY)
            {
                return false;
            }

            const std::uintptr_t region_end =
                reinterpret_cast<std::uintptr_t>(memory.BaseAddress) +
                memory.RegionSize;
            if (region_end <= cursor)
            {
                return false;
            }
            cursor = (std::min)(end, region_end);
        }
        return true;
    }

    static bool read_validated_mongo_id(
        const MongoIdSnapshot* value,
        std::string& text)
    {
        text.clear();
        if (!is_readable_memory(value, sizeof(MongoIdSnapshot)) ||
            value->string_id == nullptr ||
            !is_readable_memory(
                value->string_id, offsetof(Il2CppString, chars)))
        {
            return false;
        }

        const int length = value->string_id->length;
        if (length <= 0 || length > 64 ||
            !is_readable_memory(
                value->string_id,
                offsetof(Il2CppString, chars) +
                    static_cast<std::size_t>(length) * sizeof(wchar_t)))
        {
            return false;
        }

        text = runtime_discovery::safe_string(value->string_id);
        return !text.empty();
    }

    static bool capture_outfit_availability(
        Il2CppObject* solver,
        const Il2CppImage* assembly_csharp,
        std::vector<OutfitAvailabilitySnapshot>& snapshots)
    {
        snapshots.clear();
        Il2CppObject* availability = read_reference_member(
            solver, "_availableCustomizations");
        if (availability == nullptr || availability->klass == nullptr)
        {
            logger::warn(
                "[OutfitAvailabilityRecords] rejected reason='set_unavailable'");
            return false;
        }

        FieldInfo* slots_field = nullptr;
        FieldInfo* count_field = nullptr;
        FieldInfo* last_index_field = nullptr;
        const bool set_schema_valid = direct_field_matches(
                availability->klass, "_slots", 24, nullptr, slots_field) &&
            slots_field->type != nullptr &&
            (slots_field->type->type == IL2CPP_TYPE_ARRAY ||
                slots_field->type->type == IL2CPP_TYPE_SZARRAY) &&
            direct_field_matches(
                availability->klass, "_count", 32, "System.Int32", count_field) &&
            direct_field_matches(
                availability->klass, "_lastIndex", 36, "System.Int32",
                last_index_field);
        if (!set_schema_valid)
        {
            logger::warn(
                "[OutfitAvailabilityRecords] rejected reason='set_schema_mismatch'");
            return false;
        }
        logger::info(
            "[OutfitAvailabilityBoundary] stage='set_schema_validated'");

        auto* slots = reinterpret_cast<Il2CppArray*>(
            runtime_discovery::read_reference_field(availability, slots_field));
        Il2CppClass* slot_class = slots != nullptr && slots->klass != nullptr ?
            slots->klass->element_class : nullptr;
        FieldInfo* hash_code_field = nullptr;
        FieldInfo* next_field = nullptr;
        FieldInfo* value_field = nullptr;
        const bool slot_schema_valid = slots != nullptr &&
            slots->klass != nullptr && slot_class != nullptr &&
            slots->klass->element_size == sizeof(OutfitAvailabilitySlot) &&
            direct_field_matches(
                slot_class, "hashCode", 16, "System.Int32", hash_code_field) &&
            direct_field_matches(
                slot_class, "next", 20, "System.Int32", next_field) &&
            direct_field_matches(
                slot_class, "value", 24, "EFT.AvailableCustomization",
                value_field);
        if (!slot_schema_valid)
        {
            logger::warn("[OutfitAvailabilityRecords] rejected "
                "reason='slot_schema_mismatch' element_size=" +
                std::to_string(
                    slots != nullptr && slots->klass != nullptr ?
                        slots->klass->element_size : 0));
            return false;
        }
        logger::info(
            "[OutfitAvailabilityBoundary] stage='slot_schema_validated'");

        Il2CppClass* available_class = const_cast<Il2CppClass*>(
            il2utils::resolve_class(
                assembly_csharp, "EFT", "AvailableCustomization"));
        FieldInfo* id_field = nullptr;
        FieldInfo* type_field = nullptr;
        FieldInfo* source_field = nullptr;
        const bool value_schema_valid = direct_field_matches(
                available_class, "<Id>k__BackingField", 16, "EFT.MongoID",
                id_field) &&
            direct_field_matches(
                available_class, "<Type>k__BackingField", 40,
                "EFT.ECustomizationType", type_field) &&
            direct_field_matches(
                available_class, "<Source>k__BackingField", 44,
                "EFT.ECustomizationSource", source_field) &&
            is_expected_value_field(type_field, "EFT.ECustomizationType") &&
            is_expected_value_field(
                source_field, "EFT.ECustomizationSource");
        if (!value_schema_valid)
        {
            logger::warn("[OutfitAvailabilityRecords] rejected "
                "reason='value_schema_mismatch'");
            return false;
        }
        logger::info(
            "[OutfitAvailabilityBoundary] stage='value_schema_validated'");

        const auto availability_address =
            reinterpret_cast<std::uintptr_t>(availability);
        const int set_count = *reinterpret_cast<const int*>(
            availability_address + count_field->offset);
        const int last_index = *reinterpret_cast<const int*>(
            availability_address + last_index_field->offset);
        if (set_count < 0 || last_index < 0 || set_count > last_index)
        {
            logger::warn("[OutfitAvailabilityRecords] rejected "
                "reason='invalid_counts' count=" + std::to_string(set_count) +
                " last_index=" + std::to_string(last_index));
            return false;
        }

        const std::size_t source_count = (std::min)(
            static_cast<std::size_t>(last_index),
            static_cast<std::size_t>(slots->max_length));
        const std::size_t sampled_count = (std::min)(
            source_count, max_availability_records_);
        const auto* slot_values =
            reinterpret_cast<const OutfitAvailabilitySlot*>(&slots->data);
        if (sampled_count > 0 && !is_readable_memory(
                slot_values,
                sampled_count * sizeof(OutfitAvailabilitySlot)))
        {
            logger::warn("[OutfitAvailabilityRecords] rejected "
                "reason='slot_payload_unreadable'");
            return false;
        }
        logger::info("[OutfitAvailabilityBoundary] stage='iteration_begin' "
            "sampled=" + std::to_string(sampled_count));

        std::size_t skipped = 0;
        std::size_t duplicate_ids = 0;
        for (std::size_t slot_index = 0;
             slot_index < sampled_count;
             ++slot_index)
        {
            const OutfitAvailabilitySlot& slot = slot_values[slot_index];
            if (slot.hash_code < 0 || slot.value == nullptr ||
                !is_readable_memory(
                    slot.value,
                    static_cast<std::size_t>(source_field->offset) +
                        sizeof(std::int32_t)) ||
                slot.value->klass != available_class)
            {
                ++skipped;
                continue;
            }

            const auto value_address =
                reinterpret_cast<std::uintptr_t>(slot.value);
            const auto* id = reinterpret_cast<const MongoIdSnapshot*>(
                value_address + id_field->offset);
            OutfitAvailabilitySnapshot snapshot;
            snapshot.slot_index = slot_index;
            if (!read_validated_mongo_id(id, snapshot.id))
            {
                ++skipped;
                continue;
            }
            snapshot.raw_type = *reinterpret_cast<const std::int32_t*>(
                value_address + type_field->offset);
            snapshot.raw_source = *reinterpret_cast<const std::int32_t*>(
                value_address + source_field->offset);

            if (std::find_if(
                    snapshots.begin(), snapshots.end(),
                    [&snapshot](const OutfitAvailabilitySnapshot& existing)
                    {
                        return existing.id == snapshot.id;
                    }) != snapshots.end())
            {
                ++duplicate_ids;
            }
            snapshots.push_back(std::move(snapshot));
        }
        logger::info(
            "[OutfitAvailabilityBoundary] stage='iteration_complete'");

        logger::info("[OutfitAvailabilityRecords] class='" +
            runtime_discovery::class_name(availability->klass) +
            "' count=" + std::to_string(set_count) +
            " last_index=" + std::to_string(last_index) +
            " capacity=" + std::to_string(slots->max_length) +
            " element_size=" + std::to_string(slots->klass->element_size) +
            " sampled=" + std::to_string(sampled_count) +
            " retained=" + std::to_string(snapshots.size()) +
            " skipped=" + std::to_string(skipped) +
            " duplicate_ids=" + std::to_string(duplicate_ids) +
            " truncated=" + std::to_string(source_count > sampled_count ? 1 : 0));
        return true;
    }

    static const OutfitCatalogSnapshot* find_outfit_catalog_entry(
        const std::vector<OutfitCatalogSnapshot>& catalog,
        const std::string& id)
    {
        const auto entry = std::find_if(
            catalog.begin(), catalog.end(),
            [&id](const OutfitCatalogSnapshot& snapshot)
            {
                return snapshot.id == id || snapshot.dictionary_id == id;
            });
        return entry != catalog.end() ? &*entry : nullptr;
    }

    static void log_outfit_availability_records(
        const std::vector<OutfitAvailabilitySnapshot>& availability,
        const std::vector<OutfitCatalogSnapshot>& catalog)
    {
        std::size_t suite_matches = 0;
        std::size_t clothing_matches = 0;
        std::size_t unmatched = 0;
        for (const OutfitAvailabilitySnapshot& record : availability)
        {
            const OutfitCatalogSnapshot* catalog_entry =
                find_outfit_catalog_entry(catalog, record.id);
            if (catalog_entry == nullptr)
            {
                ++unmatched;
            }
            else if (catalog_entry->kind == "suite")
            {
                ++suite_matches;
            }
            else if (catalog_entry->kind == "clothing")
            {
                ++clothing_matches;
            }

            logger::info("[OutfitAvailabilityEntry] slot_index=" +
                std::to_string(record.slot_index) +
                " id='" + record.id +
                "' raw_type=" + std::to_string(record.raw_type) +
                " raw_source=" + std::to_string(record.raw_source) +
                " catalog_match=" +
                std::to_string(catalog_entry != nullptr ? 1 : 0) +
                " kind='" +
                (catalog_entry != nullptr ? catalog_entry->kind : "") +
                "' name='" +
                (catalog_entry != nullptr ? catalog_entry->name : "") + "'");
        }

        logger::info("[OutfitAvailabilityCorrelation] records=" +
            std::to_string(availability.size()) +
            " suite_matches=" + std::to_string(suite_matches) +
            " clothing_matches=" + std::to_string(clothing_matches) +
            " unmatched=" + std::to_string(unmatched) +
            " note='available_record is raw solver membership; no lock or "
            "ownership meaning is inferred'");
    }

    static void dump_full_offer_surface(Il2CppClass* full_offer_class)
    {
        if (full_offer_class == nullptr)
        {
            logger::warn("[OutfitFullOfferSurface] nested class unavailable");
            return;
        }

        const auto fields = runtime_discovery::collect_fields(
            full_offer_class, max_full_offer_fields_, 1);
        logger::info("[OutfitFullOfferSurface] class='" +
            runtime_discovery::class_name(full_offer_class) +
            "' direct_fields=" + std::to_string(fields.size()));
        for (std::size_t field_index = 0;
             field_index < fields.size();
             ++field_index)
        {
            const FieldInfo* field = fields[field_index];
            if (field == nullptr || field->name == nullptr)
            {
                continue;
            }
            logger::info("[OutfitFullOfferField] field_index=" +
                std::to_string(field_index) +
                " name='" + field->name +
                "' offset=" + std::to_string(field->offset) +
                " type='" + runtime_discovery::type_name(field->type) + "'");
        }

        const auto methods = runtime_discovery::collect_methods(
            full_offer_class, max_full_offer_methods_, 1);
        logger::info("[OutfitFullOfferMethods] class='" +
            runtime_discovery::class_name(full_offer_class) +
            "' direct_methods=" + std::to_string(methods.size()));
        for (std::size_t method_index = 0;
             method_index < methods.size();
             ++method_index)
        {
            const MethodInfo* method = methods[method_index];
            if (method == nullptr || method->name == nullptr)
            {
                continue;
            }
            logger::info("[OutfitFullOfferMethod] method_index=" +
                std::to_string(method_index) +
                " name='" + method->name +
                "' params=" + std::to_string(method->parameters_count) +
                " return='" +
                runtime_discovery::type_name(method->return_type) + "'");
        }
    }

    static void dump_full_offer_sample(
        Il2CppObject* offer,
        const std::size_t sample_index)
    {
        if (offer == nullptr || offer->klass == nullptr)
        {
            return;
        }

        logger::info("[OutfitFullOfferSample] sample_index=" +
            std::to_string(sample_index) +
            " class='" + runtime_discovery::class_name(offer->klass) +
            "' address=" +
            std::to_string(reinterpret_cast<std::uintptr_t>(offer)));
        const auto fields = runtime_discovery::collect_fields(
            offer->klass, max_full_offer_fields_, 1);
        for (std::size_t field_index = 0;
             field_index < fields.size();
             ++field_index)
        {
            const FieldInfo* field = fields[field_index];
            if (field == nullptr || field->name == nullptr || field->offset <= 0)
            {
                continue;
            }

            std::string value = outfit_field_value_summary(offer, field);
            Il2CppObject* reference =
                runtime_discovery::read_reference_field(offer, field);
            if (reference != nullptr && reference->klass != nullptr &&
                runtime_discovery::inherits_from(
                    reference->klass,
                    "EFT.Customization",
                    "CustomizationSuite"))
            {
                const OutfitCatalogSnapshot suite = snapshot_outfit_item(
                    reference, "suite", "");
                value += " suite_id='" + suite.id +
                    "' suite_name='" + suite.name + "'";
            }

            logger::info("[OutfitFullOfferValue] sample_index=" +
                std::to_string(sample_index) +
                " field_index=" + std::to_string(field_index) +
                " name='" + field->name +
                "' type='" + runtime_discovery::type_name(field->type) +
                "' value=" + value);
        }
    }

    static void dump_live_outfit_cards(
        Il2CppObject* live_view,
        const Il2CppImage* assembly_csharp)
    {
        Il2CppClass* clothing_item_class = const_cast<Il2CppClass*>(
            il2utils::resolve_class(
                assembly_csharp, "EFT.UI", "ClothingItem"));
        Il2CppClass* full_offer_class = const_cast<Il2CppClass*>(
            il2utils::resolve_class_nested(clothing_item_class, "FullOffer"));
        dump_full_offer_surface(full_offer_class);

        FieldInfo* views_field = nullptr;
        if (!direct_field_matches(
                live_view != nullptr ? live_view->klass : nullptr,
                "_views", 288,
                "System.Collections.Generic.List<EFT.UI.ClothingItem>",
                views_field))
        {
            logger::warn(
                "[OutfitUiCards] rejected reason='view_schema_mismatch'");
            return;
        }

        Il2CppObject* views =
            runtime_discovery::read_reference_field(live_view, views_field);
        Il2CppArray* items = nullptr;
        std::size_t item_count = 0;
        if (!runtime_discovery::collection_elements(views, items, item_count) ||
            items == nullptr || items->klass == nullptr ||
            items->klass->element_size != sizeof(Il2CppObject*))
        {
            logger::warn("[OutfitUiCards] rejected "
                "reason='view_list_unavailable' element_size=" +
                std::to_string(
                    items != nullptr && items->klass != nullptr ?
                        items->klass->element_size : 0));
            return;
        }

        FieldInfo* previewed_field = nullptr;
        FieldInfo* state_field = nullptr;
        FieldInfo* purchased_field = nullptr;
        FieldInfo* offer_field = nullptr;
        const bool card_schema_valid = direct_field_matches(
                clothing_item_class, "_previewed", 281, "System.Boolean",
                previewed_field) &&
            direct_field_matches(
                clothing_item_class, "<State>k__BackingField", 284,
                "EFT.UI.ClothingItem.EClothingItemState", state_field) &&
            direct_field_matches(
                clothing_item_class, "<IsPurchased>k__BackingField", 288,
                "System.Boolean", purchased_field) &&
            direct_field_matches(
                clothing_item_class, "<Offer>k__BackingField", 296,
                "EFT.UI.ClothingItem.FullOffer", offer_field) &&
            is_expected_value_field(
                state_field, "EFT.UI.ClothingItem.EClothingItemState");
        if (!card_schema_valid)
        {
            logger::warn(
                "[OutfitUiCards] rejected reason='card_schema_mismatch'");
            return;
        }

        std::size_t current_preview_count = 0;
        std::size_t current_equipped_count = 0;
        std::size_t body_part_count = 0;
        runtime_discovery::collection_count(
            read_reference_member(live_view, "_currentPreview"),
            current_preview_count);
        runtime_discovery::collection_count(
            read_reference_member(live_view, "_currentEquipped"),
            current_equipped_count);
        runtime_discovery::collection_count(
            read_reference_member(live_view, "_bodyParts"), body_part_count);

        const std::size_t sampled_count = (std::min)(
            item_count, max_live_clothing_cards_);
        auto** card_values = reinterpret_cast<Il2CppObject**>(&items->data);
        std::vector<Il2CppObject*> sampled_offers;
        sampled_offers.reserve(max_full_offer_samples_);
        std::size_t retained_cards = 0;
        std::size_t skipped_cards = 0;
        for (std::size_t card_index = 0;
             card_index < sampled_count;
             ++card_index)
        {
            Il2CppObject* card = card_values[card_index];
            if (card == nullptr ||
                !is_readable_memory(
                    card,
                    static_cast<std::size_t>(offer_field->offset) +
                        sizeof(Il2CppObject*)) ||
                card->klass != clothing_item_class)
            {
                ++skipped_cards;
                continue;
            }

            const auto card_address = reinterpret_cast<std::uintptr_t>(card);
            const bool previewed = *reinterpret_cast<const bool*>(
                card_address + previewed_field->offset);
            const std::int32_t raw_state = *reinterpret_cast<const std::int32_t*>(
                card_address + state_field->offset);
            const bool purchased = *reinterpret_cast<const bool*>(
                card_address + purchased_field->offset);
            Il2CppObject* offer = runtime_discovery::read_reference_field(
                card, offer_field);
            const bool offer_class_matches = offer != nullptr &&
                is_readable_memory(offer, sizeof(Il2CppObject)) &&
                offer->klass == full_offer_class;
            logger::info("[OutfitUiCard] card_index=" +
                std::to_string(card_index) +
                " name='" + runtime_discovery::object_name(card) +
                "' raw_state=" + std::to_string(raw_state) +
                " purchased=" + std::to_string(purchased ? 1 : 0) +
                " previewed=" + std::to_string(previewed ? 1 : 0) +
                " offer_class='" + runtime_discovery::class_name(
                    offer != nullptr ? offer->klass : nullptr) +
                "' offer_schema_match=" +
                std::to_string(offer_class_matches ? 1 : 0) +
                " offer_address=" + std::to_string(
                    reinterpret_cast<std::uintptr_t>(offer)));
            ++retained_cards;

            if (offer_class_matches &&
                sampled_offers.size() < max_full_offer_samples_ &&
                std::find(sampled_offers.begin(), sampled_offers.end(), offer) ==
                    sampled_offers.end())
            {
                sampled_offers.push_back(offer);
                dump_full_offer_sample(offer, sampled_offers.size() - 1);
            }
        }

        logger::info("[OutfitUiCards] list_count=" +
            std::to_string(item_count) +
            " sampled=" + std::to_string(sampled_count) +
            " retained=" + std::to_string(retained_cards) +
            " skipped=" + std::to_string(skipped_cards) +
            " offer_samples=" + std::to_string(sampled_offers.size()) +
            " current_preview_count=" +
            std::to_string(current_preview_count) +
            " current_equipped_count=" +
            std::to_string(current_equipped_count) +
            " body_part_count=" + std::to_string(body_part_count) +
            " truncated=" +
            std::to_string(item_count > sampled_count ? 1 : 0));
    }

    void run_outfit_ui_state_discovery() const
    {
        if (assembly_csharp_ == nullptr || !runtime_discovery::initialize())
        {
            logger::warn(
                "[OutfitUiState] runtime discovery API is unavailable");
            return;
        }

        Il2CppClass* view_class = const_cast<Il2CppClass*>(
            il2utils::resolve_class(
                assembly_csharp_, "EFT.UI", "TacticalClothingView"));
        Il2CppArray* views = runtime_discovery::find_all_objects(view_class);
        const std::size_t view_count = views != nullptr ?
            static_cast<std::size_t>(views->max_length) : 0;
        logger::info("[OutfitUiState] begin tactical_views=" +
            std::to_string(view_count));

        Il2CppObject* live_view = nullptr;
        Il2CppObject* solver = nullptr;
        if (views != nullptr)
        {
            auto** values = reinterpret_cast<Il2CppObject**>(&views->data);
            const std::size_t sampled_views = (std::min)(
                view_count, max_solver_owner_objects_per_class_);
            for (std::size_t view_index = 0;
                 view_index < sampled_views;
                 ++view_index)
            {
                Il2CppObject* candidate = values[view_index];
                Il2CppObject* candidate_solver = read_reference_member(
                    candidate, "_solver");
                Il2CppObject* candidate_views = read_reference_member(
                    candidate, "_views");
                std::size_t candidate_card_count = 0;
                runtime_discovery::collection_count(
                    candidate_views, candidate_card_count);
                logger::info("[OutfitUiStateView] view_index=" +
                    std::to_string(view_index) +
                    " name='" + runtime_discovery::object_name(candidate) +
                    "' solver=" + std::to_string(
                        reinterpret_cast<std::uintptr_t>(candidate_solver)) +
                    " cards=" + std::to_string(candidate_card_count));
                if (candidate_solver != nullptr && candidate_views != nullptr &&
                    solver == nullptr)
                {
                    live_view = candidate;
                    solver = candidate_solver;
                }
            }
        }

        if (live_view == nullptr || solver == nullptr)
        {
            logger::warn("[OutfitUiState] live populated view unavailable; open "
                "Ragman > Services > Tactical Clothing, wait for the offers "
                "to appear, then run Discover Outfit UI State again");
            return;
        }

        std::vector<OutfitCatalogSnapshot> catalog;
        catalog.reserve(512);
        const bool suites_captured = append_outfit_dictionary(
            read_reference_member(solver, "_customizationSuiteTemplates"),
            "suite",
            "EFT.Customization",
            "CustomizationSuite",
            catalog);
        const bool clothing_captured = append_outfit_dictionary(
            read_reference_member(solver, "_customizationClothingTemplates"),
            "clothing",
            "EFT.Customization",
            "CustomizationClothing",
            catalog);

        std::vector<OutfitAvailabilitySnapshot> availability;
        availability.reserve(128);
        const bool availability_captured = capture_outfit_availability(
            solver, assembly_csharp_, availability);
        if (availability_captured)
        {
            log_outfit_availability_records(availability, catalog);
        }
        dump_live_outfit_cards(live_view, assembly_csharp_);

        logger::info("[OutfitUiState] complete catalog_entries=" +
            std::to_string(catalog.size()) +
            " availability_records=" +
            std::to_string(availability.size()) +
            " suites_captured=" +
            std::to_string(suites_captured ? 1 : 0) +
            " clothing_captured=" +
            std::to_string(clothing_captured ? 1 : 0) +
            " availability_captured=" +
            std::to_string(availability_captured ? 1 : 0) +
            " note='read-only; raw UI state only; no preview, equip, unlock, "
            "or profile mutation'");
    }

    void run_full_outfit_catalog_discovery() const
    {
        if (assembly_csharp_ == nullptr || !runtime_discovery::initialize())
        {
            logger::warn(
                "[OutfitFullCatalog] runtime discovery API is unavailable");
            return;
        }

        Il2CppClass* view_class = const_cast<Il2CppClass*>(
            il2utils::resolve_class(
                assembly_csharp_, "EFT.UI", "TacticalClothingView"));
        Il2CppArray* views = runtime_discovery::find_all_objects(view_class);
        const std::size_t view_count = views != nullptr ?
            static_cast<std::size_t>(views->max_length) : 0;
        logger::info("[OutfitFullCatalog] begin tactical_views=" +
            std::to_string(view_count));

        Il2CppObject* live_view = nullptr;
        Il2CppObject* solver = nullptr;
        if (views != nullptr)
        {
            auto** values = reinterpret_cast<Il2CppObject**>(&views->data);
            const std::size_t sampled_views = (std::min)(
                view_count, max_solver_owner_objects_per_class_);
            for (std::size_t view_index = 0;
                 view_index < sampled_views; ++view_index)
            {
                Il2CppObject* candidate = values[view_index];
                Il2CppObject* candidate_solver = read_reference_member(
                    candidate, "_solver");
                logger::info("[OutfitFullCatalogView] view_index=" +
                    std::to_string(view_index) +
                    " name='" + runtime_discovery::object_name(candidate) +
                    "' solver=" + std::to_string(
                        reinterpret_cast<std::uintptr_t>(candidate_solver)));
                if (candidate_solver != nullptr && solver == nullptr)
                {
                    live_view = candidate;
                    solver = candidate_solver;
                }
            }
        }

        if (live_view == nullptr || solver == nullptr)
        {
            logger::warn("[OutfitFullCatalog] live solver unavailable; open "
                "Ragman > Services > Tactical Clothing, wait for the offers "
                "to appear, then run Discover Full Outfit Catalog again");
            return;
        }

        const OutfitCatalogSnapshot current_upper = snapshot_outfit_item(
            read_reference_member(live_view, "_currentUpperBodySuite"),
            "current_upper", "");
        const OutfitCatalogSnapshot current_lower = snapshot_outfit_item(
            read_reference_member(live_view, "_currentLowerBodySuite"),
            "current_lower", "");
        logger::info("[OutfitCurrentSuite] body='upper' id='" +
            current_upper.id + "' name='" + current_upper.name +
            "' class='" + current_upper.class_name + "'");
        logger::info("[OutfitCurrentSuite] body='lower' id='" +
            current_lower.id + "' name='" + current_lower.name +
            "' class='" + current_lower.class_name + "'");

        std::vector<OutfitCatalogSnapshot> snapshots;
        snapshots.reserve(512);
        const bool suites_captured = append_outfit_dictionary(
            read_reference_member(solver, "_customizationSuiteTemplates"),
            "suite",
            "EFT.Customization",
            "CustomizationSuite",
            snapshots);
        const bool clothing_captured = append_outfit_dictionary(
            read_reference_member(solver, "_customizationClothingTemplates"),
            "clothing",
            "EFT.Customization",
            "CustomizationClothing",
            snapshots);
        dump_outfit_availability_schema(solver);

        for (std::size_t index = 0; index < snapshots.size(); ++index)
        {
            log_outfit_snapshot(snapshots[index], index);
        }

        logger::info("[OutfitFullCatalog] complete entries=" +
            std::to_string(snapshots.size()) +
            " suites_captured=" + std::to_string(suites_captured ? 1 : 0) +
            " clothing_captured=" +
            std::to_string(clothing_captured ? 1 : 0) +
            " note='read-only live solver snapshot; no preview, equip, unlock, "
            "or profile mutation'");
    }

    void run_discovery() const
    {
        if (assembly_csharp_ == nullptr || !runtime_discovery::initialize())
        {
            logger::warn("[CustomizationProbe] runtime discovery API is unavailable");
            return;
        }

        const runtime_discovery::ClassSearchResult search =
            runtime_discovery::search_image_classes(
                assembly_csharp_, score_customization_class, max_candidate_classes_);
        logger::info("[CustomizationProbe] begin image_classes=" +
            std::to_string(search.image_class_count) +
            " matches=" + std::to_string(search.total_matches) +
            " retained=" + std::to_string(search.classes.size()));

        for (std::size_t class_index = 0;
             class_index < search.classes.size(); ++class_index)
        {
            const runtime_discovery::RankedClass& candidate =
                search.classes[class_index];
            logger::info("[CustomizationCandidate] #" +
                std::to_string(class_index) +
                " score=" + std::to_string(candidate.score) +
                " class='" + runtime_discovery::class_name(candidate.klass) +
                "' unity_object=" + std::to_string(
                    runtime_discovery::inherits_from(
                        candidate.klass, "UnityEngine", "Object") ? 1 : 0));
            if (class_index < max_detailed_classes_)
            {
                dump_candidate_members(candidate, class_index);
            }
        }

        std::size_t inspected_live_classes = 0;
        std::size_t logged_live_objects = 0;
        for (std::size_t class_index = 0;
             class_index < search.classes.size() &&
                inspected_live_classes < max_live_candidate_classes_ &&
                logged_live_objects < max_live_objects_total_;
             ++class_index)
        {
            Il2CppClass* klass = search.classes[class_index].klass;
            if (!runtime_discovery::inherits_from(
                    klass, "UnityEngine", "Object"))
            {
                continue;
            }
            ++inspected_live_classes;

            Il2CppArray* objects = runtime_discovery::find_all_objects(klass);
            const std::size_t object_count = objects != nullptr ?
                static_cast<std::size_t>(objects->max_length) : 0;
            logger::info("[CustomizationLiveClass] class_index=" +
                std::to_string(class_index) +
                " class='" + runtime_discovery::class_name(klass) +
                "' objects=" + std::to_string(object_count));
            if (objects == nullptr)
            {
                continue;
            }

            const std::size_t sampled_count = (std::min)(
                object_count, max_live_objects_per_class_);
            auto** values = reinterpret_cast<Il2CppObject**>(&objects->data);
            for (std::size_t sample_index = 0;
                 sample_index < sampled_count &&
                    logged_live_objects < max_live_objects_total_;
                 ++sample_index)
            {
                Il2CppObject* object = values[sample_index];
                if (object == nullptr)
                {
                    continue;
                }
                dump_live_object(object, logged_live_objects, class_index);
                ++logged_live_objects;
            }
        }

        logger::info("[CustomizationProbe] complete live_classes=" +
            std::to_string(inspected_live_classes) +
            " live_objects=" + std::to_string(logged_live_objects) +
            " note='no preview, equip, unlock, or profile mutation performed'");
    }

    void run_outfit_authority_discovery() const
    {
        if (assembly_csharp_ == nullptr || !runtime_discovery::initialize())
        {
            logger::warn("[OutfitAuthority] runtime discovery API is unavailable");
            return;
        }

        logger::info("[OutfitAuthority] begin");
        dump_outfit_method_surfaces();
        dump_outfit_solver_owners();
        dump_outfit_catalog();
        logger::info("[OutfitAuthority] complete note='metadata and live "
            "references only; no methods invoked or values mutated'");
    }
};
