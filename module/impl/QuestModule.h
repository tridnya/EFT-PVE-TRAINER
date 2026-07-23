#pragma once
#include "../game_state.h"
#include "../Module.h"
#include "../ModuleCategory.h"
#include "../../il2cpp/unity.h"
#include "ItemModule.h" // for spawn_quest_item()

// Matches EFT's MongoID struct (24 bytes): uint timeStamp @0x0, ulong counter @0x8, string @0x10.
// Used only as the key type so unity::dict computes the right entry stride for
// Dictionary<MongoID, TaskConditionCounter>.
struct mongo_id
{
    unsigned int time_stamp;
    unsigned long long counter;
    void* string_id;
};

class QuestModule : Module
{
public:
    ~QuestModule() override
    {
        for (auto option : quest_select->get_options())
        {
            delete[] option;
        }
    }

    QuestModule() : Module("Quests", Progression)
    {
    }

    SelectValue* quest_select = conf(new SelectValue(0, {}, "Quest"));
    ActionRowValue* complete = conf(new ActionRowValue([&]
    {
        complete_impl();
    }, "Complete"));
    ActionRowValue* complete_all = conf(new ActionRowValue([&]
    {
        complete_all_impl();
    }, "Complete All", true));

    bool queue_unlock = false;
    bool queue_unlock_all = false;

    void complete_all_impl()
    {
        if (!game_state::is_in_raid) return;
        queue_unlock_all = true;
    }

    void complete_impl()
    {
        if (!game_state::is_in_raid) return;
        queue_unlock = true;
    }

    bool names_lock = false;
    std::vector<Il2CppString*> quest_names = {};

    void draw_overlay(ImDrawList* draw_list) override
    {
        if (names_lock) return;
        names_lock = true;
        if (static_cast<size_t>(quest_select->get_value()) > quest_names.size())
        {
            quest_select->set("0");
        }
        for (auto option : quest_select->get_options())
        {
            delete[] option;
        }
        std::vector<const char*> new_quest_names = {};
        for (auto quest_name : quest_names)
        {
            std::string new_name = il2utils::conv_string(quest_name);
            const auto allocated_string = new char[new_name.size() + 1];
            memcpy(allocated_string, new_name.c_str(), new_name.size());
            allocated_string[new_name.size()] = NULL;
            new_quest_names.push_back(allocated_string);
        }
        quest_select->set_options(new_quest_names);
        names_lock = false;
    }

    void application_update() override
    {
    }

    void reset_gameworld_state() override
    {
        names_lock = true;
        quest_names.clear();
        queue_unlock = false;
        queue_unlock_all = false;
        names_lock = false;
    }

    void gameworld_update(const Il2CppClass* game_world_class, Il2CppObjectInstance game_world_instance,
                          Il2CppObjectInstance main_player) override
    {
        if (names_lock) return;
        names_lock = true;
        Il2CppObjectInstance quest_controller(
            main_player.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_QuestController", 0)(
                main_player.get_instance()));

