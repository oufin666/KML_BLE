#include "main.h"
#include "LCD.h"
#include "spi.h"
#include "my_spi.h"

// DMA传输完成标志（0：未完成，1：完成）
//uint8_t LCD_DMA_TX_Finished = 0;
//uint8_t LCD_DMA_Buf_Occupied = 0; // 缓冲区占用标志（0=空闲，1=占用）

//#define BLOCK_MAX_SIZE 5000U
//static uint8_t dma_buf[BLOCK_MAX_SIZE] = {0}; // 静态缓冲区，生命周期贯穿整个程序


void delay_ms(uint16_t xms) {
	HAL_Delay(xms);
}

void LCD_GPIO_Init(void) {
//	LCD_SCLK_Set();
//	LCD_MOSI_Set();
	LCD_RES_Set();
	LCD_DC_Set();
	LCD_CS_Set();
	LCD_BLK_Set();
}

#if 0
/******************************************************************************
 函数说明：硬件SPI数据传输函数（替换原软件模拟的LCD_Writ_Bus）
 入口数据：dat  要写入的8位数据
 返回值：无
 注意：严格匹配SPI配置（Motorola、MSB、CPOL=Low、CPHA=1 Edge）
 ******************************************************************************/
static void LCD_SPI_Transmit(u8 dat) {
	// 等待SPI外设空闲
	while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY)
		;

	// 硬件SPI传输1字节数据（阻塞模式，确保传输完成）
	HAL_SPI_Transmit(&hspi1, &dat, 1, HAL_MAX_DELAY);
}

/******************************************************************************
 函数说明：SPI DMA非阻塞传输（用于大数据：填充颜色）
 入口数据：pData 数据缓冲区地址，Len 数据长度（字节数）
 返回值：无
 注意：传输期间CPU可执行其他任务，通过LCD_DMA_TX_Finished标志判断完成
 ******************************************************************************/
void LCD_SPI_Transmit_DMA(uint8_t *pData, uint16_t Len) {
	while (LCD_DMA_Buf_Occupied == 1)
		; // 缓冲区被占用，拒绝新传输
	LCD_DMA_Buf_Occupied = 1; // 标记占用
	LCD_DMA_TX_Finished = 0;
	HAL_SPI_Transmit_DMA(&hspi1, pData, Len);
}

/******************************************************************************
 函数说明：DMA传输完成回调函数（需在spi.c中调用）
 入口数据：无
 返回值：无
 ******************************************************************************/
void LCD_DMA_TX_Callback(void) {
	LCD_DMA_TX_Finished = 1;
	LCD_DMA_Buf_Occupied = 0; // 传输完成，释放缓冲区
}

#endif

#if 0
/******************************************************************************
 函数说明：LCD串行数据写入函数（复用原逻辑，仅替换SPI传输部分）
 入口数据：dat  要写入的串行数据
 返回值：无
 ******************************************************************************/
void LCD_Writ_Bus(u8 dat) {
	LCD_CS_Clr();  // 拉低CS，选中LCD
	LCD_SPI_Transmit(dat);                                       // 硬件SPI传输
	LCD_CS_Set();    // 拉高CS，结束传输
}

/******************************************************************************
 函数说明：LCD写入8位数据（无修改，复用原逻辑）
 入口数据：dat 写入的数据
 返回值：无
 ******************************************************************************/
void LCD_WR_DATA8(u8 dat) {
	LCD_Writ_Bus(dat);
}

/******************************************************************************
 函数说明：LCD写入16位数据（无修改，复用原逻辑：分两次8位传输）
 入口数据：dat 写入的数据
 返回值：无
 ******************************************************************************/
void LCD_WR_DATA(u16 dat) {
	LCD_Writ_Bus(dat >> 8);  // 先传高8位（MSB）
	LCD_Writ_Bus(dat & 0xFF); // 再传低8位
}

/******************************************************************************
 函数说明：LCD写入16位数据（无修改，复用原逻辑：分两次8位传输）
 入口数据：dat 写入的数据
 返回值：无
 ******************************************************************************/
void LCD_WR_DATA_DMA(u16 dat) {
	uint8_t dma_buf[2];
	dma_buf[0] = (dat >> 8) & 0xFF;    // 高8位（先传）
	dma_buf[1] = dat & 0xFF;
	LCD_SPI_Transmit_DMA(dma_buf, 2);  // 先传高8位（MSB）再传低8位
	while (LCD_DMA_TX_Finished == 0)
		;
}

/******************************************************************************
 函数说明：LCD写入命令
 入口数据：dat 写入的命令
 返回值：  无
 ******************************************************************************/
void LCD_WR_REG(u8 dat) {
	LCD_DC_Clr(); //写命令
	LCD_Writ_Bus(dat);
	LCD_DC_Set(); //写数据
}
#endif

/******************************************************************************
 函数说明：设置起始和结束地址
 入口数据：x1,x2 设置列的起始和结束地址
 y1,y2 设置行的起始和结束地址
 返回值：  无
 ******************************************************************************/
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2) {
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

void LCD_Init(void) {
	LCD_GPIO_Init(); //初始化GPIO

	LCD_RES_Clr(); //复位
	delay_ms(100);
	LCD_RES_Set();
	delay_ms(100);

	LCD_BLK_Set(); //打开背光
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
}


/******************************************************************************
      函数说明：在指定位置画点
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_Address_Set(x,y,x,y);//设置光标位置
	LCD_WR_DATA(color);
}


/******************************************************************************
 函数说明：DMA模式填充颜色（核心修改：大批量数据用DMA传输）
 入口数据：xsta,ysta 起始坐标；xend,yend 终止坐标；color 颜色缓冲区（16位/像素）
 返回值：无
 优化点：避免双重循环，直接将16位颜色数据转为8位字节流，DMA一次性传输
 ******************************************************************************/

void LCD_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, lv_color_t  *color) {
    uint32_t total_pixels = (uint32_t)(xend - xsta + 1) * (yend - ysta + 1); // 补+1（原代码少算像素）
    uint32_t total_bytes = 2 * total_pixels;
//    #define BLOCK_MAX_SIZE 5000U // 留1字节余量，避免uint16_t溢出
    uint16_t current_block_size;
    uint32_t buf_offset = 0;

//    uint8_t *dma_buf = (uint8_t *)malloc(BLOCK_MAX_SIZE);
//    if (dma_buf == NULL) return;

    LCD_Address_Set(xsta, ysta, xend, yend);
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


