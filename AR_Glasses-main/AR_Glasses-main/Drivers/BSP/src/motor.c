#include "motor.h"

// 全局变量，用于跟踪PWM状态
static uint32_t motor_last_tick = 0;
static uint8_t motor_state = 0;
static uint8_t motor_speed = 0;

void MOTOR_Init(void)
{
    // PC13 已经在 CubeMX 中配置为 GPIO 输出
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    motor_last_tick = HAL_GetTick();
    motor_state = 0;
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
}

void MOTOR_Update(void)
{
    // 100Hz频率，10ms周期
    static const uint32_t period_ms = 10;
    
    // 计算占空比
    uint32_t on_time_ms = (motor_speed * period_ms) / 100;
    uint32_t off_time_ms = period_ms - on_time_ms;
    
    // 使用系统时钟计算经过的时间
    static uint32_t last_cycle_start = 0;
    uint32_t current_tick = HAL_GetTick();
    uint32_t elapsed_ms = current_tick - last_cycle_start;
    
    // 当达到一个周期时，重新开始
    if (elapsed_ms >= period_ms) {
        last_cycle_start = current_tick;
        motor_state = 1; // 开始高电平
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    }
    // 当达到高电平时间时，切换到低电平
    else if (motor_state && (elapsed_ms >= on_time_ms)) {
        motor_state = 0; // 切换到低电平
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    }
}

void MOTOR_On(void)
{
    // 启用PWM，设置默认速度50%
    MOTOR_SetSpeed(50);
}

void MOTOR_Off(void)
{
    // 关闭PWM，设置为低电平
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    motor_speed = 0;
    motor_state = 0;
}