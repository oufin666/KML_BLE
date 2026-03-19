#ifndef _FILTER_H
#define _FILTER_H

#include <stdbool.h>
#include "fsl_device_registers.h"
#include "mahony.h"

#define My_2PI 6.2831853071796

/* 低通滤波器结构体 */
typedef struct
{
    float sample_freq;              //输入信号频率
    float cutoff_freq;     //截止频率
    float alpha;           //滤波系数
    float pre_out;          //上一次输出
    float now_out;           //低通滤波器输出
}filter_t;

/*卡尔曼滤波结构体（一阶）*/
typedef struct
{
    float Last_P;//上次估算协方差 不可以为0 ! ! ! ! ! 
    float Now_P;//当前估算协方差
    float out;//卡尔曼滤波器输出
    float Kg;//卡尔曼增益
    float Q;//过程噪声协方差
    float R;//观测噪声协方差
}Kalman;

typedef struct
{
	float x;
	float y;
	float z;
}vector3f_t;

typedef struct
{
	int16_t x;
	int16_t y;
	int16_t z;
}vector3s_t;

extern Kalman kfp;
extern vector3f_t gyro_vector;
extern vector3f_t acc_vector;
extern float angle[3];

//低通滤波器
void Low_Filter_Init(filter_t* filter, float sample_freq, float cutoff_freq);
float Low_Filter_Out(filter_t* filter, float sample);
//卡尔曼滤波器
void Kalman_Init(void);
float KalmanFilter(Kalman* kfp, float input);

void MPU_LowFilter_Init(void);//低通滤波器初始化
void Ins_calibration(void);//陀螺仪校准
void MPU_Getdata(void);//获取陀螺仪当前值(去零漂+滤波)
void Calculate_Complementary(void);

#endif
