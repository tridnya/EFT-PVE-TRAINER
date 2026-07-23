#include "dxhook.h"

#include <array>
#include <cstdint>
#include <ios>
#include <iostream>

#include "../media/media_widget.h"
#include "../menu/menu.h"
#include "../../external/imgui/imgui_impl_dx12.h"
#include <Windows.h>

#include "../external/detours/detours.h"

#include "../../util/logger.h"

// method based on kiero hook

namespace dxhook_state
{
    inline bool initDone = false;
    inline LRESULT (*oWndProc)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    inline void (*oExecuteCommandLists)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);
    inline HRESULT (*oPresent11)(void* pSwapChain, UINT SyncInterval, UINT Flags);
    inline HRESULT (*oPresent12)(void* pSwapChain, UINT SyncInterval, UINT Flags);

    int PRESENT_INDEX_11 = 8;
    int PRESENT_INDEX_12 = 140;
    int LIST_INDEX = 54;

    ID3D11Device* p_device11 = nullptr;
    ID3D11DeviceContext* p_context11 = nullptr;
    ID3D11RenderTargetView* main_render_target_view11 = nullptr;

    ID3D12Device* p_device12 = nullptr;
    ID3D12DescriptorHeap* p_descriptor_heap = nullptr;
    ID3D12DescriptorHeap* g_pd3d_rtv_desc_heap = nullptr;
    ID3D12DescriptorHeap* g_pd3d_srv_desc_heap = nullptr;
    ID3D12GraphicsCommandList* p_command_list = nullptr;
    ID3D12Resource** back_buffers = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE* descriptor_handles = nullptr;
    ID3D12CommandAllocator* p_allocator = nullptr;
    ID3D12CommandQueue* p_command_queue = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle;
    UINT bufferCount = 0;

    bool is_dx12 = false;
    UINT64* g_methods_table = nullptr;
    HWND imgui_window = nullptr;

    inline constexpr UINT srv_descriptor_capacity = 64;
    std::array<bool, srv_descriptor_capacity> srv_descriptor_used{};
    UINT srv_descriptor_increment = 0;
    ID3D12CommandQueue* p_imgui_texture_command_queue = nullptr;
    bool owns_imgui_texture_command_queue = false;

    void reset_srv_descriptor_allocator()
    {
        srv_descriptor_used.fill(false);
        srv_descriptor_increment = 0;
    }

    void allocate_srv_descriptor(
        ImGui_ImplDX12_InitInfo* info,
        D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle,
        D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
    {
        if (!info || !info->SrvDescriptorHeap || !out_cpu_desc_handle || !out_gpu_desc_handle)
        {
            return;
        }

        if (srv_descriptor_increment == 0)
        {
            srv_descriptor_increment = p_device12->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }

        UINT descriptor_index = 0;
        for (; descriptor_index < srv_descriptor_capacity; ++descriptor_index)
        {
            if (!srv_descriptor_used[descriptor_index])
            {
                srv_descriptor_used[descriptor_index] = true;
                break;
            }
        }

        if (descriptor_index >= srv_descriptor_capacity)
        {
            descriptor_index = srv_descriptor_capacity - 1;
            logger::warn("[DX12] ImGui SRV descriptor heap exhausted");
        }

        *out_cpu_desc_handle = info->SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        *out_gpu_desc_handle = info->SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
        out_cpu_desc_handle->ptr += static_cast<SIZE_T>(descriptor_index) * srv_descriptor_increment;
        out_gpu_desc_handle->ptr += static_cast<UINT64>(descriptor_index) * srv_descriptor_increment;
    }

    void free_srv_descriptor(
        ImGui_ImplDX12_InitInfo* info,
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle,
        D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle)
    {
        (void)gpu_desc_handle;

        if (!info || !info->SrvDescriptorHeap || srv_descriptor_increment == 0)
        {
            return;
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE heap_start =
            info->SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        if (cpu_desc_handle.ptr < heap_start.ptr)
        {
            return;
        }

        const SIZE_T byte_offset = cpu_desc_handle.ptr - heap_start.ptr;
        const UINT descriptor_index = static_cast<UINT>(byte_offset / srv_descriptor_increment);
        if (descriptor_index < srv_descriptor_capacity)
        {
            srv_descriptor_used[descriptor_index] = false;
        }
    }
}

using namespace dxhook_state;

// yeah imgui is missing this for some reason
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI dxhook::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!menu::is_active())
    {
        oWndProc(hWnd, msg, wParam, lParam);
    }
    ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
    return true;
}

