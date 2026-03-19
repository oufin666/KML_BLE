#include "mahony_ahrs.h"

/* ==================== 私有宏定义 ==================== */
#define RAD_TO_DEG  57.2957795f     // 弧度转角度系数

/* ==================== 全局变量定义 ==================== */
Quaternion_t quaternion = {1.0f, 0.0f, 0.0f, 0.0f};  // 初始四元数
float mahony_roll = 0.0f;
float mahony_pitch = 0.0f;
float mahony_yaw = 0.0f;

/* ==================== 私有变量 ==================== */
static float twoKp = 1.9f;          // 2 * 比例增益 (Kp)，默认 0.95
static float twoKi = 0.2f;          // 2 * 积分增益 (Ki)，默认 0.1
static float integralFBx = 0.0f;    // 积分误差项 x
static float integralFBy = 0.0f;    // 积分误差项 y
static float integralFBz = 0.0f;    // 积分误差项 z
static float invSampleFreq = 0.001f; // 采样周期（秒）
static uint8_t anglesComputed = 0;   // 角度计算标志

/* ==================== 私有函数声明 ==================== */

/**
 * @brief  快速计算平方根倒数（Quake III 算法）
 * @param  x: 输入值
 * @retval 1/sqrt(x)
 */
static float invSqrt(float x);

/* ==================== 私有函数实现 ==================== */

/**
 * @brief  快速平方根倒数算法
 * 
 * 这是著名的 Quake III 快速平方根倒数算法
 * 用于快速计算 1/sqrt(x)，比标准库函数快约 4 倍
 */
static float invSqrt(float x)
{
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));  // 第一次牛顿迭代
    y = y * (1.5f - (halfx * y * y));  // 第二次牛顿迭代（提高精度）
    return y;
}

/* ==================== 公有函数实现 ==================== */

/**
 * @brief  初始化 Mahony AHRS 算法
 */
void Mahony_Init(float sample_frequency)
{
    /* 设置采样周期 */
    invSampleFreq = 1.0f / sample_frequency;
    
    /* 初始化四元数为单位四元数 */
    quaternion.q0 = 1.0f;
    quaternion.q1 = 0.0f;
    quaternion.q2 = 0.0f;
    quaternion.q3 = 0.0f;
    
    /* 清零积分误差 */
    integralFBx = 0.0f;
    integralFBy = 0.0f;
    integralFBz = 0.0f;
    
    /* 清零角度计算标志 */
    anglesComputed = 0;
}

/**
 * @brief  Mahony AHRS 更新函数（IMU 模式）
 * 
 * 算法原理：
 * 1. 使用加速度计测量重力方向，作为参考向量
 * 2. 使用四元数预测重力方向
 * 3. 计算预测值与测量值的误差（叉积）
 * 4. 使用 PI 控制器补偿陀螺仪漂移
 * 5. 更新四元数
 */
