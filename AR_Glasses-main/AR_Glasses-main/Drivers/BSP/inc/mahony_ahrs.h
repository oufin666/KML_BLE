#ifndef __MAHONY_AHRS_H
#define __MAHONY_AHRS_H

#include <stdint.h>
#include <math.h>

/* ==================== 四元数结构体 ==================== */
typedef struct {
    float q0;   // 标量部分
    float q1;   // i 分量
    float q2;   // j 分量
    float q3;   // k 分量
} Quaternion_t;

/* ==================== 全局变量声明 ==================== */
extern Quaternion_t quaternion;  // 四元数
extern float mahony_roll;        // 欧拉角：滚转角（弧度）
extern float mahony_pitch;       // 欧拉角：俯仰角（弧度）
extern float mahony_yaw;         // 欧拉角：偏航角（弧度）

/* ==================== Mahony AHRS 函数 ==================== */

/**
 * @brief  初始化 Mahony AHRS 算法
 * @param  sample_frequency: 采样频率（Hz），通常为 1000Hz（1ms 一次）
 * @retval 无
 * @note   必须在使用其他函数前调用
 */
void Mahony_Init(float sample_frequency);

/**
 * @brief  Mahony AHRS 更新函数（IMU 模式，仅使用加速度计和陀螺仪）
 * @param  gx, gy, gz: 陀螺仪数据（rad/s）
 * @param  ax, ay, az: 加速度计数据（m/s²）
 * @retval 无
 * @note   该函数融合加速度计和陀螺仪数据，更新四元数
 *         调用频率应与初始化时的采样频率一致
 */
void Mahony_UpdateIMU(float gx, float gy, float gz, float ax, float ay, float az);

/**
 * @brief  从四元数计算欧拉角
 * @param  无
 * @retval 无
 * @note   计算结果存储在全局变量 mahony_roll, mahony_pitch, mahony_yaw 中（弧度制）
 */
void Mahony_ComputeAngles(void);

/**
 * @brief  获取滚转角（Roll）
 * @param  无
 * @retval 滚转角（度）
 */
float Mahony_GetRoll(void);

/**
 * @brief  获取俯仰角（Pitch）
 * @param  无
 * @retval 俯仰角（度）
 */
float Mahony_GetPitch(void);

/**
 * @brief  获取偏航角（Yaw）
 * @param  无
 * @retval 偏航角（度）
 */
float Mahony_GetYaw(void);

/**
 * @brief  获取四元数
 * @param  q: 指向四元数结构体的指针
 * @retval 无
 */
void Mahony_GetQuaternion(Quaternion_t *q);

/**
 * @brief  重置四元数为初始状态
 * @param  无
 * @retval 无
 */
void Mahony_Reset(void);

/**
 * @brief  设置 Mahony 算法的增益参数
 * @param  kp: 比例增益（通常 0.5 ~ 2.0）
 * @param  ki: 积分增益（通常 0.0 ~ 0.5）
 * @retval 无
 * @note   增益越大，收敛越快，但可能产生振荡
 */
void Mahony_SetGains(float kp, float ki);

#endif /* __MAHONY_AHRS_H */

