#include "imu_filter.h"

/* ==================== 全局变量定义 ==================== */
Vector3f_t gyro_vector = {0, 0, 0};
Vector3f_t acc_vector = {0, 0, 0};
Vector3f_t gyro_offset = {0, 0, 0};
Vector3f_t acc_offset = {0, 0, 0};
float pitch = 0.0f, roll = 0.0f, yaw = 0.0f;

/* ==================== 低通滤波器实例 ==================== */
static LowPassFilter_t low_filter_acc_x;
static LowPassFilter_t low_filter_acc_y;
static LowPassFilter_t low_filter_acc_z;
static LowPassFilter_t low_filter_gyro_x;
static LowPassFilter_t low_filter_gyro_y;
static LowPassFilter_t low_filter_gyro_z;

/* ==================== 卡尔曼滤波器实例 ==================== */
static KalmanFilter_t kalman_acc_x;
static KalmanFilter_t kalman_acc_y;
static KalmanFilter_t kalman_acc_z;
static KalmanFilter_t kalman_gyro_x;
static KalmanFilter_t kalman_gyro_y;
static KalmanFilter_t kalman_gyro_z;

/* ==================== 低通滤波函数实现 ==================== */

/**
 * @brief  初始化低通滤波器
 */
void LowPassFilter_Init(LowPassFilter_t *filter, float sample_freq, float cutoff_freq)
{
    if (filter == NULL) return;
    
    filter->sample_freq = sample_freq;
    filter->cutoff_freq = cutoff_freq;
    filter->pre_out = 0.0f;
    filter->now_out = 0.0f;
    
    if (filter->sample_freq <= 0.0f || filter->cutoff_freq <= 0.0f) {
        filter->alpha = 1.0f;  // 滤波器已禁用
    } else {
        float dt = 1.0f / filter->sample_freq;  // 获取采样周期
        float rc = 1.0f / (My_2PI * filter->cutoff_freq);  // 通过截止频率计算RC时间常数
        filter->alpha = dt / (dt + rc);
        
        // 限制系数范围
        if (filter->alpha < 0.0f) {
            filter->alpha = 0.0f;
        } else if (filter->alpha > 1.0f) {
            filter->alpha = 1.0f;
        }
    }
}

/**
 * @brief  低通滤波输出
 */
float LowPassFilter_Out(LowPassFilter_t *filter, float sample)
{
    if (filter == NULL) return sample;
    
    filter->pre_out = filter->now_out;
    filter->now_out = filter->alpha * sample + (1.0f - filter->alpha) * filter->pre_out;
    return filter->now_out;
}

/* ==================== 卡尔曼滤波函数实现 ==================== */

/**
 * @brief  初始化卡尔曼滤波器
 */
void KalmanFilter_Init(KalmanFilter_t *kf)
{
    if (kf == NULL) return;
    
    kf->Last_P = 0.02f;     // 初始估计误差协方差
    kf->Now_P = 0.0f;       // 当前估计误差协方差
    kf->out = 0.0f;         // 初始输出
    kf->Kg = 0.0f;          // 初始卡尔曼增益
    kf->Q = 7e-5f;          // 过程噪声协方差（系统噪声）
    kf->R = 3e-4f;          // 测量噪声协方差（传感器噪声）
}

/**
 * @brief  卡尔曼滤波处理
 * 
 * 卡尔曼滤波算法步骤：
 * 1. 预测协方差：Now_P = Last_P + Q
 * 2. 计算卡尔曼增益：Kg = Now_P / (Now_P + R)
 * 3. 更新状态估计：out = out + Kg * (input - out)
 * 4. 更新协方差：Last_P = (1 - Kg) * Now_P
 */
float KalmanFilter_Process(KalmanFilter_t *kf, float input)
{
    if (kf == NULL) return input;
    
    /* 1. 预测协方差：k时刻系统估计协方差 = k-1时刻的系统协方差 + 过程噪声协方差 */
    kf->Now_P = kf->Last_P + kf->Q;
    
    /* 2. 计算卡尔曼增益：卡尔曼增益 = k时刻系统估计协方差 / (k时刻系统估计协方差 + 测量噪声协方差) */
    kf->Kg = kf->Now_P / (kf->Now_P + kf->R);
    
    /* 3. 更新状态估计：k时刻状态估计值 = 状态预测值 + 卡尔曼增益 * (测量值 - 状态预测值) */
    kf->out = kf->out + kf->Kg * (input - kf->out);
    
    /* 4. 更新协方差：下一时刻的系统协方差 = (1 - 卡尔曼增益) * k时刻系统估计协方差 */
    kf->Last_P = (1.0f - kf->Kg) * kf->Now_P;
    
    return kf->out;
}

/* ==================== IMU 数据处理函数实现 ==================== */

/**
 * @brief  初始化 IMU 滤波器
 */
