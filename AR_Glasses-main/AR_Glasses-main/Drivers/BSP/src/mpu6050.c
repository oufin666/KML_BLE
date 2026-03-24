#include "mpu6050.h"
#include <string.h>
#include "imu_filter.h"
#include "mahony_ahrs.h"

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
 * @brief  读取 MPU6050 的六个原始数据（指针参数版本）- 内部使用
 */
static uint8_t MPU6050_GetData(I2C_HandleTypeDef *hi2c, 
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
 * @brief  MPU6050 运行自检（初始化并测试数据读取）
 */
uint8_t MPU6050_RunSelfTest(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, MPU6050_RawData_t *raw_data)
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

void MPU6050_StartDataMonitorTask(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart)
{
    MPU6050_RawData_t raw_data;
    char tx_buffer[128];
    
    if (huart == NULL) {
        return;
    }
    
    // 首先执行一次性测试
    uint8_t test_result = MPU6050_RunSelfTest(hi2c, huart, &raw_data);
    
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

/**
 * @brief  MPU6050 初始化和数据读取封装函数
 * @param  hi2c: I2C 句柄指针
 * @param  huart: UART 句柄指针，用于输出调试信息
 * @param  raw_data: 存储原始数据的结构体指针
 * @retval 0-成功 | 1-失败
 */
uint8_t MPU6050_InitAndRead(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, MPU6050_RawData_t *raw_data)
{
    static uint8_t initialized = 0;
    char tx_buffer[128];
    
    // 第一次调用时进行初始化
    if (!initialized) {
        uint8_t test_result = MPU6050_RunSelfTest(hi2c, huart, raw_data);
        if (test_result != 0) {
            return 1; // 初始化失败
        }
        initialized = 1;
    }
    
    // 读取数据
    if (MPU6050_GetRawData(hi2c, raw_data) != 0) {
        if (huart != NULL) {
            sprintf(tx_buffer, "[MPU6050] Read Error!\r\n");
            HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
        }
        return 1; // 读取失败
    }
    
    // 输出原始数据
    if (huart != NULL) {
        sprintf(tx_buffer, "Accel: X=%6d Y=%6d Z=%6d | Gyro: X=%6d Y=%6d Z=%6d\r\n",
                raw_data->accel_x, raw_data->accel_y, raw_data->accel_z,
                raw_data->gyro_x, raw_data->gyro_y, raw_data->gyro_z);
        HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
    }
    
    return 0; // 成功
}

/**
 * @brief  MPU6050 读取数据并计算姿态角的封装函数
 * @param  hi2c: I2C 句柄指针
 * @param  huart: UART 句柄指针，用于输出调试信息
 * @param  raw_data: 存储原始数据的结构体指针
 * @param  roll: 存储Roll角的指针
 * @param  pitch: 存储Pitch角的指针
 * @param  yaw: 存储Yaw角的指针
 * @retval 0-成功 | 1-失败
 */
uint8_t MPU6050_ReadAndCalculateAttitude(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, 
                                         MPU6050_RawData_t *raw_data, 
                                         float *roll, float *pitch, float *yaw)
{
    char tx_buffer[128];
    
    // 读取数据
    if (MPU6050_InitAndRead(hi2c, huart, raw_data) != 0) {
        return 1;
    }
    
    // 计算姿态角
#if ENABLE_IMU_FILTER
    // 处理IMU数据：去偏置 + 滤波 + 单位转换
    // 加速度计：±8g，16位，灵敏度 = 8g / 32768 = 0.000244 g/LSB = 0.002394 m/s²/LSB
    // 陀螺仪：±1000°/s，16位，灵敏度 = 1000°/s / 32768 = 0.0305 °/s/LSB = 0.00533 rad/s/LSB
    IMU_ProcessRawData(raw_data->accel_x, raw_data->accel_y, raw_data->accel_z,
                      raw_data->gyro_x, raw_data->gyro_y, raw_data->gyro_z,
                      0.002394f, 0.005333f);
    
    // 更新Mahony AHRS算法
    Mahony_UpdateIMU(gyro_vector.x, gyro_vector.y, gyro_vector.z,
                    acc_vector.x, acc_vector.y, acc_vector.z);
    
    // 计算姿态角
    Mahony_ComputeAngles();
    
    // 获取姿态角
    if (roll != NULL) *roll = Mahony_GetRoll();
    if (pitch != NULL) *pitch = Mahony_GetPitch();
    if (yaw != NULL) *yaw = Mahony_GetYaw();
    
    // 打印姿态角（度）
    if (huart != NULL) {
        sprintf(tx_buffer, "Attitude: Roll=%6.2f Pitch=%6.2f Yaw=%6.2f (deg)\r\n",
            Mahony_GetRoll(), Mahony_GetPitch(), Mahony_GetYaw());
        HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
    }
#else
    // 不使用滤波，直接从原始数据计算简单姿态角
    // 仅使用加速度计数据计算Roll和Pitch
    float ax = raw_data->accel_x * 0.002394f;
    float ay = raw_data->accel_y * 0.002394f;
    float az = raw_data->accel_z * 0.002394f;
    
    // 计算Roll和Pitch（弧度）
    float roll_val = atan2(ay, az) * 57.2957795f;
    float pitch_val = atan2(-ax, sqrt(ay*ay + az*az)) * 57.2957795f;
    float yaw_val = 0.0f;  // 仅使用加速度计无法计算Yaw
    
    // 赋值
    if (roll != NULL) *roll = roll_val;
    if (pitch != NULL) *pitch = pitch_val;
    if (yaw != NULL) *yaw = yaw_val;
    
    // 打印简单姿态角（度）
    if (huart != NULL) {
        sprintf(tx_buffer, "Simple Attitude: Roll=%6.2f Pitch=%6.2f Yaw=%6.2f (deg)\r\n",
            roll_val, pitch_val, yaw_val);
        HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
    }
#endif
    
    return 0; // 成功
}

/**
 * @brief  系统初始化函数，包含I2C、IMU滤波器和MPU6050的初始化
 * @param  hi2c: I2C 句柄指针（硬件I2C模式使用，软件I2C模式传NULL）
 * @param  huart: 串口句柄指针，用于输出调试信息
 * @param  raw_data: 存储原始数据的结构体指针
 * @retval 0-成功 | 1-失败
 * @note   此函数封装了完整的MPU6050初始化流程，用户只需调用此函数即可完成所有初始化工作
 */
uint8_t MPU6050_SystemInit(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, MPU6050_RawData_t *raw_data)
{
    char tx_buffer[128];
    
    // 硬件I2C模式：初始化I2C
#if !MPU6050_USE_SOFT_I2C
    if (hi2c != NULL) {
        MX_I2C1_Init();
        sprintf(tx_buffer, "[System] I2C1 initialized\r\n");
        HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
    }
#endif
    
    // 初始化IMU滤波器和Mahony AHRS算法
#if ENABLE_IMU_FILTER
    IMU_Filter_Init(100.0f, 5.0f);  // 采样频率100Hz，截止频率5Hz
    Mahony_Init(100.0f);  // 采样频率100Hz
    
    sprintf(tx_buffer, "[System] IMU filters initialized\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
#else
    sprintf(tx_buffer, "[System] IMU filters disabled\r\n");
    HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
#endif
    
    // 执行MPU6050初始化
    if (MPU6050_InitAndRead(hi2c, huart, raw_data) == 0) {
        sprintf(tx_buffer, "[MPU6050] Initialization and first read completed successfully!\r\n");
        HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
        return 0; // 成功
    } else {
        sprintf(tx_buffer, "[MPU6050] Initialization failed!\r\n");
        HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
        return 1; // 失败
    }
}

/**
 * @brief  获取Roll角（横滚角）
 */
float MPU6050_GetRoll(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart)
{
    MPU6050_RawData_t raw_data;
    float roll = 0.0f;
    
    if (MPU6050_ReadAndCalculateAttitude(hi2c, huart, &raw_data, &roll, NULL, NULL) != 0) {
        return 0.0f; // 获取失败
    }
    
    return roll;
}

/**
 * @brief  获取Pitch角（俯仰角）
 */
float MPU6050_GetPitch(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart)
{
    MPU6050_RawData_t raw_data;
    float pitch = 0.0f;
    
    if (MPU6050_ReadAndCalculateAttitude(hi2c, huart, &raw_data, NULL, &pitch, NULL) != 0) {
        return 0.0f; // 获取失败
    }
    
    return pitch;
}

/**
 * @brief  检测翻腕状态（通过检测pitch角的大变化）
 */
int8_t MPU6050_DetectWristRaise(float current_pitch)
{
    static uint8_t initialized = 0;
    static float last_pitch = 0.0f;
    
    // 第一次调用时进行初始化
    if (!initialized) {
        last_pitch = current_pitch;
        initialized = 1;
        return 0; // 初始化完成，未检测到翻腕
    }
    
    // 计算pitch角变化量
    float pitch_diff = current_pitch - last_pitch;
    
    // 翻腕检测算法：
    // pitch角变化量超过阈值
    
    #define WRIST_RAISE_PITCH_THRESHOLD   30.0f  // pitch角变化阈值（度），减小阈值使检测更容易触发
    
    if (fabs(pitch_diff) > WRIST_RAISE_PITCH_THRESHOLD) {
        // 检测到翻腕
        last_pitch = current_pitch;
        return 1;
    }
    
    // 更新状态
    last_pitch = current_pitch;
    
    return 0; // 未检测到翻腕
}

/**
 * @brief  封装的翻腕检测任务函数
 * @param  hi2c: I2C 句柄指针（硬件I2C模式使用，软件I2C模式传NULL）
 * @param  huart: 串口句柄指针，用于输出调试信息
 * @param  interval: 检测间隔（毫秒）
 * @retval 无
 * @note   此函数实现了完整的翻腕检测功能，包括初始化、数据读取和翻腕检测
 */
void MPU6050_WristRaiseTask(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, uint32_t interval)
{
    uint8_t mpu_initialized = 0;
    uint32_t last_mpu_tick = 0;
    uint8_t wrist_raise_flag = 0; // 翻腕标志位
    char tx_buffer[128];
    MPU6050_RawData_t raw_data;
    
    // 1. 初始化MPU6050
    #if MPU6050_USE_SOFT_I2C
    mpu_initialized = (MPU6050_Init(NULL) == 0);
    #else
    mpu_initialized = (MPU6050_Init(hi2c) == 0);
    #endif

    if (mpu_initialized) {
        sprintf(tx_buffer, "[MPU6050] Initialization success!\r\n");
        HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
    } else {
        sprintf(tx_buffer, "[MPU6050] Initialization failed!\r\n");
        HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
        return;
    }
    
    // 2. 主循环
    while (1) {
        // 按指定间隔检测翻腕
        if (mpu_initialized && (HAL_GetTick() - last_mpu_tick > interval)) {
            last_mpu_tick = HAL_GetTick();
            
            // 直接读取原始数据
            uint8_t read_success = 0;
        #if MPU6050_USE_SOFT_I2C
            read_success = (MPU6050_GetRawData(NULL, &raw_data) == 0);
        #else
            read_success = (MPU6050_GetRawData(hi2c, &raw_data) == 0);
        #endif

            if (read_success) {
                // 计算简单的姿态角（仅使用加速度计）
                float ax = raw_data.accel_x * 0.002394f;
                float ay = raw_data.accel_y * 0.002394f;
                float az = raw_data.accel_z * 0.002394f;
                
                // 计算Pitch（弧度转角度）
                float pitch = atan2(-ax, sqrt(ay*ay + az*az)) * 57.2957795f;
                
                // 检测翻腕
                wrist_raise_flag = (MPU6050_DetectWristRaise(pitch) == 1) ? 1 : 0;
                
                // 发送姿态角和翻腕标志位
                sprintf(tx_buffer, "Pitch:%6.2f         Flag:%d\r\n", pitch, wrist_raise_flag);
                HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
            } else {
                sprintf(tx_buffer, "[MPU6050] Read Error!\r\n");
                HAL_UART_Transmit(huart, (uint8_t*)tx_buffer, strlen(tx_buffer), 100);
            }
        }
        
        osDelay(10); // 任务延时
    }
}