HWND dxhook::create_window(const HMODULE this_module)
{
    WNDCLASSEX window_class;
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = DefWindowProc;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = this_module;
    window_class.hIcon = nullptr;
    window_class.hCursor = nullptr;
    window_class.hbrBackground = nullptr;
    window_class.lpszMenuName = nullptr;
    window_class.lpszClassName = L"DXWindow";
    window_class.hIconSm = nullptr;
    RegisterClassEx(&window_class);
    const HWND window = CreateWindowA("DXWindow",
                                      "DXWindow",
                                      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                      0, 0,
                                      100, 100,
                                      nullptr,
                                      nullptr,
                                      this_module,
                                      nullptr);

    UnregisterClass(L"DXWindow", this_module);
    return window;
}

bool dxhook::init(const HMODULE this_module)
{
    return dx12_setup(this_module) || dx11_setup(this_module);
}


bool dxhook::dx12_setup(const HMODULE this_module)
{
    // dx 12 setup
    // get all needed instances and api
    HMODULE lib_dxgi;
    HMODULE lib_d_3d12;
    if ((lib_dxgi = ::GetModuleHandle(L"dxgi.dll")) == nullptr || (lib_d_3d12 = ::GetModuleHandle(L"d3d12.dll")) ==
        nullptr)
    {
        return false;
    }

    void* create_dxgi_factory;
    if ((create_dxgi_factory = GetProcAddress(lib_dxgi, "CreateDXGIFactory")) == nullptr) // NOLINT(clang-diagnostic-microsoft-cast)
    {
        return false;
    }

    IDXGIFactory* factory;
    if (reinterpret_cast<long(__stdcall*)(const IID&, void**)>(create_dxgi_factory)(
        __uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory)) < 0) // NOLINT(clang-diagnostic-language-extension-token)
    {
        logger::error("FACTORY FAIL");
        return false;
    }

    IDXGIAdapter* adapter;
    if (factory->EnumAdapters(0, &adapter) == DXGI_ERROR_NOT_FOUND)
    {
        logger::error("ADAPTER FAIL");
        return false;
    }

    void* d_3d12_create_device;
    if ((d_3d12_create_device = GetProcAddress(lib_d_3d12, "D3D12CreateDevice")) == nullptr) // NOLINT(clang-diagnostic-microsoft-cast)
    {
        return false;
    }

    ID3D12Device* device;
    if (reinterpret_cast<long(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, const IID&, void**)>(d_3d12_create_device)(
        adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), reinterpret_cast<void**>(&device)) < 0) // NOLINT(clang-diagnostic-language-extension-token)
    {
        logger::error("DEVICE FAIL");
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC queue_desc;
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queue_desc.Priority = 0;
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.NodeMask = 0;

    ID3D12CommandQueue* command_queue;
    if (device->CreateCommandQueue(&queue_desc, __uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(&command_queue)) < // NOLINT(clang-diagnostic-language-extension-token)
        0)
    {
        logger::error("COMMAND QUEUE FAIL");
        return false;
    }

    ID3D12CommandAllocator* command_allocator;
    if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), // NOLINT(clang-diagnostic-language-extension-token)
                                       reinterpret_cast<void**>(&command_allocator)) < 0)
    {
        logger::error("COMMAND ALLOCATOR FAIL");
        return false;
    }

    ID3D12GraphicsCommandList* command_list;
    if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator, nullptr,
                                  __uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&command_list)) < 0) // NOLINT(clang-diagnostic-language-extension-token)
    {
        logger::error("COMMAND LIST FAIL");
        return false;
    }

    const HWND window = create_window(this_module);
    if (!window)
    {
        logger::error("WINDOW FAIL");
        std::cout << std::hex << GetLastError() << "\n";
        return false;
    }

    // set up our swapchain
    DXGI_RATIONAL refresh_rate;
    refresh_rate.Numerator = 60;
    refresh_rate.Denominator = 1;

    DXGI_MODE_DESC buffer_desc;
    buffer_desc.Width = 100;
    buffer_desc.Height = 100;
    buffer_desc.RefreshRate = refresh_rate;
    buffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    buffer_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    buffer_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    DXGI_SAMPLE_DESC sample_desc;
    sample_desc.Count = 1;
    sample_desc.Quality = 0;

    DXGI_SWAP_CHAIN_DESC swap_chain_desc;
    swap_chain_desc.BufferDesc = buffer_desc;
    swap_chain_desc.SampleDesc = sample_desc;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.OutputWindow = window;
    swap_chain_desc.Windowed = 1;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    IDXGISwapChain* swap_chain;
    if (factory->CreateSwapChain(command_queue, &swap_chain_desc, &swap_chain) < 0)
    {
        logger::error("SWAPCHAIN FAIL");
        DestroyWindow(window);
        return false;
    }

    // yoink the method table
    g_methods_table = static_cast<UINT64*>(calloc(150, sizeof(UINT64)));
    memcpy(g_methods_table, *reinterpret_cast<UINT64**>(device), 44 * sizeof(UINT64));
    memcpy(g_methods_table + 44, *reinterpret_cast<UINT64**>(command_queue), 19 * sizeof(UINT64));
    memcpy(g_methods_table + 44 + 19, *reinterpret_cast<UINT64**>(command_allocator), 9 * sizeof(UINT64));
    memcpy(g_methods_table + 44 + 19 + 9, *reinterpret_cast<UINT64**>(command_list), 60 * sizeof(UINT64));
    memcpy(g_methods_table + 44 + 19 + 9 + 60, *reinterpret_cast<UINT64**>(swap_chain), 18 * sizeof(UINT64));

    // we dont need this stuff now
    device->Release();
    device = nullptr;

    command_queue->Release();
    command_queue = nullptr;

    command_allocator->Release();
    command_allocator = nullptr;

    command_list->Release();
    command_list = nullptr;

    swap_chain->Release();
    swap_chain = nullptr;

    DestroyWindow(window);

    auto target_present = reinterpret_cast<PVOID*>(g_methods_table[PRESENT_INDEX_12]); // NOLINT(performance-no-int-to-ptr)
    auto target_execute_command_lists = reinterpret_cast<PVOID*>(g_methods_table[LIST_INDEX]); // NOLINT(performance-no-int-to-ptr)

    oPresent12 = reinterpret_cast<HRESULT (*)(void*, UINT, UINT)>(target_present);
    oExecuteCommandLists = reinterpret_cast<void (*)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*)>(
        target_execute_command_lists);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // ReSharper disable once CppCStyleCast
    DetourAttach(&(PVOID&)(oPresent12), reinterpret_cast<PVOID>(dxPresent12));
    // ReSharper disable once CppCStyleCast
    DetourAttach(&(PVOID&)(oExecuteCommandLists), reinterpret_cast<PVOID>(executeCommandLists12));

    if (DetourTransactionCommit() != NO_ERROR)
    {
        logger::error("DETOUR FAIL");
        free(g_methods_table);
        g_methods_table = nullptr;
        return false;
    }

    // init done
    return true;
}

