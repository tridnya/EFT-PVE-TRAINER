#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

struct FieldInfo;
struct Il2CppArray;
struct Il2CppClass;
struct Il2CppImage;
struct Il2CppObject;
struct MethodInfo;

namespace game::customization
{
    enum class OutfitBodyRegion : std::uint8_t
    {
        Unknown,
        Upper,
        Lower
    };

    struct OutfitEntry
    {
        std::string id;
        std::string name;
        std::string class_name;
        std::string state;
        OutfitBodyRegion body_region = OutfitBodyRegion::Unknown;
        std::int32_t body_part = -1;
        std::int32_t raw_state = 0;
        bool purchased = false;
        bool previewed = false;
        bool equipped = false;
        bool external_obtain = false;
        bool internal_obtain = false;
        bool trainer_eligible = false;
    };

    struct OutfitSnapshot
    {
        std::vector<OutfitEntry> entries;
        std::string current_upper_id;
        std::string current_lower_id;
        std::string original_upper_id;
        std::string original_lower_id;
        std::string enforced_upper_id;
        std::string enforced_lower_id;
        std::string status;
        bool initialized = false;
        bool view_ready = false;
    };

    class OutfitController
    {
    public:
        bool initialize();
        void update();

        [[nodiscard]] OutfitSnapshot snapshot() const;

        void request_refresh();
        void request_preview(const std::string& outfit_id);
        void request_equip(const std::string& outfit_id);
        void request_restore();
        void request_reapply();
        void set_enforced_outfits(
            const std::string& upper_id,
            const std::string& lower_id);

    private:
        enum class CommandType : std::uint8_t
        {
            Refresh,
            Preview,
            Equip,
            Restore,
            Reapply,
            ProfileReapply
        };

        struct Command
        {
            CommandType type = CommandType::Refresh;
            std::string outfit_id;
            Il2CppObject* profile = nullptr;
        };

        struct RuntimeEntry
        {
            OutfitEntry value;
            Il2CppObject* card = nullptr;
            Il2CppObject* full_offer = nullptr;
            Il2CppObject* customization_offer = nullptr;
            Il2CppObject* suite = nullptr;
        };

        struct LiveContext
        {
            Il2CppObject* view = nullptr;
            std::vector<RuntimeEntry> entries;
            std::string current_upper_id;
            std::string current_lower_id;
        };

        void queue_command(Command command);
        void process_command(const Command& command);
        void refresh_snapshot(bool apply_enforced_outfits);
        void publish_snapshot(const LiveContext* context);
        void set_status(std::string status);
        void log_offer_diagnostics(const RuntimeEntry& entry);
        void log_customization_source_enum();
        bool enforce_applied_outfits(LiveContext& context);
        bool apply_enforced_outfits_to_profile(Il2CppObject* profile);
        bool apply_suite_to_profile(
            Il2CppObject* profile,
            Il2CppObject* suite) const;
        void cache_enforced_suites(const LiveContext& context);
        void set_enforced_outfit(const RuntimeEntry& entry);
        void clear_enforced_outfit(OutfitBodyRegion body_region);
        void queue_profile_reapply(Il2CppObject* backend_session);

        using SetProfilesSignature = void(*)(
            Il2CppObject*,
            Il2CppArray*);
        static void hk_set_profiles(
            Il2CppObject* instance,
            Il2CppArray* profiles);

        [[nodiscard]] Il2CppObject* find_live_view() const;
        [[nodiscard]] bool capture_live_context(LiveContext& context) const;
        [[nodiscard]] bool invoke_preview(
            Il2CppObject* view,
            const RuntimeEntry& entry) const;
        [[nodiscard]] Il2CppObject* invoke_equip(
            Il2CppObject* view,
            const RuntimeEntry& entry) const;
        [[nodiscard]] bool invoke_internal_apply(
            Il2CppObject* view,
            const RuntimeEntry& entry) const;

        const Il2CppImage* assembly_csharp_ = nullptr;
        Il2CppClass* tactical_clothing_view_class_ = nullptr;
        Il2CppClass* clothing_item_class_ = nullptr;
        Il2CppClass* full_offer_class_ = nullptr;
        Il2CppClass* customization_offer_class_ = nullptr;
        Il2CppClass* customization_source_class_ = nullptr;
        Il2CppClass* customization_solver_class_ = nullptr;

        FieldInfo* views_field_ = nullptr;
        FieldInfo* solver_field_ = nullptr;
        FieldInfo* suite_templates_field_ = nullptr;
        FieldInfo* profile_field_ = nullptr;
        FieldInfo* current_upper_field_ = nullptr;
        FieldInfo* current_lower_field_ = nullptr;
        FieldInfo* previewed_field_ = nullptr;
        FieldInfo* state_field_ = nullptr;
        FieldInfo* purchased_field_ = nullptr;
        FieldInfo* offer_field_ = nullptr;
        FieldInfo* customization_offer_field_ = nullptr;
        FieldInfo* suite_field_ = nullptr;
        FieldInfo* external_obtain_field_ = nullptr;
        FieldInfo* internal_obtain_field_ = nullptr;
        FieldInfo* backend_profile_field_ = nullptr;

        const MethodInfo* on_test_fit_method_ = nullptr;
        const MethodInfo* on_equip_method_ = nullptr;
        const MethodInfo* get_current_profile_method_ = nullptr;
        const MethodInfo* update_preview_method_ = nullptr;

        mutable std::mutex snapshot_mutex_;
        OutfitSnapshot snapshot_;

        std::mutex command_mutex_;
        std::vector<Command> pending_commands_;

        Il2CppObject* observed_view_ = nullptr;
        std::string original_upper_id_;
        std::string original_lower_id_;
        std::string enforced_upper_id_;
        std::string enforced_lower_id_;
        Il2CppObject* enforced_upper_suite_ = nullptr;
        Il2CppObject* enforced_lower_suite_ = nullptr;
        std::string status_;
        bool initialized_ = false;
        bool customization_source_enum_logged_ = false;

        static OutfitController* active_instance_;
        static SetProfilesSignature o_set_profiles_;
    };
}