        Il2CppObjectInstance quest_book(
            quest_controller.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_Quests", 0)(
                quest_controller.get_instance()));

        const auto status_data = quest_book.get_field<unity::list*>("_questsData");

        quest_names.clear();

        for (size_t i = 0; i < status_data->m_p_list_array->max_length; ++i)
        {
            Il2CppObject* quest_data_ptr = reinterpret_cast<Il2CppObject**>(&status_data->m_p_list_array->data)[i];
            if (!quest_data_ptr) continue;

            Il2CppObjectInstance quest_data(quest_data_ptr);

            const auto quest_ptr = quest_data.get_field<Il2CppObject*>("Quest");
            if (!quest_ptr) continue;

            Il2CppObjectInstance quest(quest_ptr);

            Il2CppObjectInstance quest_status(quest.get_field<Il2CppObject*>("_statusData"));

            // 2 = Started
            if (quest_status.get_field<int>("Status") != 2) continue;

            Il2CppString* quest_name =
                quest.get_method<Il2CppString*(*)(Il2CppObject*)>("get_Name", 0)(quest.get_instance());

            Il2CppObjectInstance quest_template(quest.get_field<Il2CppObject*>("_template"));
            Il2CppString* quest_id = quest_template.get_method<Il2CppString*(*)(Il2CppObject*)>("get_Id", 0)(
                quest_template.get_instance());

            quest_names.push_back(quest_name);

            if (queue_unlock_all || (queue_unlock && il2utils::conv_string(quest_name) == quest_select->get_selected()))
            {
                if (globals::verbose) logger::info("Unlocking Quest: " + il2utils::conv_string(quest_name));

                // 1) Fill the REAL condition counters to their target value. This is legit in-raid
                //    progress (same data the game writes when you actually do objectives), so the
                //    backend saves it at raid end instead of reverting a forced completion.
                Il2CppObjectInstance main_profile(
                    main_player.get_method<Il2CppObject*(*)(Il2CppObject*)>("get_Profile", 0)(
                        main_player.get_instance()));

                auto* counters = main_profile.get_field<unity::dict<mongo_id, Il2CppObject*>*>(
                    "TaskConditionCounters");

                int filled = 0;
                if (counters && counters->m_p_entries)
                {
                    for (int ci = 0; ci < counters->m_i_count; ++ci)
                    {
                        Il2CppObject* counter_obj = counters->get_value_by_index(ci);
                        if (!counter_obj) continue;

                        Il2CppObjectInstance counter(counter_obj);

                        // keep only counters belonging to this quest
                        if (counter.get_field<Il2CppObject*>("Conditional") != quest.get_instance()) continue;

                        Il2CppObject* template_obj = counter.get_field<Il2CppObject*>("Template");
                        if (!template_obj) continue;

                        Il2CppObjectInstance condition(template_obj);
                        const float target = condition.get_method<float(*)(Il2CppObject*)>("get_value", 0)(
                            condition.get_instance());

                        counter.get_method<void(*)(Il2CppObject*, int)>("set_Value", 1)(
                            counter.get_instance(), static_cast<int>(target));
                        filled++;
                    }
                }

                logger::info("Quest '" + il2utils::conv_string(quest_name) + "': filled " +
                    std::to_string(filled) + " condition counter(s)");

                // 1b) Mark EVERY condition id as completed in the persisted status data. This is the
                //     general fix that also covers zone / beacon ("Balisez") / optional objectives
                //     which have no counter. CompletedConditions is the HashSet<MongoID> the backend
                //     serializes, so adding the ids here makes them stick.
                int marked = 0;
                int spawned = 0;
                {
                    Il2CppObject* status_obj = quest.get_field<Il2CppObject*>("_statusData");
                    const MethodInfo* get_conditions =
                        il2utils::resolve_method(quest.get_instance()->klass, "get_Conditions", 0);

                    if (status_obj && get_conditions && get_conditions->methodPointer)
                    {
                        Il2CppObjectInstance status(status_obj);
                        Il2CppObject* completed_set = status.get_field<Il2CppObject*>("CompletedConditions");
                        Il2CppObject* conditions_obj =
                            static_cast<Il2CppObject*(*)(Il2CppObject*)>(get_conditions->methodPointer)(
                                quest.get_instance());

                        if (completed_set && completed_set->klass && conditions_obj)
                        {
                            const MethodInfo* add_m = il2utils::resolve_method(completed_set->klass, "Add", 1);
                            auto* cond_dict = reinterpret_cast<unity::dict<int, Il2CppObject*>*>(conditions_obj);

                            if (add_m && add_m->methodPointer && cond_dict->m_p_entries)
                            {
                                // generic method (HashSet<MongoID>.Add) requires the trailing
                                // MethodInfo* in the IL2CPP ABI; MongoID (24 bytes) is passed by pointer
                                auto add_fn = static_cast<bool(*)(Il2CppObject*, void*, const MethodInfo*)>(
                                    add_m->methodPointer);

                                for (int di = 0; di < cond_dict->m_i_count; ++di)
                                {
                                    Il2CppObject* collection = cond_dict->get_value_by_index(di);
                                    if (!collection) continue;

                                    Il2CppObjectInstance coll(collection);
                                    unity::list* clist = coll.get_field<unity::list*>("List");
                                    if (!clist || !clist->m_p_list_array) continue;

                                    Il2CppArray* carr = clist->m_p_list_array;
                                    for (size_t cci = 0; cci < carr->max_length; ++cci)
                                    {
                                        Il2CppObject* cond = reinterpret_cast<Il2CppObject**>(&carr->data)[cci];
                                        if (!cond) continue;

                                        Il2CppObjectInstance cond_inst(cond);

                                        // mark this condition complete
                                        mongo_id cid = cond_inst.get_field<mongo_id>("<id>k__BackingField");
                                        add_fn(completed_set, &cid, add_m);
                                        marked++;

                                        // if the condition needs us to possess/hand over an item, spawn it (FiR)
                                        const char* ccls = (cond->klass && cond->klass->name) ? cond->klass->name : "";
                                        if (strstr(ccls, "FindItem") || strstr(ccls, "HandoverItem"))
                                        {
                                            Il2CppArray* targets = cond_inst.get_field<Il2CppArray*>("target");
                                            if (targets && targets->max_length > 0)
                                            {
                                                Il2CppString* tid =
                                                    reinterpret_cast<Il2CppString**>(&targets->data)[0];
                                                if (tid)
                                                {
                                                    const std::string item_id = il2utils::conv_string(tid);

                                                    // spawn as many as the condition asks for (e.g. "loot 3 MRE")
                                                    int cnt = static_cast<int>(
                                                        cond_inst.get_method<float(*)(Il2CppObject*)>("get_value", 0)(
                                                            cond_inst.get_instance()));
                                                    if (cnt < 1) cnt = 1;

                                                    if (spawn_quest_item(main_player, item_id, cnt))
                                                    {
                                                        spawned++;
                                                        logger::info("Quest item spawned x" + std::to_string(cnt) +
                                                            " (FiR): " + item_id);
                                                    }
                                                    else
                                                    {
                                                        logger::warn("Quest item spawn FAILED: " + item_id);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                logger::info("Quest '" + il2utils::conv_string(quest_name) + "': marked " +
                    std::to_string(marked) + " condition(s), spawned " + std::to_string(spawned) + " item(s)");

                // 2) Flip status to AvailableForFinish (3) then send the hand-in. With the counters
                //    now genuinely met, the backend accepts and persists it.
                quest.get_method<void(*)(Il2CppObject*, int, bool, bool)>("SetStatus", 3)(
                    quest.get_instance(), 3 /* AvailableForFinish */, true, false);

                quest_controller.get_method<Il2CppObject*(*)(Il2CppObject*, Il2CppString*)>(
                    "SendQuestCompleteToBackend", 1)(
                    quest_controller.get_instance(), quest_id);
            }
        }

        if (queue_unlock_all)
        {
            queue_unlock_all = false;
        }
        if (queue_unlock)
        {
            queue_unlock = false;
        }
        names_lock = false;
    }

    void init() override
    {
    }
};
