#include "outfit_controller.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <unordered_map>
#include <utility>
#include <windows.h>

#include "../../il2cpp/il2utils.h"
#include "../../il2cpp/runtime_discovery.h"
#include "../../util/logger.h"

namespace game::customization::outfit_controller_detail
{
    constexpr std::size_t max_live_views = 16;
    constexpr std::size_t max_live_cards = 128;
    constexpr std::size_t max_catalog_entries = 512;
    constexpr std::size_t max_offer_fields = 32;
    constexpr std::size_t max_offer_methods = 32;
    constexpr std::size_t max_source_enum_fields = 32;
    constexpr std::size_t max_profile_fields = 256;
    constexpr unsigned int field_attribute_static = 0x0010;

    struct MongoIdSnapshot
    {
        std::uint32_t time_stamp;
        std::uint32_t alignment;
        std::uint64_t counter;
        Il2CppString* string_id;
    };

    static_assert(sizeof(MongoIdSnapshot) == 24);

    struct OutfitDictionaryEntry
    {
        std::int32_t hash_code;
        std::int32_t next;
        MongoIdSnapshot key;
        Il2CppObject* value;
    };

    static_assert(sizeof(OutfitDictionaryEntry) == 40);

    [[nodiscard]] bool is_readable_memory(
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

    [[nodiscard]] FieldInfo* find_direct_field(
        Il2CppClass* klass,
        const char* field_name)
    {
        if (klass == nullptr || field_name == nullptr)
        {
            return nullptr;
        }

        FieldInfo* field = il2utils::resolve_field(klass, field_name);
        return field != nullptr && field->parent == klass ? field : nullptr;
    }

    [[nodiscard]] bool direct_field_matches(
        Il2CppClass* klass,
        const char* field_name,
        const std::int32_t expected_offset,
        FieldInfo*& field)
    {
        field = find_direct_field(klass, field_name);
        return field != nullptr && field->offset == expected_offset &&
            field->type != nullptr;
    }

    [[nodiscard]] Il2CppObject* read_reference(
        Il2CppObject* object,
        const FieldInfo* field)
    {
        if (object == nullptr || field == nullptr || field->offset <= 0 ||
            !is_readable_memory(
                object,
                static_cast<std::size_t>(field->offset) +
                    sizeof(Il2CppObject*)))
        {
            return nullptr;
        }

        return *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
    }

    struct BodyCustomizationReference
    {
        Il2CppObject* value = nullptr;
        const FieldInfo* field = nullptr;
        std::size_t matches = 0;
    };

    [[nodiscard]] BodyCustomizationReference find_body_customization(
        Il2CppObject* profile)
    {
        BodyCustomizationReference result;
        if (profile == nullptr || profile->klass == nullptr)
        {
            return result;
        }

        const auto fields = runtime_discovery::collect_fields(
            profile->klass, max_profile_fields, 8);
        for (const FieldInfo* field : fields)
        {
            if (field == nullptr || field->type == nullptr ||
                runtime_discovery::type_name(field->type) !=
                    "EFT.BodyCustomization")
            {
                continue;
            }

            ++result.matches;
            Il2CppObject* candidate = read_reference(profile, field);
            if (candidate == nullptr || candidate->klass == nullptr ||
                runtime_discovery::class_name(candidate->klass) !=
                    "EFT.BodyCustomization")
            {
                continue;
            }

            if (result.value != nullptr)
            {
                result.value = nullptr;
                result.field = nullptr;
                return result;
            }

            result.value = candidate;
            result.field = field;
        }
        return result;
    }

    [[nodiscard]] bool read_boolean(
        Il2CppObject* object,
        const FieldInfo* field)
    {
        if (object == nullptr || field == nullptr || field->offset <= 0 ||
            !is_readable_memory(
                object,
                static_cast<std::size_t>(field->offset) + sizeof(bool)))
        {
            return false;
        }

        return *reinterpret_cast<const bool*>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
    }

    [[nodiscard]] std::string read_string_member(
        Il2CppObject* object,
        const char* field_name)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return {};
        }

        FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->offset <= 0 || field->type == nullptr ||
            field->type->type != IL2CPP_TYPE_STRING ||
            !is_readable_memory(
                object,
                static_cast<std::size_t>(field->offset) +
                    sizeof(Il2CppString*)))
        {
            return {};
        }

        Il2CppString* value = *reinterpret_cast<Il2CppString**>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
        return runtime_discovery::safe_string(value);
    }

    [[nodiscard]] std::string read_mongo_id_member(
        Il2CppObject* object,
        const char* field_name)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return {};
        }

        FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (field == nullptr || field->offset <= 0 || field->type == nullptr ||
            runtime_discovery::type_name(field->type) != "EFT.MongoID")
        {
            return {};
        }

        const auto* value = reinterpret_cast<const MongoIdSnapshot*>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
        if (!is_readable_memory(value, sizeof(MongoIdSnapshot)) ||
            value->string_id == nullptr ||
            !is_readable_memory(
                value->string_id,
                offsetof(Il2CppString, chars)))
        {
            return {};
        }

        const int length = value->string_id->length;
        if (length <= 0 || length > 64 ||
            !is_readable_memory(
                value->string_id,
                offsetof(Il2CppString, chars) +
                    static_cast<std::size_t>(length) * sizeof(wchar_t)))
        {
            return {};
        }
        return runtime_discovery::safe_string(value->string_id);
    }

    [[nodiscard]] std::int32_t read_main_body_part(Il2CppObject* suite)
    {
        if (suite == nullptr || suite->klass == nullptr)
        {
            return -1;
        }

        const MethodInfo* method = il2utils::resolve_method(
            suite->klass, "get_MainBodyPart", 0);
        if (method == nullptr || method->methodPointer == nullptr)
        {
            return -1;
        }

        using GetMainBodyPart = std::int32_t(*)(
            Il2CppObject*, const MethodInfo*);
        const auto get_main_body_part = reinterpret_cast<GetMainBodyPart>(
            method->methodPointer);
        return get_main_body_part(suite, method);
    }

    [[nodiscard]] OutfitBodyRegion classify_body_region(
        const Il2CppObject* suite)
    {
        const std::string class_name = runtime_discovery::class_name(
            suite != nullptr ? suite->klass : nullptr);
        if (class_name.find("UpperBodySuit") != std::string::npos)
        {
            return OutfitBodyRegion::Upper;
        }
        if (class_name.find("LowerBodySuit") != std::string::npos)
        {
            return OutfitBodyRegion::Lower;
        }
        return OutfitBodyRegion::Unknown;
    }

    [[nodiscard]] const char* state_name(const std::int32_t raw_state)
    {
        switch (raw_state)
        {
        case 1:
            return "purchase available";
        case 2:
            return "purchased";
        case 3:
            return "selected";
        default:
            return "unknown";
        }
    }

    [[nodiscard]] bool try_read_static_int32(
        FieldInfo* field,
        std::int32_t& value)
    {
        if (field == nullptr || field->type == nullptr ||
            (field->type->attrs & field_attribute_static) == 0)
        {
            return false;
        }

        __try
        {
            il2cpp::il2cpp_field_static_get_value(field, &value);
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }

    [[nodiscard]] std::string diagnostic_field_value(
        Il2CppObject* object,
        const FieldInfo* field)
    {
        if (object == nullptr || field == nullptr || field->offset <= 0 ||
            field->type == nullptr)
        {
            return "<unavailable>";
        }

        const std::string field_type = runtime_discovery::type_name(
            field->type);
        if (field_type == "EFT.ECustomizationSource" ||
            field_type == "EFT.ECustomizationType")
        {
            const auto* value = reinterpret_cast<const std::int32_t*>(
                reinterpret_cast<std::uintptr_t>(object) + field->offset);
            if (!is_readable_memory(value, sizeof(*value)))
            {
                return "<unavailable>";
            }
            return "raw=" + std::to_string(*value);
        }

        return runtime_discovery::field_value_summary(object, field);
    }
}

