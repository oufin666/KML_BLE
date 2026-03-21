#include "beep.h"

// 全局变量，用于跟踪PWM状态
static uint32_t beep_last_tick = 0;
static uint8_t beep_state = 0;

void BEEP_Init(void)
{
    // B7 已经在 CubeMX 中配置为 GPIO 输出
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    beep_last_tick = HAL_GetTick();
    beep_state = 0;
}

void BEEP_Update(void)
{
    // 4kHz频率，250us周期，50%占空比
    static const uint32_t period_us = 250;
    
    // 使用系统时钟计算经过的时间（微秒级）
    static uint32_t last_update = 0;
    static uint32_t last_systick = 0;
    
    // 获取当前SysTick值（递减计数器）
    uint32_t current_systick = SysTick->VAL;
    
    // 计算SysTick差值（处理递减和溢出）
    uint32_t systick_diff;
    if (last_systick == 0) {
        // 第一次调用
        last_systick = current_systick;
        last_update = HAL_GetTick() * 1000;
        return;
    } else if (current_systick < last_systick) {
        // 正常递减
        systick_diff = last_systick - current_systick;
    } else {
        // 发生溢出
        systick_diff = last_systick + (SysTick->LOAD - current_systick) + 1;
    }
    
    // 计算微秒差值
    uint32_t us_diff = systick_diff / (SystemCoreClock / 1000000);
    last_update += us_diff;
    last_systick = current_systick;
    
    // 当达到半个周期时，翻转GPIO状态
    if (last_update >= period_us / 2) {
        last_update = 0;
        
        if (beep_state) {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
        }
        beep_state = !beep_state;
    }
}

void BEEP_On(void)
{
    // 启用PWM更新
    beep_state = 0;
    beep_last_tick = HAL_GetTick();
}

void BEEP_Off(void)
{
    // 关闭PWM，设置为低电平
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    beep_state = 0;
}