#pragma once
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <Windows.h>

#include "exports.h"
#include "../hooks/hooks.h"
#include "../util/hash.h"
#include "../util/logger.h"
#include "../util/utils.h"

namespace il2utils
{
    inline Il2CppImage* resolve_image(const char* name)
    {
        Il2CppDomain* domain = il2cpp::il2cpp_domain_get();
        size_t assemblies_size = 0;
        const Il2CppAssembly** assemblies = il2cpp::il2cpp_domain_get_assemblies(domain, &assemblies_size);
        for (size_t i = 0; i < assemblies_size; ++i)
        {
            Il2CppImage* image = assemblies[i]->image;
            if (std::string(image->name) == std::string(name))
            {
                return image;
            }
        }

        logger::warn(std::string("resolve_image FAILED: ") + name);
        return nullptr;
    }

    inline const Il2CppClass* resolve_class_nested(const Il2CppClass* parent, const char* name)
    {
        if (!parent)
        {
            logger::warn(std::string("resolve_class_nested FAILED (null parent): ") + name);
            return nullptr;
        }
        void* iter = nullptr;
        Il2CppClass* nested = nullptr;
        while ((nested = il2cpp::il2cpp_class_get_nested_types(const_cast<Il2CppClass*>(parent), &iter)) != nullptr)
        {
            if (nested->name && strcmp(nested->name, name) == 0)
            {
                return nested;
            }
        }
        logger::warn(std::string("resolve_class_nested FAILED: ") + (parent->name ? parent->name : "?") + "::" + name);
        return nullptr;
    }

    inline const Il2CppClass* resolve_class(const Il2CppImage* image, const char* namespaze, const char* name)
    {
        if (!image)
        {
            logger::warn(std::string("resolve_class FAILED (null image): ") + namespaze + "." + name);
            return nullptr;
        }
        const Il2CppClass* cls = il2cpp::il2cpp_class_from_name(image, namespaze, name);
        return cls;
    }

    inline FieldInfo* resolve_field(const Il2CppClass* clazz, const char* name)
    {
        return il2cpp::il2cpp_class_get_field_from_name(const_cast<Il2CppClass*>(clazz), name);
    }

    inline const MethodInfo* resolve_method(const Il2CppClass* clazz, const char* name, int arg_count)
    {
        const MethodInfo* m = il2cpp::il2cpp_class_get_method_from_name(const_cast<Il2CppClass*>(clazz), name, arg_count);
        return m;
    }

    template <typename T>
    void set_static_field(const Il2CppClass* clazz, const char* name, T value)
    {
        FieldInfo* field = resolve_field(clazz, name);
        il2cpp_field_static_set_value(field, &value);
    }

    template <typename T>
    T get_static_field(const Il2CppClass* clazz, const char* name)
    {
        FieldInfo* field = resolve_field(clazz, name);
        T value;
        il2cpp::il2cpp_field_static_get_value(field, static_cast<PVOID>(&value));
        return value;
    }

    template <typename T>
    T get_method(const Il2CppClass* clazz, const char* name, const int args)
    {
        const MethodInfo* method = resolve_method(clazz, name, args);
        return static_cast<T>(method->methodPointer); // NOLINT(clang-diagnostic-microsoft-cast)
    }

    template <typename T, typename T2>
    void hook_method(const Il2CppClass* clazz, const char* name, const int args, T hook, T2 original)
    {
        const char* class_name = (clazz && clazz->name) ? clazz->name : "(null class)";
        if (!clazz)
        {
            logger::warn(std::string("hook_method FAILED (null class): ") + name);
            return;
        }
        const MethodInfo* method = resolve_method(clazz, name, args);
        if (!method)
        {
            logger::warn(std::string("hook_method FAILED (null method): ") + class_name + "::" + name);
            return;
        }
        if (!method->methodPointer)
        {
            logger::warn(std::string("hook_method FAILED (null methodPointer): ") + class_name + "::" + name);
            return;
        }
        hooks::hook(method->methodPointer, reinterpret_cast<PVOID>(hook), reinterpret_cast<PVOID*>(original));
    }

    template <typename T, typename T2>
    void unhook_method(const Il2CppClass* clazz, const char* name, const int args, T hook, T2 original)
    {
        const MethodInfo* method = resolve_method(clazz, name, args);
        hooks::unhook(method->methodPointer, reinterpret_cast<PVOID>(hook), reinterpret_cast<PVOID*>(original));
    }

    inline Il2CppObject* get_system_type(const Il2CppImage* image, const char* namespaze, const char* name)
    {
        const Il2CppType* type = il2cpp::il2cpp_class_get_type(
            const_cast<Il2CppClass*>(resolve_class(image, namespaze, name)));
        return il2cpp::il2cpp_type_get_object(type);
    }

    inline std::string conv_string(const Il2CppString* string)
    {
        std::stringstream ss;
        for (int i = 0; i < string->length; ++i)
        {
            ss << static_cast<char>(string->chars[i]);
        }
        return ss.str();
    }

    inline std::wstring conv_wstring(const Il2CppString* string)
    {
        std::wstringstream ss;
        for (int i = 0; i < string->length; ++i)
        {
            ss << string->chars[i];
        }
        return ss.str();
    }
}
