#include "Filter.h" 

// 初始化低通滤波器
void Low_Filter_Init(filter_t* filter,float sample_freq,float cutoff_freq)
{
	filter->sample_freq = sample_freq;
	filter->cutoff_freq = cutoff_freq;
	filter->pre_out = 0.0f;
	filter->now_out = 0.0f;
	if(filter->sample_freq <= 0.0f || filter->cutoff_freq <= 0.0f)
		filter->alpha=1;  //滤波已经无意义了
	else
	{
		float dt = 1.0f/filter->sample_freq;  //得到采样周期
      float rc = 1.0f/(My_2PI*filter->cutoff_freq); //通过截至频率计算RC的值
      filter->alpha = dt/(dt+rc);
		filter->alpha<0.0f?filter->alpha=0:(filter->alpha>1.0f?filter->alpha=1:filter->alpha);
		//对系数限幅
	}
}

//得到此次输出值
float Low_Filter_Out(filter_t* filter,float sample)
{
	filter->pre_out = filter->now_out;
	filter->now_out = filter->alpha*sample + (1.0f - filter->alpha)*filter->pre_out;
	return filter->now_out;
}

//初始化卡尔曼滤波器
Kalman kfp;
void Kalman_Init()
{
	kfp.Last_P = 0.02;			
	kfp.Now_P = 0;		
	kfp.out = 0;			
	kfp.Kg = 100;		
	kfp.Q = 7e-5;
	kfp.R = 3e-4;
}

/**
 *卡尔曼滤波器
 *@param 	Kalman *kfp 卡尔曼结构体参数
 *   			float input 需要滤波的参数的测量值（即传感器的采集值）
 *@return 滤波后的参数（最优值）
 */
float KalmanFilter(Kalman *kfp,float input)
{
   //预测协方差方程：k时刻系统估算协方差 = k-1时刻的系统协方差 + 过程噪声协方差
   kfp->Now_P = kfp->Last_P + kfp->Q;
   //卡尔曼增益方程：卡尔曼增益 = k时刻系统估算协方差 / （k时刻系统估算协方差 + 观测噪声协方差）
   kfp->Kg = kfp->Now_P / (kfp->Now_P + kfp->R);
   //更新最优值方程：k时刻状态变量的最优值 = 状态变量的预测值 + 卡尔曼增益 * （测量值 - 状态变量的预测值）
   kfp->out = kfp->out + kfp->Kg * (input -kfp->out);//因为这一次的预测值就是上一次的输出值
   //更新协方差方程: 本次的系统协方差付给 kfp->LastP 为下一次运算准备。
   kfp->Last_P = (1-kfp->Kg) * kfp->Now_P;
   return kfp->out;
}

#ifdef pitch_roll 
filter_t  low_filter_acc_x;
filter_t  low_filter_acc_y;
filter_t  low_filter_gyro_x;
filter_t  low_filter_gyro_y;

#endif
filter_t  low_filter_acc_z;
filter_t  low_filter_gyro_z;

/**
 * @brief 低通滤波器及卡尔曼滤波器初始化
 * @param 无
 * @return 无
 */
void MPU_LowFilter_Init(void)
{
#ifdef pitch_roll
	Low_Filter_Init(&low_filter_acc_x, 100, 30);
	Low_Filter_Init(&low_filter_acc_y, 100, 30);
	Low_Filter_Init(&low_filter_gyro_x, 100, 30);
	Low_Filter_Init(&low_filter_gyro_y, 100, 30);
#endif
	Low_Filter_Init(&low_filter_acc_z, 100, 30); //滤波截止频率需要测量，采样频率要考虑陀螺仪自身的采样频率，MPU6050初始化是125hz,8ms一次
	Low_Filter_Init(&low_filter_gyro_z, 100, 30);
	Kalman_Init();
}

/* 传感器校准标志位 */
bool flag_ins_calibration = true;
vector3f_t acc_vector_offset;   //加计零偏值
vector3f_t gyro_vector_offset;  //角速度计零偏值

