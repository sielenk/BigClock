/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dcf_timer.hpp"
#include "matrix.hpp"
#include "main2.hpp"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for dcfTimerTask */
osThreadId_t dcfTimerTaskHandle;
uint32_t dcfTimerTaskBuffer[ 128 ];
osStaticThreadDef_t dcfTimerTaskControlBlock;
const osThreadAttr_t dcfTimerTask_attributes = {
  .name = "dcfTimerTask",
  .cb_mem = &dcfTimerTaskControlBlock,
  .cb_size = sizeof(dcfTimerTaskControlBlock),
  .stack_mem = &dcfTimerTaskBuffer[0],
  .stack_size = sizeof(dcfTimerTaskBuffer),
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for matrixTask */
osThreadId_t matrixTaskHandle;
uint32_t matrixTaskBuffer[ 256 ];
osStaticThreadDef_t matrixTaskControlBlock;
const osThreadAttr_t matrixTask_attributes = {
  .name = "matrixTask",
  .cb_mem = &matrixTaskControlBlock,
  .cb_size = sizeof(matrixTaskControlBlock),
  .stack_mem = &matrixTaskBuffer[0],
  .stack_size = sizeof(matrixTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void dcfTimerFunc(void *argument);
extern void matrixTaskFun(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of dcfTimerTask */
  dcfTimerTaskHandle = osThreadNew(dcfTimerFunc, NULL, &dcfTimerTask_attributes);

  /* creation of matrixTask */
  matrixTaskHandle = osThreadNew(matrixTaskFun, NULL, &matrixTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_dcfTimerFunc */
/**
  * @brief  Function implementing the dcfTimerTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_dcfTimerFunc */
__weak void dcfTimerFunc(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN dcfTimerFunc */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END dcfTimerFunc */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
