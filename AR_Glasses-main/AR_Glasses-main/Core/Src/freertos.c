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

#include "ff.h"
#include "fatfs.h"
#include "GPS.h"
#include "gh_demo.h"
#include "testPage.h"
#include "APP.h"
#include "kml_parse_v2.h"
#include "../../Drivers/BSP/inc/kml_utils.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */



/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* USER CODE BEGIN PV */
// 外部变量声明
extern StreamBufferHandle_t kml_stream_buf;
extern SemaphoreHandle_t kml_file_sem;
extern FIL SDFile;  // SDFile在fatfs.c中定义，这里声明为外部变量
extern FATFS SDFatFS;  // FatFS文件系统对象

// SD卡文件操作相关全局变量（声明为volatile确保中断中能正确读取）
volatile uint8_t sd_file_opened = 0;        // SD文件打开标志
volatile uint8_t kml_transfer_active = 0;   // KML传输激活标志

// 外部数据缓冲区变量声明（在usart.c中定义）
extern volatile uint8_t transfer_complete;
/* USER CODE END PV */

/* USER CODE BEGIN Variables */
GPS_Data GPS_data_share; //别的任务可以拿到的GPS数据

GPS_Data GPS_data;
uint8_t buf[GPS_UART_BUF_SIZE];
uint8_t gps_data_change;


/* USER CODE END Variables */
/* Definitions for TaskMain */
osThreadId_t TaskMainHandle;
const osThreadAttr_t TaskMain_attributes = {
  .name = "TaskMain",
  .stack_size = 4096 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GPS_Task */
osThreadId_t GPS_TaskHandle;
const osThreadAttr_t GPS_Task_attributes = {
  .name = "GPS_Task",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for HR_Task */
osThreadId_t HR_TaskHandle;
const osThreadAttr_t HR_Task_attributes = {
  .name = "HR_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
// ★新增：BLE KML任务的句柄和属性
osThreadId_t BLE_KML_TaskHandle;
const osThreadAttr_t BLE_KML_Task_attributes = {
  .name = "BLE_KML_Task",
  .stack_size = 2048 * 4,  // 足够处理文件操作的栈大小
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
// ★新增：声明BLE KML任务函数
void StartBLEKMLTask(void *argument);

/* USER CODE END FunctionPrototypes */

void StartTaskMain(void *argument);
void StartGPSTask(void *argument);
void StartTaskHR(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationTickHook(void);

/* USER CODE BEGIN 3 */
void vApplicationTickHook(void) {

	lv_tick_inc(1);

	/* This function will be called by each tick interrupt if
	 configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be
	 added here, but the tick hook is called from an interrupt context, so
	 code must not attempt to block, and only the interrupt safe FreeRTOS API
	 functions can be used (those that end in FromISR()). */
}
/* USER CODE END 3 */

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
  // /* creation of TaskMain */
  // TaskMainHandle = osThreadNew(StartTaskMain, NULL, &TaskMain_attributes);

  // /* creation of GPS_Task */
  // GPS_TaskHandle = osThreadNew(StartGPSTask, NULL, &GPS_Task_attributes);

  /* creation of HR_Task */
  HR_TaskHandle = osThreadNew(StartTaskHR, NULL, &HR_Task_attributes);

  // ★新增：创建BLE KML任务
  BLE_KML_TaskHandle = osThreadNew(StartBLEKMLTask, NULL, &BLE_KML_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartTaskMain */
/**
 * @brief  Function implementing the TaskMain thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskMain */
void StartTaskMain(void *argument)
{
  /* USER CODE BEGIN StartTaskMain */
	lv_init();
	lv_port_init();
	/*
	lv_obj_t * img1 = lv_img_create(lv_scr_act());
	 const char *img_path = "S:/img/27.bin";
	lv_img_set_src(img1,img_path);

	 lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
*/
//	lv_example_hr_display();
//	lvgl_st7789_test_page_create();
//	App_Init();

	const char* kml_file_path = "0:/track/HK100.kml";
	parse_kml_file(kml_file_path);
	filter_kml_lines(FILE_LINE_NAME, FILE_LINE_FILTERED_NAME);
	load_segment_to_global_buffer(FILE_LINE_NAME,2);
	App_Init();
	/* Infinite loop */
	for (;;) {
/*
		if (xSemaphoreTake(gps_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {

			if(gps_data_change == 1){
				gps_data_change = 0;
				update_gps_display(lv_scr_act(), &GPS_data_share);
			}
			// 释放互斥锁（必须执行，否则其他任务会阻塞）
			xSemaphoreGive(gps_mutex);
		}
*/

		lv_timer_handler();
		vTaskDelay(pdMS_TO_TICKS(5));
	}
  /* USER CODE END StartTaskMain */
}

/* USER CODE BEGIN Header_StartGPSTask */
/**
 * @brief Function implementing the GPS_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartGPSTask */
void StartGPSTask(void *argument)
{
  /* USER CODE BEGIN StartGPSTask */

	GPS_Init();
	gps_data_change = 0;
	/* Infinite loop */
	for (;;) {
		if (UART_ReadFrame(buf) == 1) {

			if (GPS_Parse((const char*) buf, &GPS_data) == 1) {
				if (xSemaphoreTake(gps_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
					// 安全写入全局结构体
					memcpy(&GPS_data_share, &GPS_data, sizeof(GPS_Data));
					//GPS_data_share = GPS_data;
					gps_data_change = 1;
					// 释放互斥锁（必须执行，否则其他任务会阻塞）
					xSemaphoreGive(gps_mutex);
				}
			}
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}

  /* USER CODE END StartGPSTask */
}

/* USER CODE BEGIN Header_StartTaskHR */
/**
 * @brief Function implementing the HR_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskHR */
void StartTaskHR(void *argument)
{
  /* USER CODE BEGIN StartTaskHR */
//	while(Gh3x2xDemoInit()!=GH3X2X_RET_OK);
	/* Infinite loop */
	for (;;) {
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
  /* USER CODE END StartTaskHR */
}

// ★流式写入：BLE KML 任务 - 从串口接收数据并流式写入SD卡
/* USER CODE BEGIN Header_StartBLEKMLTask */
/**
 * @brief Function implementing the BLE_KML_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartBLEKMLTask */
void StartBLEKMLTask(void *argument)
{
  /* USER CODE BEGIN StartBLEKMLTask */
  const char* compressed_file = "0:/kml/compressed_kml.bin";  // 存储压缩数据
  
  // 调用封装的BLE KML任务主函数
  kml_ble_task_main(compressed_file, &sd_file_opened, &kml_transfer_active, &transfer_complete);
  
  /* USER CODE END StartBLEKMLTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