namespace game::customization
{
    OutfitController* OutfitController::active_instance_ = nullptr;
    OutfitController::SetProfilesSignature OutfitController::o_set_profiles_ =
        nullptr;

    bool OutfitController::initialize()
    {
        assembly_csharp_ = il2utils::resolve_image("Assembly-CSharp.dll");
        tactical_clothing_view_class_ = const_cast<Il2CppClass*>(
            il2utils::resolve_class(
                assembly_csharp_, "EFT.UI", "TacticalClothingView"));
        clothing_item_class_ = const_cast<Il2CppClass*>(
            il2utils::resolve_class(
                assembly_csharp_, "EFT.UI", "ClothingItem"));
        full_offer_class_ = const_cast<Il2CppClass*>(
            il2utils::resolve_class_nested(clothing_item_class_, "FullOffer"));
        customization_offer_class_ = const_cast<Il2CppClass*>(
            il2utils::resolve_class(
                assembly_csharp_, "EFT.Customization", "CustomizationOffer"));
        customization_source_class_ = const_cast<Il2CppClass*>(
            il2utils::resolve_class(
                assembly_csharp_, "EFT", "ECustomizationSource"));
        customization_solver_class_ = const_cast<Il2CppClass*>(
            il2utils::resolve_class(
                assembly_csharp_, "EFT", "CustomizationSolver"));
        const Il2CppClass* backend_session_class = il2utils::resolve_class(
            assembly_csharp_, "EFT", "EftClientBackendSession");

        const bool view_schema_valid =
            outfit_controller_detail::direct_field_matches(
                tactical_clothing_view_class_, "_views", 288, views_field_) &&
            outfit_controller_detail::direct_field_matches(
                tactical_clothing_view_class_, "_solver", 304,
                solver_field_) &&
            outfit_controller_detail::direct_field_matches(
                tactical_clothing_view_class_, "_profile", 240,
                profile_field_) &&
            outfit_controller_detail::direct_field_matches(
                tactical_clothing_view_class_, "_currentUpperBodySuite", 312,
                current_upper_field_) &&
            outfit_controller_detail::direct_field_matches(
                tactical_clothing_view_class_, "_currentLowerBodySuite", 320,
                current_lower_field_);
        const bool card_schema_valid =
            outfit_controller_detail::direct_field_matches(
                clothing_item_class_, "_previewed", 281, previewed_field_) &&
            outfit_controller_detail::direct_field_matches(
                clothing_item_class_, "<State>k__BackingField", 284,
                state_field_) &&
            outfit_controller_detail::direct_field_matches(
                clothing_item_class_, "<IsPurchased>k__BackingField", 288,
                purchased_field_) &&
            outfit_controller_detail::direct_field_matches(
                clothing_item_class_, "<Offer>k__BackingField", 296,
                offer_field_);
        const bool offer_schema_valid =
            outfit_controller_detail::direct_field_matches(
                full_offer_class_, "Suite", 24, suite_field_) &&
            outfit_controller_detail::direct_field_matches(
                full_offer_class_, "Offer", 16,
                customization_offer_field_) &&
            outfit_controller_detail::direct_field_matches(
                customization_offer_class_, "ExternalObtain", 32,
                external_obtain_field_) &&
            outfit_controller_detail::direct_field_matches(
                customization_offer_class_, "InternalObtain", 33,
                internal_obtain_field_);
        const bool solver_schema_valid =
            outfit_controller_detail::direct_field_matches(
                customization_solver_class_,
                "_customizationSuiteTemplates",
                32,
                suite_templates_field_);
        backend_profile_field_ = backend_session_class != nullptr ?
            il2utils::resolve_field(backend_session_class, "_profile") :
            nullptr;
        const bool backend_profile_schema_valid =
            backend_profile_field_ != nullptr &&
            backend_profile_field_->offset > 0 &&
            backend_profile_field_->type != nullptr &&
            runtime_discovery::type_name(backend_profile_field_->type) ==
                "EFT.Profile";

        on_test_fit_method_ = tactical_clothing_view_class_ != nullptr ?
            il2utils::resolve_method(
                tactical_clothing_view_class_, "OnTestFit", 2) : nullptr;
        on_equip_method_ = tactical_clothing_view_class_ != nullptr ?
            il2utils::resolve_method(
                tactical_clothing_view_class_, "OnEquip", 2) : nullptr;
        get_current_profile_method_ =
            tactical_clothing_view_class_ != nullptr ?
            il2utils::resolve_method(
                tactical_clothing_view_class_, "get_CurrentProfile", 0) :
            nullptr;
        update_preview_method_ =
            tactical_clothing_view_class_ != nullptr ?
            il2utils::resolve_method(
                tactical_clothing_view_class_, "UpdatePreview", 0) :
            nullptr;

        active_instance_ = this;
        if (backend_profile_schema_valid && o_set_profiles_ == nullptr)
        {
            il2utils::hook_method(
                backend_session_class,
                "SetProfiles",
                1,
                hk_set_profiles,
                &o_set_profiles_);
        }
        const bool profile_refresh_hooked = o_set_profiles_ != nullptr;

        initialized_ = assembly_csharp_ != nullptr &&
            runtime_discovery::initialize() && view_schema_valid &&
            card_schema_valid && offer_schema_valid && solver_schema_valid &&
            on_test_fit_method_ != nullptr &&
            on_test_fit_method_->methodPointer != nullptr &&
            on_equip_method_ != nullptr &&
            on_equip_method_->methodPointer != nullptr &&
            get_current_profile_method_ != nullptr &&
            get_current_profile_method_->methodPointer != nullptr &&
            update_preview_method_ != nullptr &&
            update_preview_method_->methodPointer != nullptr;

        status_ = initialized_ ?
            "Tactical clothing screen not detected" :
            "Outfit runtime schema unavailable";
        {
            std::scoped_lock lock(snapshot_mutex_);
            snapshot_.initialized = initialized_;
            snapshot_.status = status_;
        }
        logger::info("[OutfitTrainer] initialized=" +
            std::to_string(initialized_ ? 1 : 0) +
            " view_schema=" + std::to_string(view_schema_valid ? 1 : 0) +
            " card_schema=" + std::to_string(card_schema_valid ? 1 : 0) +
            " offer_schema=" + std::to_string(offer_schema_valid ? 1 : 0) +
            " solver_schema=" +
            std::to_string(solver_schema_valid ? 1 : 0) +
            " backend_profile_schema=" +
            std::to_string(backend_profile_schema_valid ? 1 : 0) +
            " profile_refresh_hook=" +
            std::to_string(profile_refresh_hooked ? 1 : 0) +
            " preview_method=" +
            std::to_string(on_test_fit_method_ != nullptr ? 1 : 0) +
            " equip_method=" +
            std::to_string(on_equip_method_ != nullptr ? 1 : 0) +
            " current_profile_method=" +
            std::to_string(
                get_current_profile_method_ != nullptr ? 1 : 0) +
            " update_preview_method=" +
            std::to_string(update_preview_method_ != nullptr ? 1 : 0));
        return initialized_;
    }

