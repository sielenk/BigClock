/*
 * dcf_timer.cpp
 *
 *  Created on: 13.04.2020
 *      Author: Marvin Sielenkemper
 */

#include "dcf_timer.hpp"

#include "tim.h"

#include "FreeRTOS.h"
#include "task.h"

enum {
  DCF_TIMER_SECOND_START = 1 << 16,
  DCF_TIMER_PULSE_START = 1 << 17,
  DCF_TIMER_PULSE_END = 1 << 18,

  DCF_TIMER_MASK = DCF_TIMER_SECOND_START | DCF_TIMER_PULSE_START
      | DCF_TIMER_PULSE_END
};

IDcfTimer *IDcfTimer::instancePtr { nullptr };
auto &dcfTimerHandle { htim3 };
auto &dcfTimer { *TIM3 };

IDcfTimer::~IDcfTimer() {
}

uint16_t
IDcfTimer::getPrescaler() const {
  return dcfTimer.PSC;
}

void
IDcfTimer::setPrescaler(uint16_t newPrescalerValue) {
  dcfTimer.PSC = newPrescalerValue;
}

constexpr uint32_t secondsPerDay { 24 * 60 * 60 };

namespace {
  struct Sample {
    uint32_t secondOfDay;
    uint16_t tick;
  };

  TaskHandle_t dcfTimerTaskHandle;

  int16_t phaseAdjustment { 0 };
  uint32_t secondOfDay { (12 * 60 + 34) * 60 + 56 };

  Sample sampledPulseStart;
  Sample sampledPulseEnd;
}

void
IDcfTimer::setSecondOfDay(uint32_t updatedSecondOfDay) {
  secondOfDay = updatedSecondOfDay;
}

void
IDcfTimer::adjustPhase(int16_t newPhaseAdjustment) {
  phaseAdjustment = newPhaseAdjustment;
}

void
HAL_TIM3_PeriodElapsedCallback() {
// This is triggered by the counter being updated after reaching its maximum value.
  BaseType_t xHigherPriorityTaskWoken { pdFALSE };

  secondOfDay = (secondOfDay + 1) % secondsPerDay;

  xTaskNotifyFromISR(dcfTimerTaskHandle, DCF_TIMER_SECOND_START, eSetBits,
                     &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void
HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &dcfTimerHandle) {
    switch (htim->Channel) {
      case HAL_TIM_ACTIVE_CHANNEL_1:
        // This is triggered by the output compare at the 50% point.
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

        if (phaseAdjustment != 0) {
          dcfTimer.CNT += phaseAdjustment;
          phaseAdjustment = 0;
        }
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
  if (htim == &dcfTimerHandle) {
    BaseType_t xHigherPriorityTaskWoken { pdFALSE };

    switch (htim->Channel) {
      case HAL_TIM_ACTIVE_CHANNEL_3: {
        // This is the end of a 100/200ms time pulse.
        sampledPulseEnd.secondOfDay = secondOfDay;
        sampledPulseEnd.tick = dcfTimer.CCR3;
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        xTaskNotifyFromISR(dcfTimerTaskHandle, DCF_TIMER_PULSE_END, eSetBits,
                           &xHigherPriorityTaskWoken);
        break;
      }

      case HAL_TIM_ACTIVE_CHANNEL_4: {
        // This is the start of a second as broadcast by the DCF sender.
        sampledPulseStart.secondOfDay = secondOfDay;
        sampledPulseStart.tick = dcfTimer.CCR4;
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        xTaskNotifyFromISR(dcfTimerTaskHandle, DCF_TIMER_PULSE_START, eSetBits,
                           &xHigherPriorityTaskWoken);
        break;
      }

      default:
        break;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void
dcfTimerFunc(void*) {
  dcfTimerTaskHandle = xTaskGetCurrentTaskHandle();

  HAL_GPIO_WritePin(LAMP_TEST_GPIO_Port, LAMP_TEST_Pin, GPIO_PIN_SET);

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

  HAL_TIM_Base_Start_IT(&dcfTimerHandle);
  HAL_TIM_OC_Start_IT(&dcfTimerHandle, TIM_CHANNEL_1);
  HAL_TIM_OC_Start_IT(&dcfTimerHandle, TIM_CHANNEL_2);
  HAL_TIM_IC_Start_IT(&dcfTimerHandle, TIM_CHANNEL_3);
  HAL_TIM_IC_Start_IT(&dcfTimerHandle, TIM_CHANNEL_4);

  for (;;) {
    uint32_t notifiedValue;
    xTaskNotifyWait(0, DCF_TIMER_MASK, &notifiedValue, portMAX_DELAY);

    if (auto const p = IDcfTimer::instancePtr) {
      auto &dcfTimer { *p };

      if (notifiedValue & DCF_TIMER_SECOND_START) {
        dcfTimer.onSecondStart(secondOfDay);
      }

      if (notifiedValue & DCF_TIMER_PULSE_START) {
        dcfTimer.onPulse(true, sampledPulseStart.secondOfDay,
                         sampledPulseStart.tick);
      }

      if (notifiedValue & DCF_TIMER_PULSE_END) {
        dcfTimer.onPulse(false, sampledPulseEnd.secondOfDay,
                         sampledPulseEnd.tick);
      }
    }
  }
}
