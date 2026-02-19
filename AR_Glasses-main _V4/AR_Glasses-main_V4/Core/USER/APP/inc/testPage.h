#ifndef TESTPAGE_H
#define TESTPAGE_H

#include "GPS.h"
#include <math.h>

void lvgl_st7789_test_page_create(void);
void update_gps_display(lv_obj_t *parent, GPS_Data *data);
void update_gps_display_forTestUpdateFre(lv_obj_t *parent, GPS_Data *data,int testCount);
void lv_example_hr_display(void);
#endif