    OutfitSnapshot OutfitController::snapshot() const
    {
        std::scoped_lock lock(snapshot_mutex_);
        return snapshot_;
    }

    void OutfitController::queue_command(Command command)
    {
        std::scoped_lock lock(command_mutex_);
        pending_commands_.push_back(std::move(command));
    }

    void OutfitController::request_refresh()
    {
        queue_command({ CommandType::Refresh, {} });
    }

    void OutfitController::request_preview(const std::string& outfit_id)
    {
        queue_command({ CommandType::Preview, outfit_id });
    }

    void OutfitController::request_equip(const std::string& outfit_id)
    {
        queue_command({ CommandType::Equip, outfit_id });
    }

    void OutfitController::request_restore()
    {
        queue_command({ CommandType::Restore, {} });
    }

    void OutfitController::request_reapply()
    {
        queue_command({ CommandType::Reapply, {} });
    }

    void OutfitController::set_enforced_outfits(
        const std::string& upper_id,
        const std::string& lower_id)
    {
        if (enforced_upper_id_ == upper_id &&
            enforced_lower_id_ == lower_id)
        {
            return;
        }

        enforced_upper_id_ = upper_id;
        enforced_lower_id_ = lower_id;
        request_reapply();
    }

    void OutfitController::update()
    {
        if (!initialized_)
        {
            return;
        }

        std::vector<Command> commands;
        {
            std::scoped_lock lock(command_mutex_);
            commands.swap(pending_commands_);
        }

        if (commands.empty())
        {
            return;
        }

        bool apply_enforced_outfits = false;
        bool refresh_requested = false;
        for (const Command& command : commands)
        {
            if (command.type == CommandType::ProfileReapply)
            {
                apply_enforced_outfits_to_profile(command.profile);
                continue;
            }
            if (command.type == CommandType::Reapply)
            {
                apply_enforced_outfits = true;
                refresh_requested = true;
                continue;
            }
            if (command.type == CommandType::Refresh)
            {
                apply_enforced_outfits = true;
            }
            refresh_requested = true;
            process_command(command);
        }

        if (refresh_requested)
        {
            refresh_snapshot(apply_enforced_outfits);
        }
    }

    void OutfitController::hk_set_profiles(
        Il2CppObject* instance,
        Il2CppArray* profiles)
    {
        if (o_set_profiles_ != nullptr)
        {
            o_set_profiles_(instance, profiles);
        }

        if (active_instance_ != nullptr)
        {
            active_instance_->queue_profile_reapply(instance);
        }
    }

    void OutfitController::queue_profile_reapply(
        Il2CppObject* backend_session)
    {
        Il2CppObject* profile = outfit_controller_detail::read_reference(
            backend_session, backend_profile_field_);
        if (profile == nullptr || profile->klass == nullptr ||
            runtime_discovery::class_name(profile->klass) != "EFT.Profile")
        {
            logger::warn(
                "[OutfitTrainer] profile refresh reapply skipped reason='profile_unavailable'");
            return;
        }

        queue_command({ CommandType::ProfileReapply, {}, profile });
        logger::info(
            "[OutfitTrainer] queued profile refresh reapply profile=" +
            std::to_string(reinterpret_cast<std::uintptr_t>(profile)));
    }

