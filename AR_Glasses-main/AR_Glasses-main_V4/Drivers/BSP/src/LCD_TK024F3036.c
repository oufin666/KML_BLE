/*
 * LCD_TK024F3036.c
 *
 *  Created on: Jan 26, 2026
 *      Author: wkt98
 */
#include "main.h"
#include "LCD_TK024F3036.h"
#include "my_spi.h"




void LCD_delay(uint16_t xms) {
	HAL_Delay(xms);
}

static void WriteComm(u8 CMD)            //写命令
{
	LCD_WR_REG(CMD);

}

static void WriteData(u8 tem_data)       //写数据
{
	LCD_WR_DATA8(tem_data);
}


void LCD_TK024F3036_GPIO_Init(void) {
//	LCD_SCLK_Set();
//	LCD_MOSI_Set();
//	LCD_RES_Set();
	LCD_DC_Set();
	LCD_CS_Set();
	LCD_BLK_Clr();
}

/**********************************************
Lcd初始化函数
***********************************************/
void LCD_TK024F3036_Initialize(void)
{
	LCD_TK024F3036_GPIO_Init();

		LCD_RES_Clr(); //复位
		delay_ms(100);
		LCD_RES_Set();
		delay_ms(100);

//		LCD_BLK_Set(); //打开背光
		delay_ms(100);

		//************* Start Initial Sequence **********//
		LCD_WR_REG(0x11); //Sleep out
		delay_ms(120);              //Delay 120ms
		//************* Start Initial Sequence **********//
		LCD_WR_REG(0x36);
		if (USE_HORIZONTAL == 0)
			LCD_WR_DATA8(0x00);
		else if (USE_HORIZONTAL == 1)
			LCD_WR_DATA8(0xC0);
		else if (USE_HORIZONTAL == 2)
			LCD_WR_DATA8(0x70);
		else
			LCD_WR_DATA8(0xA0);

		LCD_WR_REG(0x3A);
		LCD_WR_DATA8(0x05);

		LCD_WR_REG(0xB2);
		LCD_WR_DATA8(0x0C);
		LCD_WR_DATA8(0x0C);
		LCD_WR_DATA8(0x00);
		LCD_WR_DATA8(0x33);
		LCD_WR_DATA8(0x33);

		LCD_WR_REG(0xB7);
		LCD_WR_DATA8(0x35);

		LCD_WR_REG(0xBB);
		LCD_WR_DATA8(0x32); //Vcom=1.35V

		LCD_WR_REG(0xC2);
		LCD_WR_DATA8(0x01);

		LCD_WR_REG(0xC3);
		LCD_WR_DATA8(0x15); //GVDD=4.8V  颜色深度

		LCD_WR_REG(0xC4);
		LCD_WR_DATA8(0x20); //VDV, 0x20:0v

		LCD_WR_REG(0xC6);
		LCD_WR_DATA8(0x0F); //0x0F:60Hz

		LCD_WR_REG(0xD0);
		LCD_WR_DATA8(0xA4);
		LCD_WR_DATA8(0xA1);

		LCD_WR_REG(0xE0);
		LCD_WR_DATA8(0xD0);
		LCD_WR_DATA8(0x08);
		LCD_WR_DATA8(0x0E);
		LCD_WR_DATA8(0x09);
		LCD_WR_DATA8(0x09);
		LCD_WR_DATA8(0x05);
		LCD_WR_DATA8(0x31);
		LCD_WR_DATA8(0x33);
		LCD_WR_DATA8(0x48);
		LCD_WR_DATA8(0x17);
		LCD_WR_DATA8(0x14);
		LCD_WR_DATA8(0x15);
		LCD_WR_DATA8(0x31);
		LCD_WR_DATA8(0x34);

		LCD_WR_REG(0xE1);
		LCD_WR_DATA8(0xD0);
		LCD_WR_DATA8(0x08);
		LCD_WR_DATA8(0x0E);
		LCD_WR_DATA8(0x09);
		LCD_WR_DATA8(0x09);
		LCD_WR_DATA8(0x15);
		LCD_WR_DATA8(0x31);
		LCD_WR_DATA8(0x33);
		LCD_WR_DATA8(0x48);
		LCD_WR_DATA8(0x17);
		LCD_WR_DATA8(0x14);
		LCD_WR_DATA8(0x15);
		LCD_WR_DATA8(0x31);
		LCD_WR_DATA8(0x34);
		LCD_WR_REG(0x21);

		LCD_WR_REG(0x29);
//	LCD_BLK_Set();

}


