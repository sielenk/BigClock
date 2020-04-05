/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define OSC32_IN_Pin GPIO_PIN_14
#define OSC32_IN_GPIO_Port GPIOC
#define OSC32_OUT_Pin GPIO_PIN_15
#define OSC32_OUT_GPIO_Port GPIOC
#define OSC_IN_Pin GPIO_PIN_0
#define OSC_IN_GPIO_Port GPIOD
#define OSC_OUT_Pin GPIO_PIN_1
#define OSC_OUT_GPIO_Port GPIOD
#define BDC_A_Pin GPIO_PIN_0
#define BDC_A_GPIO_Port GPIOA
#define BCD_B_Pin GPIO_PIN_1
#define BCD_B_GPIO_Port GPIOA
#define BCD_C_Pin GPIO_PIN_2
#define BCD_C_GPIO_Port GPIOA
#define BCD_D_Pin GPIO_PIN_3
#define BCD_D_GPIO_Port GPIOA
#define SEG_H10_Pin GPIO_PIN_4
#define SEG_H10_GPIO_Port GPIOA
#define SEG_H01_Pin GPIO_PIN_5
#define SEG_H01_GPIO_Port GPIOA
#define SEG_M10_Pin GPIO_PIN_6
#define SEG_M10_GPIO_Port GPIOA
#define SEG_M01_Pin GPIO_PIN_7
#define SEG_M01_GPIO_Port GPIOA
#define LIGHT_LEVEL_Pin GPIO_PIN_0
#define LIGHT_LEVEL_GPIO_Port GPIOB
#define DCF_IN_Pin GPIO_PIN_1
#define DCF_IN_GPIO_Port GPIOB
#define LAMP_TEST_Pin GPIO_PIN_10
#define LAMP_TEST_GPIO_Port GPIOB
#define BLANK_Pin GPIO_PIN_11
#define BLANK_GPIO_Port GPIOB
#define MATRIX_NCS_Pin GPIO_PIN_12
#define MATRIX_NCS_GPIO_Port GPIOB
#define MATRIX_CLK_Pin GPIO_PIN_13
#define MATRIX_CLK_GPIO_Port GPIOB
#define MATRIX_DOUT_Pin GPIO_PIN_15
#define MATRIX_DOUT_GPIO_Port GPIOB
#define TX_Pin GPIO_PIN_9
#define TX_GPIO_Port GPIOA
#define RX_Pin GPIO_PIN_10
#define RX_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define SCL_Pin GPIO_PIN_6
#define SCL_GPIO_Port GPIOB
#define SDA_Pin GPIO_PIN_7
#define SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