    Il2CppObject* OutfitController::find_live_view() const
    {
        if (tactical_clothing_view_class_ == nullptr)
        {
            return nullptr;
        }

        Il2CppArray* views = runtime_discovery::find_all_objects(
            tactical_clothing_view_class_);
        if (views == nullptr || views->klass == nullptr ||
            views->klass->element_size != sizeof(Il2CppObject*))
        {
            return nullptr;
        }

        auto** values = reinterpret_cast<Il2CppObject**>(&views->data);
        const std::size_t sampled_count = (std::min)(
            static_cast<std::size_t>(views->max_length),
            outfit_controller_detail::max_live_views);
        for (std::size_t index = 0; index < sampled_count; ++index)
        {
            Il2CppObject* candidate = values[index];
            if (candidate == nullptr || candidate->klass !=
                    tactical_clothing_view_class_)
            {
                continue;
            }

            Il2CppObject* solver = outfit_controller_detail::read_reference(
                candidate, solver_field_);
            if (solver != nullptr &&
                solver->klass == customization_solver_class_)
            {
                return candidate;
            }
        }
        return nullptr;
    }

    bool OutfitController::capture_live_context(LiveContext& context) const
    {
        context = {};
        context.view = find_live_view();
        if (context.view == nullptr)
        {
            return false;
        }

        Il2CppObject* current_upper =
            outfit_controller_detail::read_reference(
                context.view, current_upper_field_);
        Il2CppObject* current_lower =
            outfit_controller_detail::read_reference(
                context.view, current_lower_field_);
        context.current_upper_id =
            outfit_controller_detail::read_mongo_id_member(
                current_upper, "Id");
        context.current_lower_id =
            outfit_controller_detail::read_mongo_id_member(
                current_lower, "Id");

        Il2CppObject* solver = outfit_controller_detail::read_reference(
            context.view, solver_field_);
        Il2CppObject* suite_templates =
            outfit_controller_detail::read_reference(
                solver, suite_templates_field_);
        Il2CppObject* entries_object = nullptr;
        int dictionary_count = 0;
        if (suite_templates != nullptr)
        {
            FieldInfo* entries_field = il2utils::resolve_field(
                suite_templates->klass, "_entries");
            entries_object = outfit_controller_detail::read_reference(
                suite_templates, entries_field);
            runtime_discovery::read_int_field(
                suite_templates, "_count", dictionary_count);
        }

        auto* dictionary_entries =
            reinterpret_cast<Il2CppArray*>(entries_object);
        std::unordered_map<std::string, std::size_t> entry_indices;
        if (dictionary_count > 0 && dictionary_entries != nullptr &&
            dictionary_entries->klass != nullptr &&
            dictionary_entries->klass->element_size ==
                sizeof(outfit_controller_detail::OutfitDictionaryEntry))
        {
            const std::size_t sampled_count = (std::min)(
                (std::min)(
                    static_cast<std::size_t>(dictionary_count),
                    static_cast<std::size_t>(
                        dictionary_entries->max_length)),
                outfit_controller_detail::max_catalog_entries);
            const auto* values = reinterpret_cast<
                const outfit_controller_detail::OutfitDictionaryEntry*>(
                    &dictionary_entries->data);
            context.entries.reserve(sampled_count);
            entry_indices.reserve(sampled_count);
            for (std::size_t index = 0; index < sampled_count; ++index)
            {
                const auto& dictionary_entry = values[index];
                Il2CppObject* suite = dictionary_entry.value;
                if (dictionary_entry.hash_code < 0 ||
                    suite == nullptr || suite->klass == nullptr ||
                    !runtime_discovery::inherits_from(
                        suite->klass,
                        "EFT.Customization",
                        "CustomizationSuite"))
                {
                    continue;
                }

                RuntimeEntry runtime_entry;
                runtime_entry.suite = suite;
                runtime_entry.value.id =
                    outfit_controller_detail::read_mongo_id_member(
                        suite, "Id");
                if (runtime_entry.value.id.empty() ||
                    entry_indices.contains(runtime_entry.value.id))
                {
                    continue;
                }

                runtime_entry.value.name =
                    outfit_controller_detail::read_string_member(
                        suite, "Name");
                if (runtime_entry.value.name.empty())
                {
                    runtime_entry.value.name = runtime_entry.value.id;
                }
                runtime_entry.value.class_name =
                    runtime_discovery::class_name(suite->klass);
                runtime_entry.value.body_region =
                    outfit_controller_detail::classify_body_region(suite);
                runtime_entry.value.body_part =
                    outfit_controller_detail::read_main_body_part(suite);
                runtime_entry.value.external_obtain =
                    !outfit_controller_detail::read_string_member(
                        suite, "ExpansionsUrl").empty();
                runtime_entry.value.trainer_eligible =
                    !runtime_entry.value.external_obtain;
                runtime_entry.value.state =
                    runtime_entry.value.external_obtain ?
                        "purchase available" : "unknown";
                runtime_entry.value.equipped =
                    runtime_entry.value.id == context.current_upper_id ||
                    runtime_entry.value.id == context.current_lower_id;

                entry_indices.emplace(
                    runtime_entry.value.id,
                    context.entries.size());
                context.entries.push_back(std::move(runtime_entry));
            }
        }

        Il2CppObject* list = outfit_controller_detail::read_reference(
            context.view, views_field_);
        Il2CppArray* items = nullptr;
        std::size_t item_count = 0;
        if (!runtime_discovery::collection_elements(
                list, items, item_count) ||
            items == nullptr || items->klass == nullptr ||
            items->klass->element_size != sizeof(Il2CppObject*))
        {
            return !context.entries.empty();
        }

        const std::size_t sampled_count = (std::min)(
            item_count, outfit_controller_detail::max_live_cards);
        auto** values = reinterpret_cast<Il2CppObject**>(&items->data);
        context.entries.reserve(context.entries.size() + sampled_count);
        for (std::size_t index = 0; index < sampled_count; ++index)
        {
            Il2CppObject* card = values[index];
            if (card == nullptr || card->klass != clothing_item_class_ ||
                !outfit_controller_detail::is_readable_memory(
                    card,
                    static_cast<std::size_t>(offer_field_->offset) +
                        sizeof(Il2CppObject*)))
            {
                continue;
            }

            Il2CppObject* full_offer = outfit_controller_detail::read_reference(
                card, offer_field_);
            if (full_offer == nullptr || full_offer->klass != full_offer_class_)
            {
                continue;
            }

            Il2CppObject* suite = outfit_controller_detail::read_reference(
                full_offer, suite_field_);
            if (suite == nullptr || suite->klass == nullptr)
            {
                continue;
            }

            RuntimeEntry runtime_entry;
            runtime_entry.card = card;
            runtime_entry.full_offer = full_offer;
            runtime_entry.customization_offer =
                outfit_controller_detail::read_reference(
                    full_offer, customization_offer_field_);
            runtime_entry.suite = suite;
            runtime_entry.value.id =
                outfit_controller_detail::read_mongo_id_member(suite, "Id");
            if (runtime_entry.value.id.empty())
            {
                continue;
            }

            runtime_entry.value.name =
                outfit_controller_detail::read_string_member(suite, "Name");
            if (runtime_entry.value.name.empty())
            {
                runtime_entry.value.name = runtime_entry.value.id;
            }
            runtime_entry.value.class_name =
                runtime_discovery::class_name(suite->klass);
            runtime_entry.value.body_region =
                outfit_controller_detail::classify_body_region(suite);
            runtime_entry.value.body_part =
                outfit_controller_detail::read_main_body_part(suite);

            const auto card_address = reinterpret_cast<std::uintptr_t>(card);
            runtime_entry.value.previewed = *reinterpret_cast<const bool*>(
                card_address + previewed_field_->offset);
            runtime_entry.value.raw_state =
                *reinterpret_cast<const std::int32_t*>(
                    card_address + state_field_->offset);
            runtime_entry.value.purchased = *reinterpret_cast<const bool*>(
                card_address + purchased_field_->offset);
            runtime_entry.value.external_obtain =
                outfit_controller_detail::read_boolean(
                    runtime_entry.customization_offer,
                    external_obtain_field_) ||
                !outfit_controller_detail::read_string_member(
                    suite, "ExpansionsUrl").empty();
            runtime_entry.value.internal_obtain =
                outfit_controller_detail::read_boolean(
                    runtime_entry.customization_offer,
                    internal_obtain_field_);
            runtime_entry.value.trainer_eligible =
                !runtime_entry.value.external_obtain;
            runtime_entry.value.state = outfit_controller_detail::state_name(
                runtime_entry.value.raw_state);
            runtime_entry.value.equipped =
                runtime_entry.value.id == context.current_upper_id ||
                runtime_entry.value.id == context.current_lower_id;

            const auto existing = entry_indices.find(
                runtime_entry.value.id);
            if (existing != entry_indices.end())
            {
                context.entries[existing->second] =
                    std::move(runtime_entry);
            }
            else
            {
                entry_indices.emplace(
                    runtime_entry.value.id,
                    context.entries.size());
                context.entries.push_back(std::move(runtime_entry));
            }
        }

        return !context.entries.empty();
    }

