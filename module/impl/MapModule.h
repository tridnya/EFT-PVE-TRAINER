#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cctype>
#include <iomanip>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <Windows.h>
#include <shellapi.h>
#include <winhttp.h>

#include "../game_state.h"
#include "../Module.h"
#include "../ModuleCategory.h"
#include "../../config/ActionRowValue.h"
#include "../../config/CheckboxValue.h"
#include "../../config/FloatSliderValue.h"
#include "../../config/SelectValue.h"
#include "../../config/TextboxValue.h"
#include "../../gui/dxhook/dxhook.h"
#include "../../gui/menu/menu.h"
#include "../../gui/webview/tarkov_map_webview.h"
#include "../../il2cpp/Il2CppObjectInstance.h"
#include "../../il2cpp/il2utils.h"
#include "../../il2cpp/unity.h"
#include "../../util/logger.h"

namespace tarkov_dev_sync_detail
{
    struct MapDefinition
    {
        const char* key;
        const char* display_name;
        std::array<const char*, 6> aliases;
    };

    inline constexpr std::array<MapDefinition, 13> maps{ {
        { "customs", "Customs", { "customs", "bigmap", "bigmapl", nullptr, nullptr, nullptr } },
        { "factory", "Factory", { "factory", "night-factory", "factory4_day", "factory4_night", nullptr, nullptr } },
        { "woods", "Woods", { "woods", "wood", nullptr, nullptr, nullptr, nullptr } },
        { "interchange", "Interchange", { "interchange", "interchangebase", nullptr, nullptr, nullptr, nullptr } },
        { "reserve", "Reserve", { "reserve", "rezervbase", nullptr, nullptr, nullptr, nullptr } },
        { "shoreline", "Shoreline", { "shoreline", "shorelinebase", nullptr, nullptr, nullptr, nullptr } },
        { "streets-of-tarkov", "Streets", { "streets-of-tarkov", "tarkovstreets", "streets", nullptr, nullptr, nullptr } },
        { "ground-zero", "Ground Zero", { "ground-zero", "ground-zero-21", "sandbox", "sandbox_high", "sandbox_2", nullptr } },
        { "lighthouse", "Lighthouse", { "lighthouse", "lighthousebase", nullptr, nullptr, nullptr, nullptr } },
        { "the-lab", "The Lab", { "the-lab", "laboratory", "lab", "labs", nullptr, nullptr } },
        { "terminal", "Terminal", { "terminal", nullptr, nullptr, nullptr, nullptr, nullptr } },
        { "the-labyrinth", "Labyrinth", { "the-labyrinth", "labyrinth", nullptr, nullptr, nullptr, nullptr } },
        { "icebreaker", "Icebreaker", { "icebreaker", nullptr, nullptr, nullptr, nullptr, nullptr } }
    } };

    inline std::string last_detected_map{};

    [[nodiscard]] inline std::string lower_copy(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
        return value;
    }

