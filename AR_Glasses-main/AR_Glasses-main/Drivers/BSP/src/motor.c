#include "motor.h"
#include "delay.h"

void MOTOR_Init(void)
{
    // PC13 已经在 CubeMX 中配置为 GPIO 输出
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}

void MOTOR_SetSpeed(uint8_t speed)
{
    if (speed == 0)
    {
        MOTOR_Off();
        return;
    }
    
    if (speed > 100) speed = 100;
    
    uint32_t on_time = (speed * 10) / 100; // 10ms周期
    uint32_t off_time = 10 - on_time;
    
    // 模拟PWM，输出一个周期
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    delay_ms(on_time);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    delay_ms(off_time);
}

void MOTOR_On(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

void MOTOR_Off(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}