    void OutfitController::publish_snapshot(const LiveContext* context)
    {
        OutfitSnapshot next;
        next.initialized = initialized_;
        next.status = status_;
        next.original_upper_id = original_upper_id_;
        next.original_lower_id = original_lower_id_;
        next.enforced_upper_id = enforced_upper_id_;
        next.enforced_lower_id = enforced_lower_id_;
        if (context != nullptr)
        {
            next.view_ready = true;
            next.current_upper_id = context->current_upper_id;
            next.current_lower_id = context->current_lower_id;
            next.entries.reserve(context->entries.size());
            for (const RuntimeEntry& entry : context->entries)
            {
                next.entries.push_back(entry.value);
            }
        }

        std::scoped_lock lock(snapshot_mutex_);
        snapshot_ = std::move(next);
    }

    void OutfitController::refresh_snapshot(
        const bool apply_enforced_outfits)
    {
        LiveContext context;
        if (!capture_live_context(context))
        {
            observed_view_ = nullptr;
            status_ = "Tactical clothing screen not detected";
            publish_snapshot(nullptr);
            return;
        }

        if (context.view != observed_view_)
        {
            observed_view_ = context.view;
            original_upper_id_ = context.current_upper_id;
            original_lower_id_ = context.current_lower_id;
            logger::info("[OutfitTrainer] captured original upper='" +
                original_upper_id_ + "' lower='" + original_lower_id_ + "'");
        }

        cache_enforced_suites(context);
        if (apply_enforced_outfits && enforce_applied_outfits(context))
        {
            LiveContext refreshed_context;
            if (capture_live_context(refreshed_context))
            {
                context = std::move(refreshed_context);
            }
        }

        if (status_.empty() ||
            status_ == "Tactical clothing screen not detected")
        {
            status_ = "Live outfits: " +
                std::to_string(context.entries.size());
        }
        publish_snapshot(&context);
    }

    bool OutfitController::enforce_applied_outfits(LiveContext& context)
    {
        const auto find_entry = [&](const std::string& id)
            -> const RuntimeEntry*
        {
            const auto iterator = std::find_if(
                context.entries.begin(),
                context.entries.end(),
                [&](const RuntimeEntry& entry)
                {
                    return entry.value.id == id;
                });
            return iterator != context.entries.end() ? &*iterator : nullptr;
        };

        const auto enforce = [&](const std::string& outfit_id,
                                 const std::string& current_id) -> bool
        {
            if (outfit_id.empty() || outfit_id == current_id)
            {
                return false;
            }

            const RuntimeEntry* entry = find_entry(outfit_id);
            if (entry == nullptr || !entry->value.trainer_eligible)
            {
                return false;
            }

            if (invoke_internal_apply(context.view, *entry))
            {
                logger::info(
                    "[OutfitTrainer] reapplied id='" + entry->value.id +
                    "' name='" + entry->value.name + "' body_part=" +
                    std::to_string(entry->value.body_part));
                return true;
            }
            return false;
        };

        const bool upper_applied = enforce(
            enforced_upper_id_, context.current_upper_id);
        const bool lower_applied = enforce(
            enforced_lower_id_, context.current_lower_id);
        return upper_applied || lower_applied;
    }

