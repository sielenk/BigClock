/*
 * main2.cpp
 *
 *  Created on: Apr 4, 2020
 *      Author: Marvin Sielenkemper
 */

#include "cmsis_os.h"
#include "spi.h"
#include "assert.h"

extern const uint8_t font[8][8];

namespace {
const uint8_t rowCount = 8;
const uint8_t chipCount = 4;
const uint16_t no_decode[] = { 0x0900, 0x0900, 0x0900, 0x0900, };
const uint16_t normal_op[] = { 0x0c01, 0x0c01, 0x0c01, 0x0c01, };
const uint16_t test_off[] = { 0x0f00, 0x0f00, 0x0f00, 0x0f00, };
const uint16_t no_blank[] = { 0x0bff, 0x0bff, 0x0bff, 0x0bff, };

const unsigned char message[] = "    Hallo!  Hurrah, eine Laufschrift...    ";

osThreadId_t threadId = nullptr;
const uint32_t spiDmaDoneFlag = 1;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi == &hspi1) {
		osThreadFlagsSet(threadId, spiDmaDoneFlag);
		threadId = nullptr;
	}
}

namespace {

#pragma pack(push,1)

struct Cell {
	uint8_t data;
	int:8;
} framebuffer[rowCount][chipCount];

#pragma pack(pop)

void send(const uint16_t *data, uint8_t count) {
	HAL_GPIO_WritePin(NCS_GPIO_Port, NCS_Pin, GPIO_PIN_RESET);
	threadId = osThreadGetId();
	osThreadFlagsClear(spiDmaDoneFlag);
	HAL_SPI_Transmit_DMA(&hspi1, (uint8_t*) data, count);
	osThreadFlagsWait(spiDmaDoneFlag, osFlagsWaitAny, osWaitForever);
	HAL_GPIO_WritePin(NCS_GPIO_Port, NCS_Pin, GPIO_PIN_SET);
}

void sendRow(const Cell *rowData, uint8_t rowSize) {
	send(reinterpret_cast<const uint16_t*>(rowData), rowSize);
}

void sendRows(const Cell *rowData, int rowCount, int rowSize) {
	for (int row = 0; row < rowCount; ++row) {
		sendRow(rowData + row * rowSize, rowSize);
	}
}

void sendFrame() {
	sendRows(framebuffer[0], rowCount, chipCount);
}

}

extern "C" void mainTaskFunc(void *argument) {
	for (int i = 0; i < chipCount; ++i) {
		for (int r = 0; r < rowCount; ++r) {
			reinterpret_cast<uint16_t&>(framebuffer[r][i]) = (r + 1) << 8;
		}
	}

	send(test_off, chipCount);
	send(normal_op, chipCount);
	send(no_blank, chipCount);
	send(no_decode, chipCount);

	for (;;) {
		for (unsigned int offset = 0;
				offset < (sizeof(message) - chipCount - 1) * 8; ++offset) {
			const auto offsetBit = offset & 7;
			const auto offsetChar = offset / 8;

			for (int i = 0; i < chipCount; ++i) {
				auto const p1 = font[message[offsetChar + i]];
				auto const p2 = font[message[offsetChar + i + 1]];

				for (int r = 0; r < rowCount; ++r) {
					framebuffer[r][i].data = (p2[r] >> (8 - offsetBit))
							| (p1[r] << offsetBit);
				}
			}

			sendFrame();

			osDelay(50);
		}
	}
}
