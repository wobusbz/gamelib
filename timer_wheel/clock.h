#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <chrono>
#include <cstdint>

class Time {
public:
  static int64_t UnixNano() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
  };
  static int64_t UnixMicro() { return UnixNano() / 1e3; };
  static int64_t UnixMilli() { return UnixMicro() / 1e3; };
  static int64_t Unix() { return UnixMilli() / 1e3; };
};

#endif
