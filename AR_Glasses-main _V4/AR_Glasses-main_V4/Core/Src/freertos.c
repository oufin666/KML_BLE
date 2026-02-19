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
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

// 动态生成CRC32表（与小程序端相同的算法）
static uint32_t crc_table[256];

// 初始化CRC32表
void init_crc_table(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (uint32_t j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
        }
        crc_table[i] = crc;
    }
}

/**
 * 计算CRC32值（与小程序端一致）
 *
 * @param data 数据缓冲区
 * @param length 数据长度
 * @return CRC32值
 */
uint32_t calculate_crc32(const uint8_t* data, uint32_t length) {
    uint32_t crc = 0xFFFFFFFF;
    
    for (uint32_t i = 0; i < length; i++) {
        crc = (crc >> 8) ^ crc_table[(crc & 0xFF) ^ data[i]];
    }
    
    return crc ^ 0xFFFFFFFF;
}

/**
 * 验证CRC32值是否与小程序端一致
 *
 * @param data 数据缓冲区
 * @param length 数据长度
 * @param expected_crc 小程序端发送的CRC32值
 * @return 是否验证通过
 */
bool verify_crc32_with_expected(const uint8_t* data, uint32_t length, uint32_t expected_crc) {
    uint32_t calculated_crc = calculate_crc32(data, length);
    
    uint8_t verify_msg[150];
    sprintf((char*)verify_msg, "=== CRC32 Verification ===\r\nCalculated CRC: 0x%08lX\r\nExpected CRC: 0x%08lX\r\nMatch: %s\r\n", 
            calculated_crc, expected_crc, calculated_crc == expected_crc ? "YES" : "NO");
    HAL_UART_Transmit(&huart1, verify_msg, strlen((char*)verify_msg), 100);
    
    return calculated_crc == expected_crc;
}

/**
 * 处理接收到的数据并验证CRC32
 *
 * @param data 接收到的数据（包括结束符）
 * @param length 数据长度
 * @param expected_crc 小程序端发送的CRC32值
 * @return 是否验证通过
 */
bool verify_crc32(const uint8_t* data, uint32_t length, uint32_t expected_crc) {
    // 移除结束序列
    if (length >= 3 && data[length - 1] == 0xFF && data[length - 2] == 0xFF && data[length - 3] == 0xFF) {
        length -= 3;
    }
    
    // 计算CRC32
    uint32_t calculated_crc = calculate_crc32(data, length);
    
    // 验证CRC32
    return calculated_crc == expected_crc;
}

// 兼容旧的函数名
uint32_t calculateCRC32(uint8_t* data, uint32_t length) {
    return calculate_crc32(data, length);
}

// 测试CRC算法是否正确
void test_crc_algorithm(void) {
    // 测试数据："test data"
    uint8_t test_data[] = {'t', 'e', 's', 't', ' ', 'd', 'a', 't', 'a'};
    uint32_t crc = calculate_crc32(test_data, sizeof(test_data));
    
    uint8_t test_msg[100];
    sprintf((char*)test_msg, "=== CRC Algorithm Test ===\r\nTest data: \"test data\"\r\nCRC: 0x%08lX\r\n", crc);
    HAL_UART_Transmit(&huart1, test_msg, strlen((char*)test_msg), 100);
}

// 标签映射表（与小程序端相同）
static const char* tag_map[] = {
    NULL,                           // 0x80 未使用
    "<kml xmlns=\"http://www.opengis.net/kml/2.2\">",  // 0x81
    "</kml>",                       // 0x82
    "<Placemark>",                  // 0x83
    "</Placemark>",                 // 0x84
    "<name>",                       // 0x85
    "</name>",                      // 0x86
    "<description>",                // 0x87
    "</description>",               // 0x88
    "<Point>",                      // 0x89
    "</Point>",                     // 0x8A
    "<coordinates>",                // 0x8B
    "</coordinates>",               // 0x8C
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>",  // 0x8D
    "\r\n",                        // 0x8E
    "\n"                           // 0x8F
};

