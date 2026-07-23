#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <Windows.h>

#include "../game_state.h"
#include "../Module.h"
#include "../ModuleCategory.h"
#include "../../config/ActionRowValue.h"
#include "../../config/CheckboxValue.h"
#include "../../config/FloatSliderValue.h"
#include "../../config/IntSliderValue.h"
#include "../../il2cpp/il2utils.h"
#include "../../il2cpp/unity.h"
#include "../../util/logger.h"

class WeatherModule : Module
{
public:
    WeatherModule() : Module("Weather", Visuals)
    {
    }

    CheckboxValue* weather_override = conf(new CheckboxValue(false, "Weather Override"));
    FloatSliderValue* rain_intensity =
        conf(new FloatSliderValue(0.0f, 0.0f, 1.0f, "Rain Intensity"));
    FloatSliderValue* clear_sky =
        conf(new FloatSliderValue(1.0f, 0.0f, 1.0f, "Clear Sky"));
    FloatSliderValue* sun_visibility =
        conf(new FloatSliderValue(1.0f, 0.0f, 1.0f, "Sun Visibility"));
    CheckboxValue* freeze_time = conf(new CheckboxValue(false, "Freeze Time"));
    FloatSliderValue* day_length =
        conf(new FloatSliderValue(3000.0f, 1.0f, 6000.0f, "Day Length (minutes)"));
    IntSliderValue* time_hour = conf(new IntSliderValue(12, 0, 23, "Time Hour"));
    IntSliderValue* time_minute = conf(new IntSliderValue(0, 0, 59, "Time Minute"));
    ActionRowValue* apply_time = conf(new ActionRowValue([this]
    {
        request_time_change();
    }, "Apply Time"));

    void draw_overlay(ImDrawList* draw_list) override
    {
        (void)draw_list;
    }

    void application_update() override
    {
        if (!game_state::is_in_raid)
        {
            discard_scene_state();
        }
    }

    void reset_gameworld_state() override
    {
        discard_scene_state();
        queue_time_change_ = false;
    }

    void gameworld_update(const Il2CppClass* game_world_class, Il2CppObjectInstance game_world_instance,
                          Il2CppObjectInstance main_player) override
    {
        (void)game_world_class;
        (void)game_world_instance;
        (void)main_player;

        const bool override_enabled = weather_override->get_value();
        if (!override_enabled)
        {
            restore_weather();
        }

        if (override_enabled || queue_time_change_)
        {
            resolve_weather_components();
        }

        apply_requested_time();
        if (!override_enabled)
        {
            return;
        }

        if (!override_active_)
        {
            capture_original_values();
            override_active_ = true;
        }

        apply_weather();
    }

    void init() override
    {
    }

private:
    using float_getter_sig = float(*)(Il2CppObject*);
    using float_setter_sig = void(*)(Il2CppObject*, float);
    using int_setter_sig = void(*)(Il2CppObject*, int);
    using action_sig = void(*)(Il2CppObject*);

    struct FloatPropertyState
    {
        Il2CppObject* object = nullptr;
        float_getter_sig getter = nullptr;
        float_setter_sig setter = nullptr;
        float original_value = 0.0f;
    };

    struct TimeState
    {
        Il2CppObject* object = nullptr;
        FieldInfo* lock_current_time = nullptr;
        FieldInfo* day_length_in_minutes = nullptr;
        bool original_lock_current_time = false;
        float original_day_length_in_minutes = 0.0f;
    };

    struct WeatherEventState
    {
        Il2CppObject* object = nullptr;
        int_setter_sig set_desired_hour = nullptr;
        int_setter_sig set_desired_minute = nullptr;
        action_sig activate_now = nullptr;
    };

    static constexpr ULONGLONG resolve_interval_ms_ = 1500;
    static constexpr std::size_t max_resolve_attempts_ = 12;
    static constexpr std::size_t max_objects_per_type_ = 256;

    Il2CppImage* assembly_csharp_ = nullptr;
    std::array<bool, 3> rain_types_resolved_{};
    bool sky_type_resolved_ = false;
    bool time_type_resolved_ = false;
    bool weather_event_type_resolved_ = false;
    std::size_t resolve_attempts_ = 0;
    ULONGLONG last_resolve_attempt_ = 0;

    std::vector<FloatPropertyState> rain_properties_;
    std::vector<FloatPropertyState> clear_sky_properties_;
    std::vector<FloatPropertyState> sun_visibility_properties_;
    std::vector<TimeState> time_states_;
    std::vector<WeatherEventState> weather_event_states_;

    bool override_active_ = false;
    bool applied_values_valid_ = false;
    float applied_rain_intensity_ = 0.0f;
    float applied_clear_sky_ = 0.0f;
    float applied_sun_visibility_ = 0.0f;
    float applied_day_length_ = 0.0f;
    bool applied_freeze_time_ = false;
    bool queue_time_change_ = false;

