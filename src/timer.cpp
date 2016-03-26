#include "timer.hpp"

void Timer::start()
{
    gettimeofday(&start_time, nullptr);
}

void Timer::stop()
{
    gettimeofday(&end_time, nullptr);
}

double Timer::elapsedSeconds()
{
    return (end_time.tv_sec + end_time.tv_usec / 1000000.0) -
           (start_time.tv_sec+ start_time.tv_usec / 1000000.0);
}
