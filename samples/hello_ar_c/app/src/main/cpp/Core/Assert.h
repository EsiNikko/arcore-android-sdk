#pragma once
#include "Log.h"
#include <filesystem>

// Alternatively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define RAY_INTERNAL_VERIFY_IMPL(type, check, msg, ...) do { if(!(check)) { RAY##type##ERROR(msg, __VA_ARGS__); RAY_DEBUG_BREAK(); } } while(false)
#define RAY_INTERNAL_VERIFY_WITH_MSG(type, check, ...) RAY_INTERNAL_VERIFY_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define RAY_INTERNAL_VERIFY_NO_MSG(type, check) RAY_INTERNAL_VERIFY_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", RAY_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define RAY_INTERNAL_VERIFY_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define RAY_INTERNAL_VERIFY_GET_MACRO(...) RAY_EXPAND_MACRO( RAY_INTERNAL_VERIFY_GET_MACRO_NAME(__VA_ARGS__, RAY_INTERNAL_VERIFY_WITH_MSG, RAY_INTERNAL_VERIFY_NO_MSG) )

#define RAY_CORE_VERIFY(...) RAY_EXPAND_MACRO( RAY_INTERNAL_VERIFY_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )

#ifdef RAY_ENABLE_ASSERTS
	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define RAY_ASSERT(...) RAY_EXPAND_MACRO( RAY_INTERNAL_VERIFY_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define RAY_CORE_ASSERT(...) RAY_EXPAND_MACRO( RAY_INTERNAL_VERIFY_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define RAY_ASSERT(...)
	#define RAY_CORE_ASSERT(...)
#endif