/******************************************************************************
 函数说明：设置起始和结束地址
 入口数据：x1,x2 设置列的起始和结束地址
 y1,y2 设置行的起始和结束地址
 返回值：  无
 ******************************************************************************/
void LCD_TK024F3036_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2) {
	if (USE_HORIZONTAL == 0) {
		LCD_WR_REG(0x2a); //列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b); //行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c); //储存器写
	} else if (USE_HORIZONTAL == 1) {
		LCD_WR_REG(0x2a); //列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b); //行地址设置
		LCD_WR_DATA(y1 + 80);
		LCD_WR_DATA(y2 + 80);
		LCD_WR_REG(0x2c); //储存器写
	} else if (USE_HORIZONTAL == 2) {
		LCD_WR_REG(0x2a); //列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b); //行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c); //储存器写
	} else {
		LCD_WR_REG(0x2a); //列地址设置
		LCD_WR_DATA(x1 + 80);
		LCD_WR_DATA(x2 + 80);
		LCD_WR_REG(0x2b); //行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c); //储存器写
	}
}


/******************************************************************************
 函数说明：DMA模式填充颜色（核心修改：大批量数据用DMA传输）
 入口数据：xsta,ysta 起始坐标；xend,yend 终止坐标；color 颜色缓冲区（16位/像素）
 返回值：无
 优化点：避免双重循环，直接将16位颜色数据转为8位字节流，DMA一次性传输
 ******************************************************************************/

void LCD_TK024F3036_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, lv_color_t  *color) {
	 uint32_t total_pixels = (uint32_t)(xend - xsta + 1) * (yend - ysta + 1); // 补+1（原代码少算像素）
	    uint32_t total_bytes = 2 * total_pixels;
	//    #define BLOCK_MAX_SIZE 5000U // 留1字节余量，避免uint16_t溢出
	    uint16_t current_block_size;
	    uint32_t buf_offset = 0;

	//    uint8_t *dma_buf = (uint8_t *)malloc(BLOCK_MAX_SIZE);
	//    if (dma_buf == NULL) return;

	    LCD_TK024F3036_Address_Set(xsta, ysta, xend, yend);
	    LCD_CS_Clr();
	    LCD_DC_Set();

	    // 分块传输核心逻辑
	    while (total_bytes > 0) {
	        // 计算当前块长度：不超过BLOCK_MAX_SIZE，且不超过剩余字节数
	        current_block_size = (total_bytes > BLOCK_MAX_SIZE) ? BLOCK_MAX_SIZE : (uint16_t)total_bytes;
	        // 确保块长度是2的倍数（16位颜色，避免半像素）
	        current_block_size = current_block_size & 0xFFFE;

	        // 填充当前块缓冲区
	        for (uint16_t i = 0; i < current_block_size; i += 2) {
	            uint32_t pixel_idx = (buf_offset + i) / 2;
	            u16 pixel_color = color[pixel_idx].full;
	            dma_buf[i] = (pixel_color >> 8) & 0xFF;  // 高8位
	            dma_buf[i + 1] = pixel_color & 0xFF;     // 低8位
	        }

	        // 启动DMA传输
	        LCD_SPI_Transmit_DMA(dma_buf, current_block_size);
	        // 等待传输完成（修复后的循环）
	        while (LCD_DMA_TX_Finished == 0);
	        LCD_DMA_TX_Finished = 0; // 重置标志

	        // 更新剩余字节和偏移
	        total_bytes -= current_block_size;
	        buf_offset += current_block_size;
	    }

	    LCD_CS_Set();
	//    free(dma_buf);
	//    dma_buf = NULL;
}

