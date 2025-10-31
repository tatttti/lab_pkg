#ifndef PERF_H
#define PERF_H

#include <chrono>
using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

inline TimePoint nowT() { return Clock::now(); }
inline double elapsedMs(TimePoint a, TimePoint b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

#endif // PERF_H
