#include "clawd_companion.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <utility>
#include <vector>

#include <Windows.h>
#include <d3d11.h>
#include <imgui.h>

#include "../../vendor/stb/stb_image.h"

namespace clawd_companion_detail
{
    constexpr const char* clawd_file_name = "clawd-mini-idle.gif";
    constexpr int min_frame_delay_ms = 35;
    constexpr int max_frame_delay_ms = 250;

    struct DecodedGifFrame
    {
        std::vector<unsigned char> pixels{};
        int delay_ms = 80;
    };

    struct DecodedGif
    {
        std::vector<DecodedGifFrame> frames{};
        int width = 0;
        int height = 0;
    };

    struct GifFrame
    {
        ID3D11Texture2D* texture = nullptr;
        ID3D11ShaderResourceView* view = nullptr;
        int delay_ms = 80;
    };

    std::vector<GifFrame> gif_frames{};
    int gif_width = 0;
    int gif_height = 0;
    bool gif_failed = false;
    double animation_start = 0.0;
    ID3D11Device* loaded_device = nullptr;

    [[nodiscard]] bool is_visible_pixel(const unsigned char* pixel) noexcept
    {
        return pixel && pixel[3] > 25;
    }

    void release_frame(GifFrame& frame) noexcept
    {
        if (frame.view)
        {
            frame.view->Release();
            frame.view = nullptr;
        }

        if (frame.texture)
        {
            frame.texture->Release();
            frame.texture = nullptr;
        }
    }

    void release_frames(bool reset_failure) noexcept
    {
        for (GifFrame& frame : gif_frames)
        {
            release_frame(frame);
        }

        gif_frames.clear();
        gif_width = 0;
        gif_height = 0;
        animation_start = 0.0;
        loaded_device = nullptr;

        if (reset_failure)
        {
            gif_failed = false;
        }
    }

    [[nodiscard]] std::filesystem::path module_directory()
    {
        char module_path[MAX_PATH]{};
        HMODULE module_handle = nullptr;

        if (!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(&clawd_companion::render),
            &module_handle))
        {
            return {};
        }

        const DWORD path_length = GetModuleFileNameA(module_handle, module_path, MAX_PATH);
        if (path_length == 0 || path_length >= MAX_PATH)
        {
            return {};
        }

