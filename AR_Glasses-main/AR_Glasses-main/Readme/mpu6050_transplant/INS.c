#include "INS.h"

/*
 * @brief  初始化陀螺仪
 * @param  none
 * @retval none
 */
void INS_init(void)
{
#if SENSOR_TYPE == 1
    while(1)
    {
        if(imu660ra_init())
        {
            printf("\r\nIMU660RA init error.");                                 // IMU660RA 初始化失败
        }
        else
        {
            break;
        }
    }
#elif SENSOR_TYPE == 2
    while(1)
    {
        if(Init_ICM42688())
        {
            printf("\r\nICM42688 init error.");                                 // IMU660RA 初始化失败
        }
        else
        {
            break;
        }
    }
#else
#endif
	Ins_calibration();//陀螺仪校准
	MPU_LowFilter_Init();//低通滤波器及卡尔曼滤波器初始化
	pit_ms_init(PIT_CH1, 1);//中断1ms
    Mahony_Init(1000);//采样频率1K(对应1ms)
    pitch = 0;
    roll = 0;
    yaw = 0;
}

/*
 * @brief  更新陀螺仪数据
 * @param  none
 * @retval none
 */	
void INS_update(void)
{
	MPU_Getdata();

#if Solution_method == 1

	MahonyAHRSupdateIMU(gyro_vector.x, gyro_vector.y, gyro_vector.z, acc_vector.x, acc_vector.y, acc_vector.z);//融合加速度计和角速度计的四元数更新函数
	Mahony_computeAngles();//计算欧拉角
	yaw = getYaw();
	
#elif Solution_method == 2
//	acc_vector.z *= G; //转化为m/s^2
	Calculate_Complementary();
	yaw = angle[2] * RadtoDeg;
#else
#endif
}