    void OutfitController::cache_enforced_suites(const LiveContext& context)
    {
        for (const RuntimeEntry& entry : context.entries)
        {
            if (!entry.value.trainer_eligible || entry.suite == nullptr)
            {
                continue;
            }

            if (!enforced_upper_id_.empty() &&
                entry.value.id == enforced_upper_id_)
            {
                enforced_upper_suite_ = entry.suite;
            }
            if (!enforced_lower_id_.empty() &&
                entry.value.id == enforced_lower_id_)
            {
                enforced_lower_suite_ = entry.suite;
            }
        }
    }

    bool OutfitController::apply_suite_to_profile(
        Il2CppObject* profile,
        Il2CppObject* suite) const
    {
        if (profile == nullptr || suite == nullptr || suite->klass == nullptr)
        {
            return false;
        }

        const auto customization =
            outfit_controller_detail::find_body_customization(profile);
        if (customization.value == nullptr ||
            customization.field == nullptr ||
            customization.matches != 1)
        {
            return false;
        }

        const MethodInfo* set_clothings_to_profile =
            il2utils::resolve_method(
                suite->klass, "SetClothingsToProfile", 1);
        if (set_clothings_to_profile == nullptr ||
            set_clothings_to_profile->methodPointer == nullptr)
        {
            return false;
        }

        using SetClothingsToProfile = void (*)(
            Il2CppObject*, Il2CppObject*, const MethodInfo*);
        const auto set_clothings =
            reinterpret_cast<SetClothingsToProfile>(
                set_clothings_to_profile->methodPointer);
        set_clothings(
            suite,
            customization.value,
            set_clothings_to_profile);
        return true;
    }

    bool OutfitController::apply_enforced_outfits_to_profile(
        Il2CppObject* profile)
    {
        std::size_t applied_count = 0;
        const auto apply = [&](
            const std::string& outfit_id,
            Il2CppObject* suite)
        {
            if (outfit_id.empty() || suite == nullptr ||
                outfit_controller_detail::read_mongo_id_member(
                    suite, "Id") != outfit_id)
            {
                return;
            }

            if (apply_suite_to_profile(profile, suite))
            {
                ++applied_count;
            }
        };

        apply(enforced_upper_id_, enforced_upper_suite_);
        apply(enforced_lower_id_, enforced_lower_suite_);

        if (applied_count > 0)
        {
            set_status(
                "Reapplied " + std::to_string(applied_count) +
                " outfit selection" + (applied_count == 1 ? "" : "s"));
        }
        logger::info(
            "[OutfitTrainer] profile refresh reapply profile=" +
            std::to_string(reinterpret_cast<std::uintptr_t>(profile)) +
            " applied=" + std::to_string(applied_count));
        return applied_count > 0;
    }

    void OutfitController::set_enforced_outfit(const RuntimeEntry& entry)
    {
        if (entry.value.body_region == OutfitBodyRegion::Upper)
        {
            enforced_upper_id_ = entry.value.id;
            enforced_upper_suite_ = entry.suite;
        }
        else if (entry.value.body_region == OutfitBodyRegion::Lower)
        {
            enforced_lower_id_ = entry.value.id;
            enforced_lower_suite_ = entry.suite;
        }
    }

    void OutfitController::clear_enforced_outfit(
        const OutfitBodyRegion body_region)
    {
        if (body_region == OutfitBodyRegion::Upper)
        {
            enforced_upper_id_.clear();
            enforced_upper_suite_ = nullptr;
        }
        else if (body_region == OutfitBodyRegion::Lower)
        {
            enforced_lower_id_.clear();
            enforced_lower_suite_ = nullptr;
        }
    }

    void OutfitController::set_status(std::string status)
    {
        status_ = std::move(status);
        std::scoped_lock lock(snapshot_mutex_);
        snapshot_.status = status_;
    }

    bool OutfitController::invoke_preview(
        Il2CppObject* view,
        const RuntimeEntry& entry) const
    {
        if (view == nullptr || entry.card == nullptr ||
            entry.value.body_part < 0 || on_test_fit_method_ == nullptr ||
            on_test_fit_method_->methodPointer == nullptr)
        {
            return false;
        }

        using OnTestFit = void(*)(
            Il2CppObject*, std::int32_t, Il2CppObject*, const MethodInfo*);
        const auto on_test_fit = reinterpret_cast<OnTestFit>(
            on_test_fit_method_->methodPointer);
        on_test_fit(
            view, entry.value.body_part, entry.card, on_test_fit_method_);
        return true;
    }

    Il2CppObject* OutfitController::invoke_equip(
        Il2CppObject* view,
        const RuntimeEntry& entry) const
    {
        if (view == nullptr || entry.card == nullptr ||
            entry.value.body_part < 0 || on_equip_method_ == nullptr ||
            on_equip_method_->methodPointer == nullptr)
        {
            return nullptr;
        }

        using Equip = Il2CppObject*(*)(
            Il2CppObject*, std::int32_t, Il2CppObject*, const MethodInfo*);
        const auto equip = reinterpret_cast<Equip>(
            on_equip_method_->methodPointer);
        return equip(
            view, entry.value.body_part, entry.card, on_equip_method_);
    }

