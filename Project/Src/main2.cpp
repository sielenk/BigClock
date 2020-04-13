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

constexpr int ticksPerSecond { 1125 };

namespace {
  class DcfTimer: public IDcfTimer {
    static DcfTimer instance;

    DcfTimer() {
      instancePtr = this;
    }

    ~DcfTimer() {
      instancePtr = nullptr;
    }

  public:
    virtual void
    onSecondStart();

    virtual void
    onPulse(bool start, uint32_t sampledValue);
  };

  uint32_t secondOfDay { (12 * 60 + 34) * 60 + 56 };
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
dcf_handleTelegram(DCF const *dcf) {
  if (dcf) {
    const auto hours { 10 * dcf->hour10 + dcf->hour01 };
    const auto minutes { 10 * dcf->minute10 + dcf->minute01 };

    current_time.tm_mday = 10 * dcf->day10 + dcf->day01;
    current_time.tm_mon = 10 * dcf->month10 + dcf->month01 - 1;
    current_time.tm_year = 10 * dcf->year10 + dcf->year01 + 100;
    current_time.tm_wday = dcf->weekday % 7;
    current_time.tm_isdst = dcf->isMesz;

    secondOfDay = (60 * hours + minutes) * 60;
  }
}


void
DcfTimer::onSecondStart() {
  // This is triggered by the counter being updated after reaching its maximum value.
  const auto secondOfDay_ { secondOfDay };

  secondOfDay = (secondOfDay + 1) % (24 * 60 * 60);

  const auto minuteSecond { div(secondOfDay_, 60) };
  const auto minuteOfDay { minuteSecond.quot };
  const auto hourMinute { div(minuteOfDay, 60) };
  const auto hour { hourMinute.quot };
  const auto minute { hourMinute.rem };

  const auto hh { div(hour, 10) };
  const auto mm { div(minute, 10) };

  current_time.tm_sec = minuteSecond.rem;
  current_time.tm_min = minute;
  current_time.tm_hour = hour;

  if (secondOfDay_ == 0) {
    current_time.tm_mday = -1;
    current_time.tm_mon = -1;
    current_time.tm_year = -1;
    current_time.tm_wday = -1;
    current_time.tm_yday = -1;
    current_time.tm_isdst = -1;
  }

  writeSegment(SEG_H10_MASK, hh.quot);
  writeSegment(SEG_H01_MASK, hh.rem);
  writeSegment(SEG_M10_MASK, mm.quot);
  writeSegment(SEG_M01_MASK, mm.rem);

  matrixSetTime (current_time);
}

constexpr int secondCount = 59;
constexpr int betaInvFactor = secondCount * (secondCount * secondCount - 1) / 12;
static_assert(
    static_cast<double>(betaInvFactor) ==
    secondCount * (secondCount * secondCount - 1.0) / 12.0);

int errorTicks[secondCount] { };
int minuteStart { -1 };
int secondStart { 0 };
int pulseEnd { 0 };
int pulseIndex { -1 };

void
DcfTimer::onPulse(bool start, uint32_t sampledValue) {
  const auto sampledTicks { sampledValue + secondOfDay * ticksPerSecond };

  if (!start) {
    // This is the end of a 100/200ms time pulse.
    pulseEnd = sampledTicks;
  } else {
    // This is the start of a second as broadcast by the DCF sender.
    const auto oldSecondStart { secondStart };
    const auto newSecondStart { sampledTicks };

    secondStart = newSecondStart;

    const auto pulseLength { (pulseEnd - oldSecondStart) * 1000 / ticksPerSecond };
    const auto pulseDistance { (newSecondStart - oldSecondStart) * 1000
        / ticksPerSecond };

    if (pulseDistance > 1750) {
      if (pulseIndex == secondCount) {
        int beta { 0 };
        {
          int x { -(secondCount - 1) / 2 };
          for (auto const y : errorTicks) {
            beta += x++ * y;
          }
        }
        // error in ticks per second
        beta /= betaInvFactor;

        updatePrescaler([beta](uint32_t value) {
            return (value * (ticksPerSecond + beta)) / ticksPerSecond;
        });
      }

      minuteStart = secondStart;
      pulseIndex = 0;
    }

    if (0 <= pulseIndex && pulseIndex < secondCount) {
      const int error { secondStart - minuteStart - pulseIndex * ticksPerSecond };

      if (std::abs(error) < 200) {
        errorTicks[pulseIndex] = error;
        ++pulseIndex;
      } else {
        pulseIndex = -1;
      }
    }

    if (pulseLength > 10) { // skip glitches
      dcf_addBit(pulseLength, pulseDistance);
    }
  }
}
