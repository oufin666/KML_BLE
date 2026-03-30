#include "motor.h"
#include "tim.h"

// 全局变量，用于跟踪PWM状态
static uint8_t motor_speed = 0;

void MOTOR_Init(void)
{
    // 初始化TIM3_CH4为PWM输出
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
    motor_speed = 0;
}

void MOTOR_SetSpeed(uint8_t speed)
{
    if (speed == 0)
    {
        MOTOR_Off();
        return;
    }
    
    if (speed > 100) speed = 100;
    
    motor_speed = speed;
    // 设置PWM占空比
    uint32_t pulse = (speed * 1000) / 100; // 1000是TIM3的周期
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, pulse);
}

void MOTOR_Update(void)
{
    // 硬件PWM不需要软件更新
}

void MOTOR_On(void)
{
    // 启用PWM，设置默认速度50%
    MOTOR_SetSpeed(50);
}

void MOTOR_Off(void)
{
    // 关闭PWM，设置占空比为0
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);
    motor_speed = 0;
}