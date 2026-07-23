#pragma once
#include <windows.h>
#include <Psapi.h>
#include <cstdint>
#include "../util/scan.h"
#include "../util/logger.h"

namespace il2cpp_offsets_detail
{
    inline uintptr_t get_export_addr(HMODULE module, const char* name)
    {
        return reinterpret_cast<uintptr_t>(GetProcAddress(module, name));
    }
}

namespace il2cpp_offsets
{
    static inline HMODULE module_base;
    static inline DWORD module_size;

    // direct pattern
#define PATTERN(pattern)  reinterpret_cast<PVOID>(scan::find_pattern(pattern, reinterpret_cast<char*>(module_base), module_size))

    // xref pattern for short call (call ?? ?? ?? ??)
#define PATTERN_CALL(pattern) reinterpret_cast<PVOID>(scan::find_pattern_xref(pattern, reinterpret_cast<char*>(module_base), module_size, 1, 5))

    // load from exports
#define IMPORT(name) reinterpret_cast<PVOID>(scan::import(name))

    // worse case scenario, static offset from the exported address to skip the protection
#define IMPORT_OFFSET(name, offset) reinterpret_cast<PVOID>(scan::import(name) + (offset))

    static inline PVOID il2cpp_class_from_name;
    static inline PVOID il2cpp_array_new;
    static inline PVOID il2cpp_class_from_type;
    static inline PVOID il2cpp_class_get_nested_types;
    static inline PVOID il2cpp_class_get_field_from_name;
    static inline PVOID il2cpp_class_get_method_from_name;
    static inline PVOID il2cpp_domain_get;
    static inline PVOID il2cpp_domain_get_assemblies;
    static inline PVOID il2cpp_field_static_get_value;
    static inline PVOID il2cpp_field_static_set_value;
    static inline PVOID il2cpp_object_new;
    static inline PVOID il2cpp_string_new;
    static inline PVOID il2cpp_type_get_name;
    static inline PVOID il2cpp_type_get_object;
    static inline PVOID il2cpp_thread_attach;
    static inline PVOID il2cpp_thread_detach;
    static inline PVOID il2cpp_free;

    inline void init()
    {
        logger::info("Loading Il2CPP API");

        module_base = GetModuleHandleA("GameAssembly.dll");
        MODULEINFO info = {};
        GetModuleInformation(GetCurrentProcess(), module_base, &info, sizeof(MODULEINFO));
        module_size = info.SizeOfImage;

#define EXPORT_OF(name) reinterpret_cast<PVOID>(il2cpp_offsets_detail::get_export_addr(module_base, name))

        il2cpp_class_from_name = PATTERN(
            "4C 89 44 24 ? 48 89 54 24 ? 53 55 56 57 41 54 41 55 41 56 41 57 48 81 EC");

        il2cpp_array_new = EXPORT_OF("il2cpp_array_new");

        il2cpp_class_from_type = EXPORT_OF("il2cpp_class_from_type");

        il2cpp_class_get_nested_types =
            PATTERN("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B FA 48 8B D9 48 85 "
                    "D2 0F 84 ? ? ? ? 48 83 79 ? ? 0F 85");

        il2cpp_class_get_field_from_name =
            PATTERN("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B EA "
                    "48 8B D9 48 85 C9 74");

        il2cpp_class_get_method_from_name =
            PATTERN("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? "
                    "41 8B E9 41 8B F8");

        il2cpp_domain_get = PATTERN(
            "48 83 EC ? 48 8B 05 ? ? ? ? 48 85 C0 75 ? 33 D2");

        il2cpp_domain_get_assemblies = EXPORT_OF("il2cpp_domain_get_assemblies");

        il2cpp_field_static_get_value = PATTERN(
            "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 41 ? 49 8B D8 48 8B FA");

        il2cpp_field_static_set_value = PATTERN(
            "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 48 8B FA");

        il2cpp_object_new = EXPORT_OF("il2cpp_object_new");

        il2cpp_string_new = PATTERN("48 C7 C2 ? ? ? ? 48 FF C2 ? ? ? ? 75 ? E9");

        il2cpp_type_get_name = EXPORT_OF("il2cpp_type_get_name");

        il2cpp_type_get_object = PATTERN(
            "48 89 4C 24 ? 53 48 83 EC ? 48 8B D9 48 C7 44 24 ? ? ? ? ? 48 8B 0D");

        il2cpp_thread_attach = EXPORT_OF("il2cpp_thread_attach");

        il2cpp_thread_detach = EXPORT_OF("il2cpp_thread_detach");

        il2cpp_free = EXPORT_OF("il2cpp_free");
        //il2cpp_domain_get_assemblies = PATTERN("");
        //il2cpp_string_new = PATTERN("");

        il2cpp_domain_get_assemblies = EXPORT_OF("il2cpp_domain_get_assemblies");
        il2cpp_string_new = EXPORT_OF("il2cpp_string_new");

        const uintptr_t base = reinterpret_cast<uintptr_t>(module_base);
        struct { const char* name; PVOID ptr; } entries[] = {
            { "il2cpp_class_from_name",          il2cpp_class_from_name },
            { "il2cpp_array_new",                il2cpp_array_new },
            { "il2cpp_class_from_type",          il2cpp_class_from_type },
            { "il2cpp_class_get_nested_types",   il2cpp_class_get_nested_types },
            { "il2cpp_class_get_field_from_name",il2cpp_class_get_field_from_name },
            { "il2cpp_class_get_method_from_name",il2cpp_class_get_method_from_name },
            { "il2cpp_domain_get",               il2cpp_domain_get },
            { "il2cpp_domain_get_assemblies",    il2cpp_domain_get_assemblies },
            { "il2cpp_field_static_get_value",   il2cpp_field_static_get_value },
            { "il2cpp_field_static_set_value",   il2cpp_field_static_set_value },
            { "il2cpp_object_new",                il2cpp_object_new },
            { "il2cpp_string_new",               il2cpp_string_new },
            { "il2cpp_type_get_name",            il2cpp_type_get_name },
            { "il2cpp_type_get_object",          il2cpp_type_get_object },
            { "il2cpp_thread_attach",            il2cpp_thread_attach },
            { "il2cpp_thread_detach",            il2cpp_thread_detach },
            { "il2cpp_free",                     il2cpp_free },
        };
        for (const auto& e : entries)
        {
            if (!e.ptr)
            {
                logger::warn(std::string("PATTERN FAILED (null): ") + e.name);
                continue;
            }
            const uintptr_t offset = reinterpret_cast<uintptr_t>(e.ptr) - base;
            const auto* b = reinterpret_cast<const uint8_t*>(e.ptr);
            char buf[128];
            std::snprintf(buf, sizeof(buf),
                "%s  +0x%llX  %02X %02X %02X %02X %02X %02X %02X %02X",
                e.name, static_cast<unsigned long long>(offset),
                b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
            logger::info(buf);
        }

        logger::info("Il2CPP API Loaded");
    }
}
