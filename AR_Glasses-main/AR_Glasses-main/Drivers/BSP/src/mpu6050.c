#include "mpu6050.h"
#include <string.h>

/* ==================== 私有函数声明 ==================== */

#if !MPU6050_USE_SOFT_I2C
/**
 * @brief  等待 I2C 传输完成（带超时）- 硬件I2C模式
 * @param  hi2c: I2C 句柄指针
 * @param  timeout: 超时时间（毫秒）
 * @retval 0-成功 | 1-超时失败
 */
static uint8_t MPU6050_WaitI2C(I2C_HandleTypeDef *hi2c, uint32_t timeout);
#endif

/* ==================== 私有函数实现 ==================== */

#if !MPU6050_USE_SOFT_I2C
/**
 * @brief  等待 I2C 传输完成 - 硬件I2C模式
 */
static uint8_t MPU6050_WaitI2C(I2C_HandleTypeDef *hi2c, uint32_t timeout)
{
    uint32_t tick_start = HAL_GetTick();
    
    while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY) {
        if ((HAL_GetTick() - tick_start) > timeout) {
            return 1;
        }
    }
    
    return 0;
}
#endif

/* ==================== 公有函数实现 ==================== */

/**
 * @brief  向 MPU6050 写入寄存器数据
 */
uint8_t MPU6050_WriteReg(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint8_t data)
{
#if MPU6050_USE_SOFT_I2C
    (void)hi2c;  // 软件I2C模式不使用hi2c参数
    return Soft_I2C_WriteReg(MPU6050_ADDRESS, reg_addr, data);
#else
    uint8_t buffer[2] = {reg_addr, data};
    HAL_StatusTypeDef status;
    
    status = HAL_I2C_Master_Transmit(hi2c, MPU6050_ADDRESS, buffer, 2, 100);
    if (status != HAL_OK) {
        return 1;
    }
    
    if (MPU6050_WaitI2C(hi2c, 100) != 0) {
        return 1;
    }
    
    return 0;
#endif
}

/**
 * @brief  从 MPU6050 读取寄存器数据
 */
uint8_t MPU6050_ReadReg(I2C_HandleTypeDef *hi2c, uint8_t reg_addr)
{
#if MPU6050_USE_SOFT_I2C
    (void)hi2c;  // 软件I2C模式不使用hi2c参数
    return Soft_I2C_ReadReg(MPU6050_ADDRESS, reg_addr);
#else
    uint8_t data = 0xFF;
    HAL_StatusTypeDef status;
    
    status = HAL_I2C_Master_Transmit(hi2c, MPU6050_ADDRESS, &reg_addr, 1, 100);
    if (status != HAL_OK) {
        return data;
    }
    
    if (MPU6050_WaitI2C(hi2c, 100) != 0) {
        return data;
    }
    
    status = HAL_I2C_Master_Receive(hi2c, MPU6050_ADDRESS, &data, 1, 100);
    if (status != HAL_OK) {
        return 0xFF;
    }
    
    if (MPU6050_WaitI2C(hi2c, 100) != 0) {
        return 0xFF;
    }
    
    return data;
#endif
}

/**
 * @brief  获取 MPU6050 的 ID
 */
uint8_t MPU6050_GetID(I2C_HandleTypeDef *hi2c)
{
    return MPU6050_ReadReg(hi2c, MPU6050_WHO_AM_I);
}

/**
 * @brief  初始化 MPU6050
 */
uint8_t MPU6050_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t id;
    
#if MPU6050_USE_SOFT_I2C
    (void)hi2c;  // 软件I2C模式不使用hi2c参数
    Soft_I2C_Init();  // 初始化软件I2C引脚
#else
    if (hi2c == NULL) {
        return 1;
    }
#endif
    
    HAL_Delay(100);
    
    id = MPU6050_GetID(hi2c);
    if (id != MPU6050_ID) {
        return 1;
    }
    
    if (MPU6050_WriteReg(hi2c, MPU6050_PWR_MGMT_1, 0x01) != 0) {
        return 1;
    }
    HAL_Delay(10);
    
    if (MPU6050_WriteReg(hi2c, MPU6050_PWR_MGMT_2, 0x00) != 0) {
        return 1;
    }
    
    if (MPU6050_WriteReg(hi2c, MPU6050_SMPLRT_DIV, 0x09) != 0) {
        return 1;
    }
    
    if (MPU6050_WriteReg(hi2c, MPU6050_CONFIG, 0x06) != 0) {
        return 1;
    }
    
    if (MPU6050_WriteReg(hi2c, MPU6050_GYRO_CONFIG, 0x18) != 0) {
        return 1;
    }
    
    if (MPU6050_WriteReg(hi2c, MPU6050_ACCEL_CONFIG, 0x18) != 0) {
        return 1;
    }
    
    HAL_Delay(50);
    
    return 0;
}

/**
 * @brief  读取 MPU6050 的六个原始数据（结构体版本）
 */
