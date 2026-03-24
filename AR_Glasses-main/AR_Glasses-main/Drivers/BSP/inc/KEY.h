#ifndef KEY_H
#define KEY_H

#include "main.h"

// 按键引脚定义
#define KEYB_PIN        GPIO_PIN_14
#define KEYB_PORT       GPIOB
#define KEYC_PIN        GPIO_PIN_13
#define KEYC_PORT       GPIOB

// 按键状态定义
#define KEY_PRESSED     0
#define KEY_RELEASED    1

// 按键检测函数
HAL_StatusTypeDef KEY_Init(void);
uint8_t KEYB_Read(void);
uint8_t KEYC_Read(void);
uint8_t KEYB_IsPressed(void);
uint8_t KEYC_IsPressed(void);
uint8_t KEYB_IsReleased(void);
uint8_t KEYC_IsReleased(void);
uint8_t KEYB_HasReleased(void);
uint8_t KEYC_HasReleased(void);
void KEY_Update(void);

#endif /* KEY_H */