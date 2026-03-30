# CubeMX配置修改指南

本指南详细说明了如何在CubeMX中正确配置项目，以确保重新生成代码后不会丢失关键功能。

## 一、备份信息

在修改CubeMX配置前，已备份以下关键文件到 `backup` 目录：
- `backup/freertos.c.backup`
- `backup/usart.c.backup`
- `backup/sdio.c.backup`
- `backup/main.c.backup`

## 二、CubeMX配置修改步骤

### 1. 打开CubeMX项目
- 双击 `STM32F412_TEST_LVGL_SD.ioc` 文件

---

### 2. 时钟配置

#### 步骤1：进入时钟配置
- 点击顶部 `Clock Configuration` 标签

#### 步骤2：配置系统时钟
- **Input Frequency**: 16 MHz (HSI)
- **PLL Source**: HSI
- **PLL M**: 16
- **PLL N**: 192
- **PLL P**: 2
- **PLL Q**: 4
- **PLL R**: 2
- **System Clock**: PLLCLK
- **AHB Prescaler**: 1
- **APB1 Prescaler**: 2
- **APB2 Prescaler**: 1
- **Flash Latency**: 3

**验证**：System Clock应显示为96 MHz

---

### 3. 调试接口配置

#### 步骤1：进入SYS配置
- 左侧菜单 → System Core → SYS

#### 步骤2：配置Debug接口
- **Debug**: Serial Wire
- **Timebase Source**: TIM6 (避免与TIM4 PWM冲突)

---

### 4. GPIO引脚配置（按功能模块）

#### 4.1 蓝牙BLE模块 (USART1)

| 引脚 | 模式 | 配置 | 说明 |
|------|------|------|------|
| PA10 | USART1_TX | 默认 | 蓝牙发送 |
| PA15 | USART1_RX | 默认 | 蓝牙接收 |
| PA11 | GPIO_Input | Pull-up | BLE_KEY 蓝牙按键 |
| PA12 | GPIO_Input | Pull-up | BLE_LINK 蓝牙连接状态 |
| PB2 | GPIO_Output | Push Pull, Low | BLE_RST 蓝牙复位 |

**USART1配置**：
- 模式：Asynchronous
- 波特率：115200
- 数据位：8
- 停止位：1
- 校验：None
- DMA：USART1_RX (DMA2 Stream 2, Circular模式)

#### 4.2 GPS模块 (USART2)

| 引脚 | 模式 | 配置 | 说明 |
|------|------|------|------|
| PA2 | USART2_TX | 默认 | GPS发送 |
| PA3 | USART2_RX | 默认 | GPS接收 |

**USART2配置**：
- 模式：Asynchronous
- 波特率：9600
- 数据位：8
- 停止位：1
- 校验：None

#### 4.3 LCD显示屏 (SPI1)

| 引脚 | 模式 | 配置 | 说明 |
|------|------|------|------|
| PA5 | SPI1_SCK | 默认 | LCD时钟 |
| PA7 | SPI1_MOSI | 默认 | LCD数据 |
| PB0 | GPIO_Output | Push Pull, High | LCD_CS 片选 |
| PA4 | GPIO_Output | Push Pull, Low | LCD_DC 数据/命令选择 |
| PB1 | GPIO_Output | Push Pull, High | LCD_BLK 背光控制 |

**SPI1配置**：
- 模式：Full Duplex Master
- 时钟极性：CPOL Low
- 时钟相位：CPHA 1 Edge
- 波特率预分频：根据需要调整

#### 4.4 SD卡 (SDIO - 4位模式)

| 引脚 | 模式 | 配置 | 说明 |
|------|------|------|------|
| PA6 | SDIO_CMD | Pull-up | SD卡命令线（必须上拉） |
| PB15 | SDIO_CK | No Pull-up | SD卡时钟线 |
| PB4 | SDIO_D0 | Pull-up | 数据线0（必须上拉） |
| PA8 | SDIO_D1 | No Pull-up | 数据线1 |
| PA9 | SDIO_D2 | No Pull-up | 数据线2 |
| PB5 | SDIO_D3 | No Pull-up | 数据线3 |

**SDIO配置**：
- SDIO模式：SD 4 bits Wide bus
- 时钟分频：根据SD卡速度调整
- DMA：启用SDIO DMA (RX: DMA2 Stream 3, TX: DMA2 Stream 6)

**重要**：PA6 (CMD) 和 PB4 (D0) 必须配置上拉电阻，否则SD卡初始化会失败！

