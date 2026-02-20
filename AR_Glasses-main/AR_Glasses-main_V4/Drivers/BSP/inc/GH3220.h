#ifndef GH3220_H
#define GH3220_H

#include "main.h"
#include "gh_drv.h"


void hal_gh3220_i2c_init();
GU8 hal_gh3220_i2c_write(GU8 device_id, const GU8 write_buffer[], GU16 length);
GU8 hal_gh3220_i2c_read(GU8 device_id, const GU8 write_buffer[], GU16 write_length, GU8 read_buffer[], GU16 read_length);
void hal_gh3220_set(void);
void hal_gh3220_reset(void);
void hal_gh3220_int_init(void);
#endif
