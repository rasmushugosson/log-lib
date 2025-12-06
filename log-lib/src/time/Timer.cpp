#include "general/pch.h"

#include <cmath>

ae::Timer::Timer() : m_ElapsedTime(std::chrono::steady_clock::duration::zero()), m_Running(false) {}

void ae::Timer::Start()
{
    m_Start = std::chrono::steady_clock::now();

    m_Running = true;
}

void ae::Timer::Stop()
{
#ifdef AE_DEBUG
    if (!m_Running)
    {
        AE_LOG_WARNING("Tried to stop Timer but it is not running");
        return;
    }
#endif // AE_DEBUG

    m_ElapsedTime += std::chrono::steady_clock::now() - m_Start;

    m_Running = false;
}

void ae::Timer::Reset()
{
    m_ElapsedTime = std::chrono::steady_clock::duration::zero();
    m_Running = false;
}

double ae::Timer::GetElapsedTime() const
{
    using namespace std::chrono;

    auto duration = m_ElapsedTime;
    if (m_Running)
    {
        duration += steady_clock::now() - m_Start;
    }

    return duration_cast<std::chrono::duration<double>>(duration).count();
}

std::string ae::Timer::GetElapsedTimeAsString(int32_t decimals) const
{
    using namespace std::chrono;

    auto duration = m_ElapsedTime;
    if (m_Running)
    {
        duration += steady_clock::now() - m_Start;
    }

    auto h = duration_cast<hours>(duration);
    duration -= h;
    auto m = duration_cast<minutes>(duration);
    duration -= m;
    auto s = duration_cast<seconds>(duration);
    duration -= s;

    double frac = duration_cast<std::chrono::duration<double>>(duration).count();

    double pow10 = 1;
    for (int i = 0; i < decimals; ++i)
    {
        pow10 *= 10;
    }

    long long frac_scaled = std::llround(frac * pow10);

    std::string result;
    if (h.count() > 0)
    {
        result = std::format("{}:{:02}:{:02}", h.count(), m.count(), s.count());
    }

    else if (m.count() > 0)
    {
        result = std::format("{}:{:02}", m.count(), s.count());
    }

    else
    {
        result = std::format("{}", s.count());
    }

    if (decimals > 0)
    {
        result += std::format(".{:0{}}", frac_scaled, decimals);
    }

    return result;
}
