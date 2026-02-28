/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stream_buffer.h"
#include "LCD.h"
#include "../../lvgl.h"
#include "lv_port.h"
#include "GPS.h"
#include "delay.h"
#include "usart.h"
#include "ff.h"  // ★新增：引入FatFS头文件，方便操作SD卡


/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

// ★流式写入：KML文件存储路径（SD卡根目录下的kml文件夹，文件名为recv.kml）
#define KML_FILE_PATH     "0:/kml/recv.kml"

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
#define LOG    1 //如果LOG = 1则启用LOG调试，使用串口2用printf输出
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_DC_Pin GPIO_PIN_4
#define LCD_DC_GPIO_Port GPIOA
#define LCD_SCL_Pin GPIO_PIN_5
#define LCD_SCL_GPIO_Port GPIOA
#define LCD_SDA_Pin GPIO_PIN_7
#define LCD_SDA_GPIO_Port GPIOA
#define LCD_CS_Pin GPIO_PIN_0
#define LCD_CS_GPIO_Port GPIOB
#define LCD_BLK_Pin GPIO_PIN_1
#define LCD_BLK_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_12
#define LED_GPIO_Port GPIOB
#define HR_INT_Pin GPIO_PIN_13
#define HR_INT_GPIO_Port GPIOB
#define HR_INT_EXTI_IRQn EXTI15_10_IRQn
#define LCD_RES_Pin GPIO_PIN_14
#define LCD_RES_GPIO_Port GPIOB
#define HR_RESET_Pin GPIO_PIN_12
#define HR_RESET_GPIO_Port GPIOA
#define EC_KEY_Pin GPIO_PIN_3
#define EC_KEY_GPIO_Port GPIOB
#define EC_KEY_EXTI_IRQn EXTI3_IRQn
#define EC_A_Pin GPIO_PIN_6
#define EC_A_GPIO_Port GPIOB
#define EC_A_EXTI_IRQn EXTI9_5_IRQn
#define EC_B_Pin GPIO_PIN_7
#define EC_B_GPIO_Port GPIOB
#define HR_SDA_Pin GPIO_PIN_8
#define HR_SDA_GPIO_Port GPIOB
#define HR_SCL_Pin GPIO_PIN_9
#define HR_SCL_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define  GPS_USART  USART2

#define  GPS_UART_HANDLE    &huart2
#define  BLE_UART_HANDLE    &huart1

extern SemaphoreHandle_t gps_mutex;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
