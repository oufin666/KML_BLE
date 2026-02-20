#ifndef __ENCODER_H
#define __ENCODER_H

#define CONFIG_SHORT_PRESS_DELAY 100

// 时间阈值（可根据需求调整）
#define DEBOUNCE_MS      20     // 去抖时间（20ms）
#define LONG_PRESS_MS    5000    // 长按判断阈值（5000ms）

// 按键事件类型
typedef enum {
    ENC_PRESS_SHORT ,   // 短按（例如：>20ms < 5000ms）
    ENC_PRESS_LONG    // 长按（例如：>=5000ms）
} EncoderPressEvent;

void Encoder_Init(void);

int32_t Get_EncoderDiff(void);
bool Get_is_pressed(void);

void Encoder_Rotate_IRQHandler(void);//旋转中断
void Encoder_Press_IRQHandler(void);//按压中断


// 回调函数指针类型（参数为事件类型）
typedef void (*EncoderPressCallback)(EncoderPressEvent event);


// 注册回调函数（用户通过此函数设置短按/长按的处理逻辑）
void Encoder_RegisterPressCallback(EncoderPressCallback cb);
void Encoder_PressCallback(EncoderPressEvent event);



#endif

