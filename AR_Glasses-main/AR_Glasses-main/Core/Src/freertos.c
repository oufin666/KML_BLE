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
#include "../../Drivers/BSP/inc/mpu6050.h"
#if !MPU6050_USE_SOFT_I2C
#include "i2c.h"  // 只在硬件I2C模式下包含
#endif
#include "../../Drivers/BSP/inc/imu_filter.h"
#include "../../Drivers/BSP/inc/mahony_ahrs.h"
#include "../../Drivers/BSP/inc/beep.h"
#include "../../Drivers/BSP/inc/motor.h"
#include <string.h>
#include <stdio.h>

/* ==================== 宏定义 ==================== */
#define ENABLE_IMU_FILTER 0  // 0-关闭滤波，1-开启滤波
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
GPS_Data GPS_data_share; //别的任务可以拿到的GPS数据

GPS_Data GPS_data;
uint8_t buf[GPS_UART_BUF_SIZE];
uint8_t gps_data_change;

// BLE KML相关变量声明
extern StreamBufferHandle_t kml_stream_buf;
extern SemaphoreHandle_t kml_file_sem;
extern FIL SDFile;  // SDFile在fatfs.c中定义，这里声明为外部变量
extern FATFS SDFatFS;  // FatFS文件系统对象

// SD卡文件操作相关全局变量（声明为volatile确保中断中能正确读取）
volatile uint8_t sd_file_opened = 0;        // SD文件打开标志
volatile uint8_t kml_transfer_active = 0;   // KML传输激活标志

