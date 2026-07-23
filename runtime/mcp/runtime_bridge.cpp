#include "runtime_bridge.h"

#include <Windows.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <vector>

#include "../../util/logger.h"
#include "protocol.h"
#include "runtime_inspector.h"

namespace runtime::mcp::bridge_state
{
    constexpr wchar_t pipe_name[] = L"\\\\.\\pipe\\veil_mcp";
    constexpr DWORD pipe_buffer_size = 1024 * 1024;
    constexpr std::size_t maximum_requests_per_frame = 4;
    constexpr auto request_timeout = std::chrono::seconds(20);

    struct PendingRequest
    {
        FrameHeader header;
        std::vector<std::uint8_t> payload;
        std::mutex mutex;
        std::condition_variable completed_condition;
        std::string response;
        bool completed = false;
        bool cancelled = false;
    };

    std::atomic_bool running = false;
    std::mutex queue_mutex;
    std::deque<std::shared_ptr<PendingRequest>> request_queue;

    bool read_exact(
        const HANDLE pipe,
        void* buffer,
        const std::size_t size)
    {
        auto* output = static_cast<std::uint8_t*>(buffer);
        std::size_t total = 0;
        while (total < size)
        {
            DWORD bytes_read = 0;
            const DWORD chunk = static_cast<DWORD>(
                (std::min<std::size_t>)(size - total, MAXDWORD));
            if (!ReadFile(
                    pipe,
                    output + total,
                    chunk,
                    &bytes_read,
                    nullptr) ||
                bytes_read == 0)
            {
                return false;
            }
            total += bytes_read;
        }
        return true;
    }

    bool write_exact(
        const HANDLE pipe,
        const void* buffer,
        const std::size_t size)
    {
        const auto* input = static_cast<const std::uint8_t*>(buffer);
        std::size_t total = 0;
        while (total < size)
        {
            DWORD bytes_written = 0;
            const DWORD chunk = static_cast<DWORD>(
                (std::min<std::size_t>)(size - total, MAXDWORD));
            if (!WriteFile(
                    pipe,
                    input + total,
                    chunk,
                    &bytes_written,
                    nullptr) ||
                bytes_written == 0)
            {
                return false;
            }
            total += bytes_written;
        }
        return true;
    }

    std::string transport_error(const std::string& message)
    {
        std::string escaped;
        escaped.reserve(message.size());
        for (const char character : message)
        {
            if (character == '"' || character == '\\')
            {
                escaped.push_back('\\');
            }
            escaped.push_back(character);
        }
        return "{\"ok\":false,\"error\":\"" + escaped + "\"}";
    }

    void send_response(
        const HANDLE pipe,
        FrameHeader header,
        const std::string& response)
    {
        header.payload_size = static_cast<std::uint32_t>(response.size());
        if (!write_exact(pipe, &header, sizeof(header)))
        {
            return;
        }
        if (!response.empty())
        {
            write_exact(pipe, response.data(), response.size());
        }
        FlushFileBuffers(pipe);
    }

    void serve_client(const HANDLE pipe)
    {
        FrameHeader header{};
        if (!read_exact(pipe, &header, sizeof(header)))
        {
            return;
        }
        if (header.magic != protocol_magic)
        {
            send_response(
                pipe,
                header,
                transport_error("invalid Veil MCP frame magic"));
            return;
        }
        if (header.version != protocol_version)
        {
            send_response(
                pipe,
                header,
                transport_error("unsupported Veil MCP protocol version"));
            return;
        }
        if (header.payload_size > maximum_payload_size)
        {
            send_response(
                pipe,
                header,
                transport_error("Veil MCP payload exceeds the size limit"));
            return;
        }

        auto request = std::make_shared<PendingRequest>();
        request->header = header;
        request->payload.resize(header.payload_size);
        if (!request->payload.empty() &&
            !read_exact(
                pipe,
                request->payload.data(),
                request->payload.size()))
        {
            return;
        }

        {
            std::scoped_lock lock(queue_mutex);
            request_queue.push_back(request);
        }

        std::unique_lock lock(request->mutex);
        const bool completed = request->completed_condition.wait_for(
            lock,
            request_timeout,
            [&request]
            {
                return request->completed;
            });
        if (!completed)
        {
            request->cancelled = true;
            request->response = transport_error(
                "Unity thread did not service the request before timeout");
        }
        send_response(pipe, header, request->response);
    }

    DWORD WINAPI pipe_thread(void*)
    {
        logger::info("[MCP] runtime bridge listening on \\\\.\\pipe\\veil_mcp");
        while (running.load())
        {
            const HANDLE pipe = CreateNamedPipeW(
                pipe_name,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                1,
                pipe_buffer_size,
                pipe_buffer_size,
                0,
                nullptr);
            if (pipe == INVALID_HANDLE_VALUE)
            {
                logger::error(
                    "[MCP] CreateNamedPipe failed error=" +
                    std::to_string(GetLastError()));
                Sleep(500);
                continue;
            }

            const BOOL connected = ConnectNamedPipe(pipe, nullptr) ?
                TRUE : GetLastError() == ERROR_PIPE_CONNECTED;
            if (connected)
            {
                serve_client(pipe);
            }

            DisconnectNamedPipe(pipe);
            CloseHandle(pipe);
        }
        return 0;
    }
}

namespace runtime::mcp
{
    bool start()
    {
        bool expected = false;
        if (!bridge_state::running.compare_exchange_strong(expected, true))
        {
            return true;
        }

        const HANDLE thread = CreateThread(
            nullptr,
            0,
            bridge_state::pipe_thread,
            nullptr,
            0,
            nullptr);
        if (thread == nullptr)
        {
            bridge_state::running.store(false);
            logger::error(
                "[MCP] failed to start runtime bridge thread error=" +
                std::to_string(GetLastError()));
            return false;
        }
        CloseHandle(thread);
        return true;
    }

    void pump()
    {
        for (std::size_t index = 0;
             index < bridge_state::maximum_requests_per_frame;
             ++index)
        {
            std::shared_ptr<bridge_state::PendingRequest> request;
            {
                std::scoped_lock lock(bridge_state::queue_mutex);
                if (bridge_state::request_queue.empty())
                {
                    return;
                }
                request = bridge_state::request_queue.front();
                bridge_state::request_queue.pop_front();
            }

            {
                std::scoped_lock request_lock(request->mutex);
                if (request->cancelled)
                {
                    request->completed = true;
                    request->completed_condition.notify_one();
                    continue;
                }
            }

            const std::string response = execute_command(
                request->header.command,
                std::span<const std::uint8_t>(
                    request->payload.data(),
                    request->payload.size()));
            {
                std::scoped_lock request_lock(request->mutex);
                if (!request->cancelled)
                {
                    request->response = response;
                }
                request->completed = true;
            }
            request->completed_condition.notify_one();
        }
    }
}
