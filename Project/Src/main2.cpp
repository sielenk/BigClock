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
#include "rtc.h"

namespace {
  unsigned short pulseStart;
  unsigned short minuteStart;

  void
  adjustTimer(int error);
}

void
HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim3) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    } else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
      const unsigned short oldPulseStart = pulseStart;
      const unsigned short oldPulseEnd = htim->Instance->CCR3;
      const unsigned short newPulseStart = htim->Instance->CCR4;

      pulseStart = newPulseStart;
      dcf_addBit(oldPulseEnd - oldPulseStart, newPulseStart - oldPulseStart);

      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    }
  }
}

void
dcf_handleTelegram(DCF const *dcf) {
  const unsigned short newMinuteStart = htim3.Instance->CNT - 100;
  const unsigned short oldMinuteStart = minuteStart;
  const unsigned short minuteLength = newMinuteStart - oldMinuteStart;
  const int minuteError = minuteLength - 60000;

  minuteStart = newMinuteStart;

  if (dcf) {
    const uint8_t hours = 10 * dcf->hour10 + dcf->hour01;
    const uint8_t minutes = 10 * dcf->minute10 + dcf->minute01;
    RTC_TimeTypeDef sTime = { 0 };
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    if (sTime.Hours != hours || sTime.Minutes != minutes) {
      sTime.Hours = hours;
      sTime.Minutes = minutes;

      HAL_RTCEx_DeactivateSecond(&hrtc);
      HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
      HAL_RTCEx_SetSecond_IT(&hrtc);
    }
  }

  if (-600 <= minuteError && minuteError <= 600) {
    adjustTimer(minuteError);
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
HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc) {
  static int test = 0;

  const unsigned short milliSecond = htim3.Instance->CNT - minuteStart;

  RTC_TimeTypeDef sTime = { 0 };
  HAL_RTC_GetTime(hrtc, &sTime, RTC_FORMAT_BIN);

  const int h10 = (test ? (milliSecond / 1000 % 10) : (sTime.Hours / 10)) % 10;
  const int h01 = (test ? (milliSecond / 100 % 10) : (sTime.Hours)) % 10;
  const int m10 = (test ? (milliSecond / 10 % 10) : (sTime.Minutes / 10)) % 10;
  const int m01 = (test ? (milliSecond / 1 % 10) : (sTime.Minutes)) % 10;

  writeSegment(SEG_H10_MASK, h10);
  writeSegment(SEG_H01_MASK, h01);
  writeSegment(SEG_M10_MASK, m10);
  writeSegment(SEG_M01_MASK, m01);
}

namespace {
  void
  adjustTimer(int error) {

  }
}

void
main_initialize() {
  HAL_GPIO_WritePin(LAMP_TEST_GPIO_Port, LAMP_TEST_Pin, GPIO_PIN_SET);
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_3);
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_4);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
  HAL_RTCEx_SetSecond_IT(&hrtc);
}

void
main_loop() {
  __WFE();
}