    static bool is_float_getter(const MethodInfo* method)
    {
        return method != nullptr && method->methodPointer != nullptr && method->parameters_count == 0 &&
            method->return_type != nullptr && method->return_type->type == IL2CPP_TYPE_R4;
    }

    static bool is_float_setter(const MethodInfo* method)
    {
        return method != nullptr && method->methodPointer != nullptr && method->parameters_count == 1 &&
            method->parameters != nullptr && method->parameters[0] != nullptr &&
            method->parameters[0]->type == IL2CPP_TYPE_R4;
    }

    static bool is_int_setter(const MethodInfo* method)
    {
        return method != nullptr && method->methodPointer != nullptr && method->parameters_count == 1 &&
            method->parameters != nullptr && method->parameters[0] != nullptr &&
            method->parameters[0]->type == IL2CPP_TYPE_I4;
    }

    static bool is_action(const MethodInfo* method)
    {
        return method != nullptr && method->methodPointer != nullptr && method->parameters_count == 0;
    }

    static bool is_field_type(const FieldInfo* field, const Il2CppTypeEnum type)
    {
        return field != nullptr && field->type != nullptr && field->type->type == type;
    }

    static float read_float_field(Il2CppObject* object, const FieldInfo* field)
    {
        return *reinterpret_cast<float*>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
    }

