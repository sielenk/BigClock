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

IDcfTimer *IDcfTimer::instancePtr { nullptr };
auto &dcfTimerHandle { htim3 };

void
IDcfTimer::updatePrescaler(const std::function<uint32_t
(uint32_t)> &updater) {
  auto &timer { *dcfTimerHandle.Instance };

  timer.PSC = updater(timer.PSC);
}

uint32_t
IDcfTimer::getPrescaler() const {
  return dcfTimerHandle.Instance->PSC;
}

namespace {
  TaskHandle_t dcfTimerTaskHandle;
  uint32_t sampledPulseStart;
  uint32_t sampledPulseEnd;
}

void
HAL_TIM3_PeriodElapsedCallback() {
// This is triggered by the counter being updated after reaching its maximum value.
  BaseType_t xHigherPriorityTaskWoken { pdFALSE };
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
    auto &instance { *htim->Instance };
    BaseType_t xHigherPriorityTaskWoken { pdFALSE };

    switch (htim->Channel) {
      case HAL_TIM_ACTIVE_CHANNEL_3: {
        // This is the end of a 100/200ms time pulse.
        sampledPulseEnd = instance.CCR3;
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        xTaskNotifyFromISR(dcfTimerTaskHandle, DCF_TIMER_PULSE_END, eSetBits,
                           &xHigherPriorityTaskWoken);
        break;
      }

      case HAL_TIM_ACTIVE_CHANNEL_4: {
        // This is the start of a second as broadcast by the DCF sender.
        sampledPulseStart = instance.CCR4;
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
        dcfTimer.onSecondStart();
      }

      if (notifiedValue & DCF_TIMER_PULSE_START) {
        dcfTimer.onPulse(true, sampledPulseStart);
      }

      if (notifiedValue & DCF_TIMER_PULSE_END) {
        dcfTimer.onPulse(false, sampledPulseEnd);
      }
    }
  }
}
