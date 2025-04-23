/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for Startup */
osThreadId_t StartupHandle;
const osThreadAttr_t Startup_attributes = {
  .name = "Startup",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for CLI */
osThreadId_t CLIHandle;
const osThreadAttr_t CLI_attributes = {
  .name = "CLI",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Screnn */
osThreadId_t ScrennHandle;
const osThreadAttr_t Screnn_attributes = {
  .name = "Screnn",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void startup_task(void *argument);
void cli_task(void *argument);
void screen_task(void *argument);

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
  /* creation of Startup */
  StartupHandle = osThreadNew(startup_task, NULL, &Startup_attributes);

  /* creation of CLI */
  CLIHandle = osThreadNew(cli_task, NULL, &CLI_attributes);

  /* creation of Screnn */
  ScrennHandle = osThreadNew(screen_task, NULL, &Screnn_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_startup_task */
/**
  * @brief  Function implementing the Startup thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_startup_task */
__weak void startup_task(void *argument)
{
  /* USER CODE BEGIN startup_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END startup_task */
}

/* USER CODE BEGIN Header_cli_task */
/**
* @brief Function implementing the CLI thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_cli_task */
__weak void cli_task(void *argument)
{
  /* USER CODE BEGIN cli_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END cli_task */
}

/* USER CODE BEGIN Header_screen_task */
/**
* @brief Function implementing the Screnn thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_screen_task */
__weak void screen_task(void *argument)
{
  /* USER CODE BEGIN screen_task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END screen_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