        return std::filesystem::path(module_path).parent_path();
    }

    [[nodiscard]] std::filesystem::path find_gif_path()
    {
        std::vector<std::filesystem::path> candidates{};

        std::filesystem::path parent_path = module_directory();
        for (int depth = 0; depth < 4 && !parent_path.empty(); ++depth)
        {
            candidates.push_back(parent_path / "assets" / "gif" / clawd_file_name);
            parent_path = parent_path.parent_path();
        }

        try
        {
            candidates.push_back(std::filesystem::current_path() / "assets" / "gif" / clawd_file_name);
        }
        catch (...)
        {
        }

        for (const std::filesystem::path& candidate : candidates)
        {
            std::error_code error{};
            if (std::filesystem::exists(candidate, error) && !error)
            {
                return candidate;
            }
        }

        return {};
    }

    [[nodiscard]] bool decode_gif_file(const std::filesystem::path& gif_path, DecodedGif& output)
    {
        std::ifstream file(gif_path, std::ios::binary);
        if (!file)
        {
            return false;
        }

        const std::vector<unsigned char> bytes{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };

        if (bytes.empty())
        {
            return false;
        }

        int* raw_delays = nullptr;
        int width = 0;
        int height = 0;
        int frame_count = 0;
        int component_count = 0;
        unsigned char* raw_pixels = stbi_load_gif_from_memory(
            bytes.data(),
            static_cast<int>(bytes.size()),
            &raw_delays,
            &width,
            &height,
            &frame_count,
            &component_count,
            4
        );

        if (!raw_pixels || width <= 0 || height <= 0 || frame_count <= 0)
        {
            if (raw_pixels)
            {
                stbi_image_free(raw_pixels);
            }

            if (raw_delays)
            {
                stbi_image_free(raw_delays);
            }

            return false;
        }

        const int first_frame = frame_count > 1 ? 1 : 0;
        int left = width;
        int top = height;
        int right = -1;
        int bottom = -1;
        const std::size_t frame_pixels = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);

        for (int frame = first_frame; frame < frame_count; ++frame)
        {
            const std::size_t frame_base = static_cast<std::size_t>(frame) * frame_pixels * 4ull;
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    const std::size_t pixel_index = frame_base +
                        ((static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)) * 4ull);

                    if (!is_visible_pixel(raw_pixels + pixel_index))
                    {
                        continue;
                    }

                    left = (std::min)(left, x);
                    top = (std::min)(top, y);
                    right = (std::max)(right, x);
                    bottom = (std::max)(bottom, y);
                }
            }
        }

        if (right < left || bottom < top)
        {
            left = 0;
            top = 0;
            right = width - 1;
            bottom = height - 1;
        }

        constexpr int crop_padding = 2;
        left = (std::max)(0, left - crop_padding);
        top = (std::max)(0, top - crop_padding);
        right = (std::min)(width - 1, right + crop_padding);
        bottom = (std::min)(height - 1, bottom + crop_padding);

        output = {};
        output.width = right - left + 1;
        output.height = bottom - top + 1;
        output.frames.reserve(static_cast<std::size_t>((std::max)(1, frame_count - first_frame)));

        for (int frame = first_frame; frame < frame_count; ++frame)
        {
            DecodedGifFrame decoded_frame{};
            const int raw_delay = raw_delays ? raw_delays[frame] : 80;
            decoded_frame.delay_ms = std::clamp(raw_delay, min_frame_delay_ms, max_frame_delay_ms);
            decoded_frame.pixels.resize(
                static_cast<std::size_t>(output.width) *
                static_cast<std::size_t>(output.height) *
                4ull
            );

            const std::size_t frame_base = static_cast<std::size_t>(frame) * frame_pixels * 4ull;
            for (int y = 0; y < output.height; ++y)
            {
                const int source_y = top + y;
                for (int x = 0; x < output.width; ++x)
                {
                    const int source_x = left + x;
                    const std::size_t source_index = frame_base +
                        ((static_cast<std::size_t>(source_y) * static_cast<std::size_t>(width) +
                            static_cast<std::size_t>(source_x)) * 4ull);
                    const std::size_t target_index =
                        (static_cast<std::size_t>(y) * static_cast<std::size_t>(output.width) +
                            static_cast<std::size_t>(x)) * 4ull;

                    decoded_frame.pixels[target_index + 0] = raw_pixels[source_index + 0];
                    decoded_frame.pixels[target_index + 1] = raw_pixels[source_index + 1];
                    decoded_frame.pixels[target_index + 2] = raw_pixels[source_index + 2];
                    decoded_frame.pixels[target_index + 3] = raw_pixels[source_index + 3];
                }
            }

            output.frames.push_back(std::move(decoded_frame));
        }

        stbi_image_free(raw_pixels);

        if (raw_delays)
        {
            stbi_image_free(raw_delays);
        }

        return !output.frames.empty();
    }

    [[nodiscard]] bool create_texture_from_rgba_pixels(
        ID3D11Device* device,
        const std::vector<unsigned char>& pixels,
        int width,
        int height,
        ID3D11Texture2D** texture,
        ID3D11ShaderResourceView** view
    )
    {
        if (!device || pixels.empty() || width <= 0 || height <= 0 || !texture || !view)
        {
            return false;
        }

        D3D11_TEXTURE2D_DESC texture_desc{};
        texture_desc.Width = static_cast<UINT>(width);
        texture_desc.Height = static_cast<UINT>(height);
        texture_desc.MipLevels = 1;
        texture_desc.ArraySize = 1;
        texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texture_desc.SampleDesc.Count = 1;
        texture_desc.Usage = D3D11_USAGE_DEFAULT;
        texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initial_data{};
        initial_data.pSysMem = pixels.data();
        initial_data.SysMemPitch = static_cast<UINT>(width * 4);

        if (FAILED(device->CreateTexture2D(&texture_desc, &initial_data, texture)))
        {
            return false;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC view_desc{};
        view_desc.Format = texture_desc.Format;
        view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        view_desc.Texture2D.MipLevels = 1;

        if (FAILED(device->CreateShaderResourceView(*texture, &view_desc, view)))
        {
            (*texture)->Release();
            *texture = nullptr;
            return false;
        }

        return true;
    }

    [[nodiscard]] bool load_frames(ID3D11Device* device)
    {
        if (!device)
        {
            return false;
        }

        if (!gif_frames.empty() && loaded_device == device)
        {
            return true;
        }

        if (!gif_frames.empty())
        {
            release_frames(false);
        }

        if (gif_failed)
        {
            return false;
        }

        DecodedGif decoded_gif{};
        const std::filesystem::path gif_path = find_gif_path();
        if (gif_path.empty() || !decode_gif_file(gif_path, decoded_gif))
        {
            gif_failed = true;
            return false;
        }

        gif_width = decoded_gif.width;
        gif_height = decoded_gif.height;
        gif_frames.reserve(decoded_gif.frames.size());

        for (const DecodedGifFrame& decoded_frame : decoded_gif.frames)
        {
            GifFrame frame{};
            frame.delay_ms = decoded_frame.delay_ms;

            if (!create_texture_from_rgba_pixels(
                device,
                decoded_frame.pixels,
                decoded_gif.width,
                decoded_gif.height,
                &frame.texture,
                &frame.view))
            {
                release_frame(frame);
                release_frames(false);
                gif_failed = true;
                return false;
            }

            gif_frames.push_back(frame);
        }

        loaded_device = device;
        animation_start = ImGui::GetTime();
        return !gif_frames.empty();
    }

    [[nodiscard]] std::size_t current_frame_index()
    {
        int total_delay = 0;
        for (const GifFrame& frame : gif_frames)
        {
            total_delay += (std::max)(1, frame.delay_ms);
        }

        if (total_delay <= 0)
        {
            return 0;
        }

        const int elapsed_ms = static_cast<int>((ImGui::GetTime() - animation_start) * 1000.0) % total_delay;
        int cursor_ms = 0;

        for (std::size_t index = 0; index < gif_frames.size(); ++index)
        {
            cursor_ms += (std::max)(1, gif_frames[index].delay_ms);
            if (elapsed_ms < cursor_ms)
            {
                return index;
            }
        }

        return gif_frames.empty() ? 0 : gif_frames.size() - 1;
    }

    void draw_badge(ImDrawList* draw_list, const ImVec2& max, float scale)
    {
        const ImVec2 badge_size(42.0f * scale, 17.0f * scale);
        const ImVec2 badge_min(max.x - badge_size.x, max.y - 2.0f * scale);
        const ImVec2 badge_max(max.x, max.y + badge_size.y - 2.0f * scale);

        draw_list->AddRectFilled(badge_min, badge_max, IM_COL32(10, 11, 12, 170), 4.0f * scale);
        draw_list->AddRect(badge_min, badge_max, IM_COL32(112, 126, 160, 82), 4.0f * scale, 0, 1.0f);
        draw_list->AddLine(
            ImVec2(badge_min.x + 7.0f * scale, badge_min.y + 1.0f * scale),
            ImVec2(badge_min.x + 22.0f * scale, badge_min.y + 1.0f * scale),
            IM_COL32(154, 178, 246, 150),
            1.0f
        );
        draw_list->AddText(ImVec2(badge_min.x + 9.0f * scale, badge_min.y + 2.0f * scale),
            IM_COL32(232, 232, 230, 220), "veil");
    }

    void draw_image_companion(ImDrawList* draw_list, const ImVec2& display_size, float scale)
    {
        if (gif_frames.empty() || gif_width <= 0 || gif_height <= 0)
        {
            return;
        }

        const GifFrame& frame = gif_frames[current_frame_index()];
        if (!frame.view)
        {
            return;
        }

        const float margin = 16.0f * scale;
        const float sprite_height = 58.0f * scale;
        const float sprite_width = sprite_height * (static_cast<float>(gif_width) / static_cast<float>(gif_height));
        const ImVec2 sprite_max(display_size.x - margin, margin + sprite_height);
        const ImVec2 sprite_min(sprite_max.x - sprite_width, margin);

        draw_list->PushClipRectFullScreen();
        draw_list->AddRectFilled(
            ImVec2(sprite_min.x + sprite_width * 0.20f, sprite_max.y - 8.0f * scale),
            ImVec2(sprite_max.x - 5.0f * scale, sprite_max.y - 2.0f * scale),
            IM_COL32(0, 0, 0, 72),
            4.0f * scale
        );
        draw_list->AddImage(
            reinterpret_cast<ImTextureID>(frame.view),
            sprite_min,
            sprite_max,
            ImVec2(0.0f, 0.0f),
            ImVec2(1.0f, 1.0f),
            IM_COL32(255, 255, 255, 245)
        );
        draw_badge(draw_list, ImVec2(sprite_max.x - 3.0f * scale, sprite_max.y - 3.0f * scale), scale);
        draw_list->PopClipRect();
    }

    void draw_fallback_companion(ImDrawList* draw_list, const ImVec2& display_size, float scale)
    {
        const float margin = 16.0f * scale;
        const ImVec2 size(96.0f * scale, 42.0f * scale);
        const ImVec2 panel_min(display_size.x - margin - size.x, margin);
        const ImVec2 panel_max(display_size.x - margin, margin + size.y);
        const ImVec2 face_center(panel_min.x + 24.0f * scale, panel_min.y + 21.0f * scale);

        draw_list->PushClipRectFullScreen();
        draw_list->AddRectFilled(panel_min, panel_max, IM_COL32(10, 11, 12, 180), 5.0f * scale);
        draw_list->AddRect(panel_min, panel_max, IM_COL32(112, 126, 160, 90), 5.0f * scale, 0, 1.0f);
        draw_list->AddLine(
            ImVec2(panel_min.x + 8.0f * scale, panel_min.y + 1.0f * scale),
            ImVec2(panel_min.x + 34.0f * scale, panel_min.y + 1.0f * scale),
            IM_COL32(154, 178, 246, 150),
            1.0f
        );

        draw_list->AddCircle(face_center, 12.0f * scale, IM_COL32(232, 232, 230, 190), 28, 1.2f * scale);
        draw_list->AddCircleFilled(ImVec2(face_center.x - 4.0f * scale, face_center.y - 1.0f * scale),
            1.8f * scale, IM_COL32(232, 232, 230, 220), 12);
        draw_list->AddCircleFilled(ImVec2(face_center.x + 4.0f * scale, face_center.y - 1.0f * scale),
            1.8f * scale, IM_COL32(232, 232, 230, 220), 12);
        draw_list->AddLine(ImVec2(face_center.x - 4.0f * scale, face_center.y + 5.0f * scale),
            ImVec2(face_center.x + 4.0f * scale, face_center.y + 5.0f * scale),
            IM_COL32(244, 114, 182, 165), 1.1f * scale);
        draw_list->AddText(ImVec2(panel_min.x + 46.0f * scale, panel_min.y + 12.0f * scale),
            IM_COL32(232, 232, 230, 220), "veil");
        draw_list->PopClipRect();
    }
}

void clawd_companion::render(ID3D11Device* device)
{
    const ImGuiIO& io = ImGui::GetIO();
    if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
    {
        return;
    }

    const float scale = std::clamp(io.DisplaySize.y / 1080.0f, 0.75f, 1.35f);
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();

    if (clawd_companion_detail::load_frames(device))
    {
        clawd_companion_detail::draw_image_companion(draw_list, io.DisplaySize, scale);
        return;
    }

    clawd_companion_detail::draw_fallback_companion(draw_list, io.DisplaySize, scale);
}

void clawd_companion::release() noexcept
{
    clawd_companion_detail::release_frames(true);
}
