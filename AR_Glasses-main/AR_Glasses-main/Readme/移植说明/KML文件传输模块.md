# BLE传输KML文件功能移植说明

## 1. 功能概述

本功能实现了通过BLE传输KML文件到设备的SD卡，并支持差分解压算法，提高传输效率。主要包括以下功能：

- 通过USART1的DMA接收BLE传输的KML文件数据
- 流式写入SD卡，无需缓存整个文件
- 支持差分解压算法，减小传输数据量
- FreeRTOS任务管理，确保实时性

## 2. 需要移植的文件

### 2.1 新增文件

| 文件路径 | 说明 |
|---------|------|
| `Drivers/BSP/inc/kml_utils.h` | KML工具函数头文件 |
| `Drivers/BSP/src/kml_utils.c` | KML工具函数实现 |

### 2.2 需要修改的文件

| 文件路径 | 说明 |
|---------|------|
| `Core/Src/usart.c` | 添加DMA接收和处理逻辑 |
| `Core/Inc/usart.h` | 添加函数声明和相关定义 |
| `Core/Src/freertos.c` | 添加BLE KML任务 |
| `Core/Src/main.c` | 确保SD卡和文件系统初始化 |
| `Core/Inc/main.h` | 添加外部变量声明 |

## 3. 移植步骤

### 3.1 复制文件

1. 将 `kml_utils.h` 复制到目标项目的 `Drivers/BSP/inc/` 目录
2. 将 `kml_utils.c` 复制到目标项目的 `Drivers/BSP/src/` 目录

### 3.2 修改 usart.c

1. 在文件开头添加必要的包含：

```c
#include "BLE.h"
#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "ff.h"
#include <string.h>
#include <stdio.h>
```

2. 添加外部变量声明：

```c
// 外部变量声明（声明为volatile确保中断中能正确读取）
extern FIL SDFile;
extern volatile uint8_t sd_file_opened;
extern volatile uint8_t kml_transfer_active;
```

3. 添加KML文件传输相关定义：

```c
// KML文件传输结束序列
#define KML_END_SEQUENCE_1 0xFF
#define KML_END_SEQUENCE_2 0xFF
#define KML_END_SEQUENCE_3 0xFF
#define END_SEQUENCE_LENGTH 3

// DMA双缓冲区实现
#define DMA_BUFFER_SIZE 4096  // 每个DMA缓冲区大小为4KB
#define NUM_BUFFERS 2         // 两个缓冲区

// 应用层缓冲区（用于处理数据）
#define APP_BUFFER_SIZE 4096
```

4. 添加全局变量：

```c
uint8_t dma_rx_buffers[NUM_BUFFERS][DMA_BUFFER_SIZE] __attribute__((aligned(4)));  // DMA双缓冲区，4字节对齐
volatile uint32_t dma_write_pos = 0;  // DMA当前写入位置
volatile uint32_t dma_read_pos = 0;   // 应用程序读取位置
volatile uint8_t current_dma_buffer = 0;  // 当前DMA使用的缓冲区索引

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
```

5. 添加函数：

```c
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
```

### 3.3 修改 usart.h

1. 添加外部函数声明：

```c
// KML文件传输相关函数声明
extern void UART1_Start_DMA_Reception(void);
extern void Process_DMA_Data(void);

// 外部变量声明
extern volatile uint8_t transfer_complete;
extern uint8_t app_buffers[2][4096];
extern volatile uint32_t app_buffer_index[2];
extern volatile uint8_t app_buffer_full[2];
```

### 3.4 修改 freertos.c

1. 添加必要的包含：

```c
#include "ff.h"
#include "fatfs.h"
#include "kml_parse_v2.h"
#include "kml_utils.h"
#include <string.h>
#include <stdio.h>
```

2. 添加外部变量声明：

```c
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
```

3. 添加BLE KML任务的句柄和属性：

```c
// ★新增：BLE KML任务的句柄和属性
osThreadId_t BLE_KML_TaskHandle;
const osThreadAttr_t BLE_KML_Task_attributes = {
  .name = "BLE_KML_Task",
  .stack_size = 2048 * 4,  // 足够处理文件操作的栈大小
  .priority = (osPriority_t) osPriorityNormal,
};
```

4. 声明BLE KML任务函数：

```c
// ★新增：声明BLE KML任务函数
void StartBLEKMLTask(void *argument);
```

5. 在 `MX_FREERTOS_Init` 函数中创建BLE KML任务：

```c
// ★新增：创建BLE KML任务
BLE_KML_TaskHandle = osThreadNew(StartBLEKMLTask, NULL, &BLE_KML_Task_attributes);
```

6. 添加BLE KML任务函数：

```c
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
```

### 3.5 修改 main.c

确保以下初始化步骤存在：

1. 初始化SD卡：

```c
MX_SDIO_SD_Init_FIX();  // 先初始化SD卡硬件
```

2. 初始化FatFS文件系统：

