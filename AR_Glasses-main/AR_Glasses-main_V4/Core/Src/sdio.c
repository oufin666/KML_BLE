/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sdio.c
  * @brief   This file provides code for the configuration
  *          of the SDIO instances.
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
#include "sdio.h"

/* USER CODE BEGIN 0 */
#include <string.h>
#include <stdio.h>
#include "usart.h"
/* USER CODE END 0 */

SD_HandleTypeDef hsd;
DMA_HandleTypeDef hdma_sdio_rx;
DMA_HandleTypeDef hdma_sdio_tx;

/* SDIO init function */

void MX_SDIO_SD_Init(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 158;
  /* USER CODE BEGIN SDIO_Init 2 */

  /* USER CODE END SDIO_Init 2 */

}

void HAL_SD_MspInit(SD_HandleTypeDef* sdHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(sdHandle->Instance==SDIO)
  {
  /* USER CODE BEGIN SDIO_MspInit 0 */

  /* USER CODE END SDIO_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDIO|RCC_PERIPHCLK_CLK48;
    PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
    PeriphClkInitStruct.SdioClockSelection = RCC_SDIOCLKSOURCE_CLK48;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* SDIO clock enable */
    __HAL_RCC_SDIO_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**SDIO GPIO Configuration
    PA6     ------> SDIO_CMD
    PB15     ------> SDIO_CK
    PB4     ------> SDIO_D0
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;  // 修改：CMD需要上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PB15 = SDIO_CK (时钟不需要上拉)
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // PB4 = SDIO_D0 (数据线需要上拉)
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;  // 修改：D0需要上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* SDIO DMA Init */
    /* SDIO_RX Init */
    hdma_sdio_rx.Instance = DMA2_Stream3;
    hdma_sdio_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_sdio_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_sdio_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdio_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdio_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdio_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdio_rx.Init.Mode = DMA_PFCTRL;
    hdma_sdio_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_sdio_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdio_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdio_rx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdio_rx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&hdma_sdio_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(sdHandle,hdmarx,hdma_sdio_rx);

    /* SDIO_TX Init */
    hdma_sdio_tx.Instance = DMA2_Stream6;
    hdma_sdio_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_sdio_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_sdio_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdio_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdio_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdio_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdio_tx.Init.Mode = DMA_PFCTRL;
    hdma_sdio_tx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_sdio_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdio_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdio_tx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdio_tx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&hdma_sdio_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(sdHandle,hdmatx,hdma_sdio_tx);

    /* SDIO interrupt Init */
    HAL_NVIC_SetPriority(SDIO_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);
  /* USER CODE BEGIN SDIO_MspInit 1 */

  /* USER CODE END SDIO_MspInit 1 */
  }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef* sdHandle)
{

  if(sdHandle->Instance==SDIO)
  {
  /* USER CODE BEGIN SDIO_MspDeInit 0 */

  /* USER CODE END SDIO_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SDIO_CLK_DISABLE();

    /**SDIO GPIO Configuration
    PA6     ------> SDIO_CMD
    PB15     ------> SDIO_CK
    PB4     ------> SDIO_D0
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15|GPIO_PIN_4);

    /* SDIO DMA DeInit */
    HAL_DMA_DeInit(sdHandle->hdmarx);
    HAL_DMA_DeInit(sdHandle->hdmatx);

    /* SDIO interrupt Deinit */
    HAL_NVIC_DisableIRQ(SDIO_IRQn);
  /* USER CODE BEGIN SDIO_MspDeInit 1 */

  /* USER CODE END SDIO_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

// 辅助函数：输出调试信息
void SD_DebugPrint(const char* msg) {
  HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
}

void MX_SDIO_SD_Init_FIX(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */
  // 配置SDIO时钟源（关键步骤！）
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDIO|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
  PeriphClkInitStruct.SdioClockSelection = RCC_SDIOCLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
    SD_DebugPrint("ERROR: SDIO clock config failed\r\n");
    return;
  }
  
  // 使能SDIO时钟和GPIO时钟（关键步骤！）
  __HAL_RCC_SDIO_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  // 初始化SDIO相关GPIO
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  // PA6 = SDIO_CMD
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  // PB15 = SDIO_CK
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  // PB4 = SDIO_D0
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  /* USER CODE END SDIO_Init 1 */
	 hsd.Instance = SDIO;
	  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
	  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
	  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
	  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
	  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
	  hsd.Init.ClockDiv = 118;  // 降低时钟频率，提高兼容性 (48MHz / 120 = 400kHz)

  /* USER CODE BEGIN SDIO_Init 2 */
  // 输出开始初始化信息
  SD_DebugPrint("Starting SD Init...\r\n");
  SD_DebugPrint("Step 1: Power on delay...\r\n");
  
  // 延时等待SD卡上电稳定（增加到500ms）
  HAL_Delay(500);
  
  SD_DebugPrint("Step 2: SDIO peripheral init...\r\n");
  // 初始化SDIO外设
  if (SDIO_Init(hsd.Instance, hsd.Init) != HAL_OK) {
    SD_DebugPrint("ERROR: SDIO peripheral init failed\r\n");
    return;
  }
  
  SD_DebugPrint("Step 3: Enable SDIO clock output...\r\n");
  // 使能SDIO时钟输出（关键步骤！）
  __HAL_SD_ENABLE(&hsd);
  
  SD_DebugPrint("Step 4: Power ON...\r\n");
  // 上电
  SDIO_PowerState_ON(hsd.Instance);
  HAL_Delay(10);  // 增加延时，等待上电稳定
  
  // 检查SDIO时钟寄存器
  uint8_t clk_msg[50];
  sprintf((char*)clk_msg, "SDIO CLKCR: 0x%08lX\r\n", hsd.Instance->CLKCR);
  SD_DebugPrint((char*)clk_msg);
  sprintf((char*)clk_msg, "SDIO POWER: 0x%08lX\r\n", hsd.Instance->POWER);
  SD_DebugPrint((char*)clk_msg);
  
  SD_DebugPrint("Step 5: Send CMD0 (GO_IDLE_STATE)...\r\n");
  // 发送CMD0，让SD卡进入空闲状态
  SDIO_CmdInitTypeDef cmd_init;
  cmd_init.Argument = 0;
  cmd_init.CmdIndex = 0;  // CMD0
  cmd_init.Response = SDIO_RESPONSE_NO;
  cmd_init.WaitForInterrupt = SDIO_WAIT_NO;
  cmd_init.CPSM = SDIO_CPSM_ENABLE;
  SDIO_SendCommand(hsd.Instance, &cmd_init);
  
  // 等待命令发送完成
  uint32_t timeout = 1000;
  while(__SDIO_GET_FLAG(hsd.Instance, SDIO_FLAG_CMDSENT) == RESET) {
    if(--timeout == 0) {
      SD_DebugPrint("ERROR: CMD0 timeout\r\n");
      uint8_t msg[50];
      sprintf((char*)msg, "SDIO STA: 0x%08lX\r\n", hsd.Instance->STA);
      SD_DebugPrint((char*)msg);
      return;
    }
  }
  __SDIO_CLEAR_FLAG(hsd.Instance, SDIO_FLAG_CMDSENT);
  SD_DebugPrint("CMD0 OK\r\n");
  
  HAL_Delay(10);
  
  SD_DebugPrint("Step 5: Send CMD8 (SEND_IF_COND)...\r\n");
  // 发送CMD8，检查SD卡版本
  cmd_init.Argument = 0x1AA;  // 电压范围2.7-3.6V，检查模式0xAA
  cmd_init.CmdIndex = 8;  // CMD8
  cmd_init.Response = SDIO_RESPONSE_SHORT;
  cmd_init.WaitForInterrupt = SDIO_WAIT_NO;
  cmd_init.CPSM = SDIO_CPSM_ENABLE;
  SDIO_SendCommand(hsd.Instance, &cmd_init);
  
  // 等待响应
  timeout = 1000;
  while(__SDIO_GET_FLAG(hsd.Instance, SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT) == RESET) {
    if(--timeout == 0) {
      SD_DebugPrint("ERROR: CMD8 timeout\r\n");
      uint8_t msg[50];
      sprintf((char*)msg, "SDIO STA: 0x%08lX\r\n", hsd.Instance->STA);
      SD_DebugPrint((char*)msg);
      // 不返回，继续执行，可能是SD V1.x卡
      SD_DebugPrint("Continuing without CMD8 (SD V1.x or MMC)\r\n");
      break;
    }
  }
  
  if(__SDIO_GET_FLAG(hsd.Instance, SDIO_FLAG_CTIMEOUT)) {
    __SDIO_CLEAR_FLAG(hsd.Instance, SDIO_FLAG_CTIMEOUT);
    SD_DebugPrint("CMD8 timeout (SD V1.x or MMC)\r\n");
  } else if(__SDIO_GET_FLAG(hsd.Instance, SDIO_FLAG_CCRCFAIL)) {
    __SDIO_CLEAR_FLAG(hsd.Instance, SDIO_FLAG_CCRCFAIL);
    SD_DebugPrint("CMD8 CRC fail\r\n");
  } else if(__SDIO_GET_FLAG(hsd.Instance, SDIO_FLAG_CMDREND)) {
    __SDIO_CLEAR_FLAG(hsd.Instance, SDIO_FLAG_CMDREND);
    uint32_t response = SDIO_GetResponse(hsd.Instance, SDIO_RESP1);
    uint8_t msg[50];
    sprintf((char*)msg, "CMD8 OK, Response: 0x%08lX\r\n", response);
    SD_DebugPrint((char*)msg);
  }
  
  SD_DebugPrint("Step 6: Calling HAL_SD_Init()...\r\n");
  
  // 实际初始化SD卡
  HAL_StatusTypeDef status = HAL_SD_Init(&hsd);
  if (status != HAL_OK)
  {
    // SD卡初始化失败，输出详细错误信息
    uint8_t msg[80];
    sprintf((char*)msg, "HAL_SD_Init failed: %d, State: %d, Error: 0x%08lX\r\n", 
            status, hsd.State, hsd.ErrorCode);
    SD_DebugPrint((char*)msg);
    
    // 尝试使用更低的时钟频率重新初始化
    HAL_Delay(500);
    SD_DebugPrint("Retrying with lower clock...\r\n");
    
    hsd.Init.ClockDiv = 158;  // 更低的时钟频率 (48MHz / 160 = 300kHz)
    status = HAL_SD_Init(&hsd);
    if (status != HAL_OK)
    {
      uint8_t fail_msg[80];
      sprintf((char*)fail_msg, "Retry failed: %d, Error: 0x%08lX\r\n", 
              status, hsd.ErrorCode);
      SD_DebugPrint((char*)fail_msg);
      return;
    }
  }
  
  // 输出SD卡初始化成功信息
  SD_DebugPrint("SD Init OK\r\n");
  
  // 输出SD卡信息
  uint8_t info_msg[100];
  sprintf((char*)info_msg, "SD Card: %lu MB, BlockSize: %lu\r\n", 
          (unsigned long)(hsd.SdCard.BlockNbr * hsd.SdCard.BlockSize / 1024 / 1024),
          (unsigned long)hsd.SdCard.BlockSize);
  SD_DebugPrint((char*)info_msg);
  
  // 初始化成功后，提高时钟频率以加快数据传输
  hsd.Init.ClockDiv = 4;  // 提高到正常速度 (48MHz / 6 = 8MHz)
  SDIO->CLKCR = (SDIO->CLKCR & ~SDIO_CLKCR_CLKDIV) | (hsd.Init.ClockDiv & SDIO_CLKCR_CLKDIV);
  
  SD_DebugPrint("SD Clock increased for data transfer\r\n");
  
  // 启用4位总线宽度
  status = HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B);
  if (status != HAL_OK)
  {
    // 总线宽度配置失败，尝试保持1位模式
    uint8_t msg2[50];
    sprintf((char*)msg2, "SD Bus config failed: %d, using 1-bit mode\r\n", status);
    SD_DebugPrint((char*)msg2);
    // 不返回，继续使用1位模式
  }
  else
  {
    // 输出总线配置成功信息
    SD_DebugPrint("SD Bus config OK (4-bit mode)\r\n");
  }
  /* USER CODE END SDIO_Init 2 */

}
/* USER CODE END 1 */
