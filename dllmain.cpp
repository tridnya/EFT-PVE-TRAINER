#include <iostream>
#include <windows.h>

#include "gui/dxhook/dxhook.h"
#include "hooks/hooks.h"
#include "il2cpp/il2utils.h"
#include "il2cpp/unity.h"
#include "module/always_on.h"
#include "module/common_hooks.h"
#include "module/ModuleManager.h"
#include "runtime/mcp/runtime_bridge.h"
#include "util/globals.h"
#include "util/logger.h"

namespace veil_entry
{
    FILE* console = nullptr;

    void show_console()
    {
        AllocConsole();
        errno_t _ = freopen_s(&console, "CONOUT$", "w", stdout);
        SetConsoleTitleA(std::string(globals::get_display_title() + " | " + globals::credits).c_str());
    }

    void hide_console()
    {
        errno_t _ = fclose(console);
        FreeConsole();
    }

    [[noreturn]] void init(HMODULE this_module)
    {
        if (globals::console) show_console();

        logger::info(globals::get_display_title());
        logger::info(globals::credits);

        // wait for GameAssembly.dll to load
        logger::info("Waiting for Unity window to become ready...");

        while (!FindWindowW(L"UnityWndClass", nullptr))
        {
            Sleep(10);
        }

        // load il2cpp offsets
        il2cpp_offsets::init();

        // init unity interop
        logger::info("Loading Unity");
        unity::init();
        logger::info("Unity Loaded");

        // init modules
        logger::info("Loading Modules");
        ModuleManager::get_instance()->init();
        logger::info("Loaded Modules");

        // wait for dx window
        while (!GetModuleHandle(L"d3d11.dll") || !GetModuleHandle(L"dxgi.dll"))
        {
            Sleep(50);
        }

        // dxhook and menu
        logger::info("Hooking Present");
        if (!dxhook::init(this_module)) logger::error("Present Hook Failed");
        else logger::info("Hooked Present");

        // load default features and common hooks
        logger::info("Loading Default Features");
        always_on::init(); 
        common_hooks::init();
        runtime::mcp::start();
        logger::info("Loaded Default Features");

        ExitThread(0);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(veil_entry::init), hModule, 0, nullptr); // NOLINT(clang-diagnostic-cast-function-type-strict)
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    default:;
    }

    return TRUE;
}
