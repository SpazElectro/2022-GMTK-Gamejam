#include "Timer.h"

Timer::Timer()
{
    startTime = std::chrono::steady_clock::now();
}

unsigned long long Timer::ElapsedMillisecs() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();
}
