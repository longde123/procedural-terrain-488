#pragma once

#include <sys/time.h>

class Timer {
public:
    void start();
    void stop();
    double elapsedSeconds();

private:
    timeval start_time;
    timeval end_time;
};
