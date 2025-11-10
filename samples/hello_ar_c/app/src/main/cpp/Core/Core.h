#pragma once
#include "PlatformDetection.h"
#include <memory>

#define _DEBUG

#ifdef _DEBUG
    #define RAY_DEBUG
#endif

#ifdef RAY_DEBUG
    #if defined(RAY_PLATFORM_WINDOWS)
        #define RAY_DEBUG_BREAK() __debugbreak()
    #elif defined(RAY_PLATFORM_ANDROID)
        #define RAY_DEBUG_BREAK() __builtin_trap()
    #else
        #warning "Platform doesn't support debugbreak yet!"
        #define RAY_DEBUG_BREAK()
    #endif
    #define RAY_ENABLE_ASSERTS
#else
    #define RAY_DEBUG_BREAK()
#endif

#define RAY_EXPAND_MACRO(x) x
#define RAY_STRINGIFY_MACRO(x) #x

#define RAY_DELETE_COPY(Class) \
    Class(const Class&) = delete; \
    Class& operator=(const Class&) = delete
#define RAY_DELETE_MOVE(Class) \
    Class(Class&&) = delete; \
    Class& operator=(Class&&) = delete

namespace Ray
{
    template<typename T>
    using Scope = std::unique_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using Ref = std::shared_ptr<T>;
    template<typename T, typename ... Args>
    constexpr Ref<T> CreateRef(Args&& ... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}

#include "Assert.h"