void IMU_Filter_Init(float sample_freq, float cutoff_freq)
{
    /* 初始化低通滤波器 */
    LowPassFilter_Init(&low_filter_acc_x, sample_freq, cutoff_freq);
    LowPassFilter_Init(&low_filter_acc_y, sample_freq, cutoff_freq);
    LowPassFilter_Init(&low_filter_acc_z, sample_freq, cutoff_freq);
    LowPassFilter_Init(&low_filter_gyro_x, sample_freq, cutoff_freq);
    LowPassFilter_Init(&low_filter_gyro_y, sample_freq, cutoff_freq);
    LowPassFilter_Init(&low_filter_gyro_z, sample_freq, cutoff_freq);
    
    /* 初始化卡尔曼滤波器 */
    KalmanFilter_Init(&kalman_acc_x);
    KalmanFilter_Init(&kalman_acc_y);
    KalmanFilter_Init(&kalman_acc_z);
    KalmanFilter_Init(&kalman_gyro_x);
    KalmanFilter_Init(&kalman_gyro_y);
    KalmanFilter_Init(&kalman_gyro_z);
}

/**
 * @brief  处理 IMU 原始数据
 * 
 * 处理流程：
 * 1. 原始数据转换为浮点数
 * 2. 去除偏置
 * 3. 卡尔曼滤波
 * 4. 低通滤波
 * 5. 单位转换（加速度转m/s²，角速度转rad/s）
 */
void IMU_ProcessRawData(int16_t raw_acc_x, int16_t raw_acc_y, int16_t raw_acc_z,
                        int16_t raw_gyro_x, int16_t raw_gyro_y, int16_t raw_gyro_z,
                        float acc_scale, float gyro_scale)
{
    float temp_acc_x, temp_acc_y, temp_acc_z;
    float temp_gyro_x, temp_gyro_y, temp_gyro_z;
    
    /* 1. 原始数据转换为浮点数并去除偏置 */
    temp_acc_x = (float)raw_acc_x - acc_offset.x;
    temp_acc_y = (float)raw_acc_y - acc_offset.y;
    temp_acc_z = (float)raw_acc_z - acc_offset.z;
    
    temp_gyro_x = (float)raw_gyro_x - gyro_offset.x;
    temp_gyro_y = (float)raw_gyro_y - gyro_offset.y;
    temp_gyro_z = (float)raw_gyro_z - gyro_offset.z;
    
    /* 2. 卡尔曼滤波 */
    temp_acc_x = KalmanFilter_Process(&kalman_acc_x, temp_acc_x);
    temp_acc_y = KalmanFilter_Process(&kalman_acc_y, temp_acc_y);
    temp_acc_z = KalmanFilter_Process(&kalman_acc_z, temp_acc_z);
    
    temp_gyro_x = KalmanFilter_Process(&kalman_gyro_x, temp_gyro_x);
    temp_gyro_y = KalmanFilter_Process(&kalman_gyro_y, temp_gyro_y);
    temp_gyro_z = KalmanFilter_Process(&kalman_gyro_z, temp_gyro_z);
    
    /* 3. 低通滤波 */
    temp_acc_x = LowPassFilter_Out(&low_filter_acc_x, temp_acc_x);
    temp_acc_y = LowPassFilter_Out(&low_filter_acc_y, temp_acc_y);
    temp_acc_z = LowPassFilter_Out(&low_filter_acc_z, temp_acc_z);
    
    temp_gyro_x = LowPassFilter_Out(&low_filter_gyro_x, temp_gyro_x);
    temp_gyro_y = LowPassFilter_Out(&low_filter_gyro_y, temp_gyro_y);
    temp_gyro_z = LowPassFilter_Out(&low_filter_gyro_z, temp_gyro_z);
    
    /* 4. 单位转换 */
    /* 加速度计：原始值 / LSB值 * g，然后转换为 m/s² */
    acc_vector.x = (temp_acc_x / acc_scale) * G;
    acc_vector.y = (temp_acc_y / acc_scale) * G;
    acc_vector.z = (temp_acc_z / acc_scale) * G;
    
    /* 陀螺仪：原始值 / LSB值 * °/s，然后转换为 rad/s */
    gyro_vector.x = (temp_gyro_x / gyro_scale) * DegtoRad;
    gyro_vector.y = (temp_gyro_y / gyro_scale) * DegtoRad;
    gyro_vector.z = (temp_gyro_z / gyro_scale) * DegtoRad;
}

/**
 * @brief  设置加速度计偏置
 */
void IMU_SetAccOffset(float offset_x, float offset_y, float offset_z)
{
    acc_offset.x = offset_x;
    acc_offset.y = offset_y;
    acc_offset.z = offset_z;
}

/**
 * @brief  设置陀螺仪偏置
 */
void IMU_SetGyroOffset(float offset_x, float offset_y, float offset_z)
{
    gyro_offset.x = offset_x;
    gyro_offset.y = offset_y;
    gyro_offset.z = offset_z;
}

