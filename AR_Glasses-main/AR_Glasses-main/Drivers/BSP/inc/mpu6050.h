#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ==================== I2C模式选择 ==================== */
/*
 * MPU6050_USE_SOFT_I2C: 选择使用软件I2C还是硬件I2C
 * 
 * 适用场景：
 * - 旧版PCB（引脚反接）：PB8->SDA, PB9->SCL，设置为 1
 * - 新版PCB（引脚正确）：PB8->SDA, PB9->SCL，设置为 0
 * 
 * 注意：
 * - 使用软件I2C时，不需要在CubeMX中启用I2C1外设
 * - 使用硬件I2C时，需要在CubeMX中正确配置I2C1
 */
#define MPU6050_USE_SOFT_I2C    1   // 1: 使用软件I2C, 0: 使用硬件I2C

/* ==================== 根据I2C模式选择包含头文件 ==================== */
#if MPU6050_USE_SOFT_I2C
#include "soft_i2c.h"
// 软件I2C模式下，定义一个空的I2C_HandleTypeDef类型，避免未定义错误
typedef void* I2C_HandleTypeDef;
#else
#include "stm32f4xx_hal_i2c.h"
#endif

/* ==================== MPU6050 寄存器地址定义 ==================== */
#define MPU6050_SMPLRT_DIV      0x19
#define MPU6050_CONFIG          0x1A
#define MPU6050_GYRO_CONFIG     0x1B
#define MPU6050_ACCEL_CONFIG    0x1C

#define MPU6050_ACCEL_XOUT_H    0x3B
#define MPU6050_ACCEL_XOUT_L    0x3C
#define MPU6050_ACCEL_YOUT_H    0x3D
#define MPU6050_ACCEL_YOUT_L    0x3E
#define MPU6050_ACCEL_ZOUT_H    0x3F
#define MPU6050_ACCEL_ZOUT_L    0x40
#define MPU6050_TEMP_OUT_H      0x41
#define MPU6050_TEMP_OUT_L      0x42
#define MPU6050_GYRO_XOUT_H     0x43
#define MPU6050_GYRO_XOUT_L     0x44
#define MPU6050_GYRO_YOUT_H     0x45
#define MPU6050_GYRO_YOUT_L     0x46
#define MPU6050_GYRO_ZOUT_H     0x47
#define MPU6050_GYRO_ZOUT_L     0x48

#define MPU6050_PWR_MGMT_1      0x6B
#define MPU6050_PWR_MGMT_2      0x6C
#define MPU6050_WHO_AM_I        0x75

/* ==================== MPU6050 I2C 地址 ==================== */
#define MPU6050_ADDRESS         0xD0  // 7位地址 0x68，左移1位得到 0xD0

/* ==================== MPU6050 ID ==================== */
#define MPU6050_ID              0x68

/* ==================== 数据结构体定义 ==================== */
/**
 * @brief MPU6050 原始数据结构体
 */
typedef struct {
    int16_t accel_x;    // 加速度计X轴原始数据
    int16_t accel_y;    // 加速度计Y轴原始数据
    int16_t accel_z;    // 加速度计Z轴原始数据
    int16_t gyro_x;     // 陀螺仪X轴原始数据
    int16_t gyro_y;     // 陀螺仪Y轴原始数据
    int16_t gyro_z;     // 陀螺仪Z轴原始数据
} MPU6050_RawData_t;

/* ==================== 函数声明 ==================== */

/**
 * @brief  初始化 MPU6050
 * @param  hi2c: I2C 句柄指针（硬件I2C模式使用，软件I2C模式传NULL）
 * @retval 0-成功 | 1-失败
 * @note   硬件I2C模式：需要在 CubeMX 中配置好 I2C 接口
 *         软件I2C模式：需要在 CubeMX 中配置好 GPIO 引脚
 */
uint8_t MPU6050_Init(I2C_HandleTypeDef *hi2c);

/**
 * @brief  向 MPU6050 写入寄存器数据
 * @param  hi2c: I2C 句柄指针（硬件I2C模式使用，软件I2C模式传NULL）
 * @param  reg_addr: 寄存器地址
 * @param  data: 要写入的数据
 * @retval 0-成功 | 1-失败
 */
uint8_t MPU6050_WriteReg(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint8_t data);

/**
 * @brief  从 MPU6050 读取寄存器数据
 * @param  hi2c: I2C 句柄指针（硬件I2C模式使用，软件I2C模式传NULL）
 * @param  reg_addr: 寄存器地址
 * @retval 读取的数据（0xFF 表示失败）
 */
uint8_t MPU6050_ReadReg(I2C_HandleTypeDef *hi2c, uint8_t reg_addr);

/**
 * @brief  获取 MPU6050 的 ID
 * @param  hi2c: I2C 句柄指针（硬件I2C模式使用，软件I2C模式传NULL）
 * @retval MPU6050 的 ID（应为 0x68）
 */
uint8_t MPU6050_GetID(I2C_HandleTypeDef *hi2c);

/**
 * @brief  读取 MPU6050 的六个原始数据（加速度 + 陀螺仪）
 * @param  hi2c: I2C 句柄指针（硬件I2C模式使用，软件I2C模式传NULL）
 * @param  raw_data: 指向原始数据结构体的指针
 * @retval 0-成功 | 1-失败
 * @note   读取顺序：AccX -> AccY -> AccZ -> GyroX -> GyroY -> GyroZ
 */
uint8_t MPU6050_GetRawData(I2C_HandleTypeDef *hi2c, MPU6050_RawData_t *raw_data);

/**
 * @brief  读取 MPU6050 的六个原始数据（使用指针参数）
 * @param  hi2c: I2C 句柄指针（硬件I2C模式使用，软件I2C模式传NULL）
 * @param  accel_x: 加速度计X轴指针
 * @param  accel_y: 加速度计Y轴指针
 * @param  accel_z: 加速度计Z轴指针
 * @param  gyro_x: 陀螺仪X轴指针
 * @param  gyro_y: 陀螺仪Y轴指针
 * @param  gyro_z: 陀螺仪Z轴指针
 * @retval 0-成功 | 1-失败
 */
uint8_t MPU6050_GetData(I2C_HandleTypeDef *hi2c, 
                        int16_t *accel_x, int16_t *accel_y, int16_t *accel_z,
                        int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z);

/**
 * @brief  MPU6050 测试函数（执行一次性测试）
 * @param  hi2c: I2C 句柄指针（硬件I2C模式使用，软件I2C模式传NULL）
 * @param  huart: 串口句柄指针，用于输出调试信息
 * @param  raw_data: 用于存储读取的原始数据（可为NULL）
 * @retval 0-初始化成功且数据读取成功 | 1-初始化失败 | 2-初始化成功但数据读取失败
 * @note   此函数执行一次性测试，返回测试结果
 */
uint8_t MPU6050_Test(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, MPU6050_RawData_t *raw_data);

/**
 * @brief  MPU6050 测试任务函数（在FreeRTOS任务中调用）
 * @param  hi2c: I2C 句柄指针（硬件I2C模式使用，软件I2C模式传NULL）
 * @param  huart: 串口句柄指针，用于输出调试信息
 * @retval None
 * @note   此函数会循环读取MPU6050数据并通过串口打印，适合在单独任务中调用
 *         输出格式：Accel: X=xxxx Y=xxxx Z=xxxx | Gyro: X=xxxx Y=xxxx Z=xxxx
 */
void MPU6050_TestTask(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart);

#endif /* __MPU6050_H */
