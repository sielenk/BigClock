/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtc.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

typedef struct {
	unsigned int zero :1;
	unsigned int :14;
	unsigned int call :1;
	unsigned int changeMezMesz :1;
	unsigned int isMesz :1;
	unsigned int isMez :1;
	unsigned int changeSec :1;
	unsigned int one :1;
	unsigned int minute01 :4;
	unsigned int minute10 :3;
	unsigned int minuteP :1;
	unsigned int hour01 :4;
	unsigned int hour10 :2;
	unsigned int hourP :1;
	unsigned int day01 :4;
	unsigned int day10 :2;
	unsigned int weekday :3;
	unsigned int month01 :4;
	unsigned int month10 :1;
	unsigned int year01 :4;
	unsigned int year10 :4;
	unsigned int dateP :1;
}__attribute__((packed)) DCF;

typedef struct {
	unsigned int zero :1;
	unsigned int :19;
	unsigned int one :1;
	unsigned int minute :8;
	unsigned int hour :7;
	unsigned int date :23;
}__attribute__((packed)) DCFCheck;

static int parity(int n) {
	n ^= n >> 1;
	n ^= n >> 2;
	n ^= n >> 4;
	n ^= n >> 8;
	n ^= n >> 16;

	return n & 1;
}

static void handleTelegram(DCF* dcf) {
	DCFCheck* check = (DCFCheck*) dcf;

	if (!check->zero && check->one && !parity(check->minute)
			&& !parity(check->hour) && !parity(check->date)) {
		RTC_TimeTypeDef sTime = { 0 };
		RTC_DateTypeDef sDate = { 0 };

		sTime.Hours = 10 * dcf->hour10 + dcf->hour01;
		sTime.Minutes = 10 * dcf->minute10 + dcf->minute01;
		sDate.Date = 10 * dcf->day10 + dcf->day01;
		sDate.Month = 10 * dcf->month10 + dcf->month01;
		sDate.Year = 10 * dcf->year10 + dcf->year01;
		sDate.WeekDay = dcf->weekday % 7;

		HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
		HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
	}
}

#define BUFFER_SIZE 65

static void addBit(unsigned short pulse, unsigned short delta) {
	static unsigned short buffer[BUFFER_SIZE] = { 0 };
	static unsigned short* p = buffer;

	const int pulseOk = (50 <= pulse && pulse <= 250);
	const int deltaOk1 = (950 <= delta && delta <= 1050);
	const int deltaOk2 = (1950 <= delta && delta <= 2050);
	const int deltaOk = deltaOk1 | deltaOk2;
	const int isLast = deltaOk2;

	if (pulseOk && deltaOk) {
		*p++ = pulse;

		if (isLast) {
			const int count = p - buffer;
			p = buffer;

			if (count == 59) {
				union {
					unsigned int buffer[2];
					DCF dcf;
				} u = { 0 };

				for (int i = 0; i < 59; ++i) {
					if (buffer[i] > 150) {
						u.buffer[i / 32] |= 1u << (i % 32);
					}
				}

				handleTelegram(&u.dcf);
			}
		} else if (p > buffer + BUFFER_SIZE) {
			p = buffer;
		}
	} else {
		p = buffer;
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim3) {
		static unsigned short pulseStart;

		switch (htim->Channel) {
		case HAL_TIM_ACTIVE_CHANNEL_4: {
			const unsigned short pulseEnd = htim->Instance->CCR3;
			const unsigned short newPulseStart = htim->Instance->CCR4;
			addBit(pulseEnd - pulseStart, newPulseStart - pulseStart);
			pulseStart = newPulseStart;
			break;
		}
		default:
			break;
		}
	}
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_RTC_Init();
	MX_TIM3_Init();
	/* USER CODE BEGIN 2 */
	HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_3);
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_4);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		const int x = HAL_GPIO_ReadPin(DCF_IN2_GPIO_Port, DCF_IN2_Pin);
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, x);

		//ITM_SendChar('x');
		//ITM_SendChar('\n');
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE
			| RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
