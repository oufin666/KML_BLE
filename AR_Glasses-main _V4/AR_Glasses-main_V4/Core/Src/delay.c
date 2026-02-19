#include "delay.h"

void delay_init(){
	HAL_TIM_Base_Start(&htim2);  // 启动TIM2计数
}

void delay_us(uint32_t us) {

	uint32_t timerMax = 0xFFFFFFFF;
	uint32_t startTick = __HAL_TIM_GET_COUNTER(&htim2);
	uint32_t targetTick = startTick + us;

	// 处理计数器溢出
	if (targetTick > timerMax) {
		// 阶段1：等待计数器从当前值溢出归零
		while (__HAL_TIM_GET_COUNTER(&htim2) >= startTick) {
			// 防止意外情况导致死循环
			if (__HAL_TIM_GET_COUNTER(&htim2) < startTick)
				break;
		}

		// 阶段2：等待计数器达到剩余时间（从0开始）
		targetTick -= (timerMax + 1);
		while (__HAL_TIM_GET_COUNTER(&htim2) < targetTick);
	} else {
		// 无溢出情况：直接等待目标值
		while (__HAL_TIM_GET_COUNTER(&htim2) < targetTick);
	}
}


