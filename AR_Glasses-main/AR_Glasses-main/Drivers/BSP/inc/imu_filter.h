#ifndef __IMU_FILTER_H
#define __IMU_FILTER_H

#include <stdint.h>
#include <math.h>

/* ==================== 常数定义 ==================== */
#define My_2PI              6.2831853071796f
#define G                   9.8f            // 重力加速度
#define DegtoRad            0.0174532925f   // 角度转弧度系数
#define RadtoDeg            57.2957795f     // 弧度转角度系数

/* ==================== 低通滤波器结构体 ==================== */
typedef struct {
    float sample_freq;      // 采样信号频率
    float cutoff_freq;      // 截止频率
    float alpha;            // 滤波系数
    float pre_out;          // 上一次输出
    float now_out;          // 当前低通滤波输出
} LowPassFilter_t;

/* ==================== 卡尔曼滤波器结构体 ==================== */
typedef struct {
    float Last_P;           // 上一次估计误差协方差
    float Now_P;            // 当前估计误差协方差
    float out;              // 卡尔曼滤波输出
    float Kg;               // 卡尔曼增益
    float Q;                // 过程噪声协方差
    float R;                // 测量噪声协方差
} KalmanFilter_t;

/* ==================== 三轴向量结构体 ==================== */
typedef struct {
    float x;
    float y;
    float z;
} Vector3f_t;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} Vector3s_t;

/* ==================== 全局变量声明 ==================== */
extern Vector3f_t gyro_vector;          // 陀螺仪向量（已滤波）
extern Vector3f_t acc_vector;           // 加速度计向量（已滤波）
extern Vector3f_t gyro_offset;          // 陀螺仪偏置
extern Vector3f_t acc_offset;           // 加速度计偏置
extern float pitch, roll, yaw;          // 欧拉角（度）

/* ==================== 低通滤波函数 ==================== */

/**
 * @brief  初始化低通滤波器
 * @param  filter: 滤波器结构体指针
 * @param  sample_freq: 采样频率（Hz）
 * @param  cutoff_freq: 截止频率（Hz）
 * @retval 无
 */
void LowPassFilter_Init(LowPassFilter_t *filter, float sample_freq, float cutoff_freq);

/**
 * @brief  低通滤波输出
 * @param  filter: 滤波器结构体指针
 * @param  sample: 输入样本值
 * @retval 滤波后的输出值
 */
float LowPassFilter_Out(LowPassFilter_t *filter, float sample);

/* ==================== 卡尔曼滤波函数 ==================== */

/**
 * @brief  初始化卡尔曼滤波器
 * @param  kf: 卡尔曼滤波器结构体指针
 * @retval 无
 */
void KalmanFilter_Init(KalmanFilter_t *kf);

/**
 * @brief  卡尔曼滤波处理
 * @param  kf: 卡尔曼滤波器结构体指针
 * @param  input: 输入测量值
 * @retval 滤波后的输出值
 */
float KalmanFilter_Process(KalmanFilter_t *kf, float input);

/* ==================== IMU 数据处理函数 ==================== */

/**
 * @brief  初始化 IMU 滤波器（低通滤波 + 卡尔曼滤波）
 * @param  sample_freq: 采样频率（Hz）
 * @param  cutoff_freq: 截止频率（Hz）
 * @retval 无
 */
void IMU_Filter_Init(float sample_freq, float cutoff_freq);

/**
 * @brief  处理 IMU 原始数据（去偏置 + 滤波 + 单位转换）
 * @param  raw_acc_x, raw_acc_y, raw_acc_z: 加速度计原始数据
 * @param  raw_gyro_x, raw_gyro_y, raw_gyro_z: 陀螺仪原始数据
 * @param  acc_scale: 加速度计缩放系数（LSB/g）
 * @param  gyro_scale: 陀螺仪缩放系数（LSB/°/s）
 * @retval 无
 * @note   处理后的数据存储在全局变量 acc_vector 和 gyro_vector 中
 */
void IMU_ProcessRawData(int16_t raw_acc_x, int16_t raw_acc_y, int16_t raw_acc_z,
                        int16_t raw_gyro_x, int16_t raw_gyro_y, int16_t raw_gyro_z,
                        float acc_scale, float gyro_scale);

/**
 * @brief  设置加速度计偏置
 * @param  offset_x, offset_y, offset_z: 偏置值
 * @retval 无
 */
void IMU_SetAccOffset(float offset_x, float offset_y, float offset_z);

/**
 * @brief  设置陀螺仪偏置
 * @param  offset_x, offset_y, offset_z: 偏置值
 * @retval 无
 */
void IMU_SetGyroOffset(float offset_x, float offset_y, float offset_z);

#endif /* __IMU_FILTER_H */

