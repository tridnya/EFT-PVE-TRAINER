#include "media_widget.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <Windows.h>
#include <d3d11.h>
#include <imgui.h>
#include <wincodec.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Storage.Streams.h>
#include <wrl/client.h>

namespace media_widget_detail
{
    using Microsoft::WRL::ComPtr;

    constexpr long long ticks_per_second = 10000000LL;
    constexpr double media_poll_interval = 0.25;
    constexpr double manager_refresh_interval = 30.0;
    constexpr double session_refresh_interval = 3.0;
    constexpr double property_refresh_interval = 3.0;
    constexpr double texture_retry_interval = 10.0;

    using MediaDisplayInfo = media_widget::MediaDisplayInfo;
    using MediaPalette = media_widget::MediaPalette;
    using MediaSnapshot = media_widget::MediaSnapshot;

    struct SessionState
    {
        std::string title{};
        std::string artist{};
        std::string track{};
        std::string cover_key{};
        std::shared_ptr<const std::string> cover_bytes{};
        winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager manager{ nullptr };
        winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSession current_session{ nullptr };
        double manager_update = 0.0;
        double session_update = 0.0;
        double properties_update = 0.0;
        double timeline_update = 0.0;
        long long position_ticks = 0;
        long long end_ticks = 0;
        bool playing = false;
        bool initialized = false;
        bool has_result = false;
    };

    struct DecodedImage
    {
        std::vector<unsigned char> pixels{};
        UINT width = 0;
        UINT height = 0;
    };

    struct MediaTexture
    {
        ID3D11Texture2D* texture = nullptr;
        ID3D11ShaderResourceView* view = nullptr;
    };

    std::jthread worker{};
    std::atomic<std::shared_ptr<const MediaSnapshot>> snapshot{};
    std::atomic_bool widget_enabled{ true };
    std::string texture_key{};
    std::string previous_texture_key{};
    MediaTexture cover_texture{};
    MediaTexture background_texture{};
    MediaTexture previous_cover_texture{};
    MediaTexture previous_background_texture{};
    MediaPalette palette{};
    MediaPalette start_palette{};
    bool has_palette = false;
    bool is_media_session_cover = false;
    bool previous_is_media_session_cover = false;
    bool transition_active = false;
    bool texture_failed = false;
    double last_texture_attempt = 0.0;
    double transition_start = 0.0;
    float smoothed_fps = 0.0f;

    struct ScopedFont
    {
        bool pushed = false;

        explicit ScopedFont(ImFont* font)
        {
            if (!font)
            {
                return;
            }

            ImGui::PushFont(font);
            pushed = true;
        }

        ScopedFont(const ScopedFont&) = delete;
        ScopedFont(ScopedFont&&) = delete;
        ScopedFont& operator=(const ScopedFont&) = delete;
        ScopedFont& operator=(ScopedFont&&) = delete;

        ~ScopedFont()
        {
            if (pushed)
            {
                ImGui::PopFont();
            }
        }
    };

    [[nodiscard]] ImFont* widget_font() noexcept
    {
        ImGuiIO& io = ImGui::GetIO();
        if (io.Fonts->Fonts.Size > 1 && io.Fonts->Fonts[1])
        {
            return io.Fonts->Fonts[1];
        }

        return ImGui::GetFont();
    }

    [[nodiscard]] double clock_seconds() noexcept
    {
        return static_cast<double>(GetTickCount64()) / 1000.0;
    }

    [[nodiscard]] std::string wide_to_utf8(const std::wstring& text)
    {
        if (text.empty())
        {
            return {};
        }

        const int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size <= 0)
        {
            return {};
        }

