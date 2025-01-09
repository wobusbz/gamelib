#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <chrono>
#include <cstdint>

class Time {
public:
    static int64_t unixNano() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    };
    static int64_t unixMicro() { return unixNano() / 1e3; };
    static int64_t unixMilli() { return unixMicro() / 1e3; };
    static int64_t unix() { return unixMilli() / 1e3; };
};

#endif
