/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "dma.h"
#include "fatfs.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sdio.h"
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
// ★流式写入：串口收到的 KML 字节流入此缓冲，BLE_KML_Task 从中取出并写入 SD，不存整文件
StreamBufferHandle_t kml_stream_buf = NULL;
SemaphoreHandle_t kml_file_sem = NULL;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
SemaphoreHandle_t gps_mutex;  // 声明gps数据互斥锁句柄
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

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
  MX_DMA_Init();
  MX_RTC_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  
  // 发送初始化开始消息
  uint8_t init_start_msg[] = "\r\n=== System Initialization Start ===\r\n";
  HAL_UART_Transmit(&huart1, init_start_msg, strlen((char*)init_start_msg), 100);
  
  /* USER CODE BEGIN 2 */
  // 发送SD卡初始化消息
  uint8_t sd_init_msg[] = "Initializing SD card...\r\n";
  HAL_UART_Transmit(&huart1, sd_init_msg, strlen((char*)sd_init_msg), 100);
  
  MX_SDIO_SD_Init_FIX();  // 先初始化SD卡硬件
  
  // 发送SD卡初始化完成消息
  uint8_t sd_init_done_msg[] = "SD card initialized\r\n";
  HAL_UART_Transmit(&huart1, sd_init_done_msg, strlen((char*)sd_init_done_msg), 100);
  
  delay_init();//启用us延时函数
  
  // 发送FatFS初始化消息
  uint8_t fatfs_init_msg[] = "Initializing FatFS...\r\n";
  HAL_UART_Transmit(&huart1, fatfs_init_msg, strlen((char*)fatfs_init_msg), 100);
  
  MX_FATFS_Init();  // 再初始化FatFS文件系统
  
  // 发送FatFS初始化完成消息
  uint8_t fatfs_init_done_msg[] = "FatFS initialized\r\n";
  HAL_UART_Transmit(&huart1, fatfs_init_done_msg, strlen((char*)fatfs_init_done_msg), 100);
  
  // gps_mutex = xSemaphoreCreateMutex(); // 注释：暂时不需要GPS功能
  // Bluetooth_Init(); // 注释：暂时不需要蓝牙心率功能，只保留BLE KML传输

  // DMA接收将在FreeRTOS任务中启动，这里不需要初始化
  // 发送DMA准备完成消息
  uint8_t uart_init_done_msg[] = "UART DMA ready\r\n";
  HAL_UART_Transmit(&huart1, uart_init_done_msg, strlen((char*)uart_init_done_msg), 100);

  /* USER CODE END 2 */

  /* Init scheduler */
  uint8_t kernel_init_msg[] = "Initializing FreeRTOS kernel...\r\n";
  HAL_UART_Transmit(&huart1, kernel_init_msg, strlen((char*)kernel_init_msg), 100);
  
  osKernelInitialize();
  
  uint8_t kernel_init_done_msg[] = "FreeRTOS kernel initialized\r\n";
  HAL_UART_Transmit(&huart1, kernel_init_done_msg, strlen((char*)kernel_init_done_msg), 100);

  /* Call init function for freertos objects (in cmsis_os2.c) */
  uint8_t freertos_init_msg[] = "Initializing FreeRTOS tasks...\r\n";
  HAL_UART_Transmit(&huart1, freertos_init_msg, strlen((char*)freertos_init_msg), 100);
  
  MX_FREERTOS_Init();
  
  uint8_t freertos_init_done_msg[] = "FreeRTOS tasks initialized\r\n";
  HAL_UART_Transmit(&huart1, freertos_init_done_msg, strlen((char*)freertos_init_done_msg), 100);

  /* Start scheduler */
  uint8_t scheduler_start_msg[] = "Starting FreeRTOS scheduler...\r\n";
  HAL_UART_Transmit(&huart1, scheduler_start_msg, strlen((char*)scheduler_start_msg), 100);
  
  osKernelStart();
  
  // 这行代码不应该被执行到
  uint8_t error_msg[] = "ERROR: Scheduler start failed!\r\n";
  HAL_UART_Transmit(&huart1, error_msg, strlen((char*)error_msg), 100);

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
#if LOG

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(FILE *f)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}
GETCHAR_PROTOTYPE
{
	uint8_t ch = 0;
	HAL_UART_Receive(&huart2,(uint8_t *)&ch, 1, 0xFFFF);
	if (ch == '\r')
	{
		__io_putchar('\r');
		ch = '\n';
	}
	return __io_putchar(ch);
}

#endif

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
