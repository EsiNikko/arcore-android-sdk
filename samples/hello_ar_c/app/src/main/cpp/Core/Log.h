#pragma once
#include <format>
#include <iostream>

namespace Ray
{
	class Log
	{
	public:
		enum class Type : uint8_t
		{
			Core = 0, Client = 1
		};
		enum class Level : uint8_t
		{
			Trace = 0, Info, Warn, Error, Fatal,
		};

		template<typename... Args>
		static void PrintMessage(Type type, Level level, std::format_string<Args...> format, Args&&... args)
		{
            PrintMessageInternal(type, level, std::format(format, std::forward<Args>(args)...));
		}
    private:
        static void PrintMessageInternal(Type type, Level level, const std::string& message);
	};
}

// Core log macros
#define RAY_CORE_TRACE(...)		::Ray::Log::PrintMessage(::Ray::Log::Type::Core, ::Ray::Log::Level::Trace, __VA_ARGS__)
#define RAY_CORE_INFO(...)		::Ray::Log::PrintMessage(::Ray::Log::Type::Core, ::Ray::Log::Level::Info, __VA_ARGS__)
#define RAY_CORE_WARN(...)		::Ray::Log::PrintMessage(::Ray::Log::Type::Core, ::Ray::Log::Level::Warn, __VA_ARGS__)
#define RAY_CORE_ERROR(...)		::Ray::Log::PrintMessage(::Ray::Log::Type::Core, ::Ray::Log::Level::Error, __VA_ARGS__)
#define RAY_CORE_CRITICAL(...)	::Ray::Log::PrintMessage(::Ray::Log::Type::Core, ::Ray::Log::Level::Fatal, __VA_ARGS__)

// Client log macros
#define RAY_TRACE(...)			::Ray::Log::PrintMessage(::Ray::Log::Type::Client, ::Ray::Log::Level::Trace, __VA_ARGS__)
#define RAY_INFO(...)			::Ray::Log::PrintMessage(::Ray::Log::Type::Client, ::Ray::Log::Level::Info, __VA_ARGS__)
#define RAY_WARN(...)			::Ray::Log::PrintMessage(::Ray::Log::Type::Client, ::Ray::Log::Level::Warn, __VA_ARGS__)
#define RAY_ERROR(...)			::Ray::Log::PrintMessage(::Ray::Log::Type::Client, ::Ray::Log::Level::Error, __VA_ARGS__)
#define RAY_CRITICAL(...)		::Ray::Log::PrintMessage(::Ray::Log::Type::Client, ::Ray::Log::Level::Fatal, __VA_ARGS__)
