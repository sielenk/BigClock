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

#include "dcf.hpp"

#include "main.h"
#include "tim.h"

#include <cstdlib>

namespace {
  const int ticksPerSecond = 1125;
  const int prescalerBase = 63999;

  int secondOfDay = (12 * 60 + 34) * 60 + 56;

  int error = 0;

  uint32_t
  filter(int error) {
    static int Kp = 750;
    static int Ki = 10;

    static int error_sum = 0;

    error_sum += error;

    const int yp = Kp * error;
    const int yi = Ki * error_sum;
    const int y = yp + yi;

    return prescalerBase + y / 1000;
  }
}

void
dcf_handleTelegram(DCF const *dcf) {
  if (dcf) {
    const uint8_t hours = 10 * dcf->hour10 + dcf->hour01;
    const uint8_t minutes = 10 * dcf->minute10 + dcf->minute01;

    secondOfDay = (60 * hours + minutes) * 60;
  }
}

#define SEG_NONE_MASK 0xf0
#define SEG_M01_MASK 0x70
#define SEG_M10_MASK 0xb0
#define SEG_H01_MASK 0xd0
#define SEG_H10_MASK 0xe0

namespace {
  void
  writeSegment(uint8_t mask, uint8_t value) {
    GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK | value;
    GPIOA->ODR = (GPIOA->ODR & ~0xff) | mask | value;
    GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK | value;
    GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK;
  }
}

void
HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim3) {
    // This is triggered by the counter being updated after reaching its maximum value.
    const auto secondOfDay_ = secondOfDay;

    secondOfDay = (secondOfDay + 1) % (24 * 60 * 60);

    const auto minuteOfDay = secondOfDay_ / 60;
    const auto hourMinute = div(minuteOfDay, 60);
    const auto hour = hourMinute.quot;
    const auto minute = hourMinute.rem;

    const auto hh = div(hour, 10);
    const auto mm = div(minute, 10);

    writeSegment(SEG_H10_MASK, hh.quot);
    writeSegment(SEG_H01_MASK, hh.rem);
    writeSegment(SEG_M10_MASK, mm.quot);
    writeSegment(SEG_M01_MASK, mm.rem);
  }
}

void
HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim3) {
    switch (htim->Channel) {
      case HAL_TIM_ACTIVE_CHANNEL_1:
        // This is triggered by the output compare at the 50% point.
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        break;

      case HAL_TIM_ACTIVE_CHANNEL_2:
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        break;

      default:
        break;
    }
  }
}

void
HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  static int pulseStart = 0;
  static int pulseEnd = 0;

  if (htim == &htim3) {
    const auto &instance(*htim->Instance);
    const auto offset = secondOfDay * ticksPerSecond;

    switch (htim->Channel) {
      case HAL_TIM_ACTIVE_CHANNEL_3: {
        // This is the end of a 100/200ms time pulse.
        pulseEnd = instance.CCR3 + offset;
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        break;
      }
      case HAL_TIM_ACTIVE_CHANNEL_4: {
        // This is the start of a second as broadcast by the DCF sender.
        const auto oldPulseStart = pulseStart;
        const auto ccr4 = instance.CCR4;

        pulseStart = ccr4 + offset;

        error = ccr4 - (ccr4 > ticksPerSecond / 2) * ticksPerSecond;

        const int pulseLength = (pulseEnd - oldPulseStart) * 1000
            / ticksPerSecond;
        int pulseDistance = (pulseStart - oldPulseStart) * 1000
            / ticksPerSecond;

        if (pulseLength > 10) { // skip glitches
          dcf_addBit(pulseLength, pulseDistance);
        }

        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        break;
      }
      default:
        break;
    }
  }
}

void
main_initialize() {
  HAL_GPIO_WritePin(LAMP_TEST_GPIO_Port, LAMP_TEST_Pin, GPIO_PIN_SET);

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

  HAL_TIM_Base_Start_IT(&htim3);
  HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);
  HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_2);
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_3);
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_4);
}

void
main_loop() {
  __WFE();
}