bool dxhook::dx11_setup(const HMODULE this_module)
{
    // dx 11 setup
    // get all needed instances and api
    HMODULE lib_d_3d11;
    if ((lib_d_3d11 = ::GetModuleHandle(L"d3d11.dll")) == nullptr)
    {
        return false;
    }

    void* d_3d11_create_device;
    if ((d_3d11_create_device = reinterpret_cast<void*>(GetProcAddress(
        lib_d_3d11, "D3D11CreateDevice"))) == nullptr)
    {
        return false;
    }

    // create our device
    constexpr D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
    };

    ID3D11Device* device11 = nullptr;
    ID3D11DeviceContext* context11 = nullptr;

    D3D_FEATURE_LEVEL feature_level;
    const HRESULT device_res = reinterpret_cast<HRESULT(*)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT,
                                                           CONST D3D_FEATURE_LEVEL*, UINT, UINT, ID3D11Device**,
                                                           D3D_FEATURE_LEVEL*,
                                                           ID3D11DeviceContext**)>(d_3d11_create_device)(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        feature_levels,
        2,
        D3D11_SDK_VERSION,
        &device11,
        &feature_level,
        &context11);

    if (device_res < 0)
    {
        logger::error("DEVICE FAIL");
        std::cout << std::hex << device_res << "\n";
        return false;
    }

    IDXGIDevice* dxgi_device;
    const HRESULT res_dxgi_device = device11->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<PVOID*>(&dxgi_device)); // NOLINT(clang-diagnostic-language-extension-token)
    if (res_dxgi_device < 0)
    {
        logger::error("DXGI DEVICE FAIL");
        std::cout << std::hex << res_dxgi_device << "\n";
        return false;
    }

    IDXGIAdapter* dxgi_adapter;
    const HRESULT res_adapter = dxgi_device->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<PVOID*>(&dxgi_adapter)); // NOLINT(clang-diagnostic-language-extension-token)
    if (res_adapter < 0)
    {
        logger::error("ADAPTER FAIL");
        std::cout << std::hex << res_adapter << "\n";
        return false;
    }

    IDXGIFactory2* factory;
    const HRESULT res_factory = dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<PVOID*>(&factory)); // NOLINT(clang-diagnostic-language-extension-token)
    if (res_factory < 0)
    {
        logger::error("FACTORY FAIL");
        std::cout << std::hex << res_factory << "\n";
        return false;
    }


    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1 = {};
    swap_chain_desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swap_chain_desc1.BufferCount = 2;
    swap_chain_desc1.Width = 100;
    swap_chain_desc1.Height = 100;
    swap_chain_desc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc1.SampleDesc.Count = 1;
    swap_chain_desc1.SampleDesc.Quality = 0;

    const HWND window = create_window(this_module);
    if (!window)
    {
        logger::error("WINDOW FAIL");
        std::cout << std::hex << GetLastError() << "\n";
        return false;
    }

    IDXGISwapChain1* swap_chain11;
    const HRESULT res_swapchain = factory->CreateSwapChainForHwnd(device11, window, &swap_chain_desc1, nullptr, nullptr,
                                                                  &swap_chain11);
    if (res_swapchain < 0)
    {
        logger::error("SWAPCHAIN FAIL");
        std::cout << std::hex << res_swapchain << "\n";
        DestroyWindow(window);
        return false;
    }


    // yoink the method table
    g_methods_table = static_cast<UINT64*>(calloc(205, sizeof(UINT64)));
    memcpy(g_methods_table, *reinterpret_cast<UINT64**>(swap_chain11), 18 * sizeof(UINT64));
    memcpy(g_methods_table + 18, *reinterpret_cast<UINT64**>(device11), 43 * sizeof(UINT64));
    memcpy(g_methods_table + 18 + 43, *reinterpret_cast<UINT64**>(context11), 144 * sizeof(UINT64));

    // we dont need this stuff now
    swap_chain11->Release();
    swap_chain11 = nullptr;

    device11->Release();
    device11 = nullptr;

    context11->Release();
    context11 = nullptr;

    DestroyWindow(window);

    // hook present func
    const auto target = reinterpret_cast<PVOID*>(g_methods_table[PRESENT_INDEX_11]); // NOLINT(performance-no-int-to-ptr)
    oPresent11 = reinterpret_cast<HRESULT (*)(void*, UINT, UINT)>(target);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // ReSharper disable once CppCStyleCast
    DetourAttach(&(PVOID&)(oPresent11), reinterpret_cast<PVOID>(dxPresent11));

    if (DetourTransactionCommit() != NO_ERROR)
    {
        logger::error("DETOUR FAIL");
        free(g_methods_table);
        g_methods_table = nullptr;
        return false;
    }

    // init done
    return true;
}