uint8_t MPU6050_GetRawData(I2C_HandleTypeDef *hi2c, MPU6050_RawData_t *raw_data)
{
    uint8_t buffer[12];
    uint8_t start_addr = MPU6050_ACCEL_XOUT_H;
    
    if (raw_data == NULL) {
        return 1;
    }
    
#if MPU6050_USE_SOFT_I2C
    (void)hi2c;  // 软件I2C模式不使用hi2c参数
    if (Soft_I2C_ReadMultiBytes(MPU6050_ADDRESS, start_addr, buffer, 12) != 0) {
        return 1;
    }
#else
    if (HAL_I2C_Master_Transmit(hi2c, MPU6050_ADDRESS, &start_addr, 1, 100) != HAL_OK) {
        return 1;
    }
    
    if (MPU6050_WaitI2C(hi2c, 100) != 0) {
        return 1;
    }
    
    if (HAL_I2C_Master_Receive(hi2c, MPU6050_ADDRESS, buffer, 12, 100) != HAL_OK) {
        return 1;
    }
    
    if (MPU6050_WaitI2C(hi2c, 100) != 0) {
        return 1;
    }
#endif
    
    raw_data->accel_x = (int16_t)((buffer[0] << 8) | buffer[1]);
    raw_data->accel_y = (int16_t)((buffer[2] << 8) | buffer[3]);
    raw_data->accel_z = (int16_t)((buffer[4] << 8) | buffer[5]);
    raw_data->gyro_x  = (int16_t)((buffer[6] << 8) | buffer[7]);
    raw_data->gyro_y  = (int16_t)((buffer[8] << 8) | buffer[9]);
    raw_data->gyro_z  = (int16_t)((buffer[10] << 8) | buffer[11]);
    
    return 0;
}

/**
 * @brief  读取 MPU6050 的六个原始数据（指针参数版本）
 */
uint8_t MPU6050_GetData(I2C_HandleTypeDef *hi2c, 
                        int16_t *accel_x, int16_t *accel_y, int16_t *accel_z,
                        int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z)
{
    MPU6050_RawData_t raw_data;
    
    if (accel_x == NULL || accel_y == NULL || accel_z == NULL ||
        gyro_x == NULL || gyro_y == NULL || gyro_z == NULL) {
        return 1;
    }
    
    if (MPU6050_GetRawData(hi2c, &raw_data) != 0) {
        return 1;
    }
    
    *accel_x = raw_data.accel_x;
    *accel_y = raw_data.accel_y;
    *accel_z = raw_data.accel_z;
    *gyro_x = raw_data.gyro_x;
    *gyro_y = raw_data.gyro_y;
    *gyro_z = raw_data.gyro_z;
    
    return 0;
}

/**
 * @brief  MPU6050 测试任务函数
 */
uint8_t MPU6050_Test(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, MPU6050_RawData_t *raw_data)
{
    MPU6050_RawData_t local_data;
    char tx_buffer[128];
    
    if (huart == NULL) {
        return 1;
    }
    
    // 初始化消息
#if MPU6050_USE_SOFT_I2C
    sprintf(tx_buffer, "[MPU6050] Using Software I2C mode (for old PCB with swapped pins)\r\n");
#else
    sprintf(tx_buffer, "[MPU6050] Using Hardware I2C mode\r\n");
#endif
    HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
    
    // 测试1: 初始化
    if (MPU6050_Init(hi2c) != 0) {
        sprintf(tx_buffer, "[MPU6050] Init Failed!\r\n");
        HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
        return 1;  // 初始化失败
    }
    
    sprintf(tx_buffer, "[MPU6050] Init Success! ID: 0x%02X\r\n", MPU6050_GetID(hi2c));
    HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
    
    // 测试2: 读取数据
    if (MPU6050_GetRawData(hi2c, &local_data) == 0) {
        sprintf(tx_buffer, "[MPU6050] Data read success!\r\n");
        HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
        
        sprintf(tx_buffer, "Accel: X=%6d Y=%6d Z=%6d | Gyro: X=%6d Y=%6d Z=%6d\r\n",
                local_data.accel_x, local_data.accel_y, local_data.accel_z,
                local_data.gyro_x, local_data.gyro_y, local_data.gyro_z);
        HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
        
        // 如果提供了raw_data指针，复制数据
        if (raw_data != NULL) {
            *raw_data = local_data;
        }
        
        return 0;  // 初始化成功且数据读取成功
    } else {
        sprintf(tx_buffer, "[MPU6050] Read Error!\r\n");
        HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
        return 2;  // 初始化成功但数据读取失败
    }
}

void MPU6050_TestTask(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart)
{
    MPU6050_RawData_t raw_data;
    char tx_buffer[128];
    
    if (huart == NULL) {
        return;
    }
    
    // 首先执行一次性测试
    uint8_t test_result = MPU6050_Test(hi2c, huart, &raw_data);
    
    // 根据测试结果决定是否继续
    if (test_result == 1) {
        // 初始化失败，退出任务
        sprintf(tx_buffer, "[MPU6050] Test failed, exiting task\r\n");
        HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
        return;
    }
    
    // 测试成功，进入循环读取模式
    sprintf(tx_buffer, "[MPU6050] Entering continuous read mode\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
    
    while (1) {
        if (MPU6050_GetRawData(hi2c, &raw_data) == 0) {
            sprintf(tx_buffer, "Accel: X=%6d Y=%6d Z=%6d | Gyro: X=%6d Y=%6d Z=%6d\r\n",
                    raw_data.accel_x, raw_data.accel_y, raw_data.accel_z,
                    raw_data.gyro_x, raw_data.gyro_y, raw_data.gyro_z);
            HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
        } else {
            sprintf(tx_buffer, "[MPU6050] Read Error!\r\n");
            HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
        }
        
        HAL_Delay(500);
    }
}