//陀螺仪校准
void Ins_calibration(void)
{
	MPU_LowFilter_Init();
	vector3s_t _acc_vector[1220];   //存放加速度计读取的原始数据
	vector3s_t _gyro_vector[1220];  //存放角速度计读取的原始数据
	vector3f_t _gyro_average;      //存放角速度计平均值
	vector3f_t _acc_average;       //存放加速度计平均值
	_acc_average.x = 0;
	_acc_average.y = 0;
	_acc_average.z = 0;
	_gyro_average.x = 0;
	_gyro_average.y = 0;
	_gyro_average.z = 0;
	do
	{
		for (int i = 0;i < 1200;i++)
		{
#if SENSOR_TYPE == 1
    // IMU660 数据读取
    imu660ra_get_acc();
    imu660ra_get_gyro();

    _acc_vector[i].x = imu660ra_acc_x;
    _acc_vector[i].y = imu660ra_acc_y;
    _acc_vector[i].z = imu660ra_acc_z;

    _gyro_vector[i].x = imu660ra_gyro_x;
    _gyro_vector[i].y = imu660ra_gyro_y;
    _gyro_vector[i].z = imu660ra_gyro_z;

#elif SENSOR_TYPE == 2
    // ICM42688 数据读取
    Get_Gyro_ICM42688();
    Get_Acc_ICM42688();

    _acc_vector[i].x = icm42688_acc_x;
    _acc_vector[i].y = icm42688_acc_y;
    _acc_vector[i].z = icm42688_acc_z;

    _gyro_vector[i].x = icm42688_gyro_x;
    _gyro_vector[i].y = icm42688_gyro_y;
    _gyro_vector[i].z = icm42688_gyro_z;
#else
    #error "Invalid sensor type! Please define SENSOR_TYPE as 1 (IMU660) or 2 (ICM42688)."
#endif
			_acc_average.z  += (_acc_vector[i].z - 4196)/500.0f; 
			_gyro_average.z += _gyro_vector[i].z / 1200.0f;        //求平均值
			system_delay_ms(2);
		}

		acc_vector_offset.z = _acc_average.z; 
		gyro_vector_offset.z = _gyro_average.z;                 //得到零偏值
		flag_ins_calibration = false;                            //校准标志位清零

	} while (flag_ins_calibration);//持续校准
}

/* 全局变量 存放初步处理后的加速度计 陀螺仪数据 */
vector3f_t gyro_vector;
vector3f_t acc_vector;