void dxhook::shutdown()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // ReSharper disable once CppCStyleCast
    DetourDetach(&(PVOID&)(oPresent11), reinterpret_cast<PVOID>(dxPresent11));
    // ReSharper disable once CppCStyleCast
    DetourDetach(&(PVOID&)(oPresent12), reinterpret_cast<PVOID>(dxPresent12));
    // ReSharper disable once CppCStyleCast
    DetourDetach(&(PVOID&)(oExecuteCommandLists), reinterpret_cast<PVOID>(executeCommandLists12));

    DetourTransactionCommit();
    free(g_methods_table);
    g_methods_table = nullptr;
}

void __fastcall dxhook::executeCommandLists12(ID3D12CommandQueue* p_command_queue1, UINT num_command_lists,
                                              ID3D12CommandList* const * pp_command_lists)
{
    if (!p_command_queue)
    {
        p_command_queue = p_command_queue1;
    }
    oExecuteCommandLists(p_command_queue1, num_command_lists, pp_command_lists);
}

HRESULT __fastcall dxhook::dxPresent11(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags)
{
    // init imgui first time
    if (!initDone)
    {
        if (SUCCEEDED(p_swap_chain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&p_device11)))) // NOLINT(clang-diagnostic-language-extension-token)
        {
            is_dx12 = false;
            p_device11->GetImmediateContext(&p_context11);
            DXGI_SWAP_CHAIN_DESC sd;
            p_swap_chain->GetDesc(&sd);
            imgui_window = sd.OutputWindow;
            ID3D11Texture2D* pBackBuffer;
            p_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer)); // NOLINT(clang-diagnostic-language-extension-token)
            p_device11->CreateRenderTargetView(pBackBuffer, nullptr, &main_render_target_view11);
            pBackBuffer->Release();
            oWndProc = reinterpret_cast<LRESULT(*)(HWND, UINT, WPARAM, LPARAM)>(SetWindowLongPtr( // NOLINT(performance-no-int-to-ptr)
                imgui_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));

            // Init Imgui
            init_imgui();

            initDone = true;
        }
        else
        {
            is_dx12 = true;
        }
    }

    // draw imgui ui
    if (!is_dx12) draw_imgui(nullptr);

    return oPresent11(p_swap_chain, sync_interval, flags);
}

