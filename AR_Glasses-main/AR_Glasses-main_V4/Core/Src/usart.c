/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "BLE.h"
#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "ff.h"
#include <string.h>
#include <stdio.h>

// 外部变量声明（声明为volatile确保中断中能正确读取）
extern FIL SDFile;
extern volatile uint8_t sd_file_opened;
extern volatile uint8_t kml_transfer_active;

/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA10     ------> USART1_RX
    PA15     ------> USART1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA2_Stream2;
    hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA10     ------> USART1_RX
    PA15     ------> USART1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10|GPIO_PIN_15);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
// KML文件传输结束序列
#define KML_END_SEQUENCE_1 0xFF
#define KML_END_SEQUENCE_2 0xFF
#define KML_END_SEQUENCE_3 0xFF
#define END_SEQUENCE_LENGTH 3

// DMA双缓冲区实现
#define DMA_BUFFER_SIZE 4096  // 每个DMA缓冲区大小为4KB
#define NUM_BUFFERS 2         // 两个缓冲区

uint8_t dma_rx_buffers[NUM_BUFFERS][DMA_BUFFER_SIZE] __attribute__((aligned(4)));  // DMA双缓冲区，4字节对齐
volatile uint32_t dma_write_pos = 0;  // DMA当前写入位置
volatile uint32_t dma_read_pos = 0;   // 应用程序读取位置
volatile uint8_t current_dma_buffer = 0;  // 当前DMA使用的缓冲区索引

// 应用层缓冲区（用于处理数据）
#define APP_BUFFER_SIZE 4096
uint8_t app_buffers[NUM_BUFFERS][APP_BUFFER_SIZE];
volatile uint32_t app_buffer_index[NUM_BUFFERS] = {0, 0};
volatile uint8_t app_buffer_full[NUM_BUFFERS] = {0, 0};
volatile uint8_t current_app_buffer = 0;
volatile uint8_t transfer_complete = 0;

// 结束序列检测缓冲区
volatile uint8_t end_sequence_buffer[END_SEQUENCE_LENGTH] = {0};
volatile uint8_t end_sequence_index = 0;

// 调试计数器
static uint32_t total_bytes_received = 0;
static uint32_t bytes_received = 0;

// 启动DMA接收
void UART1_Start_DMA_Reception(void) {
  // 启动DMA循环接收模式
  HAL_UART_Receive_DMA(&huart1, dma_rx_buffers[0], DMA_BUFFER_SIZE);
}

// 处理DMA接收的数据
void Process_DMA_Data(void) {
  static uint32_t last_print_count = 0;
  
  // 获取DMA当前写入位置
  uint32_t dma_current_pos = DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
  
  // 计算可读取的数据量
  uint32_t available_bytes;
  if (dma_current_pos >= dma_read_pos) {
    available_bytes = dma_current_pos - dma_read_pos;
  } else {
    available_bytes = DMA_BUFFER_SIZE - dma_read_pos + dma_current_pos;
  }
  
  // 处理接收到的数据
  while (available_bytes > 0) {
    uint8_t recv_byte = dma_rx_buffers[0][dma_read_pos];
    dma_read_pos = (dma_read_pos + 1) % DMA_BUFFER_SIZE;
    available_bytes--;
    
    total_bytes_received++;
    
    // 将接收到的字节添加到结束序列检测缓冲区
    end_sequence_buffer[end_sequence_index] = recv_byte;
    end_sequence_index = (end_sequence_index + 1) % END_SEQUENCE_LENGTH;
    
    // 检查是否匹配结束序列
    uint8_t is_end_sequence = 1;
    for (int i = 0; i < END_SEQUENCE_LENGTH; i++) {
      int buffer_pos = (end_sequence_index - i - 1 + END_SEQUENCE_LENGTH) % END_SEQUENCE_LENGTH;
      if (i == 0 && end_sequence_buffer[buffer_pos] != KML_END_SEQUENCE_1) {
        is_end_sequence = 0;
        break;
      } else if (i == 1 && end_sequence_buffer[buffer_pos] != KML_END_SEQUENCE_2) {
        is_end_sequence = 0;
        break;
      } else if (i == 2 && end_sequence_buffer[buffer_pos] != KML_END_SEQUENCE_3) {
        is_end_sequence = 0;
        break;
      }
    }
    
    if (!is_end_sequence) {
      // 检查当前应用缓冲区是否已满
      if (app_buffer_index[current_app_buffer] >= APP_BUFFER_SIZE) {
        // 当前缓冲区已满，设置满标志并切换
        app_buffer_full[current_app_buffer] = 1;
        current_app_buffer = (current_app_buffer + 1) % NUM_BUFFERS;
        app_buffer_index[current_app_buffer] = 0;
        app_buffer_full[current_app_buffer] = 0;
        
        // 检查新缓冲区是否也满了
        if (app_buffer_index[current_app_buffer] >= APP_BUFFER_SIZE) {
          // 数据丢失，但继续处理
          continue;
        }
      }
      
      // 写入应用缓冲区
      app_buffers[current_app_buffer][app_buffer_index[current_app_buffer]] = recv_byte;
      app_buffer_index[current_app_buffer]++;
      bytes_received++;
      
      // 禁用传输过程中的串口打印，避免数据损坏
      // if (total_bytes_received - last_print_count >= 50000) {
      //   uint8_t progress_msg[100];
      //   sprintf((char*)progress_msg, "Receiving... %ld bytes\r\n", total_bytes_received);
      //   HAL_UART_Transmit(&huart1, progress_msg, strlen((char*)progress_msg), 100);
      //   last_print_count = total_bytes_received;
      // }
    } else {
      // 收到结束序列
      transfer_complete = 1;
      kml_transfer_active = 0;
      
      // 移除结束序列的前两个字节
      if (bytes_received >= 2) {
        app_buffer_index[current_app_buffer] -= 2;
        bytes_received -= 2;
      }
      
      // 设置当前缓冲区为满
      app_buffer_full[current_app_buffer] = 1;
      
      // 打印接收完成信息
      uint8_t complete_msg[100];
      sprintf((char*)complete_msg, "\r\n=== Transfer Complete ===\r\nTotal: %ld bytes\r\n", total_bytes_received);
      HAL_UART_Transmit(&huart1, complete_msg, strlen((char*)complete_msg), 100);
      
      // 重置计数器
      bytes_received = 0;
      last_print_count = 0;
      for (int i = 0; i < END_SEQUENCE_LENGTH; i++) {
        end_sequence_buffer[i] = 0;
      }
      end_sequence_index = 0;
      
      // 停止DMA接收
      HAL_UART_DMAStop(&huart1);
      break;
    }
  }
}

// DMA半传输完成回调（可选，用于提前处理数据）
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart1) {
    // DMA缓冲区半满，可以在这里处理数据
    // 但我们在主循环中处理，所以这里可以留空
  }
}

// DMA传输完成回调
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart1) {
    // DMA缓冲区满，循环模式会自动重新开始
    // 数据处理在主循环中进行
  }
}

/* USER CODE END 1 */