    bool OutfitController::invoke_internal_apply(
        Il2CppObject* view,
        const RuntimeEntry& entry) const
    {
        if (view == nullptr ||
            entry.suite == nullptr ||
            entry.suite->klass == nullptr ||
            entry.value.body_part < 0 ||
            !entry.value.trainer_eligible ||
            profile_field_ == nullptr ||
            get_current_profile_method_ == nullptr ||
            get_current_profile_method_->methodPointer == nullptr ||
            update_preview_method_ == nullptr ||
            update_preview_method_->methodPointer == nullptr)
        {
            return false;
        }

        using GetCurrentProfile = Il2CppObject* (*)(
            Il2CppObject*, const MethodInfo*);
        const auto get_current_profile =
            reinterpret_cast<GetCurrentProfile>(
                get_current_profile_method_->methodPointer);
        const bool selection_synchronized = invoke_preview(view, entry);
        Il2CppObject* authoritative_profile =
            outfit_controller_detail::read_reference(view, profile_field_);
        Il2CppObject* preview_profile = get_current_profile(
            view, get_current_profile_method_);
        const auto authoritative_customization =
            outfit_controller_detail::find_body_customization(
                authoritative_profile);
        const auto preview_customization =
            outfit_controller_detail::find_body_customization(preview_profile);
        if (authoritative_profile == nullptr ||
            authoritative_customization.value == nullptr ||
            authoritative_customization.field == nullptr ||
            authoritative_customization.matches != 1)
        {
            logger::warn(
                "[OutfitTrainerApply] rejected reason='body_customization_unavailable'"
                " authoritative_profile_class='" +
                runtime_discovery::class_name(
                    authoritative_profile != nullptr ?
                        authoritative_profile->klass : nullptr) +
                "' matching_fields=" +
                std::to_string(authoritative_customization.matches) +
                " selection_synchronized=" +
                std::to_string(selection_synchronized ? 1 : 0));
            return false;
        }

        const MethodInfo* set_clothings_to_profile =
            il2utils::resolve_method(
                entry.suite->klass, "SetClothingsToProfile", 1);
        if (set_clothings_to_profile == nullptr ||
            set_clothings_to_profile->methodPointer == nullptr)
        {
            logger::warn(
                "[OutfitTrainerApply] rejected reason='suite_apply_method_unavailable'"
                " suite_class='" +
                runtime_discovery::class_name(entry.suite->klass) + "'");
            return false;
        }

        using SetClothingsToProfile = void (*)(
            Il2CppObject*, Il2CppObject*, const MethodInfo*);
        const auto set_clothings =
            reinterpret_cast<SetClothingsToProfile>(
                set_clothings_to_profile->methodPointer);
        set_clothings(
            entry.suite,
            authoritative_customization.value,
            set_clothings_to_profile);

        bool preview_profile_applied = false;
        if (preview_profile != nullptr &&
            preview_profile != authoritative_profile &&
            preview_customization.value != nullptr &&
            preview_customization.value != authoritative_customization.value &&
            preview_customization.field != nullptr &&
            preview_customization.matches == 1)
        {
            set_clothings(
                entry.suite,
                preview_customization.value,
                set_clothings_to_profile);
            preview_profile_applied = true;
        }

        using UpdatePreview = void (*)(
            Il2CppObject*, const MethodInfo*);
        const auto update_preview = reinterpret_cast<UpdatePreview>(
            update_preview_method_->methodPointer);
        update_preview(view, update_preview_method_);

        logger::info(
            "[OutfitTrainerApply] suite='" + entry.value.id +
            "' suite_class='" +
            runtime_discovery::class_name(entry.suite->klass) +
            "' authoritative_profile=" +
            std::to_string(
                reinterpret_cast<std::uintptr_t>(authoritative_profile)) +
            " preview_profile=" +
            std::to_string(
                reinterpret_cast<std::uintptr_t>(preview_profile)) +
            " same_profile=" +
            std::to_string(
                authoritative_profile == preview_profile ? 1 : 0) +
            " authoritative_customization_field='" +
            std::string(
                authoritative_customization.field->name != nullptr ?
                    authoritative_customization.field->name : "<unnamed>") +
            "' authoritative_customization=" +
            std::to_string(
                reinterpret_cast<std::uintptr_t>(
                    authoritative_customization.value)) +
            " preview_customization=" +
            std::to_string(
                reinterpret_cast<std::uintptr_t>(
                    preview_customization.value)) +
            " selection_synchronized=" +
            std::to_string(selection_synchronized ? 1 : 0) +
            " authoritative_applied=1 preview_applied=" +
            std::to_string(preview_profile_applied ? 1 : 0) +
            " preview_refreshed=1");
        return true;
    }

    void OutfitController::log_customization_source_enum()
    {
        if (customization_source_enum_logged_)
        {
            return;
        }
        customization_source_enum_logged_ = true;

        if (customization_source_class_ == nullptr)
        {
            logger::warn("[OutfitSourceEnum] class_unavailable");
            return;
        }

        const std::size_t field_count = (std::min)(
            static_cast<std::size_t>(customization_source_class_->field_count),
            outfit_controller_detail::max_source_enum_fields);
        logger::info("[OutfitSourceEnum] class='" +
            runtime_discovery::class_name(customization_source_class_) +
            "' fields=" + std::to_string(field_count));

        for (std::size_t index = 0; index < field_count; ++index)
        {
            FieldInfo* field = &customization_source_class_->fields[index];
            if (field->name == nullptr || field->type == nullptr)
            {
                continue;
            }

            std::int32_t value = 0;
            const bool value_read =
                outfit_controller_detail::try_read_static_int32(field, value);
            logger::info("[OutfitSourceEnumValue] index=" +
                std::to_string(index) + " name='" + field->name +
                "' offset=" + std::to_string(field->offset) +
                " attrs=" + std::to_string(field->type->attrs) +
                " static=" + std::to_string(
                    (field->type->attrs &
                        outfit_controller_detail::field_attribute_static) != 0 ?
                        1 : 0) +
                " value=" + (value_read ? std::to_string(value) : "<unread>"));
        }
    }