        std::string utf8(static_cast<std::size_t>(size), '\0');
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, utf8.data(), size, nullptr, nullptr);
        utf8.pop_back();
        return utf8;
    }

    [[nodiscard]] std::string format_ticks(long long ticks)
    {
        const long long total_seconds = (std::max)(0LL, ticks / ticks_per_second);
        char buffer[16]{};
        std::snprintf(buffer, sizeof(buffer), "%lld:%02lld", total_seconds / 60, total_seconds % 60);
        return buffer;
    }

    template <std::size_t Size>
    void copy_text(std::array<char, Size>& output, const std::string& text)
    {
        output.fill('\0');
        std::snprintf(output.data(), output.size(), "%s", text.c_str());
    }

    template <std::size_t Size>
    void copy_text(std::array<char, Size>& output, const char* text)
    {
        output.fill('\0');
        if (text)
        {
            std::snprintf(output.data(), output.size(), "%s", text);
        }
    }

    [[nodiscard]] long long estimated_position_ticks(const SessionState& state, double time) noexcept
    {
        long long position = state.position_ticks;
        if (state.playing)
        {
            const double elapsed = (std::max)(0.0, time - state.timeline_update);
            position += static_cast<long long>(elapsed * static_cast<double>(ticks_per_second));
        }

        if (state.end_ticks > 0)
        {
            position = std::clamp(position, 0LL, state.end_ticks);
        }

        return position;
    }

    [[nodiscard]] std::string read_thumbnail(
        const winrt::Windows::Storage::Streams::IRandomAccessStreamReference& thumbnail)
    {
        namespace streams = winrt::Windows::Storage::Streams;

        try
        {
            if (!thumbnail)
            {
                return {};
            }

            const auto stream = thumbnail.OpenReadAsync().get();
            const uint64_t stream_size = stream.Size();
            if (stream_size == 0 || stream_size > 4ULL * 1024ULL * 1024ULL)
            {
                return {};
            }

            streams::Buffer buffer(static_cast<uint32_t>(stream_size));
            const auto read_buffer = stream.ReadAsync(buffer, buffer.Capacity(), streams::InputStreamOptions::ReadAhead).get();
            const uint32_t length = read_buffer.Length();
            if (length == 0)
            {
                return {};
            }

            std::string bytes(length, '\0');
            auto reader = streams::DataReader::FromBuffer(read_buffer);
            reader.ReadBytes(winrt::array_view<unsigned char>(
                reinterpret_cast<unsigned char*>(bytes.data()),
                reinterpret_cast<unsigned char*>(bytes.data()) + bytes.size()
            ));
            return bytes;
        }
        catch (...)
        {
            return {};
        }
    }

    bool fill_display_info(const SessionState& state, MediaDisplayInfo& info, double time)
    {
        if (!state.has_result || state.title.empty())
        {
            return false;
        }

        const long long position = estimated_position_ticks(state, time);
        const long long remaining = state.end_ticks > 0 ? (std::max)(0LL, state.end_ticks - position) : 0LL;

        copy_text(info.title, state.title);
        copy_text(info.artist, state.artist);
        copy_text(info.status, state.playing ? "playing" : "paused");
        copy_text(info.elapsed, format_ticks(position));
        copy_text(info.remaining, std::string("-") + format_ticks(remaining));
        copy_text(info.cover_key, state.cover_key);
        info.cover_bytes = state.cover_bytes;
        info.progress = state.end_ticks > 0
            ? static_cast<float>(static_cast<double>(position) / static_cast<double>(state.end_ticks))
            : 0.0f;
        info.progress = std::clamp(info.progress, 0.0f, 1.0f);
        info.playing = state.playing;
        info.has_timeline = state.end_ticks > 0;
        return true;
    }

    [[nodiscard]] std::shared_ptr<MediaSnapshot> poll_session(SessionState& state)
    {
        namespace media = winrt::Windows::Media::Control;

        auto next_snapshot = std::make_shared<MediaSnapshot>();
        next_snapshot->initialized = true;
        const double time = clock_seconds();

        try
        {
            if (!state.initialized)
            {
                winrt::init_apartment(winrt::apartment_type::multi_threaded);
                state.initialized = true;
            }

            if (!state.manager || time - state.manager_update > manager_refresh_interval)
            {
                state.manager = media::GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
                state.manager_update = time;
                state.current_session = nullptr;
                state.session_update = 0.0;
            }

            if (!state.manager)
            {
                state.has_result = false;
                return next_snapshot;
            }

            if (!state.current_session || time - state.session_update > session_refresh_interval)
            {
                state.current_session = state.manager.GetCurrentSession();
                state.session_update = time;
            }

            const auto session = state.current_session;
            if (!session)
            {
                state.has_result = false;
                return next_snapshot;
            }

            const long long estimated_position = estimated_position_ticks(state, time);
            const std::string previous_track = state.track;
            const auto playback_info = session.GetPlaybackInfo();
            const auto playback_status = playback_info.PlaybackStatus();
            const auto timeline = session.GetTimelineProperties();

            state.position_ticks = timeline.Position().count();
            state.end_ticks = timeline.EndTime().count();
            state.timeline_update = time;
            state.playing = playback_status == media::GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;

            const bool update_properties =
                !state.has_result || state.track.empty() || time - state.properties_update > property_refresh_interval;
            if (update_properties)
            {
                const auto properties = session.TryGetMediaPropertiesAsync().get();
                const std::string title = wide_to_utf8(std::wstring(properties.Title()));
                const std::string artist = wide_to_utf8(std::wstring(properties.Artist()));
                if (title.empty())
                {
                    state.has_result = false;
                    return next_snapshot;
                }

                state.title = title;
                state.artist = artist;
                state.track = state.artist.empty() ? state.title : state.artist + " - " + state.title;
                state.properties_update = time;

                if (state.track != previous_track)
                {
                    state.cover_key.clear();
                    state.cover_bytes.reset();
                }

                if (!state.cover_bytes)
                {
                    std::string cover_bytes = read_thumbnail(properties.Thumbnail());
                    if (!cover_bytes.empty())
                    {
                        state.cover_key = std::string("media-session:") + state.track;
                        state.cover_bytes = std::make_shared<const std::string>(std::move(cover_bytes));
                    }
                }
            }

            if (state.track.empty())
            {
                state.has_result = false;
                return next_snapshot;
            }

            if (state.playing && state.track == previous_track && estimated_position > state.position_ticks)
            {
                state.position_ticks = estimated_position;
            }

            state.has_result = true;
            next_snapshot->has_media = fill_display_info(state, next_snapshot->media, time);
        }
        catch (...)
        {
            state.has_result = false;
        }

        return next_snapshot;
    }

    void publish_snapshot(std::shared_ptr<MediaSnapshot> next_snapshot) noexcept
    {
        if (!next_snapshot)
        {
            return;
        }

        std::shared_ptr<const MediaSnapshot> readonly_snapshot = std::move(next_snapshot);
        snapshot.store(readonly_snapshot, std::memory_order_release);
    }

    void worker_loop(std::stop_token stop_token)
    {
        SessionState state{};
        auto next_poll = std::chrono::steady_clock::now();

        while (!stop_token.stop_requested())
        {
            publish_snapshot(poll_session(state));
            next_poll += std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>(media_poll_interval));

            const auto now = std::chrono::steady_clock::now();
            if (now < next_poll)
            {
                std::this_thread::sleep_until(next_poll);
            }
            else
            {
                next_poll = now;
            }
        }

        if (state.initialized)
        {
            winrt::uninit_apartment();
        }
    }

    void release_resource(MediaTexture& resource) noexcept
    {
        if (resource.view)
        {
            resource.view->Release();
            resource.view = nullptr;
        }

        if (resource.texture)
        {
            resource.texture->Release();
            resource.texture = nullptr;
        }
    }

    [[nodiscard]] ImU32 pack_color(float red, float green, float blue, int alpha) noexcept
    {
        const int red_byte = std::clamp(static_cast<int>(red + 0.5f), 0, 255);
        const int green_byte = std::clamp(static_cast<int>(green + 0.5f), 0, 255);
        const int blue_byte = std::clamp(static_cast<int>(blue + 0.5f), 0, 255);
        return IM_COL32(red_byte, green_byte, blue_byte, alpha);
    }

    [[nodiscard]] float mix(float value, float target, float amount) noexcept
    {
        return value + ((target - value) * amount);
    }

    void strengthen_color(float& red, float& green, float& blue, float saturation_amount, float min_channel_max) noexcept
    {
        const float luma = (red * 0.299f) + (green * 0.587f) + (blue * 0.114f);
        red = std::clamp(luma + ((red - luma) * saturation_amount), 0.0f, 255.0f);
        green = std::clamp(luma + ((green - luma) * saturation_amount), 0.0f, 255.0f);
        blue = std::clamp(luma + ((blue - luma) * saturation_amount), 0.0f, 255.0f);

        const float max_channel = (std::max)(red, (std::max)(green, blue));
        if (max_channel < min_channel_max)
        {
            const float lift = min_channel_max / (std::max)(max_channel, 1.0f);
            red = std::clamp(red * lift, 0.0f, 255.0f);
            green = std::clamp(green * lift, 0.0f, 255.0f);
            blue = std::clamp(blue * lift, 0.0f, 255.0f);
        }
    }

    [[nodiscard]] ImU32 derived_secondary_color(ImU32 color) noexcept
    {
        const ImVec4 value = ImGui::ColorConvertU32ToFloat4(color);
        float red = (value.z * 255.0f * 0.62f) + 78.0f;
        float green = (value.x * 255.0f * 0.46f) + 92.0f;
        float blue = (value.y * 255.0f * 0.58f) + 88.0f;
        strengthen_color(red, green, blue, 1.18f, 120.0f);
        return pack_color(red, green, blue, static_cast<int>(std::clamp(value.w, 0.0f, 1.0f) * 255.0f));
    }

    struct PaletteResult
    {
        MediaPalette palette{};
        bool has_palette = false;
    };

    [[nodiscard]] PaletteResult palette_from_pixels(const std::vector<unsigned char>& pixels)
    {
        struct PaletteBucket
        {
            double red = 0.0;
            double green = 0.0;
            double blue = 0.0;
            int count = 0;
        };

        PaletteResult result{};
        if (pixels.size() < 4)
        {
            return result;
        }

        std::array<PaletteBucket, 4096> buckets{};
        double fallback_red = 0.0;
        double fallback_green = 0.0;
        double fallback_blue = 0.0;
        double color_presence_red = 0.0;
        double color_presence_green = 0.0;
        double color_presence_blue = 0.0;
        double color_presence_weight = 0.0;
        double saturation_sum = 0.0;
        int sample_count = 0;
        const std::size_t pixel_count = pixels.size() / 4;
        const std::size_t step = (std::max)(static_cast<std::size_t>(1), pixel_count / 4096);

        for (std::size_t pixel_index = 0; pixel_index < pixel_count; pixel_index += step)
        {
            const std::size_t offset = pixel_index * 4;
            const float blue = static_cast<float>(pixels[offset + 0]);
            const float green = static_cast<float>(pixels[offset + 1]);
            const float red = static_cast<float>(pixels[offset + 2]);
            const float alpha = static_cast<float>(pixels[offset + 3]);
            if (alpha < 16.0f)
            {
                continue;
            }

            const float max_channel = (std::max)(red, (std::max)(green, blue));
            const float min_channel = (std::min)(red, (std::min)(green, blue));
            const float saturation = max_channel > 1.0f ? (max_channel - min_channel) / max_channel : 0.0f;
            const float brightness = max_channel / 255.0f;

            fallback_red += red;
            fallback_green += green;
            fallback_blue += blue;
            saturation_sum += saturation;
            ++sample_count;

            const float color_presence = saturation * std::clamp((brightness - 0.08f) / 0.42f, 0.0f, 1.0f);
            if (color_presence > 0.0f)
            {
                color_presence_red += static_cast<double>(red) * color_presence;
                color_presence_green += static_cast<double>(green) * color_presence;
                color_presence_blue += static_cast<double>(blue) * color_presence;
                color_presence_weight += color_presence;
            }

            const int bucket_red = std::clamp(static_cast<int>(red) >> 4, 0, 15);
            const int bucket_green = std::clamp(static_cast<int>(green) >> 4, 0, 15);
            const int bucket_blue = std::clamp(static_cast<int>(blue) >> 4, 0, 15);
            PaletteBucket& bucket = buckets[(bucket_red << 8) | (bucket_green << 4) | bucket_blue];
            bucket.red += red;
            bucket.green += green;
            bucket.blue += blue;
            ++bucket.count;
        }

        if (sample_count <= 0)
        {
            return result;
        }

        fallback_red /= sample_count;
        fallback_green /= sample_count;
        fallback_blue /= sample_count;
        const double average_saturation = saturation_sum / sample_count;
        const double color_presence_ratio = color_presence_weight / sample_count;

        if (color_presence_weight > 0.0)
        {
            color_presence_red /= color_presence_weight;
            color_presence_green /= color_presence_weight;
            color_presence_blue /= color_presence_weight;
        }
        else
        {
            color_presence_red = fallback_red;
            color_presence_green = fallback_green;
            color_presence_blue = fallback_blue;
        }

        int dominant_color_count = 0;
        float dominant_color_red = static_cast<float>(color_presence_red);
        float dominant_color_green = static_cast<float>(color_presence_green);
        float dominant_color_blue = static_cast<float>(color_presence_blue);
        float dominant_color_saturation = 0.0f;

        for (const PaletteBucket& bucket : buckets)
        {
            if (bucket.count <= 0)
            {
                continue;
            }

            const float red = static_cast<float>(bucket.red / bucket.count);
            const float green = static_cast<float>(bucket.green / bucket.count);
            const float blue = static_cast<float>(bucket.blue / bucket.count);
            const float max_channel = (std::max)(red, (std::max)(green, blue));
            const float min_channel = (std::min)(red, (std::min)(green, blue));
            const float saturation = max_channel > 1.0f ? (max_channel - min_channel) / max_channel : 0.0f;
            const float brightness = max_channel / 255.0f;

            if (saturation < 0.045f || brightness < 0.035f || brightness > 0.92f)
            {
                continue;
            }

            if (bucket.count > dominant_color_count)
            {
                dominant_color_count = bucket.count;
                dominant_color_red = red;
                dominant_color_green = green;
                dominant_color_blue = blue;
                dominant_color_saturation = saturation;
            }
        }

        const float dominant_color_coverage = static_cast<float>(dominant_color_count) / static_cast<float>(sample_count);
        const float dominant_color_blend = dominant_color_saturation > 0.0f
            ? std::clamp((dominant_color_coverage - 0.055f) / 0.30f, 0.0f, 0.42f)
            : 0.0f;

        result.has_palette = true;

        if (average_saturation < 0.075 && color_presence_ratio < 0.018)
        {
            const float neutral =
                std::clamp(static_cast<float>((fallback_red + fallback_green + fallback_blue) / 3.0), 112.0f, 214.0f);
            float accent_red = mix(neutral, 255.0f, 0.18f);
            float accent_green = accent_red;
            float accent_blue = accent_red;

            const float grayscale_color_blend = std::clamp(dominant_color_blend, 0.0f, 0.28f);
            accent_red = mix(accent_red, dominant_color_red, grayscale_color_blend);
            accent_green = mix(accent_green, dominant_color_green, grayscale_color_blend);
            accent_blue = mix(accent_blue, dominant_color_blue, grayscale_color_blend);

            result.palette.accent = pack_color(accent_red, accent_green, accent_blue, 242);
            result.palette.accent_secondary = derived_secondary_color(result.palette.accent);
            return result;
        }

        double best_score = -1.0;
        int best_count = 0;
        float accent_red = static_cast<float>(fallback_red);
        float accent_green = static_cast<float>(fallback_green);
        float accent_blue = static_cast<float>(fallback_blue);

        for (const PaletteBucket& bucket : buckets)
        {
            if (bucket.count <= 0)
            {
                continue;
            }

            const float red = static_cast<float>(bucket.red / bucket.count);
            const float green = static_cast<float>(bucket.green / bucket.count);
            const float blue = static_cast<float>(bucket.blue / bucket.count);
            const float max_channel = (std::max)(red, (std::max)(green, blue));
            const float min_channel = (std::min)(red, (std::min)(green, blue));
            const float saturation = max_channel > 1.0f ? (max_channel - min_channel) / max_channel : 0.0f;
            const float brightness = max_channel / 255.0f;
            const float brightness_score =
                std::clamp(1.15f - (std::abs(brightness - 0.55f) * 1.55f), 0.12f, 1.0f);
            const float saturation_score = 0.25f + (saturation * 2.35f);
            const float gray_penalty = saturation < 0.10f && brightness > 0.20f ? 0.25f : 1.0f;
            const double score = static_cast<double>(bucket.count) * saturation_score * brightness_score * gray_penalty;

            if (score > best_score)
            {
                best_score = score;
                best_count = bucket.count;
                accent_red = red;
                accent_green = green;
                accent_blue = blue;
            }
        }

        const float best_coverage = static_cast<float>(best_count) / static_cast<float>(sample_count);
        const float coverage_blend = std::clamp((best_coverage - 0.03f) / 0.22f, 0.0f, 0.55f);
        const float presence_blend = std::clamp(static_cast<float>((color_presence_ratio - 0.018) / 0.09), 0.0f, 0.38f);
        const float amount_blend = (std::max)(coverage_blend, presence_blend);

        accent_red = mix(accent_red, static_cast<float>(color_presence_red), amount_blend);
        accent_green = mix(accent_green, static_cast<float>(color_presence_green), amount_blend);
        accent_blue = mix(accent_blue, static_cast<float>(color_presence_blue), amount_blend);
        accent_red = mix(accent_red, dominant_color_red, dominant_color_blend);
        accent_green = mix(accent_green, dominant_color_green, dominant_color_blend);
        accent_blue = mix(accent_blue, dominant_color_blue, dominant_color_blend);

        strengthen_color(accent_red, accent_green, accent_blue, 1.28f, 110.0f);
        result.palette.accent = pack_color(
            mix(accent_red, 255.0f, 0.10f),
            mix(accent_green, 255.0f, 0.10f),
            mix(accent_blue, 255.0f, 0.10f),
            242);
        result.palette.accent_secondary = derived_secondary_color(result.palette.accent);
        return result;
    }

    [[nodiscard]] ImU32 lerp_color(ImU32 start_color, ImU32 end_color, float amount) noexcept
    {
        const ImVec4 start = ImGui::ColorConvertU32ToFloat4(start_color);
        const ImVec4 end = ImGui::ColorConvertU32ToFloat4(end_color);
        return ImGui::ColorConvertFloat4ToU32({
            start.x + ((end.x - start.x) * amount),
            start.y + ((end.y - start.y) * amount),
            start.z + ((end.z - start.z) * amount),
            start.w + ((end.w - start.w) * amount)
        });
    }

    [[nodiscard]] float ease_out_cubic(float amount) noexcept
    {
        const float inverse = 1.0f - amount;
        return 1.0f - (inverse * inverse * inverse);
    }

    [[nodiscard]] ImU32 with_alpha(ImU32 color, float alpha) noexcept
    {
        ImVec4 value = ImGui::ColorConvertU32ToFloat4(color);
        value.w = std::clamp(alpha, 0.0f, 1.0f);
        return ImGui::ColorConvertFloat4ToU32(value);
    }

    void release_previous_texture() noexcept
    {
        release_resource(previous_cover_texture);
        release_resource(previous_background_texture);
        previous_texture_key.clear();
        previous_is_media_session_cover = false;
    }

    void reset_palette() noexcept
    {
        palette = MediaPalette{};
        start_palette = palette;
        has_palette = false;
        is_media_session_cover = false;
        previous_is_media_session_cover = false;
        transition_active = false;
    }

    void release_textures() noexcept
    {
        release_resource(cover_texture);
        release_resource(background_texture);
        release_previous_texture();
        texture_key.clear();
        texture_failed = false;
        last_texture_attempt = 0.0;
        transition_start = 0.0;
        reset_palette();
    }

    void snapshot_palette(float amount) noexcept
    {
        start_palette.panel_top = lerp_color(start_palette.panel_top, palette.panel_top, amount);
        start_palette.panel_top_right = lerp_color(start_palette.panel_top_right, palette.panel_top_right, amount);
        start_palette.panel_bottom = lerp_color(start_palette.panel_bottom, palette.panel_bottom, amount);
        start_palette.panel_bottom_left = lerp_color(start_palette.panel_bottom_left, palette.panel_bottom_left, amount);
        start_palette.border = lerp_color(start_palette.border, palette.border, amount);
        start_palette.accent = lerp_color(start_palette.accent, palette.accent, amount);
        start_palette.accent_secondary = lerp_color(start_palette.accent_secondary, palette.accent_secondary, amount);
        start_palette.accent_dim = lerp_color(start_palette.accent_dim, palette.accent_dim, amount);
        start_palette.cover_border = lerp_color(start_palette.cover_border, palette.cover_border, amount);
        start_palette.secondary_text = lerp_color(start_palette.secondary_text, palette.secondary_text, amount);
        start_palette.timer_text = lerp_color(start_palette.timer_text, palette.timer_text, amount);
        start_palette.control = lerp_color(start_palette.control, palette.control, amount);
    }

    [[nodiscard]] float get_transition_amount() noexcept
    {
        if (!transition_active)
        {
            return 1.0f;
        }

        constexpr double transition_duration = 0.55;
        const float amount = std::clamp(
            static_cast<float>((ImGui::GetTime() - transition_start) / transition_duration),
            0.0f,
            1.0f
        );

        return ease_out_cubic(amount);
    }

    void begin_texture_transition(const std::string& key)
    {
        const float amount = get_transition_amount();
        snapshot_palette(amount);
        release_previous_texture();

        previous_texture_key = texture_key;
        previous_cover_texture = cover_texture;
        previous_background_texture = background_texture;
        previous_is_media_session_cover = is_media_session_cover;

        cover_texture = {};
        background_texture = {};
        texture_key = key;
        is_media_session_cover = texture_key.rfind("media-session:", 0) == 0;
        texture_failed = false;
        last_texture_attempt = 0.0;
        transition_active = true;
        transition_start = ImGui::GetTime();
    }

    void finish_texture_transition(float amount) noexcept
    {
        if (!transition_active || amount < 0.999f)
        {
            return;
        }

        release_previous_texture();
        transition_active = false;
    }

    [[nodiscard]] ImU32 fade_color(ImU32 start_color, ImU32 target_color, ImU32 fallback_color, float amount) noexcept
    {
        const ImU32 resolved_target = has_palette ? target_color : fallback_color;
        return transition_active ? lerp_color(start_color, resolved_target, amount) : resolved_target;
    }

    [[nodiscard]] bool create_texture_from_pixels(
        ID3D11Device* device,
        const std::vector<unsigned char>& pixels,
        UINT width,
        UINT height,
        MediaTexture& output)
    {
        if (!device || pixels.empty() || width == 0 || height == 0)
        {
            return false;
        }

        D3D11_TEXTURE2D_DESC texture_desc{};
        texture_desc.Width = width;
        texture_desc.Height = height;
        texture_desc.MipLevels = 1;
        texture_desc.ArraySize = 1;
        texture_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        texture_desc.SampleDesc.Count = 1;
        texture_desc.Usage = D3D11_USAGE_DEFAULT;
        texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initial_data{};
        initial_data.pSysMem = pixels.data();
        initial_data.SysMemPitch = width * 4;

        if (FAILED(device->CreateTexture2D(&texture_desc, &initial_data, &output.texture)))
        {
            return false;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC view_desc{};
        view_desc.Format = texture_desc.Format;
        view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        view_desc.Texture2D.MipLevels = 1;

        if (FAILED(device->CreateShaderResourceView(output.texture, &view_desc, &output.view)))
        {
            output.texture->Release();
            output.texture = nullptr;
            return false;
        }

        return true;
    }

    void box_blur_image(
        const std::vector<unsigned char>& source,
        std::vector<unsigned char>& target,
        UINT width,
        UINT height,
        int radius)
    {
        target.resize(source.size());

        for (UINT y = 0; y < height; ++y)
        {
            for (UINT x = 0; x < width; ++x)
            {
                int channels[4]{};
                int count = 0;

                for (int offset_y = -radius; offset_y <= radius; ++offset_y)
                {
                    const int sample_y = std::clamp(static_cast<int>(y) + offset_y, 0, static_cast<int>(height) - 1);
                    for (int offset_x = -radius; offset_x <= radius; ++offset_x)
                    {
                        const int sample_x = std::clamp(static_cast<int>(x) + offset_x, 0, static_cast<int>(width) - 1);
                        const std::size_t sample_index =
                            ((static_cast<std::size_t>(sample_y) * width) + static_cast<std::size_t>(sample_x)) * 4ull;

                        for (int channel = 0; channel < 4; ++channel)
                        {
                            channels[channel] += source[sample_index + channel];
                        }

                        ++count;
                    }
                }

                const std::size_t target_index =
                    ((static_cast<std::size_t>(y) * width) + static_cast<std::size_t>(x)) * 4ull;
                for (int channel = 0; channel < 4; ++channel)
                {
                    target[target_index + channel] = static_cast<unsigned char>(channels[channel] / count);
                }
                target[target_index + 3] = 255;
            }
        }
    }

    [[nodiscard]] bool create_blurred_background_texture(
        ID3D11Device* device,
        const std::vector<unsigned char>& pixels,
        UINT width,
        UINT height,
        MediaTexture& output)
    {
        if (!device || pixels.empty() || width == 0 || height == 0)
        {
            return false;
        }

        constexpr UINT background_size = 64;
        std::vector<unsigned char> downsampled(static_cast<std::size_t>(background_size) * background_size * 4ull);

        for (UINT y = 0; y < background_size; ++y)
        {
            for (UINT x = 0; x < background_size; ++x)
            {
                const UINT source_x =
                    (std::min)(width - 1, static_cast<UINT>((static_cast<uint64_t>(x) * width) / background_size));
                const UINT source_y =
                    (std::min)(height - 1, static_cast<UINT>((static_cast<uint64_t>(y) * height) / background_size));
                const std::size_t source_index =
                    ((static_cast<std::size_t>(source_y) * width) + static_cast<std::size_t>(source_x)) * 4ull;
                const std::size_t target_index =
                    ((static_cast<std::size_t>(y) * background_size) + static_cast<std::size_t>(x)) * 4ull;

                downsampled[target_index + 0] = pixels[source_index + 0];
                downsampled[target_index + 1] = pixels[source_index + 1];
                downsampled[target_index + 2] = pixels[source_index + 2];
                downsampled[target_index + 3] = 255;
            }
        }

        std::vector<unsigned char> blur_a{};
        std::vector<unsigned char> blur_b{};
        box_blur_image(downsampled, blur_a, background_size, background_size, 4);
        box_blur_image(blur_a, blur_b, background_size, background_size, 4);
        box_blur_image(blur_b, blur_a, background_size, background_size, 3);

        return create_texture_from_pixels(device, blur_a, background_size, background_size, output);
    }

    [[nodiscard]] bool decode_image_bytes(const std::string& bytes, DecodedImage& image)
    {
        if (bytes.empty())
        {
            return false;
        }

        const HRESULT com_result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(com_result) && com_result != RPC_E_CHANGED_MODE)
        {
            return false;
        }

        ComPtr<IWICImagingFactory> factory{};
        ComPtr<IWICStream> stream{};
        ComPtr<IWICBitmapDecoder> decoder{};
        ComPtr<IWICBitmapFrameDecode> frame{};
        ComPtr<IWICFormatConverter> converter{};

        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory))))
        {
            return false;
        }

        if (FAILED(factory->CreateStream(&stream)))
        {
            return false;
        }

        if (FAILED(stream->InitializeFromMemory(
            reinterpret_cast<BYTE*>(const_cast<char*>(bytes.data())),
            static_cast<DWORD>(bytes.size()))))
        {
            return false;
        }

        if (FAILED(factory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnLoad, &decoder)))
        {
            return false;
        }

        if (FAILED(decoder->GetFrame(0, &frame)))
        {
            return false;
        }

        if (FAILED(factory->CreateFormatConverter(&converter)))
        {
            return false;
        }

        if (FAILED(converter->Initialize(
            frame.Get(),
            GUID_WICPixelFormat32bppBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeCustom)))
        {
            return false;
        }

        if (FAILED(converter->GetSize(&image.width, &image.height)) || image.width == 0 || image.height == 0)
        {
            return false;
        }

        image.pixels.resize(static_cast<std::size_t>(image.width) * static_cast<std::size_t>(image.height) * 4ull);
        const UINT stride = image.width * 4;
        return SUCCEEDED(converter->CopyPixels(nullptr, stride, static_cast<UINT>(image.pixels.size()), image.pixels.data()));
    }

    [[nodiscard]] ImU32 alpha_color(float alpha) noexcept
    {
        const int alpha_byte = std::clamp(static_cast<int>((alpha * 255.0f) + 0.5f), 0, 255);
        return IM_COL32(255, 255, 255, alpha_byte);
    }

    void draw_album_cover_image(
        ImDrawList* draw_list,
        ID3D11ShaderResourceView* view,
        bool media_session_cover,
        const ImVec2& min,
        const ImVec2& max,
        float alpha) noexcept
    {
        if (!draw_list || !view || alpha <= 0.0f)
        {
            return;
        }

        const ImVec2 uv_min = media_session_cover ? ImVec2(0.14f, 0.04f) : ImVec2(0.0f, 0.0f);
        const ImVec2 uv_max = media_session_cover ? ImVec2(0.86f, 0.76f) : ImVec2(1.0f, 1.0f);
        draw_list->AddImageRounded(reinterpret_cast<ImTextureID>(view), min, max, uv_min, uv_max, alpha_color(alpha), 6.0f);
    }

    void draw_album_background_image(
        ImDrawList* draw_list,
        ID3D11ShaderResourceView* view,
        const ImVec2& min,
        const ImVec2& max,
        float alpha,
        float rounding) noexcept
    {
        if (!draw_list || !view || alpha <= 0.0f)
        {
            return;
        }

        draw_list->AddImageRounded(
            reinterpret_cast<ImTextureID>(view),
            min,
            max,
            ImVec2(0.0f, 0.0f),
            ImVec2(1.0f, 1.0f),
            alpha_color(alpha),
            rounding);
    }

    struct MediaTextMetrics
    {
        std::array<char, 96> title{};
        std::array<char, 96> subtitle{};
        std::array<char, 16> remaining{};
        float title_width = 0.0f;
        float subtitle_width = 0.0f;
        float remaining_width = 0.0f;
    };

    [[nodiscard]] const MediaTextMetrics& get_media_text_metrics(const MediaDisplayInfo& media)
    {
        static MediaTextMetrics metrics{};
        const char* subtitle = media.artist[0] ? media.artist.data() : media.status.data();

        if (std::strcmp(metrics.title.data(), media.title.data()) != 0)
        {
            std::snprintf(metrics.title.data(), metrics.title.size(), "%s", media.title.data());
            metrics.title_width = ImGui::CalcTextSize(metrics.title.data()).x;
        }

        if (std::strcmp(metrics.subtitle.data(), subtitle) != 0)
        {
            std::snprintf(metrics.subtitle.data(), metrics.subtitle.size(), "%s", subtitle);
            metrics.subtitle_width = ImGui::CalcTextSize(metrics.subtitle.data()).x;
        }

        if (std::strcmp(metrics.remaining.data(), media.remaining.data()) != 0)
        {
            std::snprintf(metrics.remaining.data(), metrics.remaining.size(), "%s", media.remaining.data());
            metrics.remaining_width = ImGui::CalcTextSize(metrics.remaining.data()).x;
        }

        return metrics;
    }

    void update_cover_texture(ID3D11Device* device, const MediaDisplayInfo& media)
    {
        const char* key = media.cover_key.data();
        if (!key[0] || !media.cover_bytes || media.cover_bytes->empty())
        {
            return;
        }

        const std::string media_key = key;
        if (texture_key != media_key)
        {
            begin_texture_transition(media_key);
        }

        if (cover_texture.view)
        {
            return;
        }

        const double time = ImGui::GetTime();
        if (texture_failed && time - last_texture_attempt < texture_retry_interval)
        {
            return;
        }

        last_texture_attempt = time;
        if (!device)
        {
            texture_failed = true;
            return;
        }

        DecodedImage image{};
        if (!decode_image_bytes(*media.cover_bytes, image))
        {
            release_resource(cover_texture);
            release_resource(background_texture);
            release_previous_texture();
            reset_palette();
            texture_key = media_key;
            texture_failed = true;
            return;
        }

        MediaTexture next_cover_texture{};
        if (!create_texture_from_pixels(device, image.pixels, image.width, image.height, next_cover_texture))
        {
            release_resource(cover_texture);
            release_resource(background_texture);
            release_previous_texture();
            reset_palette();
            texture_key = media_key;
            texture_failed = true;
            return;
        }

        const PaletteResult palette_result = palette_from_pixels(image.pixels);
        palette = palette_result.palette;
        has_palette = palette_result.has_palette;
        cover_texture = next_cover_texture;

        MediaTexture next_background_texture{};
        if (create_blurred_background_texture(device, image.pixels, image.width, image.height, next_background_texture))
        {
            background_texture = next_background_texture;
        }

        texture_failed = false;
    }

    void draw_media(const MediaDisplayInfo& media)
    {
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
        if (!draw_list)
        {
            return;
        }

        ScopedFont font_scope(widget_font());
        ImGuiIO& io = ImGui::GetIO();
        const MediaTextMetrics& text_metrics = get_media_text_metrics(media);
        const float text_content_width = (std::max)(text_metrics.title_width, text_metrics.subtitle_width);
        const float width = (std::max)(338.0f, 52.0f + 12.0f + text_content_width + 70.0f + 24.0f);
        const ImVec2 size(width, 88.0f);
        const ImVec2 pos(io.DisplaySize.x - size.x - 12.0f, 12.0f);
        const ImVec2 panel_max(pos.x + size.x, pos.y + size.y);
        constexpr float rounding = 10.0f;
        const float transition_amount = get_transition_amount();
        const bool cover_matches_media = media.cover_key[0] && texture_key == media.cover_key.data();
        ID3D11ShaderResourceView* current_cover_view = cover_matches_media ? cover_texture.view : nullptr;
        ID3D11ShaderResourceView* current_background_view = cover_matches_media ? background_texture.view : nullptr;
        const bool current_media_session_cover = cover_matches_media && is_media_session_cover;

        const ImU32 border_color = fade_color(
            start_palette.border,
            palette.border,
            IM_COL32(72, 72, 76, 150),
            transition_amount);
        const ImU32 accent_color = IM_COL32(245, 245, 242, 238);
        const ImU32 accent_dim_color = IM_COL32(190, 190, 190, 82);
        const ImU32 cover_border_color = fade_color(
            start_palette.cover_border,
            palette.cover_border,
            IM_COL32(178, 178, 184, 145),
            transition_amount);
        const ImU32 secondary_text_color = fade_color(
            start_palette.secondary_text,
            palette.secondary_text,
            IM_COL32(205, 205, 210, 255),
            transition_amount);
        const ImU32 timer_text_color = fade_color(
            start_palette.timer_text,
            palette.timer_text,
            IM_COL32(184, 184, 190, 255),
            transition_amount);
        const ImU32 control_shadow_color = IM_COL32(172, 172, 176, 180);
        const ImU32 control_color = IM_COL32(248, 248, 246, 245);

        if (previous_background_texture.view && transition_active)
        {
            draw_album_background_image(
                draw_list,
                previous_background_texture.view,
                pos,
                panel_max,
                1.0f - transition_amount,
                rounding);
        }

        if (current_background_view)
        {
            draw_album_background_image(
                draw_list,
                current_background_view,
                pos,
                panel_max,
                previous_background_texture.view ? transition_amount : 1.0f,
                rounding);
        }

        draw_list->AddRectFilled(pos, panel_max, IM_COL32(18, 18, 20, 124), rounding);
        draw_list->AddRectFilled(pos, panel_max, IM_COL32(6, 6, 8, 82), rounding);
        draw_list->AddRect(pos, panel_max, border_color, rounding);

        const ImVec2 cover_min(pos.x + 10.0f, pos.y + 14.0f);
        const ImVec2 cover_max(cover_min.x + 52.0f, cover_min.y + 52.0f);
        if (current_cover_view)
        {
            if (previous_cover_texture.view && transition_active)
            {
                draw_album_cover_image(
                    draw_list,
                    previous_cover_texture.view,
                    previous_is_media_session_cover,
                    cover_min,
                    cover_max,
                    1.0f - transition_amount);
            }

            draw_album_cover_image(
                draw_list,
                current_cover_view,
                current_media_session_cover,
                cover_min,
                cover_max,
                previous_cover_texture.view ? transition_amount : 1.0f);
        }
        else if (previous_cover_texture.view && transition_active)
        {
            draw_album_cover_image(
                draw_list,
                previous_cover_texture.view,
                previous_is_media_session_cover,
                cover_min,
                cover_max,
                1.0f);
        }
        else
        {
            draw_list->AddRectFilled(cover_min, cover_max, IM_COL32(79, 123, 220, 255), 6.0f);
            draw_list->AddRectFilled(
                cover_min,
                ImVec2(cover_max.x, cover_min.y + 18.0f),
                IM_COL32(142, 158, 255, 210),
                6.0f);
        }

        draw_list->AddRect(cover_min, cover_max, cover_border_color, 6.0f);

        const ImVec2 text_pos(cover_max.x + 12.0f, pos.y + 10.0f);
        const ImVec2 right_edge(pos.x + size.x - 12.0f, pos.y + size.y);
        const char* subtitle = media.artist[0] ? media.artist.data() : media.status.data();

        draw_list->AddText(text_pos, IM_COL32(238, 243, 255, 255), media.title.data());
        draw_list->AddText(ImVec2(text_pos.x, text_pos.y + 18.0f), secondary_text_color, subtitle);

        const ImVec2 bar_min(text_pos.x, pos.y + 57.0f);
        const ImVec2 bar_max(right_edge.x, bar_min.y + 4.0f);
        draw_list->AddRectFilled(bar_min, bar_max, accent_dim_color, 2.0f);
        if (media.has_timeline)
        {
            draw_list->AddRectFilled(
                bar_min,
                ImVec2(bar_min.x + ((bar_max.x - bar_min.x) * media.progress), bar_max.y),
                accent_color,
                2.0f);
        }

        draw_list->AddText(ImVec2(text_pos.x, bar_max.y + 6.0f), timer_text_color, media.elapsed.data());
        draw_list->AddText(
            ImVec2(right_edge.x - text_metrics.remaining_width, bar_max.y + 6.0f),
            timer_text_color,
            media.remaining.data());

        const ImVec2 button_center(pos.x + size.x - 31.0f, pos.y + 39.0f);
        if (media.playing)
        {
            draw_list->AddRectFilled(
                ImVec2(button_center.x - 6.0f, button_center.y - 9.0f),
                ImVec2(button_center.x - 1.0f, button_center.y + 11.0f),
                control_shadow_color,
                1.0f);
            draw_list->AddRectFilled(
                ImVec2(button_center.x + 4.0f, button_center.y - 9.0f),
                ImVec2(button_center.x + 9.0f, button_center.y + 11.0f),
                control_shadow_color,
                1.0f);
            draw_list->AddRectFilled(
                ImVec2(button_center.x - 7.0f, button_center.y - 10.0f),
                ImVec2(button_center.x - 2.0f, button_center.y + 10.0f),
                control_color,
                1.0f);
            draw_list->AddRectFilled(
                ImVec2(button_center.x + 3.0f, button_center.y - 10.0f),
                ImVec2(button_center.x + 8.0f, button_center.y + 10.0f),
                control_color,
                1.0f);
        }
        else
        {
            draw_list->AddTriangleFilled(
                ImVec2(button_center.x - 4.0f, button_center.y - 10.0f),
                ImVec2(button_center.x - 4.0f, button_center.y + 12.0f),
                ImVec2(button_center.x + 13.0f, button_center.y + 1.0f),
                control_shadow_color);
            draw_list->AddTriangleFilled(
                ImVec2(button_center.x - 5.0f, button_center.y - 11.0f),
                ImVec2(button_center.x - 5.0f, button_center.y + 11.0f),
                ImVec2(button_center.x + 12.0f, button_center.y),
                control_color);
        }

        finish_texture_transition(transition_amount);
    }

    void draw_fallback()
    {
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
        if (!draw_list)
        {
            return;
        }

        ScopedFont font_scope(widget_font());
        ImGuiIO& io = ImGui::GetIO();
        char text[64]{};
        std::snprintf(text, sizeof(text), "veil | %.0f fps", smoothed_fps);
        const ImVec2 text_size = ImGui::CalcTextSize(text);
        const ImVec2 pad(10.0f, 6.0f);
        const ImVec2 pos(io.DisplaySize.x - text_size.x - (pad.x * 2.0f) - 12.0f, 12.0f);
        const ImVec2 size(text_size.x + (pad.x * 2.0f), text_size.y + (pad.y * 2.0f));
        const ImVec2 panel_max(pos.x + size.x, pos.y + size.y);

        draw_list->AddRectFilled(pos, panel_max, IM_COL32(8, 11, 18, 232), 4.0f);
        draw_list->AddRect(pos, panel_max, IM_COL32(45, 56, 76, 255), 4.0f);
        draw_list->AddRectFilledMultiColor(
            pos,
            ImVec2(panel_max.x, pos.y + 2.0f),
            IM_COL32(142, 158, 255, 255),
            palette.accent,
            palette.accent,
            IM_COL32(20, 42, 128, 255));
        draw_list->AddText(ImVec2(pos.x + pad.x, pos.y + pad.y), IM_COL32(204, 217, 255, 255), text);
    }

    void ensure_snapshot()
    {
        if (snapshot.load(std::memory_order_acquire))
        {
            return;
        }

        auto empty_snapshot = std::make_shared<MediaSnapshot>();
        std::shared_ptr<const MediaSnapshot> readonly_snapshot = std::move(empty_snapshot);
        snapshot.store(readonly_snapshot, std::memory_order_release);
    }
}

