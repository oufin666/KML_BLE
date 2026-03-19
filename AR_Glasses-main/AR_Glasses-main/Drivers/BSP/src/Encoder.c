#include "main.h"
#include "Encoder.h"

// 编码器驱动 - 保留空实现以避免编译错误
// 如需使用编码器功能，请恢复完整实现

void Encoder_Init(void) {
    // 空实现
}

int32_t Get_EncoderDiff(void) {
    return 0;
}

bool Get_is_pressed(void) {
    return false;
}

void Encoder_RegisterPressCallback(EncoderPressCallback cb) {
    (void)cb;
}

void Encoder_PressCallback(EncoderPressEvent event) {
    (void)event;
}

void Encoder_Rotate_IRQHandler(void) {
    // 空实现
}

void Encoder_Press_IRQHandler(void) {
    // 空实现
}
