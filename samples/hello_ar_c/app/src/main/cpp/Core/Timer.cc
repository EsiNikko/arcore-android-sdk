#include "Timer.h"
#include <ctime>

namespace  Ray
{
    std::string Timer::GetLocalTimeString()
    {
        using namespace std::chrono;
        // Get current system time
        const auto now = system_clock::now();
        // Convert to time_t for calendar formatting
        std::time_t current_time = system_clock::to_time_t(now);

        // Break down into calendar time
        std::tm local_time{};
        // Use localtime_r for thread-safe conversion to local time.
        // It returns a pointer to the tm structure, or nullptr on error.
        if (localtime_r(&current_time, &local_time) == nullptr) {
            // Handle error, e.g., log it or return an empty string
            return "00:00:00";
        }

        // Format into hh:mm:ss
        std::string timeStr = std::format("{:02}:{:02}:{:02}",
                                          local_time.tm_hour,
                                          local_time.tm_min,
                                          local_time.tm_sec);
        return timeStr;
    }
}
