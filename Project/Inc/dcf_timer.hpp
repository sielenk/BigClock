/*
 * dcf_timer.hpp
 *
 *  Created on: Apr 13, 2020
 *      Author: Marv
 */

#ifndef DCF_TIMER_HPP_
#define DCF_TIMER_HPP_
#ifdef __cplusplus

#include <cstdint>
#include <functional>

extern "C" {
#endif

  void
  dcfTimerFunc(void *argument) __attribute__ ((noreturn));

#ifdef __cplusplus
}

class IDcfTimer {
protected:
  virtual
  ~IDcfTimer();

public:
  static constexpr uint16_t ticksPerSecond { 1125 };

  static IDcfTimer *instancePtr;

  virtual void
  onSecondStart(uint32_t secondOfDay) = 0;

  virtual void
  onPulse(bool start, uint32_t secondOfDay, uint16_t tick) = 0;

  uint16_t
  getPrescaler() const;

  void
  setPrescaler(uint16_t newPrescalerValue);

  void
  setSecondOfDay(uint32_t secondOfDay);
};

#endif
#endif /* DCF_TIMER_HPP_ */