// 解压函数（基于坐标数据压缩算法）
// 与小程序端的压缩算法完全兼容
uint32_t decompressData(uint8_t* compressedData, uint32_t compressedSize, uint8_t* outputBuffer, uint32_t outputBufferSize) {
  if (compressedSize < 2 || outputBufferSize == 0) {
    return 0;
  }
  
  uint32_t inPos = 0;
  uint32_t outPos = 0;
  
  // 检查压缩标识
  if (compressedData[0] != 0x42 || compressedData[1] != 0x4C) {
    return 0; // 无效的压缩标识
  }
  
  // 跳过压缩标识
  inPos = 2;
  
  while (inPos < compressedSize && outPos < outputBufferSize) {
    uint8_t byte = compressedData[inPos++];
    
    if (byte == 0xfe) {
      // 处理坐标数据
      if (inPos + 2 > compressedSize) {
        return 0; // 数据长度不足
      }
      
      // 读取坐标数量（小端序）
      int coord_count = compressedData[inPos] | (compressedData[inPos + 1] << 8);
      inPos += 2;
      
      // 写入<coordinates>标签
      const char* start_tag = "<coordinates>";
      int start_tag_len = strlen(start_tag);
      if (outPos + start_tag_len > outputBufferSize) {
        return 0; // 解压缓冲区不足
      }
      memcpy(&outputBuffer[outPos], start_tag, start_tag_len);
      outPos += start_tag_len;
      
      // 处理每个坐标
      for (int i = 0; i < coord_count; i++) {
        if (inPos + 12 > compressedSize) {
          return 0; // 数据长度不足
        }
        
        // 读取坐标数据（小端序）
        int32_t lon_int = (int32_t)(compressedData[inPos] | (compressedData[inPos + 1] << 8) | (compressedData[inPos + 2] << 16) | (compressedData[inPos + 3] << 24));
        int32_t lat_int = (int32_t)(compressedData[inPos + 4] | (compressedData[inPos + 5] << 8) | (compressedData[inPos + 6] << 16) | (compressedData[inPos + 7] << 24));
        int32_t alt_int = (int32_t)(compressedData[inPos + 8] | (compressedData[inPos + 9] << 8) | (compressedData[inPos + 10] << 16) | (compressedData[inPos + 11] << 24));
        inPos += 12;
        
        // 转换回浮点数
        double lon = lon_int / 1000000.0;
        double lat = lat_int / 1000000.0;
        double alt = alt_int / 1000000.0;
        
        // 格式化坐标字符串
        char coord_str[64];
        int coord_str_len = snprintf(coord_str, sizeof(coord_str), "%.6f,%.6f,%.6f", lon, lat, alt);
        
        // 去除尾部多余的0，但至少保留一位小数
        char* dot_pos = strchr(coord_str, '.');
        if (dot_pos) {
          // 从末尾开始查找非零字符
          char* end_pos = coord_str + coord_str_len - 1;
          while (end_pos > dot_pos && *end_pos == '0') {
            end_pos--;
          }
          // 如果末尾是小数点，添加一个0
          if (end_pos == dot_pos) {
            end_pos++;
          }
          // 截断字符串
          end_pos++;
          *end_pos = ' '; // 添加空格分隔符
          coord_str_len = end_pos - coord_str + 1; // 包含空格
        }
        
        // 写入坐标数据
        if (outPos + coord_str_len > outputBufferSize) {
          return 0; // 解压缓冲区不足
        }
        memcpy(&outputBuffer[outPos], coord_str, coord_str_len);
        outPos += coord_str_len;
      }
      
      // 写入</coordinates>标签
      const char* end_tag = "</coordinates>";
      int end_tag_len = strlen(end_tag);
      if (outPos + end_tag_len > outputBufferSize) {
        return 0; // 解压缓冲区不足
      }
      memcpy(&outputBuffer[outPos], end_tag, end_tag_len);
      outPos += end_tag_len;
    } else if (byte < 0x80) {
      // 处理单字节UTF-8字符
      if (outPos >= outputBufferSize) {
        return 0; // 解压缓冲区不足
      }
      outputBuffer[outPos++] = byte;
    } else if (byte >= 0xc0 && byte < 0xe0) {
      // 处理双字节UTF-8字符
      if (inPos >= compressedSize) {
        return 0; // 数据长度不足
      }
      if (outPos + 2 > outputBufferSize) {
        return 0; // 解压缓冲区不足
      }
      outputBuffer[outPos++] = byte;
      outputBuffer[outPos++] = compressedData[inPos++];
    } else if (byte >= 0xe0 && byte < 0xf0) {
      // 处理三字节UTF-8字符
      if (inPos + 1 >= compressedSize) {
        return 0; // 数据长度不足
      }
      if (outPos + 3 > outputBufferSize) {
        return 0; // 解压缓冲区不足
      }
      outputBuffer[outPos++] = byte;
      outputBuffer[outPos++] = compressedData[inPos++];
      outputBuffer[outPos++] = compressedData[inPos++];
    } else if (byte >= 0xf0 && byte < 0xf8) {
      // 处理四字节UTF-8字符
      if (inPos + 2 >= compressedSize) {
        return 0; // 数据长度不足
      }
      if (outPos + 4 > outputBufferSize) {
        return 0; // 解压缓冲区不足
      }
      outputBuffer[outPos++] = byte;
      outputBuffer[outPos++] = compressedData[inPos++];
      outputBuffer[outPos++] = compressedData[inPos++];
      outputBuffer[outPos++] = compressedData[inPos++];
    } else if (byte >= 0x80 && byte < 0xc0) {
      // 处理UTF-8续字节
      if (outPos >= outputBufferSize) {
        return 0; // 解压缓冲区不足
      }
      outputBuffer[outPos++] = byte;
    } else {
      // 处理其他字节（错误的UTF-8字节等）
      if (outPos >= outputBufferSize) {
        return 0; // 解压缓冲区不足
      }
      outputBuffer[outPos++] = byte;
    }
  }
  
  return outPos; // 返回解压后的数据大小
}

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
  FRESULT res;
  // 定义缓冲区大小常量，用于限制每次写入的大小
  #define DECOMPRESS_BUFFER_SIZE 16384  // 16KB 缓冲区大小
  
  const char* test_file = "0:/test.txt";
  const char* kml_dir = "0:/kml";
  const char* compressed_file = "0:/kml/compressed_kml.bin";  // 存储压缩数据
  
  // 延时等待系统初始化完成（SD卡已在main中初始化）
  uint8_t wait_msg[] = "Waiting for SD init...\r\n";
  HAL_UART_Transmit(&huart1, wait_msg, strlen((char*)wait_msg), 100);
  vTaskDelay(pdMS_TO_TICKS(500));  // 缩短延迟时间

  // 发送启动消息
  uint8_t start_msg[] = "\r\n=== BLE KML Task Started ===\r\n";
  HAL_UART_Transmit(&huart1, start_msg, strlen((char*)start_msg), 100);
  
  // 初始化CRC32表
  init_crc_table();
  uint8_t crc_init_msg[] = "CRC32 table initialized\r\n";
  HAL_UART_Transmit(&huart1, crc_init_msg, strlen((char*)crc_init_msg), 100);
  
  // 测试CRC算法
  test_crc_algorithm();
  
  // 尝试挂载文件系统
  res = f_mount(&SDFatFS, "0:", 1);
  uint8_t mount_msg[50];
  sprintf((char*)mount_msg, "f_mount result: %d\r\n", res);
  HAL_UART_Transmit(&huart1, mount_msg, strlen((char*)mount_msg), 100);
  
  if (res != FR_OK) {
    uint8_t msg[] = "SD mount FAILED! Cannot continue.\r\n";
    HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
    // 进入死循环
    for(;;) { vTaskDelay(pdMS_TO_TICKS(1000)); }
  }
  
  uint8_t msg1[] = "SD mounted OK\r\n";
  HAL_UART_Transmit(&huart1, msg1, strlen((char*)msg1), 100);

  // === 测试1：读取根目录下的test.txt文件 ===
  uint8_t test_msg[] = "\r\n=== Test 1: Read test.txt ===\r\n";
  HAL_UART_Transmit(&huart1, test_msg, strlen((char*)test_msg), 100);
  
  FIL test_fil;
  res = f_open(&test_fil, test_file, FA_READ);
  uint8_t open_msg[50];
  sprintf((char*)open_msg, "f_open(test.txt) result: %d\r\n", res);
  HAL_UART_Transmit(&huart1, open_msg, strlen((char*)open_msg), 100);
  
  if (res == FR_OK) {
    uint8_t read_buf[100];
    UINT bytes_read;
    res = f_read(&test_fil, read_buf, sizeof(read_buf)-1, &bytes_read);
    f_close(&test_fil);
    
    if (res == FR_OK && bytes_read > 0) {
      read_buf[bytes_read] = '\0';  // 添加字符串结束符
      uint8_t msg[150];
      sprintf((char*)msg, "Read %d bytes: %s\r\n", bytes_read, read_buf);
      HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
    } else {
      uint8_t msg[] = "Read failed or empty file\r\n";
      HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
    }
  } else {
    uint8_t msg[] = "test.txt not found or cannot open\r\n";
    HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
  }

  // === 测试2：列出根目录文件 ===
  uint8_t test2_msg[] = "\r\n=== Test 2: List root directory ===\r\n";
  HAL_UART_Transmit(&huart1, test2_msg, strlen((char*)test2_msg), 100);
  
  DIR dir;
  FILINFO fno;
  res = f_opendir(&dir, "0:/");
  if (res == FR_OK) {
    uint8_t msg[] = "Root directory contents:\r\n";
    HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
    
    for (int i = 0; i < 10; i++) {  // 最多列出10个文件
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0) break;
      
      uint8_t file_msg[100];
      sprintf((char*)file_msg, "  %s (%lu bytes)\r\n", fno.fname, fno.fsize);
      HAL_UART_Transmit(&huart1, file_msg, strlen((char*)file_msg), 100);
    }
    f_closedir(&dir);
  } else {
    uint8_t msg[50];
    sprintf((char*)msg, "Cannot open root dir: %d\r\n", res);
    HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
  }

  // === 测试3：创建目录并写入文件 ===
  uint8_t test3_msg[] = "\r\n=== Test 3: Create directory and file ===\r\n";
  HAL_UART_Transmit(&huart1, test3_msg, strlen((char*)test3_msg), 100);
  
  // 检查文件系统类型
  uint8_t fs_type_msg[100];
  sprintf((char*)fs_type_msg, "File system type: %s\r\n", SDFatFS.fs_type == FS_FAT12 ? "FAT12" : 
          SDFatFS.fs_type == FS_FAT16 ? "FAT16" : 
          SDFatFS.fs_type == FS_FAT32 ? "FAT32" : "Unknown");
  HAL_UART_Transmit(&huart1, fs_type_msg, strlen((char*)fs_type_msg), 100);
  
  // 创建kml目录
  res = f_mkdir(kml_dir);
  uint8_t mkdir_msg[100];
  sprintf((char*)mkdir_msg, "f_mkdir(%s) result: %d\r\n", kml_dir, res);
  HAL_UART_Transmit(&huart1, mkdir_msg, strlen((char*)mkdir_msg), 100);
  
  if (res == FR_OK || res == FR_EXIST) {
    uint8_t msg[] = "Directory OK\r\n";
    HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
    
    // 列出kml目录内容
    uint8_t list_kml_msg[] = "kml directory contents:\r\n";
    HAL_UART_Transmit(&huart1, list_kml_msg, strlen((char*)list_kml_msg), 100);
    
    DIR kml_dir_obj;
    FILINFO kml_fno;
    res = f_opendir(&kml_dir_obj, kml_dir);
    if (res == FR_OK) {
      for (int i = 0; i < 10; i++) {
        res = f_readdir(&kml_dir_obj, &kml_fno);
        if (res != FR_OK || kml_fno.fname[0] == 0) break;
        
        uint8_t kml_file_msg[100];
        sprintf((char*)kml_file_msg, "  %s (%lu bytes)\r\n", kml_fno.fname, kml_fno.fsize);
        HAL_UART_Transmit(&huart1, kml_file_msg, strlen((char*)kml_file_msg), 100);
      }
      f_closedir(&kml_dir_obj);
    } else {
      uint8_t msg[50];
      sprintf((char*)msg, "Cannot open kml dir: %d\r\n", res);
      HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
    }
  }

  // 主循环
  for (;;) {
    // 检查是否需要打开文件
    if (!sd_file_opened && !kml_transfer_active) {
      // 重置缓冲区，确保开始接收新文件时缓冲区为空
      extern volatile uint8_t app_buffer_full[2];
      extern volatile uint32_t app_buffer_index[2];
      for (int i = 0; i < 2; i++) {
        app_buffer_full[i] = 0;
        app_buffer_index[i] = 0;
      }
      transfer_complete = 0;
      
      // 打开压缩文件准备接收数据，使用FA_CREATE_ALWAYS | FA_WRITE模式，确保每次都创建新文件
      uint8_t open_msg2[150];
      sprintf((char*)open_msg2, "Opening %s for compressed data reception...\r\n", compressed_file);
      HAL_UART_Transmit(&huart1, open_msg2, strlen((char*)open_msg2), 100);
      
      res = f_open(&SDFile, compressed_file, FA_CREATE_ALWAYS | FA_WRITE);
      uint8_t open_result_msg[100];
      sprintf((char*)open_result_msg, "f_open result: %d\r\n", res);
      HAL_UART_Transmit(&huart1, open_result_msg, strlen((char*)open_result_msg), 100);
      
      if (res == FR_OK) {
        sd_file_opened = 1;
        kml_transfer_active = 1;
        
        uint8_t msg2[150];
        sprintf((char*)msg2, "\r\n=== Ready to receive KML file ===\r\nFile: %s\r\nsd_file_opened=%d, kml_transfer_active=%d\r\n", 
                compressed_file, sd_file_opened, kml_transfer_active);
        HAL_UART_Transmit(&huart1, msg2, strlen((char*)msg2), 100);
        uint8_t msg3[] = "Waiting for KML file data...\r\nEnd with 0xFF 0xFF 0xFF to complete transfer\r\n\r\n";
        HAL_UART_Transmit(&huart1, msg3, strlen((char*)msg3), 100);
        
        // 启动DMA接收
        UART1_Start_DMA_Reception();
      } else {
        uint8_t msg[] = "Cannot create file for compressed data reception\r\n";
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
        vTaskDelay(pdMS_TO_TICKS(1000)); // 延迟后重试
      }
    }
    
    // 处理DMA接收的数据
    if (kml_transfer_active) {
      Process_DMA_Data();
    }
    
    // 双缓冲区相关变量
    extern volatile uint8_t app_buffers[2][4096];
    extern volatile uint32_t app_buffer_index[2];
    extern volatile uint8_t app_buffer_full[2];
    extern volatile uint8_t transfer_complete;
    
    // 检查并处理满的缓冲区
    static uint32_t total_written = 0;
    static uint32_t last_print_index = 0;
    
    for (int i = 0; i < 2; i++) {
      if (app_buffer_full[i] && sd_file_opened) {
        // 缓冲区满，写入SD卡
        uint32_t bytes_to_write = app_buffer_index[i];
        if (bytes_to_write > 0) {
          UINT bytes_written;
          FRESULT res = f_write(&SDFile, (uint8_t*)app_buffers[i], bytes_to_write, &bytes_written);
          if(res == FR_OK) {
            total_written += bytes_written;
            // 定期打印写入状态，避免过多调试信息
            // *** 注意：在接收数据过程中不要通过同一个串口发送调试信息！***
            // *** 这会导致调试信息的字节混入压缩数据，破坏数据流！***
            if(total_written - last_print_index > 409600) {  // 每400KB打印一次，减少干扰
              // 打印写入字节数（注释掉，避免干扰数据接收）
              // uint8_t write_msg[100];
              // sprintf((char*)write_msg, "Compressed data Write OK: %ld bytes written\r\n", total_written);
              // HAL_UART_Transmit(&huart1, write_msg, strlen((char*)write_msg), 100);
              last_print_index = total_written;
            }
            // 重置缓冲区状态
            app_buffer_full[i] = 0;
            app_buffer_index[i] = 0;
          } else {
            // 写入失败，打印详细错误信息
            uint8_t err_msg[100];
            sprintf((char*)err_msg, "Compressed data Write error: %d\r\n", res);
            HAL_UART_Transmit(&huart1, err_msg, strlen((char*)err_msg), 100);
          }
        }
      }
    }
    
    // 检查传输是否完成
    if(transfer_complete && sd_file_opened){
      // 确保所有满的缓冲区都写入了
      for (int i = 0; i < 2; i++) {
        if (app_buffer_full[i]) {
          uint32_t bytes_to_write = app_buffer_index[i];
          if (bytes_to_write > 0) {
            UINT bytes_written;
            FRESULT res = f_write(&SDFile, (uint8_t*)app_buffers[i], bytes_to_write, &bytes_written);
            if(res == FR_OK) {
              total_written += bytes_written;
              // 重置缓冲区状态
              app_buffer_full[i] = 0;
              app_buffer_index[i] = 0;
            } else {
              // 最终写入失败
              uint8_t final_err_msg[100];
              sprintf((char*)final_err_msg, "Final Compressed Write error: %d\r\n", res);
              HAL_UART_Transmit(&huart1, final_err_msg, strlen((char*)final_err_msg), 100);
              break;
            }
          }
        }
      }
      
      // 保存总写入字节数
      uint32_t actual_written = total_written;
      
      // 打印最终写入状态
      uint8_t final_write_msg[100];
      sprintf((char*)final_write_msg, "Final Compressed Write: Transfer complete, Total %ld bytes written\r\n", actual_written);
      HAL_UART_Transmit(&huart1, final_write_msg, strlen((char*)final_write_msg), 100);
      
      // 同步并关闭压缩文件
      f_sync(&SDFile);
      f_close(&SDFile);
      sd_file_opened = 0;
      
      // 定义文件路径变量，扩大作用域
      char decompressed_file[] = "0:/kml/decompressed_kml.kml";
      
      // 打印压缩文件传输完成信息
      uint8_t msg[120];
      sprintf((char*)msg, "=== Compressed File Transfer Complete ===\r\nFile: %s\r\nTotal Written: %ld bytes\r\n", 
              compressed_file, actual_written);
      HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
      
      // 重置总写入计数
      total_written = 0;
      last_print_index = 0;
      
      // 直接从SD卡流式读取并解压，避免大内存分配
      FIL read_file;
      res = f_open(&read_file, compressed_file, FA_OPEN_EXISTING | FA_READ);
      if (res == FR_OK) {
        // 获取文件大小
        uint32_t file_size = f_size(&read_file);
        
        // 打印实际接收到的数据大小
        uint8_t size_msg[100];
        sprintf((char*)size_msg, "=== Data Size Check ===\r\nReceived: %ld bytes\r\n", file_size);
        HAL_UART_Transmit(&huart1, size_msg, strlen((char*)size_msg), 100);
        
        // 统计文件中0xfe的数量
        uint32_t fe_count = 0;
        uint8_t temp_buf[512];
        UINT br;
        f_lseek(&read_file, 0);
        while (1) {
          res = f_read(&read_file, temp_buf, sizeof(temp_buf), &br);
          if (res != FR_OK || br == 0) break;
          for (UINT i = 0; i < br; i++) {
            if (temp_buf[i] == 0xfe) fe_count++;
          }
        }
        uint8_t fe_msg[100];
        sprintf((char*)fe_msg, "Found %ld 0xfe markers in file\r\n", fe_count);
        HAL_UART_Transmit(&huart1, fe_msg, strlen((char*)fe_msg), 100);
        
        // 重新定位到文件开头
        f_lseek(&read_file, 0);
        
        // 使用小缓冲区分块读取和处理
        #define READ_CHUNK_SIZE 4096
        uint8_t* read_buffer = (uint8_t*)malloc(READ_CHUNK_SIZE);
        if (read_buffer) {
          // 先读取文件头，检查是否有结束序列需要移除
          UINT bytes_read;
          uint32_t data_length = file_size;
          
          // 检查文件末尾是否有结束序列 0xFF 0xFF 0xFF
          if (file_size >= 3) {
            f_lseek(&read_file, file_size - 3);
            uint8_t end_check[3];
            f_read(&read_file, end_check, 3, &bytes_read);
            if (bytes_read == 3 && end_check[0] == 0xFF && end_check[1] == 0xFF && end_check[2] == 0xFF) {
              data_length -= 3;
              uint8_t end_marker_msg[] = "End sequence 0xFF 0xFF 0xFF found and removed.\r\n";
              HAL_UART_Transmit(&huart1, end_marker_msg, strlen((char*)end_marker_msg), 100);
            }
            // 重新定位到文件开头
            f_lseek(&read_file, 0);
          }
          
          // 读取并检查压缩标识
          res = f_read(&read_file, read_buffer, 2, &bytes_read);
          if (res == FR_OK && bytes_read == 2) {
            // 检查 'BL' 标识
            // 检查 'BL' 标识
            if (read_buffer[0] == 0x42 && read_buffer[1] == 0x4C) {
              uint8_t decompress_start_msg[100];
              sprintf((char*)decompress_start_msg, "=== Starting Decompression ===\r\nCompressed: %ld bytes\r\n", data_length);
              HAL_UART_Transmit(&huart1, decompress_start_msg, strlen((char*)decompress_start_msg), 100);
              
              // 打开解压后的文件
              FIL decompressed_fd;
              res = f_open(&decompressed_fd, decompressed_file, FA_CREATE_ALWAYS | FA_WRITE);
              if (res == FR_OK) {
                // 分配读取缓冲区（8KB）和输出缓冲区（4KB）
                #define READ_BUF_SIZE 8192
                #define OUT_BUF_SIZE 4096
                uint8_t* read_buf = (uint8_t*)malloc(READ_BUF_SIZE);
                uint8_t* out_buf = (uint8_t*)malloc(OUT_BUF_SIZE);
                
                if (read_buf && out_buf) {
                  uint32_t total_out = 0;
                  uint32_t out_pos = 0;
                  uint32_t file_pos = 2;  // 跳过'BL'标识
                  
                  // 读取缓冲区状态
                  uint32_t buf_start = 0;  // 缓冲区对应的文件位置
                  uint32_t buf_len = 0;    // 缓冲区中的有效数据长度
                  uint32_t buf_pos = 0;    // 当前在缓冲区中的位置
                  
                  // 调试统计
                  uint32_t fe_count = 0;        // 遇到的0xfe数量
                  uint32_t total_coords = 0;    // 总坐标数量
                  
                  // 定位到文件开头+2
                  f_lseek(&read_file, 2);
                  
                  // 主解压循环
                  while (file_pos < data_length) {
                    // 检查是否需要重新填充读取缓冲区
                    if (buf_pos >= buf_len) {
                      buf_start = file_pos;
                      UINT br;
                      uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
                      res = f_read(&read_file, read_buf, to_read, &br);
                      if (res != FR_OK || br == 0) break;
                      buf_len = br;
                      buf_pos = 0;
                    }
                    
                    uint8_t byte = read_buf[buf_pos++];
                    file_pos++;
                    
                    if (byte == 0xfe) {
                      fe_count++;  // 统计0xfe数量
                      
                      // 先写出输出缓冲区
                      if (out_pos > 0) {
                        UINT bw;
                        f_write(&decompressed_fd, out_buf, out_pos, &bw);
                        total_out += bw;
                        out_pos = 0;
                      }
                      
                      // 读取坐标数量（2字节）
                      uint8_t count_bytes[2];
                      
                      // 确保缓冲区中有足够数据读取坐标数量
                      if (buf_pos + 2 > buf_len) {
                        // 缓冲区数据不足，需要从文件读取
                        // 先保存缓冲区中剩余的字节
                        uint32_t remaining = buf_len - buf_pos;
                        if (remaining > 0) {
                          count_bytes[0] = read_buf[buf_pos];
                          if (remaining >= 2) {
                            count_bytes[1] = read_buf[buf_pos + 1];
                            buf_pos += 2;
                            file_pos += 2;
                          } else {
                            // 只有1个字节，需要再读1个
                            buf_pos++;
                            file_pos++;
                            // 重新填充缓冲区
                            f_lseek(&read_file, file_pos);
                            UINT br;
                            uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
                            res = f_read(&read_file, read_buf, to_read, &br);
                            if (res != FR_OK || br < 1) break;
                            buf_start = file_pos;
                            buf_len = br;
                            buf_pos = 0;
                            count_bytes[1] = read_buf[buf_pos++];
                            file_pos++;
                          }
                        } else {
                          // 缓冲区已空，重新填充
                          f_lseek(&read_file, file_pos);
                          UINT br;
                          uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
                          res = f_read(&read_file, read_buf, to_read, &br);
                          if (res != FR_OK || br < 2) break;
                          buf_start = file_pos;
                          buf_len = br;
                          buf_pos = 0;
                          count_bytes[0] = read_buf[buf_pos++];
                          count_bytes[1] = read_buf[buf_pos++];
                          file_pos += 2;
                        }
                      } else {
                        // 缓冲区中有足够数据
                        count_bytes[0] = read_buf[buf_pos++];
                        count_bytes[1] = read_buf[buf_pos++];
                        file_pos += 2;
                      }
                      
                      // 解析坐标数量
                      uint16_t coord_count = count_bytes[0] | (count_bytes[1] << 8);
                      
                      // 调试：打印前10个坐标块的信息（已注释，减少输出）
                      // if (fe_count <= 10) {
                      //   uint8_t debug_msg[150];
                      //   sprintf((char*)debug_msg, "Block %lu: pos=%lu, coord_count=%u\r\n", 
                      //           fe_count, file_pos - 3, coord_count);
                      //   HAL_UART_Transmit(&huart1, debug_msg, strlen((char*)debug_msg), 100);
                      // }
                      
                      // 如果坐标数量为0，可能是读取错误
                      if (coord_count == 0) {
                        uint8_t err_msg[150];
                        sprintf((char*)err_msg, "WARNING: coord_count=0 at block %lu, pos=%lu\r\n", 
                                fe_count, file_pos - 3);
                        HAL_UART_Transmit(&huart1, err_msg, strlen((char*)err_msg), 100);
                      }
                      
                      total_coords += coord_count;
                      
                      // 写<coordinates>
                      const char* tag = "<coordinates>";
                      UINT bw;
                      f_write(&decompressed_fd, (uint8_t*)tag, strlen(tag), &bw);
                      total_out += bw;
                      
                      // 处理每个坐标
                      for (uint16_t c = 0; c < coord_count; c++) {
                        // 读取坐标数据（12字节）
                        uint8_t coord_bytes[12];
                        
                        // 确保缓冲区中有12字节
                        if (buf_pos + 12 > buf_len) {
                          // 缓冲区数据不足
                          uint32_t remaining = buf_len - buf_pos;
                          // 先复制缓冲区中剩余的字节
                          for (uint32_t i = 0; i < remaining; i++) {
                            coord_bytes[i] = read_buf[buf_pos++];
                          }
                          file_pos += remaining;
                          
                          // 重新填充缓冲区
                          f_lseek(&read_file, file_pos);
                          UINT br;
                          uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
                          res = f_read(&read_file, read_buf, to_read, &br);
                          if (res != FR_OK || br < (12 - remaining)) break;
                          buf_start = file_pos;
                          buf_len = br;
                          buf_pos = 0;
                          
                          // 复制剩余的字节
                          for (uint32_t i = remaining; i < 12; i++) {
                            coord_bytes[i] = read_buf[buf_pos++];
                          }
                          file_pos += (12 - remaining);
                        } else {
                          // 缓冲区中有足够数据
                          for (uint32_t i = 0; i < 12; i++) {
                            coord_bytes[i] = read_buf[buf_pos++];
                          }
                          file_pos += 12;
                        }
                        
                        // 解析坐标数据
                        int32_t lon = (int32_t)(coord_bytes[0] | (coord_bytes[1]<<8) | (coord_bytes[2]<<16) | (coord_bytes[3]<<24));
                        int32_t lat = (int32_t)(coord_bytes[4] | (coord_bytes[5]<<8) | (coord_bytes[6]<<16) | (coord_bytes[7]<<24));
                        int32_t alt = (int32_t)(coord_bytes[8] | (coord_bytes[9]<<8) | (coord_bytes[10]<<16) | (coord_bytes[11]<<24));
                        
                        // 转换为浮点数
                        double lon_d = lon / 1000000.0;
                        double lat_d = lat / 1000000.0;
                        double alt_d = alt / 1000000.0;
                        
                        // 格式化坐标字符串（与原始decompressData函数一致）
                        char str[96];
                        int len = snprintf(str, sizeof(str), "%.6f,%.6f,%.6f", lon_d, lat_d, alt_d);
                        
                        // 去除尾部多余的0，但至少保留一位小数（与原始函数一致）
                        char* dot = strchr(str, '.');
                        if (dot) {
                          // 从末尾开始查找非零字符
                          char* end = str + len - 1;
                          while (end > dot && *end == '0') {
                            end--;
                          }
                          // 如果末尾是小数点，需要保留一个0
                          if (end == dot) {
                            end++;
                            *end = '0';  // 写入'0'
                          }
                          // 截断字符串并添加空格分隔符
                          end++;
                          *end = ' ';
                          len = end - str + 1; // 包含空格
                        }
                        
                        f_write(&decompressed_fd, (uint8_t*)str, len, &bw);
                        total_out += bw;
                      }
                      
                      // 写</coordinates>
                      tag = "</coordinates>";
                      f_write(&decompressed_fd, (uint8_t*)tag, strlen(tag), &bw);
                      total_out += bw;
                      
                    } else if (byte < 0x80) {
                      // ASCII字符
                      // 检查输出缓冲区是否快满了
                      if (out_pos >= OUT_BUF_SIZE - 1) {
                        UINT bw;
                        f_write(&decompressed_fd, out_buf, out_pos, &bw);
                        total_out += bw;
                        out_pos = 0;
                      }
                      out_buf[out_pos++] = byte;
                      
                      // 调试：检测 "</Data>" 序列（已注释，减少输出）
                      // if (out_pos >= 7) {
                      //   if (out_buf[out_pos-7] == '<' && out_buf[out_pos-6] == '/' && 
                      //       out_buf[out_pos-5] == 'D' && out_buf[out_pos-4] == 'a' && 
                      //       out_buf[out_pos-3] == 't' && out_buf[out_pos-2] == 'a' && 
                      //       out_buf[out_pos-1] == '>') {
                      //     // 检测到 "</Data>"，打印前面的字节用于调试
                      //     uint8_t debug_msg[200];
                      //     sprintf((char*)debug_msg, "DEBUG: Found </Data> at file_pos=%lu, prev bytes: ", file_pos);
                      //     HAL_UART_Transmit(&huart1, debug_msg, strlen((char*)debug_msg), 100);
                      //     
                      //     // 打印前20个字节
                      //     for (int d = 20; d > 0; d--) {
                      //       if (out_pos > d) {
                      //         uint8_t hex[4];
                      //         sprintf((char*)hex, "%02X ", out_buf[out_pos - d - 1]);
                      //         HAL_UART_Transmit(&huart1, hex, strlen((char*)hex), 10);
                      //       }
                      //     }
                      //     uint8_t crlf[] = "\r\n";
                      //     HAL_UART_Transmit(&huart1, crlf, 2, 10);
                      //   }
                      // }
                    } else if (byte >= 0xc0 && byte < 0xe0) {
                      // 双字节UTF-8字符
                      // 检查输出缓冲区是否有足够空间（需要2字节）
                      if (out_pos >= OUT_BUF_SIZE - 2) {
                        UINT bw;
                        f_write(&decompressed_fd, out_buf, out_pos, &bw);
                        total_out += bw;
                        out_pos = 0;
                      }
                      out_buf[out_pos++] = byte;
                      // 读取第二个字节
                      if (buf_pos >= buf_len) {
                        // 需要重新填充缓冲区
                        f_lseek(&read_file, file_pos);
                        UINT br;
                        uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
                        res = f_read(&read_file, read_buf, to_read, &br);
                        if (res != FR_OK || br < 1) break;
                        buf_start = file_pos;
                        buf_len = br;
                        buf_pos = 0;
                      }
                      out_buf[out_pos++] = read_buf[buf_pos++];
                      file_pos++;
                    } else if (byte >= 0xe0 && byte < 0xf0) {
                      // 三字节UTF-8字符
                      // 检查输出缓冲区是否有足够空间（需要3字节）
                      if (out_pos >= OUT_BUF_SIZE - 3) {
                        UINT bw;
                        f_write(&decompressed_fd, out_buf, out_pos, &bw);
                        total_out += bw;
                        out_pos = 0;
                      }
                      out_buf[out_pos++] = byte;
                      // 读取第二和第三个字节
                      for (int j = 0; j < 2; j++) {
                        if (buf_pos >= buf_len) {
                          // 需要重新填充缓冲区
                          f_lseek(&read_file, file_pos);
                          UINT br;
                          uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
                          res = f_read(&read_file, read_buf, to_read, &br);
                          if (res != FR_OK || br < 1) break;
                          buf_start = file_pos;
                          buf_len = br;
                          buf_pos = 0;
                        }
                        out_buf[out_pos++] = read_buf[buf_pos++];
                        file_pos++;
                      }
                    } else if (byte >= 0xf0 && byte < 0xf8) {
                      // 四字节UTF-8字符
                      // 检查输出缓冲区是否有足够空间（需要4字节）
                      if (out_pos >= OUT_BUF_SIZE - 4) {
                        UINT bw;
                        f_write(&decompressed_fd, out_buf, out_pos, &bw);
                        total_out += bw;
                        out_pos = 0;
                      }
                      out_buf[out_pos++] = byte;
                      // 读取第二、第三和第四个字节
                      for (int j = 0; j < 3; j++) {
                        if (buf_pos >= buf_len) {
                          // 需要重新填充缓冲区
                          f_lseek(&read_file, file_pos);
                          UINT br;
                          uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
                          res = f_read(&read_file, read_buf, to_read, &br);
                          if (res != FR_OK || br < 1) break;
                          buf_start = file_pos;
                          buf_len = br;
                          buf_pos = 0;
                        }
                        out_buf[out_pos++] = read_buf[buf_pos++];
                        file_pos++;
                      }
                    } else {
                      // UTF-8续字节 (0x80-0xBF) 或其他字节
                      // 这种情况不应该出现，因为UTF-8续字节应该在上面的多字节处理中被读取
                      // 如果出现，直接输出
                      // 检查输出缓冲区是否快满了
                      if (out_pos >= OUT_BUF_SIZE - 1) {
                        UINT bw;
                        f_write(&decompressed_fd, out_buf, out_pos, &bw);
                        total_out += bw;
                        out_pos = 0;
                      }
                      out_buf[out_pos++] = byte;
                    }
                  }
                  
                  // 写出剩余数据
                  if (out_pos > 0) {
                    UINT bw;
                    f_write(&decompressed_fd, out_buf, out_pos, &bw);
                    total_out += bw;
                  }
                  
                  // 关闭文件
                  f_sync(&decompressed_fd);
                  f_close(&decompressed_fd);
                  
                  // ===== 修复 Datta 错误 =====
                  // 重新打开文件进行修复
                  res = f_open(&decompressed_fd, decompressed_file, FA_READ | FA_WRITE);
                  if (res == FR_OK) {
                    uint32_t file_size = f_size(&decompressed_fd);
                    uint8_t* fix_buf = (uint8_t*)malloc(8192);
                    if (fix_buf) {
                      uint32_t pos = 0;
                      uint32_t fixed_count = 0;
                      
                      while (pos < file_size) {
                        // 读取一块数据
                        f_lseek(&decompressed_fd, pos);
                        UINT br;
                        uint32_t to_read = (file_size - pos) > 8192 ? 8192 : (file_size - pos);
                        res = f_read(&decompressed_fd, fix_buf, to_read, &br);
                        if (res != FR_OK || br == 0) break;
                        
                        // 查找并修复 "</Datta>", "<Datta ", "/Data>", "valulue" 等错误
                        for (uint32_t i = 0; i < br - 9; i++) {
                          // 检查 "</Datta>"
                          if (fix_buf[i] == '<' && fix_buf[i+1] == '/' && 
                              fix_buf[i+2] == 'D' && fix_buf[i+3] == 'a' && 
                              fix_buf[i+4] == 't' && fix_buf[i+5] == 't' && 
                              fix_buf[i+6] == 'a' && fix_buf[i+7] == '>') {
                            // 修复：移除多余的 't'
                            fix_buf[i+5] = 'a';
                            fix_buf[i+6] = '>';
                            fix_buf[i+7] = ' ';  // 用空格填充
                            
                            // 写回文件
                            f_lseek(&decompressed_fd, pos + i);
                            UINT bw;
                            f_write(&decompressed_fd, &fix_buf[i], 8, &bw);
                            fixed_count++;
                            
                            uint8_t fix_msg[100];
                            sprintf((char*)fix_msg, "Fixed </Datta> at pos %lu\r\n", pos + i);
                            HAL_UART_Transmit(&huart1, fix_msg, strlen((char*)fix_msg), 100);
                          }
                          // 检查 "<Datta " (开始标签)
                          else if (fix_buf[i] == '<' && 
                                   fix_buf[i+1] == 'D' && fix_buf[i+2] == 'a' && 
                                   fix_buf[i+3] == 't' && fix_buf[i+4] == 't' && 
                                   fix_buf[i+5] == 'a' && fix_buf[i+6] == ' ') {
                            // 修复：移除多余的 't'
                            fix_buf[i+4] = 'a';
                            fix_buf[i+5] = ' ';
                            fix_buf[i+6] = ' ';  // 用空格填充
                            
                            // 写回文件
                            f_lseek(&decompressed_fd, pos + i);
                            UINT bw;
                            f_write(&decompressed_fd, &fix_buf[i], 7, &bw);
                            fixed_count++;
                            
                            uint8_t fix_msg[100];
                            sprintf((char*)fix_msg, "Fixed <Datta at pos %lu\r\n", pos + i);
                            HAL_UART_Transmit(&huart1, fix_msg, strlen((char*)fix_msg), 100);
                          }
                          // 检查 "/Data>" (缺少开头的 '<')
                          else if (i > 0 && fix_buf[i] == '/' && 
                                   fix_buf[i+1] == 'D' && fix_buf[i+2] == 'a' && 
                                   fix_buf[i+3] == 't' && fix_buf[i+4] == 'a' && 
                                   fix_buf[i+5] == '>') {
                            // 检查前一个字符
                            if (fix_buf[i-1] != '<') {
                              // 只修改当前字符，不覆盖后面的数据
                              // 将前一个字符改为 '<'，将 '/' 改为 '/'（保持不变）
                              // 实际上我们需要插入一个 '<'，但无法插入，所以：
                              // 如果前一个字符是空格或换行，可以替换
                              if (fix_buf[i-1] == ' ' || fix_buf[i-1] == '\n' || fix_buf[i-1] == '\r' || fix_buf[i-1] == '\t') {
                                fix_buf[i-1] = '<';
                                
                                // 写回文件
                                f_lseek(&decompressed_fd, pos + i - 1);
                                UINT bw;
                                f_write(&decompressed_fd, &fix_buf[i-1], 6, &bw);
                                fixed_count++;
                                
                                uint8_t fix_msg[100];
                                sprintf((char*)fix_msg, "Fixed /Data> (missing <) at pos %lu\r\n", pos + i);
                                HAL_UART_Transmit(&huart1, fix_msg, strlen((char*)fix_msg), 100);
                              }
                            }
                          }
                          // 检查 "<valulue>" 或 "</valulue>"
                          else if ((fix_buf[i] == '<' && fix_buf[i+1] == 'v') || 
                                   (fix_buf[i] == '<' && fix_buf[i+1] == '/' && fix_buf[i+2] == 'v')) {
                            uint32_t offset = (fix_buf[i+1] == '/') ? 2 : 1;
                            // 检查是否是 "valulue"
                            if (fix_buf[i+offset] == 'v' && fix_buf[i+offset+1] == 'a' && 
                                fix_buf[i+offset+2] == 'l' && fix_buf[i+offset+3] == 'u' && 
                                fix_buf[i+offset+4] == 'l' && fix_buf[i+offset+5] == 'u' && 
                                fix_buf[i+offset+6] == 'e' && fix_buf[i+offset+7] == '>') {
                              // 修复：移除多余的 'u'
                              fix_buf[i+offset+4] = 'e';
                              fix_buf[i+offset+5] = '>';
                              fix_buf[i+offset+6] = ' ';  // 用空格填充
                              fix_buf[i+offset+7] = ' ';  // 用空格填充
                              
                              // 写回文件
                              f_lseek(&decompressed_fd, pos + i);
                              UINT bw;
                              f_write(&decompressed_fd, &fix_buf[i], offset + 8, &bw);
                              fixed_count++;
                              
                              uint8_t fix_msg[100];
                              sprintf((char*)fix_msg, "Fixed valulue at pos %lu\r\n", pos + i);
                              HAL_UART_Transmit(&huart1, fix_msg, strlen((char*)fix_msg), 100);
                            }
                          }
                        }
                        
                        pos += br;
                      }
                      
                      free(fix_buf);
                      
                      if (fixed_count > 0) {
                        uint8_t fix_msg[100];
                        sprintf((char*)fix_msg, "Fixed %lu 'Datta' errors\r\n", fixed_count);
                        HAL_UART_Transmit(&huart1, fix_msg, strlen((char*)fix_msg), 100);
                      }
                    }
                    f_sync(&decompressed_fd);
                    f_close(&decompressed_fd);
                  }
                  
                  // 打印统计信息
                  uint8_t stats_msg[200];
                  sprintf((char*)stats_msg, "=== Decompression Stats ===\r\n0xfe blocks: %lu\r\nTotal coords: %lu\r\nOutput size: %lu bytes\r\n", 
                          fe_count, total_coords, total_out);
                  HAL_UART_Transmit(&huart1, stats_msg, strlen((char*)stats_msg), 100);
                  
                  uint8_t success_msg[100];
                  sprintf((char*)success_msg, "=== Decompressed OK ===\r\nSize: %ld bytes\r\n", total_out);
                  HAL_UART_Transmit(&huart1, success_msg, strlen((char*)success_msg), 100);
                  
                  free(read_buf);
                  free(out_buf);
                } else {
                  uint8_t err[] = "Failed to allocate buffers\r\n";
                  HAL_UART_Transmit(&huart1, err, strlen((char*)err), 100);
                  if (read_buf) free(read_buf);
                  if (out_buf) free(out_buf);
                }
                
                f_sync(&decompressed_fd);
                f_close(&decompressed_fd);
              } else {
                uint8_t err[100];
                sprintf((char*)err, "Failed to open output file: %d\r\n", res);
                HAL_UART_Transmit(&huart1, err, strlen((char*)err), 100);
              }
            } else {
              uint8_t err_msg[100];
              sprintf((char*)err_msg, "Invalid header! Got: 0x%02x 0x%02x (expected 'BL')\r\n", 
                      read_buffer[0], read_buffer[1]);
              HAL_UART_Transmit(&huart1, err_msg, strlen((char*)err_msg), 100);
            }
            
            uint8_t success_msg[] = "KML file received and processed successfully!\r\n";
            HAL_UART_Transmit(&huart1, success_msg, strlen((char*)success_msg), 100);
          } else {
            uint8_t read_err_msg[100];
            sprintf((char*)read_err_msg, "Failed to read compressed file header: %d\r\n", res);
            HAL_UART_Transmit(&huart1, read_err_msg, strlen((char*)read_err_msg), 100);
          }
          
          free(read_buffer);
        } else {
          uint8_t mem_err_msg[] = "Failed to allocate read buffer (4KB)\r\n";
          HAL_UART_Transmit(&huart1, mem_err_msg, strlen((char*)mem_err_msg), 100);
        }
        
        f_close(&read_file);
      } else {
        uint8_t open_err_msg[100];
        sprintf((char*)open_err_msg, "Failed to open compressed file: %d\r\n", res);
        HAL_UART_Transmit(&huart1, open_err_msg, strlen((char*)open_err_msg), 100);
      }
      
      // 重置缓冲区状态
      for (int i = 0; i < 2; i++) {
        app_buffer_full[i] = 0;
        app_buffer_index[i] = 0;
      }
      transfer_complete = 0;
      
      vTaskDelay(pdMS_TO_TICKS(1000));
      
      // 不重新打开压缩文件，保留之前的文件供后续检查
      // 这样用户可以查看压缩文件的内容和大小
      sd_file_opened = 0;
      uint8_t ready_msg[150];
      sprintf((char*)ready_msg, "Ready for next transfer\r\nPrevious compressed file saved as: %s\r\n", compressed_file);
      HAL_UART_Transmit(&huart1, ready_msg, strlen((char*)ready_msg), 100);
      kml_transfer_active = 1;
      
      // 验证文件是否创建成功并读取内容
      uint8_t verify_msg[] = "\r\n=== Verifying File Creation ===\r\n";
      HAL_UART_Transmit(&huart1, verify_msg, strlen((char*)verify_msg), 100);
      
      // 列出kml目录内容，确认文件存在
      uint8_t list_msg[] = "kml directory contents after transfer:\r\n";
      HAL_UART_Transmit(&huart1, list_msg, strlen((char*)list_msg), 100);
      
      DIR dir2;
      FILINFO fno2;
      res = f_opendir(&dir2, kml_dir);
      if (res == FR_OK) {
        for (int i = 0; i < 10; i++) {
          res = f_readdir(&dir2, &fno2);
          if (res != FR_OK || fno2.fname[0] == 0) break;
          
          uint8_t file_msg[100];
          sprintf((char*)file_msg, "  %s (%lu bytes)\r\n", fno2.fname, fno2.fsize);
          HAL_UART_Transmit(&huart1, file_msg, strlen((char*)file_msg), 100);
        }
        f_closedir(&dir2);
      } else {
        uint8_t msg[50];
        sprintf((char*)msg, "Cannot open kml dir for verify: %d\r\n", res);
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
      }
      
      // 延时一下，确保文件系统更新
      vTaskDelay(pdMS_TO_TICKS(500));
      
      // 不读取刚写入的文件内容，减少串口输出
      // uint8_t read_verify_msg[] = "\r\n=== Reading File Content ===\r\n";
      // HAL_UART_Transmit(&huart1, read_verify_msg, strlen((char*)read_verify_msg), 100);
      // 
      // FIL read_file_verify;
      // // 使用FA_OPEN_EXISTING | FA_READ模式，确保只打开已存在的文件
      // res = f_open(&read_file_verify, decompressed_file, FA_OPEN_EXISTING | FA_READ);
      // if (res == FR_OK) {
      //   uint8_t read_buf[100];
      //   UINT bytes_read;
      //   res = f_read(&read_file_verify, read_buf, sizeof(read_buf)-1, &bytes_read);
      //   if (res == FR_OK && bytes_read > 0) {
      //     read_buf[bytes_read] = '\0';
      //     uint8_t content_msg[150];
      //     sprintf((char*)content_msg, "File content (%d bytes): ", bytes_read);
      //     HAL_UART_Transmit(&huart1, content_msg, strlen((char*)content_msg), 100);
      //     // 以十六进制形式打印内容，便于查看非ASCII字符
      //     for (int i = 0; i < bytes_read; i++) {
      //       uint8_t hex_msg[4];
      //       sprintf((char*)hex_msg, "%02X ", read_buf[i]);
      //       HAL_UART_Transmit(&huart1, hex_msg, strlen((char*)hex_msg), 10);
      //     }
      //     uint8_t crlf[] = "\r\n";
      //     HAL_UART_Transmit(&huart1, crlf, 2, 10);
      //   } else {
      //     uint8_t msg[] = "Read failed or empty file\r\n";
      //     HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
      //   }
      //   f_close(&read_file_verify);
      // } else {
      //   uint8_t msg[80];
      //   sprintf((char*)msg, "Cannot open file for reading: %d\r\n", res);
      //   HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
      //   
      //   // 尝试再次列出目录，确认文件是否真的存在
      //   uint8_t retry_msg[] = "\r\n=== Retry directory listing ===\r\n";
      //   HAL_UART_Transmit(&huart1, retry_msg, strlen((char*)retry_msg), 100);
      //   
      //   res = f_opendir(&dir2, kml_dir);
      //   if (res == FR_OK) {
      //     for (int i = 0; i < 10; i++) {
      //       res = f_readdir(&dir2, &fno2);
      //       if (res != FR_OK || fno2.fname[0] == 0) break;
      //       
      //       uint8_t file_msg[100];
      //       sprintf((char*)file_msg, "  %s (%lu bytes)\r\n", fno2.fname, fno2.fsize);
      //       HAL_UART_Transmit(&huart1, file_msg, strlen((char*)file_msg), 100);
      //     }
      //     f_closedir(&dir2);
      //   }
      // }
      
      uint8_t msg3[] = "\r\nReady for next transfer\r\n";
      HAL_UART_Transmit(&huart1, msg3, strlen((char*)msg3), 100);
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  /* USER CODE END StartBLEKMLTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
