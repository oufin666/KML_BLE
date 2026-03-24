#include "beep.h"
#include "tim.h"

// 全局变量，用于跟踪PWM状态
static uint8_t beep_state = 0;
uint8_t beep_enabled = 0;

void BEEP_Init(void)
{
    // B7 已经在 CubeMX 中配置为 GPIO 输出
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    beep_state = 0;
    beep_enabled = 0;
    
    // 初始化TIM3
    MX_TIM3_Init();
}

void BEEP_Update(void)
{
    // TIM3中断频率为6kHz，每次中断翻转一次GPIO，产生3kHz PWM
    if (!beep_enabled) {
        return;
    }
    
    if (beep_state) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    }
    beep_state = !beep_state;
}

void BEEP_On(void)
{
    // 启用PWM更新
    beep_state = 0;
    beep_enabled = 1;
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    
    // 启动TIM3
    HAL_TIM_Base_Start_IT(&htim3);
}

void BEEP_Off(void)
{
    // 停止TIM3
    HAL_TIM_Base_Stop_IT(&htim3);
    
    // 关闭PWM，设置为低电平
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    beep_state = 0;
    beep_enabled = 0;
}