#### 4.5 MPU6050传感器 (I2C1)

MPU6050支持两种I2C通信方式：**硬件I2C** 和 **软件I2C**。请根据您的硬件版本选择合适的配置方式。

##### 方式一：硬件I2C（推荐，适用于新版PCB）

| 引脚 | 模式 | 配置 | 说明 |
|------|------|------|------|
| PB9 | I2C1_SDA | Pull-up | I2C数据线 |
| PB8 | I2C1_SCL | Pull-up | I2C时钟线 |
| PB6 | GPIO_Input | Pull-up | MPU_INT 中断引脚 |

**硬件I2C详细配置步骤**：

1. **启用I2C1外设**
   - 左侧菜单 → Connectivity → I2C1
   - 点击 I2C1 下拉菜单，选择 **I2C** 模式
   - 此时 PB8 和 PB9 引脚会自动配置为 I2C1_SDA 和 I2C1_SCL

2. **配置I2C参数**
   - **I2C Speed Mode**: Fast Mode (400kHz) 或 Standard Mode (100kHz)
   - **I2C Clock Speed (Hz)**: 400000 (Fast Mode) 或 100000 (Standard Mode)
   - **Rise Time (ns)**: 300
   - **Fall Time (ns)**: 300

3. **配置GPIO引脚**
   - 在引脚视图中点击 PB8 和 PB9
   - **GPIO mode**: Alternate Function Open Drain
   - **GPIO Pull-up/Pull-down**: Pull-up (必须配置上拉！)
   - **Maximum output speed**: High

4. **配置中断引脚 (可选)**
   - PB6 配置为 GPIO_Input
   - **GPIO Pull-up/Pull-down**: Pull-up
   - 用于接收MPU6050的数据就绪中断

**重要提示**：
- I2C 的 SDA 和 SCL 引脚必须配置为 **Open Drain** 模式
- 必须启用 **内部上拉** 或外部接上拉电阻（4.7kΩ典型值）
- 如果MPU6050没有响应，尝试降低I2C时钟速度到100kHz

##### 方式二：软件I2C（适用于旧版PCB，SDA/SCL引脚反接）

**适用场景**：如果您的PCB上 PB8 连接到了 MPU6050 的 SDA，PB9 连接到了 MPU6050 的 SCL（引脚反接），请使用软件I2C方式。

| 引脚 | 模式 | 配置 | 说明 |
|------|------|------|------|
| PB8 | GPIO_Output | Output Open Drain, Pull-up | 软件I2C - SDA（连接到MPU6050的SDA） |
| PB9 | GPIO_Output | Output Open Drain, Pull-up | 软件I2C - SCL（连接到MPU6050的SCL） |
| PB6 | GPIO_Input | Pull-up | MPU_INT 中断引脚 |

**软件I2C详细配置步骤**：

1. **配置GPIO引脚（不启用I2C外设）**
   - 左侧菜单 → System Core → GPIO
   - 或直接在引脚视图中点击 PB8 和 PB9

2. **配置PB8（软件SDA）**
   - 点击 PB8 引脚
   - **GPIO mode**: GPIO_Output
   - **GPIO output level**: High
   - **GPIO mode**: Output Open Drain（开漏输出，模拟I2C）
   - **GPIO Pull-up/Pull-down**: Pull-up (必须配置上拉！)
   - **Maximum output speed**: Very High
   - **User Label**: SOFT_I2C_SDA

3. **配置PB9（软件SCL）**
   - 点击 PB9 引脚
   - **GPIO mode**: GPIO_Output
   - **GPIO output level**: High
   - **GPIO mode**: Output Open Drain（开漏输出，模拟I2C）
   - **GPIO Pull-up/Pull-down**: Pull-up (必须配置上拉！)
   - **Maximum output speed**: Very High
   - **User Label**: SOFT_I2C_SCL

4. **配置中断引脚 (可选)**
   - PB6 配置为 GPIO_Input
   - **GPIO Pull-up/Pull-down**: Pull-up

**重要提示**：
- 软件I2C不需要启用I2C1外设
- 引脚必须配置为 **Open Drain** 模式，以模拟I2C的开漏特性
- 必须启用 **内部上拉** 或外部接上拉电阻（4.7kΩ典型值）
- 软件I2C的通信速度比硬件I2C慢，但足够MPU6050使用

##### 如何选择I2C模式

在代码中，通过修改宏定义来选择使用硬件I2C还是软件I2C：

