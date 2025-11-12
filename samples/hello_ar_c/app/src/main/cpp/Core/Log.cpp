#include "Log.h"
#include "Timer.h"
#include <android/log.h>

namespace Ray
{
    void Log::PrintMessageInternal(Log::Type type, Log::Level level, const std::string &message)
    {
        std::string timeStr = Timer::GetLocalTimeString();
        std::string_view typeStr = type == Type::Core ? "Core" : "App";
        std::string_view levelStr;
        android_LogPriority androidLogPriority;
        switch (level)
        {
            case Level::Trace: levelStr = "Trace"; androidLogPriority = ANDROID_LOG_VERBOSE; break;
            case Level::Info:  levelStr = "Info";  androidLogPriority = ANDROID_LOG_INFO; break;
            case Level::Warn:  levelStr = "Warn";  androidLogPriority = ANDROID_LOG_WARN; break;
            case Level::Error: levelStr = "Error"; androidLogPriority = ANDROID_LOG_ERROR; break;
            case Level::Fatal: levelStr = "Fatal"; androidLogPriority = ANDROID_LOG_FATAL; break;
        }

        std::stringstream stream;
        stream << std::format("[{}][{}][{}] ", timeStr, typeStr, levelStr) << message << "\n";
        __android_log_print(androidLogPriority, "RayAR_C++", "%s", stream.str().c_str());
    }
}