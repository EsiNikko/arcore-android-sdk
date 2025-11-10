#pragma once
#include <chrono>
#include <string>

namespace Ray
{
    class Timer
    {
    public:
        static std::string GetLocalTimeString();

        Timer()
        {
            Reset();
        }

        void Reset()
        {
            m_Start = std::chrono::high_resolution_clock::now();
        }

        [[nodiscard]] float Elapsed() const
        {
            const auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count();
            return static_cast<float>(static_cast<double>(now) * (0.001 * 0.001 * 0.001));
        }

        [[nodiscard]] float ElapsedMillis() const
        {
            return Elapsed() * 1000.0f;
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
    };
}
