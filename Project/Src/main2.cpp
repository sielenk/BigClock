/**
 ******************************************************************************
 * File Name          : main2.cpp
 * Description        : The not-generated main file.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 Marvin Sielenkemper.
 * All rights reserved.</center></h2>
 *
 ******************************************************************************
 */

#include "main2.hpp"
#include "matrix.hpp"

#include "dcf.hpp"
#include "dcf_timer.hpp"
#include "7segment.hpp"

#include "FreeRTOS.h"
#include "task.h"

#include <cstdlib>
#include <ctime>

namespace {
  constexpr int secondCount { 59 };
  constexpr int betaInvFactor { secondCount * (secondCount * secondCount - 1)
      / 12 };
  static_assert(
      static_cast<double>(betaInvFactor) ==
      secondCount * (secondCount * secondCount - 1.0) / 12.0);

  class DcfTimer: public IDcfTimer {
    static constexpr uint16_t zeroPulseTicks { ticksPerSecond / 10 }; // 100ms
    static constexpr uint16_t onePulseTicks { ticksPerSecond / 5 }; // 200ms
    static constexpr uint16_t maxPulseError { ticksPerSecond / 22 }; // 45ms
    static constexpr uint16_t maxSecondError { maxPulseError };

    static DcfTimer instance;

    DcfTimer() {
      instancePtr = this;
    }

    ~DcfTimer() {
      instancePtr = nullptr;
    }

  public:
    virtual void
    onSecondStart(uint32_t secondOfDay);

    virtual void
    onPulse(bool start, uint32_t secondOfDay, uint16_t tick);

    void
    processDcfMessage(DCF dcf);

    void
    processPulseStartTicks(uint32_t (&pulseStartTicks)[secondCount]);
  };
}

DcfTimer DcfTimer::instance;

std::tm current_time {
  .tm_sec = -1,
  .tm_min = -1,
  .tm_hour = -1,
  .tm_mday = -1,
  .tm_mon = -1,
  .tm_year = -1,
  .tm_wday = -1,
  .tm_yday = -1,
  .tm_isdst = -1};

void
DcfTimer::processDcfMessage(DCF dcf) {
  const auto hours { 10 * dcf.hour10 + dcf.hour01 };
  const auto minutes { 10 * dcf.minute10 + dcf.minute01 };

  current_time.tm_mday = 10 * dcf.day10 + dcf.day01;
  current_time.tm_mon = 10 * dcf.month10 + dcf.month01 - 1;
  current_time.tm_year = 10 * dcf.year10 + dcf.year01 + 100;
  current_time.tm_wday = dcf.weekday % 7;
  current_time.tm_isdst = dcf.isMesz;

  setSecondOfDay((60 * hours + minutes) * 60);
}

void
DcfTimer::processPulseStartTicks(uint32_t (&pulseStartTicks)[secondCount]) {
  int32_t beta { 0 };
  {
    int x { (1 - secondCount) / 2 };
    for (auto const y : pulseStartTicks) {
      beta += x++ * y;
    }
  }

  setPrescaler(
      (getPrescaler() * static_cast<int64_t>(beta))
          / (ticksPerSecond * betaInvFactor));
}

void
DcfTimer::onSecondStart(uint32_t secondOfDay) {
  // This is triggered by the counter being updated after reaching its maximum value.

  const auto minuteSecond { div(secondOfDay, 60) };
  const auto minuteOfDay { minuteSecond.quot };
  const auto hourMinute { div(minuteOfDay, 60) };
  const auto hour { hourMinute.quot };
  const auto minute { hourMinute.rem };
  const auto hh { div(hour, 10) };
  const auto mm { div(minute, 10) };

  writeSegment(SEG_H10_MASK, hh.quot);
  writeSegment(SEG_H01_MASK, hh.rem);
  writeSegment(SEG_M10_MASK, mm.quot);
  writeSegment(SEG_M01_MASK, mm.rem);

  current_time.tm_sec = minuteSecond.rem;
  current_time.tm_min = minute;
  current_time.tm_hour = hour;

  if (secondOfDay == 0) {
    current_time.tm_mday = -1;
    current_time.tm_mon = -1;
    current_time.tm_year = -1;
    current_time.tm_wday = -1;
    current_time.tm_yday = -1;
    current_time.tm_isdst = -1;
  }

  matrixSetTime (current_time);
}

uint32_t pulseStartTicks[secondCount] { };
union {uint64_t bits; DCF dcf;}dcfMessage {.bits = 0};
uint32_t pulseStartTick { 0 };
int pulseIndex { -1 };

void
DcfTimer::onPulse(bool start, uint32_t secondOfDay, uint16_t tick) {
  const uint32_t tickOfDay { secondOfDay * ticksPerSecond + tick };

  if (!start) {
    // This is the end of a 100/200ms time pulse.
    const auto pulseEndTick { tickOfDay };
    const auto pulseLength { static_cast<int32_t>(pulseEndTick - pulseStartTick) };
    const auto isZero { std::abs(pulseLength - zeroPulseTicks) <= maxPulseError };
    const auto isOne { std::abs(pulseLength - onePulseTicks) <= maxPulseError };

    if ((isZero != isOne) && 0 <= pulseIndex) {
      if (isOne) {
        dcfMessage.bits |= uint64_t { 1 } << pulseIndex;
      }

      ++pulseIndex;
    } else {
      pulseIndex = -1;
    }
  } else {
    // This is the start of a second as broadcast by the DCF sender.
    const auto oldPulseStartTick { pulseStartTick };

    pulseStartTick = tickOfDay;

    const int32_t pulseDistance(pulseStartTick - oldPulseStartTick);
    const auto secondError { std::abs(pulseDistance - ticksPerSecond) };
    const auto biSecondError { std::abs(pulseDistance - 2 * ticksPerSecond) };
    const auto isSecond { secondError <= maxSecondError };
    const auto isBiSecond { biSecondError <= maxSecondError };

    if (isSecond != isBiSecond) {
      if (isBiSecond) {
        if (secondCount <= pulseIndex) {
          if (dcfCheck(dcfMessage.dcf, pulseIndex)) {
            processDcfMessage(dcfMessage.dcf);
          }

          processPulseStartTicks(pulseStartTicks);
        }

        dcfMessage.bits = 0;
        pulseIndex = 0;
        pulseStartTicks[0] = pulseStartTick;
      }

      if (isSecond && 0 <= pulseIndex && pulseIndex < secondCount) {
        pulseStartTicks[pulseIndex] = pulseStartTick;
      }
    } else {
      pulseIndex = -1;
    }
  }
}
