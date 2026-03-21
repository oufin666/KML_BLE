#ifndef __SOFT_I2C_H
#define __SOFT_I2C_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ==================== 软件I2C引脚定义 ==================== */
/* 
 * 注意：旧版PCB的引脚连接是反的
 * PB8 -> MPU6050 SDA
 * PB9 -> MPU6050 SCL
 */
#define SOFT_I2C_SDA_PORT       GPIOB
#define SOFT_I2C_SDA_PIN        GPIO_PIN_8
#define SOFT_I2C_SCL_PORT       GPIOB
#define SOFT_I2C_SCL_PIN        GPIO_PIN_9

/* ==================== 软件I2C延时参数 ==================== */
#define SOFT_I2C_DELAY_US       5   // 延时微秒数，控制I2C速度

/* ==================== SDA方向控制 ==================== */
#define SOFT_I2C_SDA_HIGH()     HAL_GPIO_WritePin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN, GPIO_PIN_SET)
#define SOFT_I2C_SDA_LOW()      HAL_GPIO_WritePin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN, GPIO_PIN_RESET)
#define SOFT_I2C_SDA_READ()     HAL_GPIO_ReadPin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN)

/* ==================== SCL控制 ==================== */
#define SOFT_I2C_SCL_HIGH()     HAL_GPIO_WritePin(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN, GPIO_PIN_SET)
#define SOFT_I2C_SCL_LOW()      HAL_GPIO_WritePin(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN, GPIO_PIN_RESET)
#define SOFT_I2C_SCL_READ()     HAL_GPIO_ReadPin(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN)

/* ==================== 函数声明 ==================== */

/**
 * @brief  初始化软件I2C引脚
 * @retval None
 * @note   需要在CubeMX中配置好GPIO引脚为开漏输出模式
 */
void Soft_I2C_Init(void);

/**
 * @brief  产生I2C起始信号
 * @retval None
 */
void Soft_I2C_Start(void);

/**
 * @brief  产生I2C停止信号
 * @retval None
 */
void Soft_I2C_Stop(void);

/**
 * @brief  等待从机应答信号
 * @retval 0-收到应答 | 1-未收到应答
 */
uint8_t Soft_I2C_WaitAck(void);

/**
 * @brief  主机发送应答信号
 * @param  ack: 0-发送ACK, 1-发送NACK
 * @retval None
 */
void Soft_I2C_SendAck(uint8_t ack);

/**
 * @brief  发送一个字节
 * @param  data: 要发送的数据
 * @retval None
 */
void Soft_I2C_SendByte(uint8_t data);

/**
 * @brief  接收一个字节
 * @param  ack: 0-发送ACK, 1-发送NACK
 * @retval 接收到的数据
 */
uint8_t Soft_I2C_ReceiveByte(uint8_t ack);

/**
 * @brief  写入一个字节到指定设备寄存器
 * @param  dev_addr: 设备地址（8位，已左移1位）
 * @param  reg_addr: 寄存器地址
 * @param  data: 要写入的数据
 * @retval 0-成功 | 1-失败
 */
uint8_t Soft_I2C_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);

/**
 * @brief  从指定设备寄存器读取一个字节
 * @param  dev_addr: 设备地址（8位，已左移1位）
 * @param  reg_addr: 寄存器地址
 * @retval 读取的数据（0xFF表示失败）
 */
uint8_t Soft_I2C_ReadReg(uint8_t dev_addr, uint8_t reg_addr);

/**
 * @brief  从指定设备寄存器连续读取多个字节
 * @param  dev_addr: 设备地址（8位，已左移1位）
 * @param  reg_addr: 起始寄存器地址
 * @param  buffer: 数据缓冲区
 * @param  length: 要读取的字节数
 * @retval 0-成功 | 1-失败
 */
uint8_t Soft_I2C_ReadMultiBytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *buffer, uint16_t length);

#endif /* __SOFT_I2C_H */