void Mahony_UpdateIMU(float gx, float gy, float gz, float ax, float ay, float az)
{
    float recipNorm;
    float halfvx, halfvy, halfvz;
    float halfex, halfey, halfez;
    float qa, qb, qc;
    
    /* 检查加速度计数据的有效性（避免除零错误） */
    float acc_norm = sqrtf(ax * ax + ay * ay + az * az);
    if (acc_norm > 0.1f && acc_norm < 20.0f) {  // 合理的加速度范围
        /* 归一化加速度计测量值 */
        recipNorm = invSqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;
        
        /* 从四元数估计重力方向（单位向量） */
        halfvx = quaternion.q1 * quaternion.q3 - quaternion.q0 * quaternion.q2;
        halfvy = quaternion.q0 * quaternion.q1 + quaternion.q2 * quaternion.q3;
        halfvz = quaternion.q0 * quaternion.q0 - 0.5f + quaternion.q3 * quaternion.q3;
        
        /* 计算误差：测量值与估计值的叉积 */
        halfex = (ay * halfvz - az * halfvy);
        halfey = (az * halfvx - ax * halfvz);
        halfez = (ax * halfvy - ay * halfvx);
        
        /* 积分误差（如果启用积分增益） */
        if (twoKi > 0.0f) {
            integralFBx += twoKi * halfex * invSampleFreq;
            integralFBy += twoKi * halfey * invSampleFreq;
            integralFBz += twoKi * halfez * invSampleFreq;
            gx += integralFBx;  // 应用积分反馈
            gy += integralFBy;
            gz += integralFBz;
        } else {
            integralFBx = 0.0f;
            integralFBy = 0.0f;
            integralFBz = 0.0f;
        }
        
        /* 应用比例反馈 */
        gx += twoKp * halfex;
        gy += twoKp * halfey;
        gz += twoKp * halfez;
    }
    
    /* 积分四元数变化率 */
    gx *= (0.5f * invSampleFreq);  // 预乘采样周期
    gy *= (0.5f * invSampleFreq);
    gz *= (0.5f * invSampleFreq);
    
    qa = quaternion.q0;
    qb = quaternion.q1;
    qc = quaternion.q2;
    
    /* 四元数微分方程 */
    quaternion.q0 += (-qb * gx - qc * gy - quaternion.q3 * gz);
    quaternion.q1 += (qa * gx + qc * gz - quaternion.q3 * gy);
    quaternion.q2 += (qa * gy - qb * gz + quaternion.q3 * gx);
    quaternion.q3 += (qa * gz + qb * gy - qc * gx);
    
    /* 归一化四元数 */
    float quat_norm = quaternion.q0 * quaternion.q0 + quaternion.q1 * quaternion.q1 + 
                      quaternion.q2 * quaternion.q2 + quaternion.q3 * quaternion.q3;
    
    if (quat_norm > 1e-6f) {  // 避免除零
        recipNorm = invSqrt(quat_norm);
        quaternion.q0 *= recipNorm;
        quaternion.q1 *= recipNorm;
        quaternion.q2 *= recipNorm;
        quaternion.q3 *= recipNorm;
    }
    
    /* 标记角度需要重新计算 */
    anglesComputed = 0;
}

/**
 * @brief  从四元数计算欧拉角
 * 
 * 转换公式（ZYX 旋转顺序）：
 * Roll  = atan2(2*(q0*q1 + q2*q3), 1 - 2*(q1^2 + q2^2))
 * Pitch = asin(2*(q0*q2 - q3*q1))
 * Yaw   = atan2(2*(q0*q3 + q1*q2), 1 - 2*(q2^2 + q3^2))
 */
void Mahony_ComputeAngles(void)
{
    mahony_roll = atan2f(quaternion.q0 * quaternion.q1 + quaternion.q2 * quaternion.q3, 
                         0.5f - quaternion.q1 * quaternion.q1 - quaternion.q2 * quaternion.q2);
    
    mahony_pitch = asinf(2.0f * (quaternion.q1 * quaternion.q3 - quaternion.q0 * quaternion.q2));
    
    mahony_yaw = atan2f(quaternion.q1 * quaternion.q2 + quaternion.q0 * quaternion.q3, 
                        0.5f - quaternion.q2 * quaternion.q2 - quaternion.q3 * quaternion.q3);
    
    anglesComputed = 1;
}

/**
 * @brief  获取滚转角（度）
 */
float Mahony_GetRoll(void)
{
    if (!anglesComputed) {
        Mahony_ComputeAngles();
    }
    return mahony_roll * RAD_TO_DEG;
}

/**
 * @brief  获取俯仰角（度）
 */
float Mahony_GetPitch(void)
{
    if (!anglesComputed) {
        Mahony_ComputeAngles();
    }
    return mahony_pitch * RAD_TO_DEG;
}

/**
 * @brief  获取偏航角（度）
 */
float Mahony_GetYaw(void)
{
    if (!anglesComputed) {
        Mahony_ComputeAngles();
    }
    return mahony_yaw * RAD_TO_DEG;
}

/**
 * @brief  获取四元数
 */
void Mahony_GetQuaternion(Quaternion_t *q)
{
    if (q != NULL) {
        q->q0 = quaternion.q0;
        q->q1 = quaternion.q1;
        q->q2 = quaternion.q2;
        q->q3 = quaternion.q3;
    }
}

/**
 * @brief  重置四元数为初始状态
 */
void Mahony_Reset(void)
{
    quaternion.q0 = 1.0f;
    quaternion.q1 = 0.0f;
    quaternion.q2 = 0.0f;
    quaternion.q3 = 0.0f;
    
    integralFBx = 0.0f;
    integralFBy = 0.0f;
    integralFBz = 0.0f;
    
    anglesComputed = 0;
}

/**
 * @brief  设置 Mahony 算法的增益参数
 */
void Mahony_SetGains(float kp, float ki)
{
    twoKp = 2.0f * kp;
    twoKi = 2.0f * ki;
}

