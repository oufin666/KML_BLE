#include "GH3220.h"

// GH3220心率传感器驱动 - 保留空实现以避免编译错误
// 如需使用心率功能，请恢复完整实现

void hal_gh3220_i2c_init() {
    // 空实现
}

GU8 hal_gh3220_i2c_write(GU8 device_id, const GU8 write_buffer[], GU16 length) {
    (void)device_id;
    (void)write_buffer;
    (void)length;
    return 1;
}

GU8 hal_gh3220_i2c_read(GU8 device_id, const GU8 write_buffer[],
        GU16 write_length, GU8 read_buffer[], GU16 read_length) {
    (void)device_id;
    (void)write_buffer;
    (void)write_length;
    (void)read_buffer;
    (void)read_length;
    return 1;
}

void hal_gh3220_set(void) {
    // 空实现
}

void hal_gh3220_reset(void) {
    // 空实现
}

void hal_gh3220_int_init(void) {
    // 空实现
}
