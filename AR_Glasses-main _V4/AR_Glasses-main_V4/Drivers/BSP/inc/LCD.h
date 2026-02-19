#ifndef __LCD_H
#define __LCD_H

#include "main.h"
#include "lv_port.h"
#include "../../lvgl.h"

typedef uint8_t  u8;    // u8 等价于 uint8_t（无符号8位整数）
typedef uint16_t u16;   // u16 等价于 uint16_t（无符号16位整数

#define SCREEN_WIDTH 240
#define SCREEN_HEIGH 240

#define USE_HORIZONTAL 0



//-----------------LCD端口定义----------------

#define LCD_SCLK_Clr() HAL_GPIO_WritePin(LCD_SCL_GPIO_Port,LCD_SCL_Pin,GPIO_PIN_RESET)//SCL=SCLK
#define LCD_SCLK_Set() HAL_GPIO_WritePin(LCD_SCL_GPIO_Port,LCD_SCL_Pin,GPIO_PIN_SET)

#define LCD_MOSI_Clr() HAL_GPIO_WritePin(LCD_SDA_GPIO_Port,LCD_SDA_Pin,GPIO_PIN_RESET)//SDA=MOSI
#define LCD_MOSI_Set() HAL_GPIO_WritePin(LCD_SDA_GPIO_Port,LCD_SDA_Pin,GPIO_PIN_SET)

#define LCD_RES_Clr()  HAL_GPIO_WritePin(LCD_RES_GPIO_Port,LCD_RES_Pin,GPIO_PIN_RESET)//RES
#define LCD_RES_Set()  HAL_GPIO_WritePin(LCD_RES_GPIO_Port,LCD_RES_Pin,GPIO_PIN_SET)

#define LCD_DC_Clr()   HAL_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_RESET)//DC
#define LCD_DC_Set()   HAL_GPIO_WritePin(LCD_DC_GPIO_Port,LCD_DC_Pin,GPIO_PIN_SET)
 		     
#define LCD_CS_Clr()   HAL_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_RESET)//CS
#define LCD_CS_Set()   HAL_GPIO_WritePin(LCD_CS_GPIO_Port,LCD_CS_Pin,GPIO_PIN_SET)

#define LCD_BLK_Clr()  HAL_GPIO_WritePin(LCD_BLK_GPIO_Port,LCD_BLK_Pin,GPIO_PIN_RESET)//BLK
#define LCD_BLK_Set()  HAL_GPIO_WritePin(LCD_BLK_GPIO_Port,LCD_BLK_Pin,GPIO_PIN_SET)


void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(u8 dat);//模拟SPI时序
void LCD_WR_DATA8(u8 dat);//写入一个字节
void LCD_WR_DATA(u16 dat);//写入两个字节
void LCD_WR_REG(u8 dat);//写入一个指令
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);//设置坐标函数
void LCD_Init(void);//LCD初始化
void LCD_DMA_TX_Callback(void);
void LCD_DrawPoint(u16 x,u16 y,u16 color);

void LCD_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, lv_color_t  *color) ;//在指定区域填充颜色

#endif