```c
MX_FATFS_Init();  // 再初始化FatFS文件系统
```

3. 确保USART1初始化：

```c
MX_USART1_UART_Init();
```

### 3.6 修改 main.h

添加外部变量声明：

```c
// SD卡文件操作相关全局变量
extern volatile uint8_t sd_file_opened;        // SD文件打开标志
extern volatile uint8_t kml_transfer_active;   // KML传输激活标志
```

## 4. 功能配置

### 4.1 功能开关

在 `kml_utils.h` 文件中，可以通过以下宏定义控制功能：

```c
// 功能开关宏定义
#define KML_ENABLE_CRC          0   // 1: 开启CRC校验, 0: 关闭CRC校验（新算法不需要）
#define KML_ENABLE_DECOMPRESS   1   // 1: 开启解压缩, 0: 关闭解压缩（使用新的差分解压算法）
```

### 4.2 缓冲区大小

在 `usart.c` 文件中，可以根据需要调整缓冲区大小：

```c
// DMA双缓冲区实现
#define DMA_BUFFER_SIZE 4096  // 每个DMA缓冲区大小为4KB
#define NUM_BUFFERS 2         // 两个缓冲区

// 应用层缓冲区（用于处理数据）
#define APP_BUFFER_SIZE 4096
```

## 5. 依赖项

### 5.1 硬件依赖

- STM32F4系列微控制器
- SD卡接口（SDIO或SPI）
- USART1接口（用于BLE通信）
- DMA控制器

### 5.2 软件依赖

- STM32 HAL库
- FreeRTOS
- FatFS文件系统

## 6. 使用方法

1. 确保SD卡已正确插入设备
2. 启动设备，系统会自动初始化SD卡和文件系统
3. BLE KML任务会自动启动，等待接收数据
4. 通过BLE发送KML文件数据，结束时发送 `0xFF 0xFF 0xFF` 作为结束序列
5. 数据接收完成后，系统会自动解压（如果启用）并保存到SD卡

## 7. 注意事项

1. 确保USART1的波特率设置正确（默认115200）
2. 确保SD卡有足够的空间存储KML文件
3. 传输过程中不要移除SD卡
4. 如遇传输失败，检查BLE连接和SD卡状态

## 8. 故障排查

| 问题 | 可能原因 | 解决方案 |
|------|---------|----------|
| 传输失败 | BLE连接断开 | 重新连接BLE设备 |
| 文件写入失败 | SD卡未挂载 | 检查SD卡是否正确插入 |
| 解压失败 | 压缩数据损坏 | 重新传输文件 |
| 缓冲区溢出 | 数据传输速度过快 | 调整缓冲区大小或降低传输速度 |

## 9. 测试方法

1. 准备一个KML文件，使用差分压缩算法压缩
2. 通过BLE发送压缩后的文件数据
3. 发送结束序列 `0xFF 0xFF 0xFF`
4. 检查SD卡中是否生成了解压后的KML文件
5. 验证文件内容是否正确

## 10. 代码结构说明

### 10.1 核心函数

| 函数名 | 说明 | 所在文件 |
|-------|------|---------|
| `kml_init_system` | 初始化KML系统 | kml_utils.c |
| `kml_open_compressed_file` | 打开压缩文件并准备接收数据 | kml_utils.c |
| `kml_process_buffers` | 处理数据缓冲区 | kml_utils.c |
| `kml_handle_transfer_complete` | 处理传输完成 | kml_utils.c |
| `stream_decompress_file` | 差分解压函数 | kml_utils.c |
| `kml_ble_task_main` | BLE KML任务主函数 | kml_utils.c |
| `UART1_Start_DMA_Reception` | 启动DMA接收 | usart.c |
| `Process_DMA_Data` | 处理DMA接收的数据 | usart.c |
| `StartBLEKMLTask` | BLE KML任务函数 | freertos.c |

### 10.2 数据流程

1. BLE设备发送压缩的KML文件数据
2. USART1通过DMA接收数据
3. `Process_DMA_Data` 函数处理接收到的数据，检测结束序列
4. 数据写入应用缓冲区，满后写入SD卡
5. 收到结束序列后，`kml_handle_transfer_complete` 处理传输完成
6. 如果启用解压缩，`stream_decompress_file` 解压文件
7. 系统准备接收下一个文件

## 11. 性能优化

1. **内存优化**：使用双缓冲区和流式处理，避免缓存整个文件
2. **速度优化**：使用DMA接收，减少CPU占用
3. **存储优化**：使用差分解压算法，减小文件大小
4. **稳定性优化**：添加错误处理和状态检查

## 12. 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.0 | 2025-12-01 | 初始版本，实现基本功能 |
| 1.1 | 2025-12-15 | 优化差分解压算法 |
| 1.2 | 2025-12-20 | 改进错误处理和稳定性 |

---

**移植完成后，请确保进行充分的测试，确保功能正常工作。**

---