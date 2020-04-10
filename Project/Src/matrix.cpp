/*
 * matrix.cpp
 *
 *  Created on: Apr 5, 2020
 *      Author: Marv
 */

#include "matrix.hpp"

#include "font.h"
#include "spi.h"
#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

class SpiDmaLock {
  static const uint32_t spiDmaDoneFlag = 1;
  static SpiDmaLock *instance;

  SPI_HandleTypeDef &hspi;
  TaskHandle_t taskHandle;

public:
  static void
  finished(SPI_HandleTypeDef *pspi) {
    if (instance && pspi == &instance->hspi) {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      xTaskNotifyFromISR(instance->taskHandle, spiDmaDoneFlag, eSetBits,
                         &xHigherPriorityTaskWoken);
      instance = nullptr;
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }

  SpiDmaLock(SPI_HandleTypeDef &hspi) :
      hspi(hspi), taskHandle(xTaskGetCurrentTaskHandle()) {
    xTaskNotifyWait(0, spiDmaDoneFlag, nullptr, 0);
    instance = this;
  }

  ~SpiDmaLock() {
    xTaskNotifyWait(0, spiDmaDoneFlag, nullptr, portMAX_DELAY);
  }
};

SpiDmaLock *SpiDmaLock::instance = nullptr;

void
spiSend(SPI_HandleTypeDef &hspi, const uint16_t *data, uint8_t count) {
  HAL_GPIO_WritePin(MATRIX_NCS_GPIO_Port, MATRIX_NCS_Pin, GPIO_PIN_RESET);
  {
    SpiDmaLock lock(hspi);
    HAL_SPI_Transmit_DMA(&hspi, (uint8_t*)data, count);
  }
  HAL_GPIO_WritePin(MATRIX_NCS_GPIO_Port, MATRIX_NCS_Pin, GPIO_PIN_SET);
}

void
HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
  SpiDmaLock::finished(hspi);
}

template<uint8_t rowCount, uint8_t chipCount>
  class Framebuffer {
    SPI_HandleTypeDef &hspi;

#pragma pack(push,1)
    struct Cell {
      uint8_t data;
      int :8;
    } buffer[rowCount][chipCount];
#pragma pack(pop)

    void
    spiSend(const uint16_t *data) const {
      ::spiSend(hspi, data, chipCount);
    }

    void
    spiSend(uint16_t data) const {
      uint16_t buffer[chipCount];

      for (uint8_t chip = 0; chip < chipCount; ++chip) {
        buffer[chip] = data;
      }

      spiSend(buffer);
    }

  public:
    Framebuffer(SPI_HandleTypeDef &hspi) :
        hspi(hspi) {
      for (int i = 0; i < chipCount; ++i) {
        for (int r = 0; r < rowCount; ++r) {
          reinterpret_cast<uint16_t&>(buffer[r][i]) = (r + 1) << 8;
        }
      }

      spiSend(0xf00); // test: off
      spiSend(0xc01); // mode: normal
      spiSend(0xbff); // no blanking
      spiSend(0xa03); // Intensity 7/32
      spiSend(0x900); // no decode

      send(); // sync the fb memory to the matrix
    }

    uint8_t&
    operator()(uint8_t row, uint8_t chip) {
      return buffer[row][chip].data;
    }

    uint8_t
    operator()(uint8_t row, uint8_t chip) const {
      return const_cast<Framebuffer*>(this)->operator()(row, chip);
    }

    void
    send() const {
      for (int row = 0; row < rowCount; ++row) {
        spiSend(reinterpret_cast<const uint16_t*>(buffer[row]));
      }
    }
  };

void
matrixTaskFun(void*) {
  constexpr uint8_t rowCount = 8;
  constexpr uint8_t chipCount = 11;
  constexpr char message[] = "the quick brown fox jumps over the lazy dog. "
      "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG. ";
  constexpr size_t messageLength = sizeof(message) - 1;

  Framebuffer<rowCount, chipCount> fb(hspi2);

  auto lastWakeTime = xTaskGetTickCount();
  for (;;) {
    for (unsigned int offset = 0; offset < messageLength * 8; ++offset) {
      const auto offsetBit = offset & 7;
      const auto offsetChar = offset / 8;

      for (int i = 0; i < chipCount; ++i) {
        auto const p1 = font[message[(offsetChar + i) % messageLength]];
        auto const p2 = font[message[(offsetChar + i + 1) % messageLength]];

        for (int r = 0; r < rowCount; ++r) {
          fb(r, i) = (p2[r] >> (8 - offsetBit)) | (p1[r] << offsetBit);
        }
      }

      fb.send();

      vTaskDelayUntil(&lastWakeTime, 20);
    }
  }
}
