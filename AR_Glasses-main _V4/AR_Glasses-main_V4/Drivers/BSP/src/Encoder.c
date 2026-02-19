#include "main.h"
#include "Encoder.h"


static bool EncoderEnable = true;
static volatile int32_t EncoderDiff = 0;
static bool EncoderDiffDisable = false;


static volatile bool is_pressed = false;          // 按键是否处于按下状态
static volatile uint32_t press_start_time = 0;    // 按下起始时间（毫秒）
static volatile uint32_t last_irq_time = 0;       // 上一次中断时间（去抖用）

// 回调函数指针（默认NULL，用户需注册）
static EncoderPressCallback press_cb = NULL;


void Encoder_Init()
{
	Encoder_RegisterPressCallback(Encoder_PressCallback);//注册回调函数
}

int32_t Get_EncoderDiff(void)
{
	return EncoderDiff;
}

bool Get_is_pressed(void)
{
	if(is_pressed == true){
		is_pressed = false;
		return true;
	}
	return false;
}

// 注册回调函数（用户调用此函数设置短按和长按的处理逻辑）
void Encoder_RegisterPressCallback(EncoderPressCallback cb) {
    press_cb = cb;
}

//按压回调函数
void Encoder_PressCallback(EncoderPressEvent event){
	
}

//旋转中断
void Encoder_Rotate_IRQHandler()
{
	if(!EncoderEnable || EncoderDiffDisable)
    {
        return;
    }
    int dir = (HAL_GPIO_ReadPin(EC_B_GPIO_Port,EC_B_Pin) == GPIO_PIN_RESET ? -1 : +1);
    EncoderDiff += dir;
}	

//按压中断
void Encoder_Press_IRQHandler()
{
	uint32_t current_time = HAL_GetTick();  // 获取当前系统时间（毫秒）
    uint16_t pin_level = HAL_GPIO_ReadPin(EC_KEY_GPIO_Port,EC_KEY_Pin);  // 读取当前引脚电平

    // 1. 去抖判断：若两次中断间隔小于20ms，视为抖动，忽略

    if (current_time - last_irq_time < DEBOUNCE_MS) {
        last_irq_time = current_time;
        return;
    }
    last_irq_time = current_time;  // 更新上一次中断时间


    // 2. 下降沿（按键按下：电平从高→低，因上拉输入，按下时为低）
    if (pin_level == GPIO_PIN_RESET && !is_pressed) {
        is_pressed = true;                  // 标记为按下状态
        press_start_time = current_time;    // 记录按下开始时间
    }

    // 3. 上升沿（按键释放：电平从低→高）
    else if (pin_level == GPIO_PIN_SET && is_pressed) {
//        is_pressed = false;                 // 标记为释放状态
        uint32_t press_duration = current_time - press_start_time;  // 计算按下持续时间

        // 根据持续时间判断事件类型，并调用对应回调
        if (press_duration >= LONG_PRESS_MS) {  // 长按
            if (press_cb != NULL) {
                press_cb(ENC_PRESS_LONG);
            }
        } else if(press_duration >= DEBOUNCE_MS){  // 短按
            if (press_cb != NULL) {
                press_cb(ENC_PRESS_SHORT);
            }
        }
    }
}


	
