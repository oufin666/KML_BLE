/*
 * zjut_MahonyAHRS.c
 *
 *  Created on: 2023年3月13日
 *      Author: djw
 */
// Date         Author          Notes
// 29/09/2011   SOH Madgwick    Initial release
// 02/10/2011   SOH Madgwick    Optimised for reduced CPU load

//-------------------------------------------------------------------------------------------
// Header files

#include "mahony.h"

//-------------------------------------------------------------------------------------------
// Definitions
//#pragma section all "cpu0_dsram"         //将变量放入cpu0_dsram段 提高速度
//---------------------------------------***********************************
float twoKi;        // 2 * integral gain (Ki)
float q0, q1, q2, q3;   // quaternion of sensor frame relative to auxiliary frame
float integralFBx, integralFBy, integralFBz;  // integral error terms scaled by Ki
float invSampleFreq;
char anglesComputed;
//*-*-*-**-----------------------------------------------------------------------------
// 调整积分增益
#define twoKpDef    (2.0f * 0.95f)   // 2 * proportional gain
#define twoKiDef    (2.0f * 0.1f)   // 2 * integral gain
void Mahony_Init(float sampleFrequency)
{
    twoKi = twoKiDef;   // 2 * integral gain (Ki)
    q0 = 1.0f;
    q1 = 0.0f;
    q2 = 0.0f;
    q3 = 0.0f;
    integralFBx = 0.0f;
    integralFBy = 0.0f;
    integralFBz = 0.0f;
    anglesComputed = 0;
    invSampleFreq = 1.0f / sampleFrequency;
}
float Mahony_invSqrt(float x)//一个快速计算平方根倒数的算法
{
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long*)&y;
    i = 0x5f3759df - (i>>1);
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));
    y = y * (1.5f - (halfx * y * y));
    return y;
}

//---------------------------------------------------------------------------------------------------
// IMU algorithm update
//融合加速度计和角速度计的四元数更新函数
void MahonyAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az) {
   float recipNorm;
    float halfvx, halfvy, halfvz;
    float halfex, halfey, halfez;
    float qa, qb, qc;

    // 检查加速度计数据的有效性
    float acc_norm = sqrtf(ax * ax + ay * ay + az * az);
    if (acc_norm > 0.1f && acc_norm < 2.0f) {
        // Normalise accelerometer measurement
        recipNorm = Mahony_invSqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        // Estimated direction of gravity and vector perpendicular to magnetic flux
        halfvx = q1 * q3 - q0 * q2;
        halfvy = q0 * q1 + q2 * q3;
        halfvz = q0 * q0 - 0.5f + q3 * q3;

        // Error is sum of cross product between estimated and measured direction of gravity
        halfex = (ay * halfvz - az * halfvy);
        halfey = (az * halfvx - ax * halfvz);
        halfez = (ax * halfvy - ay * halfvx);

        // Compute and apply integral feedback if enabled
        if (twoKi > 0.0f) {
            integralFBx += twoKi * halfex * invSampleFreq;
            integralFBy += twoKi * halfey * invSampleFreq;
            integralFBz += twoKi * halfez * invSampleFreq;
            gx += integralFBx;
            gy += integralFBy;
            gz += integralFBz;
        } else {
            integralFBx = 0.0f;
            integralFBy = 0.0f;
            integralFBz = 0.0f;
        }

        // Apply proportional feedback
        gx += twoKpDef * halfex;
        gy += twoKpDef * halfey;
        gz += twoKpDef * halfez;
    }

    // Integrate rate of change of quaternion
    gx *= (0.5f * invSampleFreq);
    gy *= (0.5f * invSampleFreq);
    gz *= (0.5f * invSampleFreq);
    qa = q0;
    qb = q1;
    qc = q2;
    q0 += (-qb * gx - qc * gy - q3 * gz);
    q1 += (qa * gx + qc * gz - q3 * gy);
    q2 += (qa * gy - qb * gz + q3 * gx);
    q3 += (qa * gz + qb * gy - qc * gx);

    // Normalise quaternion
    float quat_norm = q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3;
    if (quat_norm > 1e-6f) {
        recipNorm = Mahony_invSqrt(quat_norm);
        q0 *= recipNorm;
        q1 *= recipNorm;
        q2 *= recipNorm;
        q3 *= recipNorm;
    }
}

void Mahony_computeAngles()
{
    roll = atan2f(q0*q1 + q2*q3, 0.5f - q1*q1 - q2*q2); 
    pitch = asinf(2.0f * (q1*q3 - q0*q2));
    yaw = atan2f(q1*q2 + q0*q3, 0.5f - q2*q2 - q3*q3); 
    anglesComputed = 1;
}
float getRoll() {
    if (!anglesComputed) Mahony_computeAngles();
    return roll * 57.29578f;
}
float getPitch() {
    if (!anglesComputed) Mahony_computeAngles();
    return pitch * 57.29578f;
}
float getYaw() {
    if (!anglesComputed) Mahony_computeAngles();

    return yaw * 57.29578f + 0.0f;
}
//============================================================================================
// END OF CODE
//============================================================================================