```c
// 在 mpu6050.h 或配置文件中修改此宏定义
#define MPU6050_USE_SOFT_I2C    1   // 1: 使用软件I2C, 0: 使用硬件I2C
```

- **新版PCB**：设置 `MPU6050_USE_SOFT_I2C` 为 `0`，使用硬件I2C
- **旧版PCB（引脚反接）**：设置 `MPU6050_USE_SOFT_I2C` 为 `1`，使用软件I2C

#### 4.6 蜂鸣器驱动 (GPIO翻转 + TIM3中断)

| 引脚 | 模式 | 配置 | 说明 |
|------|------|------|------|
| PB7 | GPIO_Output | Push Pull, Low | 蜂鸣器控制（GPIO翻转） |

**配置步骤**：

1. **配置GPIO引脚**
   - 左侧菜单 → System Core → GPIO
   - 或直接在引脚视图中点击 PB7
   - **GPIO mode**: GPIO_Output
   - **GPIO output level**: Low
   - **GPIO mode**: Output Push Pull
   - **GPIO Pull-up/Pull-down**: No pull-up and no pull-down
   - **Maximum output speed**: High
   - **User Label**: BEEP

2. **配置TIM3**
   - 左侧菜单 → Timers → TIM3
   - 勾选 `Internal Clock` 选项
   - 点击上方 `Parameter Settings` 标签
   - **Clock Source**: Internal Clock
   - **Counter Settings**:
     - `Prescaler (PSC)`: 83（时钟频率96MHz，96MHz/(83+1)=1.142MHz）
     - `Counter Period (ARR)`: 999（1.142MHz/1000=1.142kHz，每次中断翻转GPIO产生约571Hz PWM）
   - **NVIC Settings**:
     - 勾选 `TIM3 global interrupt` 使能中断
     - **Priority**: 10（或根据需要调整）

#### 4.7 电机驱动 (硬件PWM)

| 引脚 | 模式 | 配置 | 说明 |
|------|------|------|------|
| PB1 | TIM3_CH4 | AF2, Push Pull, No Pull | 电机控制（硬件PWM） |

**配置说明**：
- 使用TIM3_CH4的硬件PWM输出控制电机
- PB1作为TIM3_CH4的输出引脚

**详细配置步骤**：

1. **配置TIM3**
   - 左侧菜单 → Timers → TIM3
   - 勾选 `Internal Clock` 选项
   - 点击上方 `Parameter Settings` 标签
   - **Clock Source**: Internal Clock
   - **Counter Settings**:
     - `Prescaler (PSC)`: 7（时钟频率96MHz，96MHz/(7+1)=12MHz）
     - `Counter Period (ARR)`: 999（12MHz/1000=12kHz PWM频率）
   - **PWM Generation Channel 4**:
     - 点击 `Channel 4` 标签
     - **Mode**: PWM Generation CH4
     - **Pulse**: 0（初始占空比0%）
     - **Output Compare Polarity**: High
     - **Output Compare Fast Mode**: Disable

2. **配置GPIO引脚**
   - 左侧菜单 → System Core → GPIO
   - 或直接在引脚视图中点击 PB1
   - **GPIO mode**: Alternate Function Push Pull
   - **GPIO output level**: Low
   - **GPIO Pull-up/Pull-down**: No pull-up and no pull-down
   - **Maximum output speed**: High
   - **Alternate Function**: AF2_TIM3
   - **User Label**: MOTOR

#### 4.8 按键检测模块

| 引脚 | 模式 | 配置 | 说明 |
|------|------|------|------|
| PB14 | GPIO_Input | Pull-up | KEYB 按键B |
| PB13 | GPIO_Input | Pull-up | KEYC 按键C |

**配置步骤**：

1. **配置GPIO引脚**
   - 左侧菜单 → System Core → GPIO
   - 或直接在引脚视图中点击 PB13 和 PB14

2. **配置PB13（KEYC）**
   - 点击 PB13 引脚
   - **GPIO mode**: GPIO_Input
   - **GPIO Pull-up/Pull-down**: Pull-up
   - **Maximum output speed**: Low
   - **User Label**: KEYC

3. **配置PB14（KEYB）**
   - 点击 PB14 引脚
   - **GPIO mode**: GPIO_Input
   - **GPIO Pull-up/Pull-down**: Pull-up
   - **Maximum output speed**: Low
   - **User Label**: KEYB

**硬件连接**：
- 按键通过10kΩ上拉电阻连接到3.3V
- 按下时接地，未按下时为高电平
- 每个按键并联100nF电容用于去抖

#### 4.9 其他GPIO

