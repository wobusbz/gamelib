#ifndef __TIMER_WHEEL_H__
#define __TIMER_WHEEL_H__

#include <functional>
#include <list>
#include <unordered_map>
#include <vector>

class TimerBucket;
class TimerWheel;

class Timer {
    using timerFunc = std::function<void()>;
    friend TimerBucket;
    friend TimerWheel;

public:
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer(int id, int rounds, int deadLine, timerFunc cb);

    bool Expire();

private:
    TimerBucket* m_bucket = nullptr;

    int m_id = 0;
    int m_rounds = 0;
    int m_deadline = 0;
    timerFunc m_cb = nullptr;
};

class TimerBucket {
    friend TimerWheel;

public:
    TimerBucket(const TimerBucket&) = delete;
    TimerBucket& operator=(const TimerBucket&) = delete;
    TimerBucket();
    ~TimerBucket();

    void addTimer(Timer* timer);
    void expiredTimer(int deadLine, std::vector<Timer*>& expired);
    void remove(Timer* timer);
    void cleanTimers(std::unordered_map<int, Timer*>& set);

private:
    std::list<Timer*> m_timers;
};

class TimerWheel {
public:
    TimerWheel(const TimerWheel&) = delete;
    TimerWheel& operator=(const TimerWheel&) = delete;
    TimerWheel(int size = 100);
    ~TimerWheel();

    int timerFunc(int id, int64_t duration, Timer::timerFunc cb);

    void tick(int64_t now);
    void cancel(int timerId);
    int size();

private:
    void purge();
    void tick();

    std::vector<TimerBucket*> m_wheels;
    std::unordered_map<int, Timer*> m_timers;

    int m_ticks;
    int64_t m_startAt;
    int64_t m_lastAt;
    int64_t m_tickDuration;
    int m_timeUnit;
};

#endif