    [[nodiscard]] inline bool text_contains_map_alias(const std::string& text, const MapDefinition& map)
    {
        const std::string lower_text = lower_copy(text);
        for (const char* alias : map.aliases)
        {
            if (alias && lower_text.find(alias) != std::string::npos)
            {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] inline const MapDefinition* find_map_by_alias(const std::string& text)
    {
        if (text.empty())
        {
            return nullptr;
        }

        for (const MapDefinition& map : maps)
        {
            if (text_contains_map_alias(text, map))
            {
                return &map;
            }
        }

        return nullptr;
    }

    [[nodiscard]] inline std::string read_string_field(Il2CppObject* object, const char* field_name)
    {
        if (!object || !field_name)
        {
            return {};
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, field_name);
        if (!field || field->offset <= 0)
        {
            return {};
        }

        Il2CppString* value = *reinterpret_cast<Il2CppString**>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
        return value ? il2utils::conv_string(value) : std::string{};
    }

    [[nodiscard]] inline std::string read_object_string_field(
        Il2CppObject* object,
        const char* object_field_name,
        const char* string_field_name)
    {
        if (!object || !object_field_name || !string_field_name)
        {
            return {};
        }

        const FieldInfo* field = il2utils::resolve_field(object->klass, object_field_name);
        if (!field || field->offset <= 0)
        {
            return {};
        }

        Il2CppObject* child = *reinterpret_cast<Il2CppObject**>(
            reinterpret_cast<std::uintptr_t>(object) + field->offset);
        return read_string_field(child, string_field_name);
    }

    [[nodiscard]] inline const MapDefinition* detect_map_from_game_world(Il2CppObjectInstance& game_world_instance)
    {
        Il2CppObject* game_world = game_world_instance.get_instance();
        const std::array<std::string, 12> candidates{
            read_string_field(game_world, "LocationId"),
            read_string_field(game_world, "locationId"),
            read_string_field(game_world, "Location"),
            read_string_field(game_world, "location"),
            read_string_field(game_world, "Id"),
            read_string_field(game_world, "Name"),
            read_object_string_field(game_world, "Location", "Id"),
            read_object_string_field(game_world, "Location", "Name"),
            read_object_string_field(game_world, "location", "Id"),
            read_object_string_field(game_world, "location", "Name"),
            read_object_string_field(game_world, "<Location>k__BackingField", "Id"),
            read_object_string_field(game_world, "<Location>k__BackingField", "Name")
        };

        for (const std::string& candidate : candidates)
        {
            if (const MapDefinition* map = find_map_by_alias(candidate))
            {
                last_detected_map = map->key;
                return map;
            }
        }

        return last_detected_map.empty() ? nullptr : find_map_by_alias(last_detected_map);
    }

    [[nodiscard]] inline std::string generate_session_id()
    {
        static constexpr char characters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        static thread_local std::mt19937 generator{
            static_cast<std::mt19937::result_type>(
                std::chrono::high_resolution_clock::now().time_since_epoch().count())
        };

        std::uniform_int_distribution<int> distribution(0, 35);
        std::string result{};
        result.reserve(4);
        for (int index = 0; index < 4; ++index)
        {
            result.push_back(characters[distribution(generator)]);
        }

        return result;
    }

    [[nodiscard]] inline std::string sanitize_session_id(const std::string& value)
    {
        std::string result{};
        result.reserve(4);
        for (unsigned char character : value)
        {
            if (!std::isalnum(character))
            {
                continue;
            }

            result.push_back(static_cast<char>(std::toupper(character)));
            if (result.size() == 4)
            {
                break;
            }
        }

        return result;
    }

    [[nodiscard]] inline std::wstring websocket_path_for_session(const std::string& session_id)
    {
        std::wstring path = L"/?sessionid=";
        for (char character : session_id)
        {
            path.push_back(static_cast<wchar_t>(character));
        }
        return path;
    }

    [[nodiscard]] inline std::string json_escape(const std::string& value)
    {
        std::string escaped{};
        escaped.reserve(value.size() + 8);
        for (char character : value)
        {
            switch (character)
            {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped.push_back(character);
                break;
            }
        }
        return escaped;
    }

    inline void draw_outlined_text(
        ImDrawList* draw_list,
        const ImVec2& position,
        ImU32 color,
        const char* text)
    {
        if (!draw_list || !text || text[0] == '\0')
        {
            return;
        }

        const ImU32 outline = IM_COL32(0, 0, 0, 210);
        draw_list->AddText({ position.x - 1.0f, position.y }, outline, text);
        draw_list->AddText({ position.x + 1.0f, position.y }, outline, text);
        draw_list->AddText({ position.x, position.y - 1.0f }, outline, text);
        draw_list->AddText({ position.x, position.y + 1.0f }, outline, text);
        draw_list->AddText(position, color, text);
    }

    class TarkovDevSocketClient
    {
    public:
        TarkovDevSocketClient() = default;

        ~TarkovDevSocketClient()
        {
            stop();
        }

        void start(const std::string& session_id)
        {
            const std::string sanitized = sanitize_session_id(session_id);
            if (sanitized.size() != 4)
            {
                set_status("invalid local session");
                return;
            }

            if (running.load() && sanitized == local_session_id)
            {
                return;
            }

            stop();
            local_session_id = sanitized;
            running.store(true);
            worker_thread = std::thread([this] { worker_loop(); });
        }

        void stop()
        {
            running.store(false);
            close_handles();
            if (worker_thread.joinable())
            {
                worker_thread.join();
            }
            connected.store(false);
        }

        [[nodiscard]] bool is_connected() const
        {
            return connected.load();
        }

        [[nodiscard]] std::string get_status() const
        {
            std::lock_guard lock(status_mutex);
            return status_text;
        }

        bool send_map_command(const std::string& target_session_id, const std::string& map_key)
        {
            const std::string target = sanitize_session_id(target_session_id);
            if (target.size() != 4 || map_key.empty())
            {
                return false;
            }

            std::ostringstream payload{};
            payload << "{\"sessionID\":\"" << target << "\",\"type\":\"command\",\"data\":{"
                    << "\"type\":\"map\",\"value\":\"" << json_escape(map_key) << "\"}}";
            return send_json(payload.str());
        }

        bool send_player_position(
            const std::string& target_session_id,
            const std::string& map_key,
            const unity::vector3& position,
            float rotation)
        {
            const std::string target = sanitize_session_id(target_session_id);
            if (target.size() != 4 || map_key.empty())
            {
                return false;
            }

            std::ostringstream payload{};
            payload << std::fixed << std::setprecision(3)
                    << "{\"sessionID\":\"" << target << "\",\"type\":\"command\",\"data\":{"
                    << "\"type\":\"playerPosition\","
                    << "\"map\":\"" << json_escape(map_key) << "\","
                    << "\"position\":{\"x\":" << position.x
                    << ",\"y\":" << position.y
                    << ",\"z\":" << position.z << "},"
                    << "\"rotation\":" << rotation
                    << "}}";
            return send_json(payload.str());
        }

    private:
        std::atomic_bool running = false;
        std::atomic_bool connected = false;
        std::thread worker_thread{};
        std::string local_session_id{};

        mutable std::mutex handle_mutex{};
        HINTERNET session_handle = nullptr;
        HINTERNET connect_handle = nullptr;
        HINTERNET websocket_handle = nullptr;

        mutable std::mutex status_mutex{};
        std::string status_text = "idle";

        void set_status(const std::string& status)
        {
            std::lock_guard lock(status_mutex);
            status_text = status;
        }

        [[nodiscard]] HINTERNET current_socket() const
        {
            std::lock_guard lock(handle_mutex);
            return websocket_handle;
        }

        void close_handles()
        {
            std::lock_guard lock(handle_mutex);
            if (websocket_handle)
            {
                WinHttpWebSocketClose(
                    websocket_handle,
                    WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
                    nullptr,
                    0);
                WinHttpCloseHandle(websocket_handle);
                websocket_handle = nullptr;
            }

            if (connect_handle)
            {
                WinHttpCloseHandle(connect_handle);
                connect_handle = nullptr;
            }

            if (session_handle)
            {
                WinHttpCloseHandle(session_handle);
                session_handle = nullptr;
            }
        }

        [[nodiscard]] bool send_json(const std::string& payload)
        {
            std::lock_guard lock(handle_mutex);
            if (!websocket_handle)
            {
                return false;
            }

            const DWORD result = WinHttpWebSocketSend(
                websocket_handle,
                WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
                const_cast<char*>(payload.data()),
                static_cast<DWORD>(payload.size()));
            if (result != NO_ERROR)
            {
                set_status("send failed: " + std::to_string(result));
                return false;
            }

            return true;
        }

        [[nodiscard]] bool connect_socket()
        {
            set_status("connecting");

            HINTERNET new_session = WinHttpOpen(
                L"Veil tarkov.dev sync/1.0",
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                WINHTTP_NO_PROXY_NAME,
                WINHTTP_NO_PROXY_BYPASS,
                0);
            if (!new_session)
            {
                set_status("WinHttpOpen failed");
                return false;
            }

            HINTERNET new_connect = WinHttpConnect(
                new_session,
                L"socket.tarkov.dev",
                INTERNET_DEFAULT_HTTPS_PORT,
                0);
            if (!new_connect)
            {
                WinHttpCloseHandle(new_session);
                set_status("WinHttpConnect failed");
                return false;
            }

            const std::wstring path = websocket_path_for_session(local_session_id);
            HINTERNET request = WinHttpOpenRequest(
                new_connect,
                L"GET",
                path.c_str(),
                nullptr,
                WINHTTP_NO_REFERER,
                WINHTTP_DEFAULT_ACCEPT_TYPES,
                WINHTTP_FLAG_SECURE);
            if (!request)
            {
                WinHttpCloseHandle(new_connect);
                WinHttpCloseHandle(new_session);
                set_status("WinHttpOpenRequest failed");
                return false;
            }

            if (!WinHttpSetOption(request, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, nullptr, 0) ||
                !WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0) ||
                !WinHttpReceiveResponse(request, nullptr))
            {
                const DWORD error = GetLastError();
                WinHttpCloseHandle(request);
                WinHttpCloseHandle(new_connect);
                WinHttpCloseHandle(new_session);
                set_status("connect failed: " + std::to_string(error));
                return false;
            }

            HINTERNET new_websocket = WinHttpWebSocketCompleteUpgrade(request, 0);
            WinHttpCloseHandle(request);
            if (!new_websocket)
            {
                const DWORD error = GetLastError();
                WinHttpCloseHandle(new_connect);
                WinHttpCloseHandle(new_session);
                set_status("upgrade failed: " + std::to_string(error));
                return false;
            }

            {
                std::lock_guard lock(handle_mutex);
                session_handle = new_session;
                connect_handle = new_connect;
                websocket_handle = new_websocket;
            }

            connected.store(true);
            set_status("connected");
            return true;
        }

        void receive_loop()
        {
            std::array<char, 4096> buffer{};
            while (running.load())
            {
                HINTERNET socket = current_socket();
                if (!socket)
                {
                    break;
                }

                DWORD bytes_read = 0;
                WINHTTP_WEB_SOCKET_BUFFER_TYPE buffer_type{};
                const DWORD result = WinHttpWebSocketReceive(
                    socket,
                    buffer.data(),
                    static_cast<DWORD>(buffer.size() - 1),
                    &bytes_read,
                    &buffer_type);

                if (result != NO_ERROR)
                {
                    set_status("receive failed: " + std::to_string(result));
                    break;
                }

                if (buffer_type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE)
                {
                    set_status("disconnected");
                    break;
                }

                if (buffer_type != WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE &&
                    buffer_type != WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE)
                {
                    continue;
                }

                buffer[bytes_read] = '\0';
                const std::string message(buffer.data(), bytes_read);
                if (message.find("\"type\":\"ping\"") != std::string::npos ||
                    message.find("\"type\": \"ping\"") != std::string::npos)
                {
                    (void)send_json("{\"type\":\"pong\"}");
                }
            }
        }

        void worker_loop()
        {
            while (running.load())
            {
                if (connect_socket())
                {
                    receive_loop();
                }

                connected.store(false);
                close_handles();

                if (!running.load())
                {
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            }
        }
    };
}

class MapModule : public Module
{
public:
    MapModule() : Module("Map", World)
    {
        local_session_id = tarkov_dev_sync_detail::generate_session_id();
    }

    ~MapModule() override
    {
        internal_webview.shutdown();
        socket_client.stop();
    }

    CheckboxValue* enabled = conf(new CheckboxValue(false, "Tarkov.dev Sync"));
    CheckboxValue* internal_map = conf(new CheckboxValue(true, "Internal Map"));
    CheckboxValue* internal_minimap = conf(new CheckboxValue(true, "Bottom Right Minimap"));
    FloatSliderValue* internal_map_size = conf(new FloatSliderValue(260.0f, 160.0f, 460.0f, "Minimap Size"));
    TextboxValue* remote_id = conf(new TextboxValue("", "Remote ID"));
    SelectValue* selected_map = conf(new SelectValue(0, {
        "Auto",
        "Customs",
        "Factory",
        "Woods",
        "Interchange",
        "Reserve",
        "Shoreline",
        "Streets",
        "Ground Zero",
        "Lighthouse",
        "The Lab",
        "Terminal",
        "Labyrinth",
        "Icebreaker"
    }, "Map"));
    FloatSliderValue* update_interval = conf(new FloatSliderValue(0.35f, 0.10f, 2.00f, "Update Seconds"));
    CheckboxValue* show_status = conf(new CheckboxValue(true, "Status Overlay"));
    ActionRowValue* refresh_internal_map = conf(new ActionRowValue([&] { refresh_internal_map_view(); }, "Refresh Internal Map"));
    ActionRowValue* open_map = conf(new ActionRowValue([&] { open_tarkov_dev_map(); }, "Open External Map"));

    void draw_overlay(ImDrawList* draw_list) override
    {
        if ((!enabled->get_value() && !internal_map->get_value()) || !show_status->get_value())
        {
            return;
        }
    

        /* 
        const ImGuiIO& io = ImGui::GetIO();
        const float scale = menu::get_scale_factor();
        const ImVec2 position{ io.DisplaySize.x - (330.0f * scale), io.DisplaySize.y - (96.0f * scale) };
        const ImVec2 max{ io.DisplaySize.x - (18.0f * scale), io.DisplaySize.y - (22.0f * scale) };
        draw_list->AddRectFilled(position, max, IM_COL32(6, 7, 9, 172), 6.0f);
        draw_list->AddRect(position, max, IM_COL32(150, 165, 205, 82), 6.0f);

        const ImU32 title_color = socket_client.is_connected()
            ? IM_COL32(116, 228, 154, 235)
            : IM_COL32(245, 190, 95, 235);
        tarkov_dev_sync_detail::draw_outlined_text(
            draw_list,
            { position.x + 10.0f * scale, position.y + 9.0f * scale },
            title_color,
            "tarkov.dev sync");

        tarkov_dev_sync_detail::draw_outlined_text(
            draw_list,
            { position.x + 10.0f * scale, position.y + 30.0f * scale },
            IM_COL32(220, 223, 230, 230),
            status_text.c_str());

        const std::string webview_status = "internal: " + internal_webview.get_status();
        tarkov_dev_sync_detail::draw_outlined_text(
            draw_list,
            { position.x + 10.0f * scale, position.y + 51.0f * scale },
            IM_COL32(176, 196, 255, 220),
            webview_status.c_str());

       */
    }

    void application_update() override
    {
        const bool wants_internal_map = internal_map->get_value();
        const bool wants_sync = enabled->get_value() || wants_internal_map;
        if (!wants_sync)
        {
            socket_client.stop();
            internal_webview.hide();
            status_text = "off";
            last_sent_map.clear();
            return;
        }

        const std::string target_session_id = wants_internal_map
            ? ensure_remote_id()
            : tarkov_dev_sync_detail::sanitize_session_id(remote_id->get());
        if (target_session_id.size() != 4)
        {
            socket_client.stop();
            internal_webview.hide();
            status_text = "remote id needed";
            return;
        }

        socket_client.start(local_session_id);

        const tarkov_dev_sync_detail::MapDefinition* map = get_active_map();
        const std::string map_key_for_view = map ? map->key : "customs";
        if (wants_internal_map)
        {
            update_internal_map_view(target_session_id, map_key_for_view);
        }
        else
        {
            internal_webview.hide();
        }

        if (!game_state::is_in_raid || !has_player_position || !map)
        {
            status_text = socket_client.get_status() + " | waiting for raid/map";
            return;
        }

        const auto now = std::chrono::steady_clock::now();
        const auto interval = std::chrono::duration<float>(update_interval->get_value());
        if (now - last_update_time < interval)
        {
            status_text = socket_client.get_status() + " | " + target_session_id;
            return;
        }
        last_update_time = now;

        if (last_sent_map != map->key)
        {
            (void)socket_client.send_map_command(target_session_id, map->key);
            last_sent_map = map->key;
        }

        const bool sent = socket_client.send_player_position(
            target_session_id,
            map->key,
            player_position,
            player_yaw);
        status_text = sent
            ? std::string("synced ") + map->display_name + " -> " + target_session_id
            : socket_client.get_status() + " | send pending";
    }

    void gameworld_update(
        const Il2CppClass* game_world_class,
        Il2CppObjectInstance game_world_instance,
        Il2CppObjectInstance main_player) override
    {
        (void)game_world_class;

        Il2CppObject* main_player_game_object = unity::component_get_game_object(main_player.get_instance());
        Il2CppObject* main_player_transform =
            main_player_game_object ? unity::gameobject_get_transform(main_player_game_object) : nullptr;
        if (!main_player_transform)
        {
            has_player_position = false;
            return;
        }

        player_position = unity::transform_get_pos(main_player_transform);
        const unity::quaternion rotation = unity::transform_get_rotation(main_player_transform);
        player_yaw = rotation.to_euler().y;
        has_player_position = true;
        auto_detected_map = tarkov_dev_sync_detail::detect_map_from_game_world(game_world_instance);
    }

    void reset_gameworld_state() override
    {
        player_position = {};
        player_yaw = 0.0f;
        has_player_position = false;
        auto_detected_map = nullptr;
        last_sent_map.clear();
        last_update_time = {};
        internal_webview.hide();
    }

    void init() override
    {
    }

private:
    unity::vector3 player_position{};
    float player_yaw = 0.0f;
    bool has_player_position = false;
    std::string local_session_id{};
    std::string status_text = "off";
    std::string last_sent_map{};
    std::chrono::steady_clock::time_point last_update_time{};
    tarkov_dev_sync_detail::TarkovDevSocketClient socket_client{};
    TarkovMapWebView internal_webview{};
    const tarkov_dev_sync_detail::MapDefinition* auto_detected_map = nullptr;

    [[nodiscard]] const tarkov_dev_sync_detail::MapDefinition* get_active_map() const
    {
        const int index = selected_map->get_value();
        if (index > 0)
        {
            const std::size_t map_index = static_cast<std::size_t>(index - 1);
            if (map_index < tarkov_dev_sync_detail::maps.size())
            {
                return &tarkov_dev_sync_detail::maps[map_index];
            }
        }

        return auto_detected_map;
    }

    [[nodiscard]] std::string ensure_remote_id()
    {
        std::string session_id = tarkov_dev_sync_detail::sanitize_session_id(remote_id->get());
        if (session_id.size() != 4)
        {
            session_id = tarkov_dev_sync_detail::generate_session_id();
            remote_id->set(session_id);
        }

        return session_id;
    }

    [[nodiscard]] RECT get_internal_map_bounds() const
    {
        RECT bounds{};
        HWND window = dxhook::get_window();
        RECT client{};
        if (!window || !GetClientRect(window, &client))
        {
            return bounds;
        }

        const int client_width = client.right - client.left;
        const int client_height = client.bottom - client.top;
        if (client_width <= 0 || client_height <= 0)
        {
            return bounds;
        }

        const float scale = menu::get_scale_factor();
        const int margin = (std::max)(10, static_cast<int>(18.0f * scale));

        if (internal_minimap->get_value())
        {
            const int max_size = (std::max)(140, (std::min)(client_width, client_height) - (margin * 2));
            const int min_size = (std::min)(180, max_size);
            const int size = (std::clamp)(
                static_cast<int>(internal_map_size->get_value() * scale),
                min_size,
                max_size);
            bounds.left = client.right - size - margin;
            bounds.top = client.bottom - size - margin;
            bounds.right = bounds.left + size;
            bounds.bottom = bounds.top + size;
        }

        return bounds;
    }

    void update_internal_map_view(const std::string& target_session_id, const std::string& map_key)
    {
        RECT bounds = get_internal_map_bounds();
        if (bounds.right <= bounds.left || bounds.bottom <= bounds.top)
        {
            internal_webview.hide();
            return;
        }

        HWND window = dxhook::get_window();
        if (!window)
        {
            internal_webview.hide();
            return;
        }

        internal_webview.show(window, bounds, target_session_id, map_key);
    }

    void refresh_internal_map_view()
    {
        internal_map->set("true");
        enabled->set("true");
        (void)ensure_remote_id();
        last_sent_map.clear();
        internal_webview.refresh();
    }

    void open_tarkov_dev_map()
    {
        const std::string session_id = ensure_remote_id();
        const tarkov_dev_sync_detail::MapDefinition* map = get_active_map();
        const std::string map_key = map ? map->key : "customs";
        const std::string url = "https://tarkov.dev/map/" + map_key + "?connection=" + session_id;
        ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        enabled->set("true");
        last_sent_map.clear();
    }
};
