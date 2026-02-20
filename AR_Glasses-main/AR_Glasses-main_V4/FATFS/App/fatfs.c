/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
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
#include "fatfs.h"

uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  // 挂载文件系统
  if(retSD == 0)
  {
    f_mount(&SDFatFS, SDPath, 1);
  }
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return fat_GetFatTimeFromRTC();
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */
// ★新增：辅助函数 - 创建文件夹（如果文件夹不存在）
FRESULT create_dir_if_not_exist(const char* path)
{
  FRESULT res;
  FATFS *fs;
  // 检查路径是否存在
  res = f_stat(path, NULL);
  if(res == FR_NO_FILE)
  {
    // 文件夹不存在，创建
    res = f_mkdir(path);
  }
  return res;
}

/* USER CODE END Application */