bool media_widget::start()
{
    media_widget_detail::ensure_snapshot();
    if (media_widget_detail::worker.joinable())
    {
        return false;
    }

    media_widget_detail::worker = std::jthread([](std::stop_token stop_token)
    {
        media_widget_detail::worker_loop(stop_token);
    });
    return true;
}

void media_widget::stop() noexcept
{
    if (!media_widget_detail::worker.joinable())
    {
        return;
    }

    media_widget_detail::worker.request_stop();
    media_widget_detail::worker.join();
}

void media_widget::release() noexcept
{
    media_widget_detail::release_textures();
}

void media_widget::set_enabled(const bool enabled) noexcept
{
    media_widget_detail::widget_enabled.store(enabled, std::memory_order_release);
}

bool media_widget::is_enabled() noexcept
{
    return media_widget_detail::widget_enabled.load(std::memory_order_acquire);
}

void media_widget::render(ID3D11Device* device)
{
    if (!is_enabled())
    {
        return;
    }

    media_widget_detail::ensure_snapshot();

    ImGuiIO& io = ImGui::GetIO();
    const float current_fps = io.DeltaTime > 0.0f ? 1.0f / io.DeltaTime : 0.0f;
    media_widget_detail::smoothed_fps = media_widget_detail::smoothed_fps <= 0.0f
        ? current_fps
        : (media_widget_detail::smoothed_fps * 0.90f) + (current_fps * 0.10f);

    const auto current_snapshot = media_widget_detail::snapshot.load(std::memory_order_acquire);
    if (current_snapshot && current_snapshot->has_media)
    {
        media_widget_detail::update_cover_texture(device, current_snapshot->media);
        media_widget_detail::draw_media(current_snapshot->media);
        return;
    }

    media_widget_detail::draw_fallback();
}

