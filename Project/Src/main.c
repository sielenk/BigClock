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
#include "adc.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dcf.h"
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

static unsigned short pulseStart;
static unsigned short minuteStart;

static void adjustTimer(int error);

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim3 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

		const unsigned short pulseEnd = htim->Instance->CCR3;
		const unsigned short newPulseStart = htim->Instance->CCR4;

		dcf_addBit(pulseEnd - pulseStart, newPulseStart - pulseStart);
		pulseStart = newPulseStart;
	}
}

void dcf_handleTelegram(DCF* dcf) {
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

void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc) {
	static int counter = 0;
	static int test = 0;

	++counter;

	//HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

	RTC_TimeTypeDef sTime = { 0 };

	HAL_RTC_GetTime(hrtc, &sTime, RTC_FORMAT_BIN);

	const int h10 = (test ? (counter + 0) : (sTime.Hours / 10)) % 10; // (sTime.Hours / 10) % 10;
	const int h01 = (test ? (counter + 1) : sTime.Hours) % 10; // sTime.Hours % 10;
	const int m10 = (test ? (counter + 2) : (sTime.Minutes / 10)) % 10; // (sTime.Minutes / 10) % 10;
	const int m01 = (test ? (counter + 3) : sTime.Minutes) % 10; // sTime.Minutes % 10;

	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK | h10;
	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_H10_MASK | h10;
	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK | h10;
	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK;

	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK | h01;
	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_H01_MASK | h01;
	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK | h01;
	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK;

	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK | m10;
	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_M10_MASK | m10;
	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK | m10;
	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK;

	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK | m01;
	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_M01_MASK | m01;
	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK | m01;
	GPIOA->ODR = (GPIOA->ODR & ~0xff) | SEG_NONE_MASK;
}

void adjustTimer(int error) {

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
	MX_ADC1_Init();
	MX_I2C1_Init();
	MX_USB_PCD_Init();
	MX_USART1_UART_Init();
	MX_TIM2_Init();
	/* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(LAMP_TEST_GPIO_Port, LAMP_TEST_Pin, GPIO_PIN_SET);
	HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_3);
	HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_4);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
	HAL_RTCEx_SetSecond_IT(&hrtc);

	htim2.Instance->CCR4 = 0xffff;

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		//const int x = HAL_GPIO_ReadPin(DCF_IN_GPIO_Port, DCF_IN_Pin);
		//HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, x);
		//ITM_SendChar('x');
		//ITM_SendChar('\n');
		__WFE();
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
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_ADC
			| RCC_PERIPHCLK_USB;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
	PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
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