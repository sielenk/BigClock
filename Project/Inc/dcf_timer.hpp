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

enum {
  DCF_TIMER_SECOND_START = 1 << 16,
  DCF_TIMER_PULSE_START = 1 << 17,
  DCF_TIMER_PULSE_END = 1 << 18,

  DCF_TIMER_MASK = DCF_TIMER_SECOND_START | DCF_TIMER_PULSE_START
      | DCF_TIMER_PULSE_END
};

class IDcfTimer {
protected:
  virtual
  ~IDcfTimer() {
  }

public:
  static IDcfTimer *instancePtr;

  virtual void
  onSecondStart() = 0;

  virtual void
  onPulse(bool start, uint32_t sampledValue) = 0;

  void
  updatePrescaler(const std::function<uint32_t(uint32_t)>&);

  uint32_t
  getPrescaler() const;
};

#endif
#endif /* DCF_TIMER_HPP_ */