std::shared_ptr<const media_widget::MediaSnapshot> media_widget::get_snapshot() noexcept
{
    media_widget_detail::ensure_snapshot();
    return media_widget_detail::snapshot.load(std::memory_order_acquire);
}

ImU32 media_widget::get_accent_color() noexcept
{
    constexpr ImU32 fallback_color = IM_COL32(184, 184, 186, 245);
    const ImU32 target_color = media_widget_detail::has_palette ? media_widget_detail::palette.accent : fallback_color;
    if (!media_widget_detail::transition_active)
    {
        return target_color;
    }

    return media_widget_detail::lerp_color(
        media_widget_detail::start_palette.accent,
        target_color,
        media_widget_detail::get_transition_amount());
}

ImU32 media_widget::get_secondary_color() noexcept
{
    constexpr ImU32 fallback_color = IM_COL32(244, 114, 182, 245);
    const ImU32 target_color = media_widget_detail::has_palette
        ? media_widget_detail::palette.accent_secondary
        : fallback_color;
    if (!media_widget_detail::transition_active)
    {
        return target_color;
    }

    return media_widget_detail::lerp_color(
        media_widget_detail::start_palette.accent_secondary,
        target_color,
        media_widget_detail::get_transition_amount());
}

ImU32 media_widget::get_accent_color_faded() noexcept
{
    return media_widget_detail::with_alpha(get_accent_color(), 0.65f);
}