    void OutfitController::log_offer_diagnostics(
        const RuntimeEntry& entry)
    {
        log_customization_source_enum();

        Il2CppObject* offer = entry.customization_offer;
        logger::info("[OutfitOfferProbe] id='" + entry.value.id +
            "' name='" + entry.value.name + "' purchased=" +
            std::to_string(entry.value.purchased ? 1 : 0) +
            " raw_state=" + std::to_string(entry.value.raw_state) +
            " full_offer=" + std::to_string(
                reinterpret_cast<std::uintptr_t>(entry.full_offer)) +
            " offer=" + std::to_string(
                reinterpret_cast<std::uintptr_t>(offer)));
        if (offer == nullptr || offer->klass == nullptr)
        {
            logger::warn("[OutfitOfferProbe] selected offer is unavailable");
            return;
        }

        const auto fields = runtime_discovery::collect_fields(
            offer->klass, outfit_controller_detail::max_offer_fields, 4);
        logger::info("[OutfitOfferSurface] class='" +
            runtime_discovery::class_name(offer->klass) + "' fields=" +
            std::to_string(fields.size()));
        for (std::size_t index = 0; index < fields.size(); ++index)
        {
            const FieldInfo* field = fields[index];
            if (field == nullptr || field->name == nullptr ||
                field->type == nullptr)
            {
                continue;
            }

            logger::info("[OutfitOfferField] index=" +
                std::to_string(index) + " declaring_class='" +
                runtime_discovery::class_name(field->parent) + "' name='" +
                field->name + "' offset=" + std::to_string(field->offset) +
                " type='" + runtime_discovery::type_name(field->type) +
                "' value=" +
                outfit_controller_detail::diagnostic_field_value(
                    offer, field));
        }

        const std::size_t method_count = (std::min)(
            static_cast<std::size_t>(offer->klass->method_count),
            outfit_controller_detail::max_offer_methods);
        logger::info("[OutfitOfferMethods] class='" +
            runtime_discovery::class_name(offer->klass) + "' methods=" +
            std::to_string(method_count));
        for (std::size_t index = 0; index < method_count; ++index)
        {
            const MethodInfo* method = offer->klass->methods[index];
            if (method == nullptr || method->name == nullptr)
            {
                continue;
            }

            logger::info("[OutfitOfferMethod] index=" +
                std::to_string(index) + " name='" + method->name +
                "' params=" + std::to_string(method->parameters_count) +
                " return='" +
                runtime_discovery::type_name(method->return_type) + "'");
        }
    }

    void OutfitController::process_command(const Command& command)
    {
        if (command.type == CommandType::Refresh)
        {
            status_.clear();
            return;
        }

        LiveContext context;
        if (!capture_live_context(context))
        {
            set_status("Tactical clothing screen not detected");
            logger::warn("[OutfitTrainer] command rejected reason='view_unavailable'");
            return;
        }

        const auto find_entry = [&](const std::string& id)
            -> const RuntimeEntry*
        {
            const auto iterator = std::find_if(
                context.entries.begin(),
                context.entries.end(),
                [&](const RuntimeEntry& entry)
                {
                    return entry.value.id == id;
                });
            return iterator != context.entries.end() ? &*iterator : nullptr;
        };

        if (command.type == CommandType::Restore)
        {
            enforced_upper_id_.clear();
            enforced_lower_id_.clear();
            const RuntimeEntry* upper = find_entry(original_upper_id_);
            const RuntimeEntry* lower = find_entry(original_lower_id_);
            std::size_t restored = 0;
            if (upper != nullptr && upper->value.purchased &&
                invoke_equip(context.view, *upper) != nullptr)
            {
                ++restored;
            }
            if (lower != nullptr && lower->value.purchased &&
                invoke_equip(context.view, *lower) != nullptr)
            {
                ++restored;
            }

            set_status(restored == 2 ?
                "Original outfit restore requested" :
                "Original outfit is not fully available on this screen");
            logger::info("[OutfitTrainer] restore requested upper='" +
                original_upper_id_ + "' lower='" + original_lower_id_ +
                "' invoked=" + std::to_string(restored));
            return;
        }

        const RuntimeEntry* entry = find_entry(command.outfit_id);
        if (entry == nullptr)
        {
            set_status("Selected outfit is no longer available");
            logger::warn("[OutfitTrainer] command rejected id='" +
                command.outfit_id + "' reason='card_unavailable'");
            return;
        }

        if (command.type == CommandType::Preview)
        {
            log_offer_diagnostics(*entry);
            const bool invoked = invoke_preview(context.view, *entry);
            set_status(invoked ?
                "Preview requested: " + entry->value.name :
                "Preview method unavailable for this outfit");
            logger::info("[OutfitTrainer] preview id='" + entry->value.id +
                "' name='" + entry->value.name + "' body_part=" +
                std::to_string(entry->value.body_part) + " invoked=" +
                std::to_string(invoked ? 1 : 0));
            return;
        }

        const bool owned_or_equipped =
            entry->value.purchased || entry->value.equipped;
        if (!owned_or_equipped && !entry->value.trainer_eligible)
        {
            set_status(entry->value.external_obtain ?
                "Preview only: externally obtained outfit" :
                "Preview only: outfit is not internally obtainable");
            logger::warn("[OutfitTrainer] equip rejected id='" +
                entry->value.id + "' reason='source_not_eligible'" +
                " external=" +
                std::to_string(entry->value.external_obtain ? 1 : 0) +
                " internal=" +
                std::to_string(entry->value.internal_obtain ? 1 : 0));
            return;
        }

        if (owned_or_equipped)
        {
            Il2CppObject* task = invoke_equip(context.view, *entry);
            if (task != nullptr)
            {
                clear_enforced_outfit(entry->value.body_region);
            }
            set_status(task != nullptr ?
                "Equip requested: " + entry->value.name :
                "Equip request did not return a task");
            logger::info("[OutfitTrainer] equip id='" + entry->value.id +
                "' name='" + entry->value.name + "' body_part=" +
                std::to_string(entry->value.body_part) + " task=" +
                std::to_string(reinterpret_cast<std::uintptr_t>(task)) +
                " path='owned'");
            return;
        }

        const bool applied = invoke_internal_apply(context.view, *entry);
        if (applied)
        {
            set_enforced_outfit(*entry);
        }
        set_status(applied ?
            "Applied locally: " + entry->value.name :
            "Local outfit apply was unavailable");
        logger::info("[OutfitTrainer] equip id='" + entry->value.id +
            "' name='" + entry->value.name + "' body_part=" +
            std::to_string(entry->value.body_part) + " applied=" +
            std::to_string(applied ? 1 : 0) +
            " path='internal_local'");
    }
}