    static void write_float_field(Il2CppObject* object, const FieldInfo* field, const float value)
    {
        *reinterpret_cast<float*>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset) = value;
    }

    static bool read_bool_field(Il2CppObject* object, const FieldInfo* field)
    {
        return *reinterpret_cast<bool*>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
    }

    static void write_bool_field(Il2CppObject* object, const FieldInfo* field, const bool value)
    {
        *reinterpret_cast<bool*>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset) = value;
    }

    std::vector<Il2CppObject*> find_objects(const char* namespaze, const char* class_name)
    {
        std::vector<Il2CppObject*> objects;
        if (assembly_csharp_ == nullptr || unity::find_objects_of_type_internal == nullptr)
        {
            return objects;
        }

        const Il2CppClass* klass = il2utils::resolve_class(assembly_csharp_, namespaze, class_name);
        if (klass == nullptr)
        {
            return objects;
        }

        const Il2CppType* type = il2cpp::il2cpp_class_get_type(const_cast<Il2CppClass*>(klass));
        Il2CppObject* system_type = type != nullptr ? il2cpp::il2cpp_type_get_object(type) : nullptr;
        Il2CppArray* array = system_type != nullptr ?
            unity::find_objects_of_type_internal(system_type) : nullptr;
        if (array == nullptr)
        {
            return objects;
        }

        const std::size_t object_count =
            (std::min)(static_cast<std::size_t>(array->max_length), max_objects_per_type_);
        auto** data = reinterpret_cast<Il2CppObject**>(&array->data);
        objects.reserve(object_count);
        for (std::size_t index = 0; index < object_count; ++index)
        {
            if (data[index] != nullptr)
            {
                objects.push_back(data[index]);
            }
        }

        return objects;
    }

    static bool make_float_property(Il2CppObject* object, const char* getter_name,
                                    const char* setter_name, FloatPropertyState& state)
    {
        if (object == nullptr || object->klass == nullptr)
        {
            return false;
        }

        const MethodInfo* getter = il2utils::resolve_method(object->klass, getter_name, 0);
        const MethodInfo* setter = il2utils::resolve_method(object->klass, setter_name, 1);
        if (!is_float_getter(getter) || !is_float_setter(setter))
        {
            return false;
        }

        state.object = object;
        state.getter = reinterpret_cast<float_getter_sig>(getter->methodPointer);
        state.setter = reinterpret_cast<float_setter_sig>(setter->methodPointer);
        state.original_value = state.getter(object);
        return true;
    }

    void resolve_rain_type(const std::size_t type_index, const char* class_name)
    {
        if (rain_types_resolved_[type_index])
        {
            return;
        }

        const std::vector<Il2CppObject*> objects = find_objects("", class_name);
        for (Il2CppObject* object : objects)
        {
            FloatPropertyState state;
            if (make_float_property(object, "get_Intensity", "set_Intensity", state))
            {
                rain_properties_.push_back(state);
            }
        }

        rain_types_resolved_[type_index] = !objects.empty();
    }

    void resolve_sky_type()
    {
        if (sky_type_resolved_)
        {
            return;
        }

        const std::vector<Il2CppObject*> objects = find_objects("", "TOD_Sky");
        for (Il2CppObject* object : objects)
        {
            FloatPropertyState clear_sky_state;
            if (make_float_property(object, "get_ClearSky", "set_ClearSky", clear_sky_state))
            {
                clear_sky_properties_.push_back(clear_sky_state);
            }

            FloatPropertyState sun_visibility_state;
            if (make_float_property(object, "get_SunVisibility", "set_SunVisibility",
                                    sun_visibility_state))
            {
                sun_visibility_properties_.push_back(sun_visibility_state);
            }
        }

        sky_type_resolved_ = !objects.empty();
    }

    void resolve_time_type()
    {
        if (time_type_resolved_)
        {
            return;
        }

        const std::vector<Il2CppObject*> objects = find_objects("", "TOD_Time");
        for (Il2CppObject* object : objects)
        {
            FieldInfo* lock_current_time =
                il2utils::resolve_field(object->klass, "LockCurrentTime");
            FieldInfo* day_length_in_minutes =
                il2utils::resolve_field(object->klass, "DayLengthInMinutes");
            if (!is_field_type(lock_current_time, IL2CPP_TYPE_BOOLEAN) ||
                !is_field_type(day_length_in_minutes, IL2CPP_TYPE_R4))
            {
                continue;
            }

            TimeState state;
            state.object = object;
            state.lock_current_time = lock_current_time;
            state.day_length_in_minutes = day_length_in_minutes;
            state.original_lock_current_time = read_bool_field(object, lock_current_time);
            state.original_day_length_in_minutes =
                read_float_field(object, day_length_in_minutes);
            time_states_.push_back(state);
        }

        time_type_resolved_ = !objects.empty();
    }

    void resolve_weather_event_type()
    {
        if (weather_event_type_resolved_)
        {
            return;
        }

        const std::vector<Il2CppObject*> objects = find_objects("", "WeatherEventController");
        for (Il2CppObject* object : objects)
        {
            if (object == nullptr || object->klass == nullptr)
            {
                continue;
            }

            const MethodInfo* set_desired_hour =
                il2utils::resolve_method(object->klass, "set_DesiredHour", 1);
            const MethodInfo* set_desired_minute =
                il2utils::resolve_method(object->klass, "set_DesiredMinute", 1);
            const MethodInfo* activate_now =
                il2utils::resolve_method(object->klass, "ActivateNow", 0);
            if (!is_int_setter(set_desired_hour) ||
                !is_int_setter(set_desired_minute) ||
                !is_action(activate_now))
            {
                continue;
            }

            weather_event_states_.push_back({
                object,
                reinterpret_cast<int_setter_sig>(set_desired_hour->methodPointer),
                reinterpret_cast<int_setter_sig>(set_desired_minute->methodPointer),
                reinterpret_cast<action_sig>(activate_now->methodPointer)
            });
        }

        weather_event_type_resolved_ = !weather_event_states_.empty();
    }

    void resolve_weather_components()
    {
        const ULONGLONG now = GetTickCount64();
        if (resolve_attempts_ >= max_resolve_attempts_ ||
            now - last_resolve_attempt_ < resolve_interval_ms_)
        {
            return;
        }

        last_resolve_attempt_ = now;
        ++resolve_attempts_;
        if (assembly_csharp_ == nullptr)
        {
            assembly_csharp_ = il2utils::resolve_image("Assembly-CSharp.dll");
            if (assembly_csharp_ == nullptr)
            {
                return;
            }
        }

        const std::size_t old_rain_count = rain_properties_.size();
        const std::size_t old_sky_count = clear_sky_properties_.size();
        const std::size_t old_time_count = time_states_.size();
        const std::size_t old_event_count = weather_event_states_.size();

        resolve_rain_type(0, "RainScreenDrops");
        resolve_rain_type(1, "RainFallDrops");
        resolve_rain_type(2, "RainSplashController");
        resolve_sky_type();
        resolve_time_type();
        resolve_weather_event_type();

        if (old_rain_count != rain_properties_.size() ||
            old_sky_count != clear_sky_properties_.size() ||
            old_time_count != time_states_.size() ||
            old_event_count != weather_event_states_.size() ||
            resolve_attempts_ == 1)
        {
            logger::info(
                "[Weather] resolved rain=" + std::to_string(rain_properties_.size()) +
                " sky=" + std::to_string(clear_sky_properties_.size()) +
                " time=" + std::to_string(time_states_.size()) +
                " event=" + std::to_string(weather_event_states_.size()) +
                " attempt=" + std::to_string(resolve_attempts_));
        }
    }

    void request_time_change()
    {
        if (!game_state::is_in_raid)
        {
            logger::warn("[Weather] enter a raid before applying a time change");
            return;
        }

        queue_time_change_ = true;
        logger::info("[Weather] queued time change hour=" +
            std::to_string(time_hour->get_value()) +
            " minute=" + std::to_string(time_minute->get_value()));
    }

    void apply_requested_time()
    {
        if (!queue_time_change_)
        {
            return;
        }

        if (weather_event_states_.empty())
        {
            if (resolve_attempts_ >= max_resolve_attempts_)
            {
                logger::warn("[Weather] WeatherEventController was not available; time change cancelled");
                queue_time_change_ = false;
            }
            return;
        }

        const int hour = std::clamp(time_hour->get_value(), 0, 23);
        const int minute = std::clamp(time_minute->get_value(), 0, 59);
        for (const WeatherEventState& state : weather_event_states_)
        {
            state.set_desired_hour(state.object, hour);
            state.set_desired_minute(state.object, minute);
            state.activate_now(state.object);
        }

        queue_time_change_ = false;
        logger::info("[Weather] applied time hour=" + std::to_string(hour) +
            " minute=" + std::to_string(minute) +
            " controllers=" + std::to_string(weather_event_states_.size()));
    }

    void capture_original_values()
    {
        for (FloatPropertyState& state : rain_properties_)
        {
            state.original_value = state.getter(state.object);
        }
        for (FloatPropertyState& state : clear_sky_properties_)
        {
            state.original_value = state.getter(state.object);
        }
        for (FloatPropertyState& state : sun_visibility_properties_)
        {
            state.original_value = state.getter(state.object);
        }
        for (TimeState& state : time_states_)
        {
            state.original_lock_current_time =
                read_bool_field(state.object, state.lock_current_time);
            state.original_day_length_in_minutes =
                read_float_field(state.object, state.day_length_in_minutes);
        }
    }

    void apply_weather()
    {
        const float rain = rain_intensity->get_value();
        const float clear = clear_sky->get_value();
        const float sun = sun_visibility->get_value();
        const float length = day_length->get_value();
        const bool frozen = freeze_time->get_value();

        for (const FloatPropertyState& state : rain_properties_)
        {
            state.setter(state.object, rain);
        }
        for (const FloatPropertyState& state : clear_sky_properties_)
        {
            state.setter(state.object, clear);
        }
        for (const FloatPropertyState& state : sun_visibility_properties_)
        {
            state.setter(state.object, sun);
        }
        for (const TimeState& state : time_states_)
        {
            write_bool_field(state.object, state.lock_current_time, frozen);
            write_float_field(state.object, state.day_length_in_minutes, length);
        }

        const bool values_changed = !applied_values_valid_ ||
            std::fabs(applied_rain_intensity_ - rain) > 0.001f ||
            std::fabs(applied_clear_sky_ - clear) > 0.001f ||
            std::fabs(applied_sun_visibility_ - sun) > 0.001f ||
            std::fabs(applied_day_length_ - length) > 0.001f ||
            applied_freeze_time_ != frozen;
        if (values_changed)
        {
            logger::info(
                "[Weather] applied rain=" + std::to_string(rain) +
                " clear_sky=" + std::to_string(clear) +
                " sun_visibility=" + std::to_string(sun) +
                " freeze_time=" + std::to_string(frozen ? 1 : 0) +
                " day_length=" + std::to_string(length));
        }

        applied_values_valid_ = true;
        applied_rain_intensity_ = rain;
        applied_clear_sky_ = clear;
        applied_sun_visibility_ = sun;
        applied_day_length_ = length;
        applied_freeze_time_ = frozen;
    }

    void restore_weather()
    {
        if (!override_active_)
        {
            return;
        }

        for (const FloatPropertyState& state : rain_properties_)
        {
            state.setter(state.object, state.original_value);
        }
        for (const FloatPropertyState& state : clear_sky_properties_)
        {
            state.setter(state.object, state.original_value);
        }
        for (const FloatPropertyState& state : sun_visibility_properties_)
        {
            state.setter(state.object, state.original_value);
        }
        for (const TimeState& state : time_states_)
        {
            write_bool_field(
                state.object, state.lock_current_time, state.original_lock_current_time);
            write_float_field(
                state.object, state.day_length_in_minutes,
                state.original_day_length_in_minutes);
        }

        logger::info("[Weather] restored raid weather values");
        override_active_ = false;
        applied_values_valid_ = false;
    }

    void discard_scene_state()
    {
        assembly_csharp_ = nullptr;
        rain_types_resolved_.fill(false);
        sky_type_resolved_ = false;
        time_type_resolved_ = false;
        weather_event_type_resolved_ = false;
        resolve_attempts_ = 0;
        last_resolve_attempt_ = 0;
        rain_properties_.clear();
        clear_sky_properties_.clear();
        sun_visibility_properties_.clear();
        time_states_.clear();
        weather_event_states_.clear();
        override_active_ = false;
        applied_values_valid_ = false;
    }
};