//获取陀螺仪当前值
void MPU_Getdata(void)
{
	/* 保存最近三次的数据 */
	static vector3f_t gyro_vector_last[3] = { 0 };
	static vector3f_t acc_vector_last[3] = { 0 };
	static uint8_t num = 0;
	if (num > 2) num = 0;  //防止数组溢出

#if SENSOR_TYPE == 1
    // IMU660_RA 数据读取
    imu660ra_get_acc();  // 获取IMU660_RA加速度数据
    imu660ra_get_gyro(); // 获取IMU660_RA角速度数据

    #ifdef pitch_roll
        // 读取x轴和y轴数据
        acc_vector_last[num].x = imu660ra_acc_x;  
        gyro_vector_last[num].x = imu660ra_gyro_x; 
        acc_vector_last[num].y = imu660ra_acc_y;  
        gyro_vector_last[num].y = imu660ra_gyro_y; 
    #else
        // 不读取x轴数据，置为0
        acc_vector_last[num].x = 0;
        gyro_vector_last[num].x = 0;
        acc_vector_last[num].y = 0;  
        gyro_vector_last[num].y = 0; 
    #endif

    acc_vector_last[num].z = imu660ra_acc_z;  
    gyro_vector_last[num].z = imu660ra_gyro_z; 
		
#elif SENSOR_TYPE == 2
    // ICM42688 数据读取
    Get_Gyro_ICM42688();
    Get_Acc_ICM42688();

    #ifdef pitch_roll
        // 读取x轴和y轴数据
        acc_vector_last[num].x = icm42688_acc_x;  
        gyro_vector_last[num].x = icm42688_gyro_x; 
		acc_vector_last[num].y = icm42688_acc_y;      
		gyro_vector_last[num].y = icm42688_gyro_y;     
    #else
        // 不读取x轴数据，置为0
        acc_vector_last[num].x = 0;
        gyro_vector_last[num].x = 0;
		acc_vector_last[num].y = 0;      
		gyro_vector_last[num].y = 0;     
    #endif
    acc_vector_last[num].z = icm42688_acc_z;      
    gyro_vector_last[num].z = icm42688_gyro_z;     
#else
    #error "Invalid sensor type! Please define SENSOR_TYPE as 1 (ICM20602) or 2 (ICM42688)."
#endif

//	acc_vector_last[num].z -= acc_vector_offset.z;
	gyro_vector_last[num].z -= gyro_vector_offset.z; //去零偏
	
// 根据FILTER_MODE的值决定是否进行滤波
#if FILTER_MODE == 1
    //对陀螺仪的值进行滤波
	#ifdef pitch_roll
	//对陀螺仪的值进行滤波
	acc_vector_last[num].x = KalmanFilter(&kfp, acc_vector_last[num].x);
	acc_vector.x = Low_Filter_Out(&low_filter_acc_x, acc_vector_last[num].x);
	gyro_vector_last[num].x = KalmanFilter(&kfp, gyro_vector_last[num].x);
	gyro_vector.x = Low_Filter_Out(&low_filter_gyro_x, gyro_vector_last[num].x);
	
	acc_vector_last[num].y = KalmanFilter(&kfp, acc_vector_last[num].y);
	acc_vector.y = Low_Filter_Out(&low_filter_acc_y, acc_vector_last[num].y);
	gyro_vector_last[num].y = KalmanFilter(&kfp, gyro_vector_last[num].y);
	gyro_vector.y = Low_Filter_Out(&low_filter_gyro_y, gyro_vector_last[num].y);
	
	#else
	#endif
	
	acc_vector_last[num].z = KalmanFilter(&kfp, acc_vector_last[num].z);
	acc_vector.z = Low_Filter_Out(&low_filter_acc_y, acc_vector_last[num].z);
	gyro_vector_last[num].z = KalmanFilter(&kfp, gyro_vector_last[num].z);
	gyro_vector.z = Low_Filter_Out(&low_filter_gyro_y, gyro_vector_last[num].z);
#elif FILTER_MODE == 2
    // 不滤波时直接使用原始值
    acc_vector.x = acc_vector_last[num].x;
    gyro_vector.x = gyro_vector_last[num].x;
    acc_vector.y = acc_vector_last[num].y;
    gyro_vector.y = gyro_vector_last[num].y;
    acc_vector.z = acc_vector_last[num].z;
    gyro_vector.z = gyro_vector_last[num].z;
#endif
	
	acc_vector.x *= Acc_Gain * G; //转化为m/s^2
	gyro_vector.x *= Gyro_Gain * DegtoRad;  //转化为度
	acc_vector.y *= Acc_Gain * G; //转化为m/s^2
	gyro_vector.y *= Gyro_Gain * DegtoRad;  //转化为度
	acc_vector.z *= Acc_Gain * G; //转化为m/s^2
	gyro_vector.z *= Gyro_Gain * DegtoRad;  //转化为度

	num++;
}

//const float dt = 0.01f;									 //时间周期(设置为10ms在中断中调用一次该函数,在主函数初始化中设置)
const float dt = 0.001f;
float angle[3] = { 0 };										//0、1、2对应pitch、roll、yaw
float angle_last[3] = { 0 };                           //（上一时刻）0、1、2对应pitch、roll、yaw
const float R = 0.98f;											//(待确定)互补系数，越低跟随度越低，滤波效果越低

void Calculate_Complementary(void)
{
	MPU_Getdata();

	float increase[3] = { 0 };
#ifdef pitch_roll 
	increase[0] = gyro_vector.x * dt;
	if (abs(increase[0]) < 0.02) increase[0] = 0;
	angle[0] = R * (angle_last[0] + increase[0]) + (1 - R) * RadtoDeg * atan(acc_vector.x / acc_vector.z);
	angle_last[0] = angle[0];										//处理pitch

	increase[1] = gyro_vector.y * dt;
	if (abs(increase[1]) < 0.0) increase[1] = 0;
	angle[1] = R * (angle_last[1] + increase[1]) + (1 - R) * RadtoDeg * atan(acc_vector.y / acc_vector.z);
	angle_last[1] = angle[1];											//处理roll
#endif		
	increase[2] = -gyro_vector.z * dt;
	/*if(Motor_flag==0)*/ {if (fabs(increase[2]) < 0.002) increase[2] = 0;} //过滤系数需要自测0.002貌似效果还可以
	angle[2] = angle_last[2] + increase[2];
	angle_last[2] = angle[2];				//处理yaw

	//控制转弯角度范围在-180-180以内
	if (angle_last[2] > 180)
		angle_last[2] -= 360;
	else if (angle_last[2] < -180)
		angle_last[2] += 360;
}