// 外部数据缓冲区变量声明（在usart.c中定义）
extern volatile uint8_t transfer_complete;
/* USER CODE END Variables */
/* Definitions for TaskMain */
osThreadId_t TaskMainHandle;
const osThreadAttr_t TaskMain_attributes = {
  .name = "TaskMain",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GPS_Task */
osThreadId_t GPS_TaskHandle;
const osThreadAttr_t GPS_Task_attributes = {
  .name = "GPS_Task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for HR_Task */
osThreadId_t HR_TaskHandle;
const osThreadAttr_t HR_Task_attributes = {
  .name = "HR_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for BLE_KML_Task */
osThreadId_t BLE_KML_TaskHandle;
const osThreadAttr_t BLE_KML_Task_attributes = {
  .name = "BLE_KML_Task",
  .stack_size = 2048 * 4,
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
void StartBLEKMLTask(void *argument);

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
  /* creation of TaskMain */
//   TaskMainHandle = osThreadNew(StartTaskMain, NULL, &TaskMain_attributes);

  /* creation of GPS_Task */
//   GPS_TaskHandle = osThreadNew(StartGPSTask, NULL, &GPS_Task_attributes);

  /* creation of HR_Task */
  HR_TaskHandle = osThreadNew(StartTaskHR, NULL, &HR_Task_attributes);

  /* creation of BLE_KML_Task */
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

	// 标记是否已初始化
	static uint8_t initialized = 0;
	
	/* Infinite loop */
	for (;;) {
		// 只初始化一次
		if (!initialized) {
			initialized = 1;
			
			// 尝试初始化KML文件
			const char* kml_file_path = "0:/track/HK100.kml";
			int result = parse_kml_file(kml_file_path);
			if (result == KML_OK) {
				filter_kml_lines(FILE_LINE_NAME, FILE_LINE_FILTERED_NAME);
				load_segment_to_global_buffer(FILE_LINE_NAME, 2);
			}
			
			// 初始化应用
			App_Init();
		}

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
				// 直接写入全局结构体（暂时不需要互斥锁，因为GPS功能未启用）
				memcpy(&GPS_data_share, &GPS_data, sizeof(GPS_Data));
				gps_data_change = 1;
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

	uint32_t last_led_tick = 0;
	uint32_t last_mpu_tick = 0;
	char tx_buffer[128];
	MPU6050_RawData_t raw_data;
	uint8_t mpu_initialized = 0;

#if !MPU6050_USE_SOFT_I2C
	// 硬件I2C模式：初始化I2C
	MX_I2C1_Init();
#endif

	// 初始化IMU滤波器和Mahony AHRS算法
#if ENABLE_IMU_FILTER
	IMU_Filter_Init(100.0f, 5.0f);  // 采样频率100Hz，截止频率5Hz
	Mahony_Init(100.0f);  // 采样频率100Hz

	sprintf(tx_buffer, "[System] IMU filters initialized\r\n");
	HAL_UART_Transmit(BLE_UART_HANDLE, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
#else
	sprintf(tx_buffer, "[System] IMU filters disabled\r\n");
	HAL_UART_Transmit(BLE_UART_HANDLE, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
#endif

	// 初始化蜂鸣器和电机
	BEEP_Init();
	MOTOR_Init();
	sprintf(tx_buffer, "[System] Beep and Motor initialized\r\n");
	HAL_UART_Transmit(BLE_UART_HANDLE, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);

	// 启动蜂鸣器和电机
	BEEP_On();
	MOTOR_SetSpeed(50);  // 电机速度50%
	sprintf(tx_buffer, "[System] Beep and Motor started\r\n");
	HAL_UART_Transmit(BLE_UART_HANDLE, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);

	// 执行一次性测试
/*
#if MPU6050_USE_SOFT_I2C
	uint8_t test_result = MPU6050_Test(NULL, BLE_UART_HANDLE, &raw_data);  // 软件I2C模式传NULL
#else
	uint8_t test_result = MPU6050_Test(&hi2c1, BLE_UART_HANDLE, &raw_data);  // 硬件I2C模式传hi2c1
#endif
	
	// 输出测试结果
	switch (test_result) {
	case 0:
		sprintf(tx_buffer, "[MPU6050] Test completed successfully!\r\n");
		mpu_initialized = 1;
		break;
	case 1:
		sprintf(tx_buffer, "[MPU6050] Test failed: Initialization error\r\n");
		break;
	case 2:
		sprintf(tx_buffer, "[MPU6050] Test failed: Data read error\r\n");
		break;
	default:
		sprintf(tx_buffer, "[MPU6050] Test failed: Unknown error\r\n");
		break;
	}
	HAL_UART_Transmit(BLE_UART_HANDLE, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
*/

	// 主循环：LED闪烁和PWM更新
	for (;;) {
		// LED 闪烁（1秒一次）
		if (HAL_GetTick() - last_led_tick > 1000) {
			last_led_tick = HAL_GetTick();
			HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);		
		}
		
		// 持续更新蜂鸣器PWM（4kHz，占空比50%）
		BEEP_Update();
		
		// 持续更新电机PWM（100Hz，占空比50%）
		MOTOR_Update();
		
		/*
		// 每500毫秒读取一次MPU6050数据（如果初始化成功）
		if (mpu_initialized && (HAL_GetTick() - last_mpu_tick > 500)) {
			last_mpu_tick = HAL_GetTick();
			
		#if MPU6050_USE_SOFT_I2C
			if (MPU6050_GetRawData(NULL, &raw_data) == 0) {  // 软件I2C模式传NULL
		#else
			if (MPU6050_GetRawData(&hi2c1, &raw_data) == 0) {  // 硬件I2C模式传hi2c1
		#endif
				sprintf(tx_buffer, "Accel: X=%6d Y=%6d Z=%6d | Gyro: X=%6d Y=%6d Z=%6d\r\n",
					raw_data.accel_x, raw_data.accel_y, raw_data.accel_z,
					raw_data.gyro_x, raw_data.gyro_y, raw_data.gyro_z);
				HAL_UART_Transmit(BLE_UART_HANDLE, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
				
				// 处理IMU数据并计算姿态角
			#if ENABLE_IMU_FILTER
			// 处理IMU数据：去偏置 + 滤波 + 单位转换
			// 加速度计：±8g，16位，灵敏度 = 8g / 32768 = 0.000244 g/LSB = 0.002394 m/s²/LSB
			// 陀螺仪：±1000°/s，16位，灵敏度 = 1000°/s / 32768 = 0.0305 °/s/LSB = 0.00533 rad/s/LSB
			IMU_ProcessRawData(raw_data.accel_x, raw_data.accel_y, raw_data.accel_z,
			                      raw_data.gyro_x, raw_data.gyro_y, raw_data.gyro_z,
			                      0.002394f, 0.005333f);
			
			// 更新Mahony AHRS算法
			Mahony_UpdateIMU(gyro_vector.x, gyro_vector.y, gyro_vector.z,
			                    acc_vector.x, acc_vector.y, acc_vector.z);
			
			// 计算姿态角
			Mahony_ComputeAngles();
			
			// 打印姿态角（度）
			sprintf(tx_buffer, "Attitude: Roll=%6.2f Pitch=%6.2f Yaw=%6.2f (deg)\r\n",
				Mahony_GetRoll(), Mahony_GetPitch(), Mahony_GetYaw());
			HAL_UART_Transmit(BLE_UART_HANDLE, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
		#else
			// 不使用滤波，直接从原始数据计算简单姿态角
			// 仅使用加速度计数据计算Roll和Pitch
			float ax = raw_data.accel_x * 0.002394f;
			float ay = raw_data.accel_y * 0.002394f;
			float az = raw_data.accel_z * 0.002394f;
			
			// 计算Roll和Pitch（弧度）
			float roll = atan2(ay, az) * 57.2957795f;
			float pitch = atan2(-ax, sqrt(ay*ay + az*az)) * 57.2957795f;
			float yaw = 0.0f;  // 仅使用加速度计无法计算Yaw
			
			// 打印简单姿态角（度）
			sprintf(tx_buffer, "Simple Attitude: Roll=%6.2f Pitch=%6.2f Yaw=%6.2f (deg)\r\n",
				roll, pitch, yaw);
			HAL_UART_Transmit(BLE_UART_HANDLE, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
		#endif
			} else {
				sprintf(tx_buffer, "[MPU6050] Read Error!\r\n");
				HAL_UART_Transmit(BLE_UART_HANDLE, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
			}
		}
		*/
		
		vTaskDelay(pdMS_TO_TICKS(1)); // 减少延时，提高PWM更新频率
	}


  /* USER CODE END StartTaskHR */
}

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
  #if KML_ENABLE_JSON_MODE
  const char* file_path = "0:/kml/data.json";  // JSON模式：存储JSON字符串
  #else
  const char* file_path = "0:/kml/compressed_kml.bin";  // BIN模式：存储压缩数据
  #endif
  
  // 调用封装的BLE KML任务主函数
  kml_ble_task_main(file_path, &sd_file_opened, &kml_transfer_active, &transfer_complete);
  
  /* USER CODE END StartBLEKMLTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

