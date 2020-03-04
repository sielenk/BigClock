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
  if (htim == &htim3 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

    const unsigned short pulseEnd = htim->Instance->CCR3;
    const unsigned short newPulseStart = htim->Instance->CCR4;

    dcf_addBit(pulseEnd - pulseStart, newPulseStart - pulseStart);
    pulseStart = newPulseStart;
  }
}

void
dcf_handleTelegram(DCF *dcf) {
  if (dcf) {
    RTC_TimeTypeDef sTime = { 0 };
    RTC_DateTypeDef sDate = { 0 };

    sTime.Hours = 10 * dcf->hour10 + dcf->hour01;
    sTime.Minutes = 10 * dcf->minute10 + dcf->minute01;
    sDate.Date = 10 * dcf->day10 + dcf->day01;
    sDate.Month = 10 * dcf->month10 + dcf->month01;
    sDate.Year = 10 * dcf->year10 + dcf->year01;
    sDate.WeekDay = dcf->weekday % 7;

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  }

  const unsigned short newMinuteStart = htim3.Instance->CNT;
  const unsigned short minuteLength = newMinuteStart - minuteStart;
  const int minuteError = minuteLength - 60000;

  if (-600 <= minuteError && minuteError <= 600) {
    adjustTimer(minuteError);
  }

  minuteStart = newMinuteStart;
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
  static int counter = 0;
  static int test = 0;

  ++counter;

  //HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

  RTC_TimeTypeDef sTime = { 0 };

  HAL_RTC_GetTime(hrtc, &sTime, RTC_FORMAT_BIN);

  const int h10 = (test ? (counter + 0) : (sTime.Hours / 10)) % 10;
  const int h01 = (test ? (counter + 1) : (sTime.Hours)) % 10;
  const int m10 = (test ? (counter + 2) : (sTime.Minutes / 10)) % 10;
  const int m01 = (test ? (counter + 3) : (sTime.Minutes)) % 10;

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
  HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_4);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
  HAL_RTCEx_SetSecond_IT(&hrtc);

  htim2.Instance->CCR4 = 0xffff;
}

void
main_loop() {
  //const int x = HAL_GPIO_ReadPin(DCF_IN_GPIO_Port, DCF_IN_Pin);
  //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, x);
  //ITM_SendChar('x');
  //ITM_SendChar('\n');
  __WFE();
}