HRESULT __fastcall dxhook::dxPresent12(IDXGISwapChain3* p_swap_chain, UINT sync_interval, UINT flags)
{
    // init imgui first time
    if (!initDone)
    {
        if (SUCCEEDED(p_swap_chain->GetDevice(__uuidof(ID3D12Device), reinterpret_cast<void**>(&p_device12)))) // NOLINT(clang-diagnostic-language-extension-token)
        {
            is_dx12 = true;
            DXGI_SWAP_CHAIN_DESC sd;
            p_swap_chain->GetDesc(&sd);
            imgui_window = sd.OutputWindow;

            DXGI_SWAP_CHAIN_DESC swapChainDesc;
            bufferCount = p_swap_chain->GetDesc(&swapChainDesc);
            bufferCount = swapChainDesc.BufferCount;

            p_device12->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&p_allocator));
            p_device12->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, p_allocator, nullptr,
                                          IID_PPV_ARGS(&p_command_list));
            p_command_list->Close();

            p_device12->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            D3D12_DESCRIPTOR_HEAP_DESC desc;
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = bufferCount;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            desc.NodeMask = 1;
            p_device12->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3d_rtv_desc_heap));
            SIZE_T rtvDescriptorSize = p_device12->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            rtv_handle = g_pd3d_rtv_desc_heap->GetCPUDescriptorHandleForHeapStart();
            back_buffers = new ID3D12Resource*[bufferCount];
            descriptor_handles = new D3D12_CPU_DESCRIPTOR_HANDLE[bufferCount];
            for (UINT i = 0; i < bufferCount; i++)
            {
                ID3D12Resource* pBackBuffer = nullptr;
                p_swap_chain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
                p_device12->CreateRenderTargetView(pBackBuffer, nullptr, rtv_handle);
                back_buffers[i] = pBackBuffer;
                descriptor_handles[i] = rtv_handle;
                rtv_handle.ptr += rtvDescriptorSize;
            }

            oWndProc = reinterpret_cast<LRESULT(*)(HWND, UINT, WPARAM, LPARAM)>(SetWindowLongPtr( // NOLINT(performance-no-int-to-ptr)
                imgui_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));

            // Init Imgui
            init_imgui();

            initDone = true;
        }
        else
        {
            is_dx12 = false;
        }
    }

    // draw imgui ui
    if (is_dx12) draw_imgui(p_swap_chain);

    return oPresent12(p_swap_chain, sync_interval, flags);
}

