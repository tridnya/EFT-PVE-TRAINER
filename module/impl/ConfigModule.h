#pragma once
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <windows.h>

#include "../Module.h"
#include "../ModuleCategory.h"
#include "../../config/ActionRowValue.h"
#include "../../config/CheckboxValue.h"
#include "../../config/SelectValue.h"
#include "../game_state.h"
#include "../../util/logger.h"

namespace config_module_state
{
    static std::vector<Module*> modules = {};
    static ConfigValue* force_offline_value = nullptr;
    static ConfigValue* offline_labs_blackout_value = nullptr;

    static void append_config_value(std::string& output, ConfigValue* config_value)
    {
        if (!config_value)
        {
            return;
        }

        output.append(config_value->get_imgui_title());
        output.append(";");
        output.append(config_value->get());
        output.append(";");
    }

    static std::string get_config_path()
    {
        char* appdata_value = nullptr;
        size_t len = 0;

        if (_dupenv_s(&appdata_value, &len, "APPDATA") != 0 || appdata_value == nullptr)
        {
            return ".";
        }

        const std::string root_path = std::string(appdata_value) + "\\Imperil";
        free(appdata_value);

        CreateDirectoryA(root_path.c_str(), nullptr);

        const std::string config_path = root_path + "\\Veil";
        CreateDirectoryA(config_path.c_str(), nullptr);

        return config_path;
    }
}

class ConfigModule : Module
{
public:
    ConfigModule(const std::vector<Module*>& m) : Module("Config", Config)
    {
        config_module_state::modules = m;
    }

    SelectValue* config = conf(new SelectValue(0, {
                                                   "Legit", "Rage", "Config 1", "Config 2", "Config 3", "Config 4",
                                                   "Config 5"
                                               }, "File"));

    CheckboxValue* force_offline = conf(new CheckboxValue(true, "Force Offline"));
    CheckboxValue* offline_labs_blackout =
        conf(new CheckboxValue(false, "Offline Labs Blackout"));

    ActionRowValue* load = conf(new ActionRowValue([&]
    {
        load_config(config->get_selected());
        sync_raid_options();
    }, "Load"));

    ActionRowValue* save = conf(new ActionRowValue([&]
    {
        sync_raid_options();
        save_config(config->get_selected());
    }, "Save", true));

    void draw_overlay(ImDrawList* draw_list) override
    {
    }

    void application_update() override
    {
        sync_raid_options();
    }

    void gameworld_update(const Il2CppClass* game_world_class, Il2CppObjectInstance game_world_instance,
                          Il2CppObjectInstance main_player) override
    {
    }

    void init() override
    {
        config_module_state::force_offline_value = force_offline;
        config_module_state::offline_labs_blackout_value =
            offline_labs_blackout;
        load_config(config->get_selected());
        sync_raid_options();
    }

    void sync_raid_options() const
    {
        game_state::force_offline = force_offline->get_value();
        game_state::offline_labs_blackout =
            offline_labs_blackout->get_value();
    }

    static void load_config(const std::string& config)
    {
        if (globals::verbose) logger::info("Loading Config " + config);

        const std::string path = config_module_state::get_config_path();

        // read file;
        std::ifstream t(path + "\\" + config);

        if (!t.good())
        {
            t = create_config(config);
        }

        std::stringstream buffer;
        buffer << t.rdbuf();
        t.close();

        // map keys to values
        auto res = utils::split(buffer.str(), ';');
        auto values = std::unordered_map<std::string, std::string>();
        for (size_t i = 0; i < res.size(); i += 2)
        {
            values[res.at(i)] = res.at(i + 1);
        }

        // write into modules
        for (auto module : config_module_state::modules)
        {
            module->apply([values](ConfigValue* c)
            {
                auto iter = values.find(c->get_imgui_title());
                if (iter == values.end()) c->set(c->get_default());
                else c->set(values.at(c->get_imgui_title()));
            });
        }

        if (config_module_state::force_offline_value)
        {
            auto iter = values.find(config_module_state::force_offline_value->get_imgui_title());
            if (iter == values.end()) config_module_state::force_offline_value->set(
                config_module_state::force_offline_value->get_default());
            else config_module_state::force_offline_value->set(iter->second);
        }

        if (config_module_state::offline_labs_blackout_value)
        {
            auto iter = values.find(
                config_module_state::offline_labs_blackout_value->
                    get_imgui_title());
            if (iter == values.end())
            {
                config_module_state::offline_labs_blackout_value->set(
                    config_module_state::offline_labs_blackout_value->
                        get_default());
            }
            else
            {
                config_module_state::offline_labs_blackout_value->set(
                    iter->second);
            }
        }
    }

    static void save_config(const std::string& config)
    {
        if (globals::verbose) logger::info("Saving Config " + config);

        const std::string path = config_module_state::get_config_path();

        // create config string
        std::string value;
        for (auto module : config_module_state::modules)
        {
            module->apply([&value](ConfigValue* c)
            {
                config_module_state::append_config_value(value, c);
            });
        }
        config_module_state::append_config_value(value, config_module_state::force_offline_value);
        config_module_state::append_config_value(
            value,
            config_module_state::offline_labs_blackout_value);

        // write file
        std::ofstream t(path + "\\" + config);
        t.write(value.c_str(), static_cast<std::streamsize>(value.size()));
        t.close();
    }

    static std::ifstream create_config(const std::string& config)
    {
        std::string value;
        for (auto module : config_module_state::modules)
        {
            module->apply([&value](ConfigValue* c)
            {
                value.append(c->get_imgui_title());
                value.append(";");
                value.append(c->get_default());
                value.append(";");
            });
        }
        if (config_module_state::force_offline_value)
        {
            value.append(config_module_state::force_offline_value->get_imgui_title());
            value.append(";");
            value.append(config_module_state::force_offline_value->get_default());
            value.append(";");
        }
        if (config_module_state::offline_labs_blackout_value)
        {
            value.append(
                config_module_state::offline_labs_blackout_value->
                    get_imgui_title());
            value.append(";");
            value.append(
                config_module_state::offline_labs_blackout_value->
                    get_default());
            value.append(";");
        }

        const std::string path = config_module_state::get_config_path();

        std::ofstream ot(path + "\\" + config);
        ot.write(value.c_str(), static_cast<std::streamsize>(value.size()));
        ot.close();

        return std::ifstream(path + "\\" + config);
    }
};
