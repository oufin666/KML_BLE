/**
 * @file lv_port_disp_templ.c
 *
 */

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port.h"
#include "../../lvgl.h"
//#include "LCD.h"
#include "LCD_TK024F3036.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
	disp_init();
	//这里将SCREEN_WIDTH * SCREEN_HEIGH优化为SCREEN_WIDTH * 15
	static lv_disp_draw_buf_t draw_buf_dsc_1;
	static lv_color_t buf_1[SCREEN_WIDTH * 30]; /*A buffer for 10 rows*/
	lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, SCREEN_WIDTH * 30);

	static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
	lv_disp_drv_init(&disp_drv); /*Basic initialization*/
	disp_drv.hor_res = SCREEN_WIDTH;
	disp_drv.ver_res = SCREEN_HEIGH;
	disp_drv.flush_cb = disp_flush;
	disp_drv.draw_buf = &draw_buf_dsc_1;
	lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
//   LCD_Init();
	LCD_TK024F3036_Initialize();
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
	/*
// 1. 边界检查：避免超出屏幕范围（240×240）
if(area->x1 > SCREEN_WIDTH || area->y1 > SCREEN_HEIGH || area->x2 < 0 || area->y2 < 0) {
lv_disp_flush_ready(disp_drv);
return;
}
// 2. 设置整个刷新区域的地址（仅调用1次，替代逐像素设置）
LCD_Address_Set(area->x1, area->y1, area->x2, area->y2);
// 3. 计算需要传输的像素总数
uint32_t pixel_num = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
// 4. 批量传输所有像素数据（删除逐像素循环，减少SPI指令）
for(uint32_t i = 0; i < pixel_num; i++) {
LCD_WR_DATA(color_p->full); // 直接传输，无需逐像素设地址
color_p++;
}*/
//	LCD_Fill(area->x1,area->y1,area->x2,area->y2,color_p);
	LCD_TK024F3036_Fill(area->x1,area->y1,area->x2,area->y2,color_p);
// 5. 通知LVGL刷新完成（必须保留）
lv_disp_flush_ready(disp_drv);
}

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}


#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