| 引脚 | 模式 | 配置 | 说明 |
|------|------|------|------|
| PA0 | GPIO_Input | Analog | BAT_DET 电池检测(ADC) |
| PA1 | GPIO_Input | Pull-up | EC_KEY_DET 按键检测 |
| PB10 | GPIO_Output | Push Pull, High | PWR_EN 电源使能 |
| PB12 | GPIO_Output | Push Pull, Low | LED 指示灯 |
| PB3 | GPIO_Input | Pull-up | WAKE 唤醒 |

---

### 5. FreeRTOS配置

#### 步骤1：启用FreeRTOS
- 左侧菜单 → Middleware → FREERTOS
- Interface：CMSIS-RTOS V2

#### 步骤2：配置任务
添加以下任务：

| 任务名称 | 优先级 | 栈大小(Words) | 栈大小(Bytes) | 入口函数 | 说明 |
|---------|--------|--------------|--------------|---------|------|
| TaskMain | osPriorityNormal | 1024 | 4096 | StartTaskMain | 主任务 |
| GPS_Task | osPriorityNormal | 512 | 2048 | StartTaskGPS | GPS任务 |
| HR_Task | osPriorityNormal | 256 | 1024 | StartTaskHR | 心率任务 |
| BLE_KML_Task | osPriorityNormal | 2048 | 8192 | StartBLEKMLTask | BLE KML传输任务 |

**栈大小计算说明**：
- STM32F412CGU6 总RAM：128KB (131072 Bytes)
- 系统预留：约30KB (用于HAL、驱动、全局变量等)
- FreeRTOS内核：约5KB
- 任务栈总和：约15KB (4096+2048+1024+8192)
- 剩余可用：约78KB (可用于其他功能扩展)

**栈大小调整原则**：
1. **TaskMain**：4KB (1024 Words) - 处理文件操作和系统管理
2. **GPS_Task**：2KB (512 Words) - 处理串口数据接收和解析
3. **HR_Task**：1KB (256 Words) - 处理心率传感器数据
4. **BLE_KML_Task**：8KB (2048 Words) - 处理BLE通信和文件传输

#### 步骤3：配置堆内存
- 左侧菜单 → FREERTOS → Config parameters
- 展开 **Memory management settings** 部分
- **Memory Allocation**：Dynamic / Static
- **TOTAL_HEAP_SIZE**：32768 (32KB) - 足够容纳所有任务栈
- **Memory Management scheme**：heap_4

#### 步骤4：配置时钟节拍
- TICK_RATE_HZ: 1000 (1ms节拍)
- 使用TIM6作为时基（避免与HAL_Delay和TIM4 PWM冲突）

---

### 6. FatFS配置

#### 步骤1：启用FatFS
- 左侧菜单 → Middleware → FATFS
- 选择SD Card模式

#### 步骤2：配置参数
- 扇区大小：512
- 簇大小：根据SD卡容量设置
- 长文件名：启用(UTF-8编码)

---

### 7. 生成代码

#### 步骤1：检查配置
- 点击顶部 `Project` → `Settings`
- 确认Toolchain/IDE设置正确

#### 步骤2：生成代码
- 点击右上角 `GENERATE CODE` 按钮
- 等待生成完成

---

## 三、代码验证清单

生成代码后，检查以下内容：

### 3.1 引脚配置验证

检查 `Core/Src/gpio.c` 中的配置：

```c
// 确认PA6 (SDIO_CMD) 配置为上拉
GPIO_InitStruct.Pin = GPIO_PIN_6;
GPIO_InitStruct.Pull = GPIO_PULLUP;

// 确认PB4 (SDIO_D0) 配置为上拉
GPIO_InitStruct.Pin = GPIO_PIN_4;
GPIO_InitStruct.Pull = GPIO_PULLUP;
```

### 3.2 外设初始化顺序

检查 `Core/Src/main.c` 中的初始化顺序：

```c
// 正确顺序：
MX_GPIO_Init();
MX_DMA_Init();
MX_RTC_Init();
MX_USART2_UART_Init();
MX_TIM2_Init();
MX_SPI1_Init();
MX_USART1_UART_Init();
// ... 其他初始化
```

### 3.3 FreeRTOS任务创建

检查 `Core/Src/freertos.c` 中的任务创建：