void dxhook::init_imgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplWin32_Init(imgui_window);
    if (is_dx12)
    {
        reset_srv_descriptor_allocator();

        D3D12_DESCRIPTOR_HEAP_DESC descriptor = {};
        descriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        descriptor.NumDescriptors = srv_descriptor_capacity;
        descriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        p_device12->CreateDescriptorHeap(&descriptor, IID_PPV_ARGS(&p_descriptor_heap));

        p_imgui_texture_command_queue = p_command_queue;
        owns_imgui_texture_command_queue = false;
        if (!p_imgui_texture_command_queue)
        {
            D3D12_COMMAND_QUEUE_DESC queue_desc = {};
            queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            queue_desc.NodeMask = 1;
            if (SUCCEEDED(p_device12->CreateCommandQueue(
                &queue_desc,
                IID_PPV_ARGS(&p_imgui_texture_command_queue))))
            {
                owns_imgui_texture_command_queue = true;
            }
        }

        ImGui_ImplDX12_InitInfo init_info{};
        init_info.Device = p_device12;
        init_info.CommandQueue = p_imgui_texture_command_queue;
        init_info.NumFramesInFlight = 2;
        init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        init_info.SrvDescriptorHeap = p_descriptor_heap;
        init_info.SrvDescriptorAllocFn = allocate_srv_descriptor;
        init_info.SrvDescriptorFreeFn = free_srv_descriptor;
        ImGui_ImplDX12_Init(&init_info);
    }
    else
    {
        ImGui_ImplDX11_Init(p_device11, p_context11);
    }

    menu::setup();
    media_widget::start();

    initDone = true;
}

void dxhook::shutdown_imgui()
{
    initDone = false;
    media_widget::stop();
    media_widget::release();

    if (is_dx12)
    {
        ImGui_ImplDX12_Shutdown();
    }
    else
    {
        ImGui_ImplDX11_Shutdown();
    }
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    if (oWndProc != nullptr)
    {
        SetWindowLongPtr(imgui_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(oWndProc));
    }
    if (is_dx12)
    {
        if (p_descriptor_heap)
        {
            p_descriptor_heap->Release();
            p_descriptor_heap = nullptr;
        }

        if (g_pd3d_srv_desc_heap)
        {
            g_pd3d_srv_desc_heap->Release();
            g_pd3d_srv_desc_heap = nullptr;
        }

        if (p_allocator)
        {
            p_allocator->Release();
            p_allocator = nullptr;
        }

        if (p_command_list)
        {
            p_command_list->Release();
            p_command_list = nullptr;
        }

        for (size_t i = 0; i < bufferCount; ++i)
        {
            if (back_buffers[i])
            {
                back_buffers[i]->Release();
            }
        }

        delete[] back_buffers;
        back_buffers = nullptr;

        delete[] descriptor_handles;
        descriptor_handles = nullptr;

        if (owns_imgui_texture_command_queue && p_imgui_texture_command_queue)
        {
            p_imgui_texture_command_queue->Release();
        }
        p_imgui_texture_command_queue = nullptr;
        owns_imgui_texture_command_queue = false;
        reset_srv_descriptor_allocator();
    }
    else
    {
        main_render_target_view11->Release();
    }
}

void dxhook::draw_imgui(IDXGISwapChain3* p_swap_chain)
{
    if (is_dx12)
    {
        ImGui_ImplDX12_NewFrame();
    }
    else
    {
        ImGui_ImplDX11_NewFrame();
    }
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    {
        menu::draw();
        media_widget::render(is_dx12 ? nullptr : p_device11);
    }
    ImGui::EndFrame();
    ImGui::Render();

    if (is_dx12)
    {
        size_t index = p_swap_chain->GetCurrentBackBufferIndex();
        ID3D12Resource* p_back_buffer = back_buffers[index];
        p_allocator->Reset();
        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = p_back_buffer;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        p_command_list->Reset(p_allocator, nullptr);
        p_command_list->ResourceBarrier(1, &barrier);
        p_command_list->OMSetRenderTargets(1, &descriptor_handles[index], FALSE, nullptr);
        p_command_list->SetDescriptorHeaps(1, &p_descriptor_heap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), p_command_list);
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        p_command_list->ResourceBarrier(1, &barrier);
        p_command_list->Close();

        // skip if we cont have the queue right now
        if (p_command_queue == nullptr) return;
        p_command_queue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&p_command_list));
    }
    else
    {
        p_context11->OMSetRenderTargets(1, &main_render_target_view11, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
}

ID3D11Device* dxhook::get_d3d11_device()
{
    return p_device11;
}

HWND dxhook::get_window()
{
    return imgui_window;
}
