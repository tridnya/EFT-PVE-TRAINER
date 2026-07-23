#pragma once
#include <functional>

#include "../util/logger.h"
#include "Module.h"
#include "impl/AssetDiscoveryModule.h"
#include "impl/ConfigModule.h"
#include "impl/CustomizationDiscoveryModule.h"
#include "impl/HideoutModule.h"
#include "impl/ItemModule.h"
#include "impl/MapModule.h"
#include "impl/OutfitModule.h"
#include "impl/PlayerModule.h"
#include "impl/QuestModule.h"
#include "impl/WatermarkModule.h"
#include "impl/ViewModule.h"
#include "impl/WeatherModule.h"
#include "impl/WeaponModule.h"
#include "impl/EntityVisualizerModule.h"
#include "impl/WorldVisualizerModule.h"

class ModuleManager;
inline ModuleManager* module_manager_instance = nullptr;

class ModuleManager
{
public:
    ~ModuleManager()
    {
        for (auto module : modules)
        {
            delete static_cast<Module*>(module);
        }
    }

    ModuleManager()
        = default;

    void init() const
    {
        for (auto module : modules)
        {
            auto* m = static_cast<Module*>(module);
            logger::info("  init: " + m->name);
            m->init();
            logger::info("  done: " + m->name);
        }
    }

    std::vector<PVOID> modules = std::vector<PVOID>();

    void apply(const std::function<void(Module*)>& function) const
    {
        for (auto module : modules)
        {
            function(static_cast<Module*>(module));
        }
    }

    template <typename T>
    T* module(T* value)
    {
        modules.push_back(static_cast<PVOID>(value));
        return value;
    }

    std::vector<Module*> get_modules() const
    {
        auto res = std::vector<Module*>();
        for (auto module : modules)
        {
            res.push_back(static_cast<Module*>(module));
        }
        return res;
    }

    // SPT trainer modules only; keep this focused on PvE utility and progression tools.
    PlayerModule* player_module = module(new PlayerModule());
    ViewModule* view_module = module(new ViewModule());
    WeatherModule* weather_module = module(new WeatherModule());
    WeaponModule* weapon_module = module(new WeaponModule());
    EntityVisualizerModule* entity_visualizer_module = module(new EntityVisualizerModule());
    WorldVisualizerModule* world_visualizer_module = module(new WorldVisualizerModule());
    AssetDiscoveryModule* asset_discovery_module = module(new AssetDiscoveryModule());
    MapModule* map_module = module(new MapModule());
    ItemModule* item_module = module(new ItemModule());
    QuestModule* quest_module = module(new QuestModule());
    HideoutModule* hideout_module = module(new HideoutModule());
    OutfitModule* outfit_module = module(new OutfitModule());
    CustomizationDiscoveryModule* customization_discovery_module =
        module(new CustomizationDiscoveryModule());

    // overlay
    WatermarkModule* watermark_module = module(new WatermarkModule());

    // this has to get instantiated last
    ConfigModule* config_module = module(new ConfigModule(get_modules()));

    static ModuleManager* get_instance()
    {
        if (!module_manager_instance)
        {
            module_manager_instance = new ModuleManager();
        }
        return module_manager_instance;
    }

    bool has_module(const std::string& name) const
    {
        for (auto module : modules)
        {
            if (static_cast<Module*>(module)->name == name) return true;
        }
        return false;
    }
};