```c
// 创建所有任务
TaskMainHandle = osThreadNew(StartTaskMain, NULL, &TaskMain_attributes);
GPS_TaskHandle = osThreadNew(StartTaskGPS, NULL, &GPS_Task_attributes);
HR_TaskHandle = osThreadNew(StartTaskHR, NULL, &HR_Task_attributes);
BLE_KML_TaskHandle = osThreadNew(StartBLEKMLTask, NULL, &BLE_KML_Task_attributes);
```

---

## 四、常见问题排查

### 4.1 SD卡初始化失败

**可能原因**：
- PA6或PB4未配置上拉
- SDIO时钟配置不正确
- 电源不稳定

**解决方法**：
1. 检查CubeMX中PA6和PB4的Pull配置是否为Pull-up
2. 降低SDIO时钟分频值
3. 检查电源供电是否稳定

### 4.2 蓝牙通信异常

**可能原因**：
- USART1波特率配置错误
- DMA配置不正确
- 引脚复用配置错误

**解决方法**：
1. 确认USART1波特率为115200
2. 检查DMA配置是否为Circular模式
3. 确认PA10和PA15的Alternate Function为USART1

### 4.3 LCD显示异常

**可能原因**：
- SPI时钟配置不正确
- 引脚配置错误
- 背光未开启

**解决方法**：
1. 降低SPI时钟频率
2. 检查SPI引脚配置
3. 确认PB1输出高电平开启背光

### 4.4 电机不转

**可能原因**：
- 引脚模式错误
- 电源未使能
- 软件模拟PWM实现问题

**解决方法**：
1. 确认PC13配置为GPIO Output Push Pull模式
2. 确认PB10 (PWR_EN) 输出高电平
3. 检查软件模拟PWM代码实现是否正确

### 4.5 I2C通信失败 / MPU6050无法识别

**可能原因**：
- CubeMX中未启用I2C外设
- I2C引脚未配置上拉电阻
- GPIO模式配置错误（不是Open Drain）
- I2C时钟速度过快
- HAL库中I2C模块未启用

**解决方法**：
1. **检查CubeMX配置**：
   - 确认 Connectivity → I2C1 已选择 I2C 模式
   - 确认 PB8 和 PB9 已自动分配为 I2C1_SDA 和 I2C1_SCL

2. **检查GPIO配置**：
   - PB8/PB9 的 GPIO mode 必须是 **Alternate Function Open Drain**
   - PB8/PB9 必须启用 **Pull-up**（内部上拉）

3. **检查HAL配置**：
   - 打开 `Core/Inc/stm32f4xx_hal_conf.h`
   - 确认 `#define HAL_I2C_MODULE_ENABLED` 已启用（未被注释）

4. **降低I2C速度**：
   - 尝试将 I2C Clock Speed 从 400kHz 改为 100kHz

5. **检查硬件连接**：
   - 确认MPU6050供电正常（3.3V）
   - 确认SDA/SCL连接正确，没有反接
   - 确认I2C地址正确（MPU6050默认地址为 0x68）

### 4.6 FreeRTOS堆内存不足

**可能原因**：
- 任务栈大小设置过大
- 堆内存配置不足
- 内存泄漏

**解决方法**：
1. 按照本指南调整各任务栈大小
2. 增加TOTAL_HEAP_SIZE配置
3. 检查代码中是否有内存泄漏

---

## 五、配置文件对照表

| 功能模块 | 主要配置文件 | 关键配置项 |
|---------|------------|-----------|
| 系统时钟 | `rcc.c` | PLL配置、分频系数 |
| GPIO | `gpio.c` | 引脚模式、上下拉、速度 |
| USART1 | `usart.c` | 波特率115200、DMA RX |
| USART2 | `usart.c` | 波特率9600 |
| SPI1 | `spi.c` | 时钟极性/相位、波特率 |
| SDIO | `sdio.c` | 4位模式、GPIO上拉 |
| I2C1 | `i2c.c` | 时钟速度、上拉 |
| 按键检测 | `KEY.c` | 按键状态读取 |
| FreeRTOS | `freertos.c` | 任务创建、栈大小 |
| FatFS | `fatfs.c` | SD卡模式、长文件名 |

---

## 六、版本信息

**文档版本**：2.5
**更新日期**：2026-03-24
**适用项目**：AR_Glasses
**目标芯片**：STM32F412CGU6

**更新内容**：
- 更新了蜂鸣器驱动配置，从TIM4硬件PWM改为TIM3中断+GPIO翻转实现
- 修正了蜂鸣器引脚配置参数
- 添加了按键检测模块配置说明（KEYB和KEYC）
- 更新了电机驱动配置，从PC13软件模拟PWM改为PB1硬件PWM（TIM3_CH4）
