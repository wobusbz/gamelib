#include "timer_wheel.h"
#include "clock.h"
#include <algorithm>
#include <cassert>

Timer::Timer(int id, int rounds, int deadLine, TimerFunc cb)
    : m_id(id), m_rounds(rounds), m_deadline(deadLine), m_cb(cb) {}

bool Timer::Expire() {

  if (m_cb == nullptr) {
    return false;
  }
  m_cb();
  return true;
}

TimerBucket::TimerBucket() {}

TimerBucket::~TimerBucket() {}

void TimerBucket::AddTimer(Timer *timer) {
  if (!timer) {
    return;
  }
  timer->m_bucket = this;
  m_timers.push_back(timer);
}

void TimerBucket::ExpiredTimer(int deadLine, std::vector<Timer *> &expired) {
  for (auto it = m_timers.begin(); it != m_timers.end();) {
    if ((*it)->m_rounds <= 0) {
      expired.push_back(*it);
      it = m_timers.erase(it);
    } else {
      (*it)->m_rounds--;
      ++it;
    }
  }
}

void TimerBucket::Remove(Timer *timer) {
  m_timers.erase(std::find_if(m_timers.begin(), m_timers.end(),
                              [&](Timer *tm) -> bool { return tm == timer; }));
}

void TimerBucket::CleanTimers(std::unordered_map<int, Timer *> &set) {}

TimerWheel::TimerWheel(int size) {
  m_startAt = Time::UnixMilli();
  m_lastAt = Time::UnixMilli();
  m_tickDuration = 10;
  m_timeUnit = 10;
  m_ticks = 0;
  m_wheels.resize(size);
  for (int i = 0; i < size; i++) {
    m_wheels[i] = new TimerBucket();
  }
}

TimerWheel::~TimerWheel() { purge(); }

int TimerWheel::TimerFunc(int id, int64_t duration, Timer::TimerFunc cb) {
  int64_t deadlien = Time::UnixMilli() + duration;
  int rounds = (deadlien - m_startAt) / m_tickDuration;
  Timer *newTimer =
      new Timer(id, (rounds - m_ticks) / m_wheels.size(), deadlien, cb);
  int ticks = rounds < m_ticks ? m_ticks : rounds;
  int idx = ticks & (m_wheels.size() - 1);
  m_wheels[idx]->AddTimer(newTimer);
  m_timers[id] = newTimer;
  return id;
}

void TimerWheel::Tick(int64_t now) {
  if (size() == 0) {
    return;
  }

  int64_t elapsed = now - m_lastAt;
  int tickn = elapsed / m_timeUnit; // 10ms
  if (tickn <= 0) {
    return;
  }
  m_lastAt = now;

  for (int i = 0; i < tickn; i++) {
    tick();
  }
}

void TimerWheel::Cancel(int timerId) {
  auto itTimer = m_timers.find(timerId);
  if (itTimer == m_timers.end()) {
    return;
  }
  m_timers.erase(itTimer);
  auto timer = itTimer->second;
  if (!timer) {
    return;
  }
  timer->m_bucket->Remove(timer);
  delete timer;
  timer = nullptr;
}

int TimerWheel::size() { return (int)m_timers.size(); }

void TimerWheel::tick() {
  int64_t deadline = m_startAt + m_tickDuration * (m_ticks + 1);
  int idx = m_ticks % (m_wheels.size() - 1);
  auto bucket = m_wheels[idx];
  std::vector<Timer *> expired;
  bucket->ExpiredTimer(deadline, expired);
  for (auto time : expired) {
    time->Expire();
    m_timers.erase(time->m_id);
    delete time;
    time = nullptr;
  }
  m_ticks++;
}

void TimerWheel::purge() {
  for (auto buckets : m_wheels) {
    for (auto it = buckets->m_timers.begin(); it != buckets->m_timers.end();
         ++it) {
      if (!*it) {
        continue;
      }
      delete *it;
    }
    buckets->m_timers.clear();
  }
  m_wheels.clear();
}
