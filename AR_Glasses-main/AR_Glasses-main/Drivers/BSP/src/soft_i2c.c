#include "soft_i2c.h"

/* ==================== 私有函数声明 ==================== */
static void Soft_I2C_Delay(void);

/* ==================== 私有函数实现 ==================== */

/**
 * @brief  软件I2C延时函数
 */
static void Soft_I2C_Delay(void)
{
    uint32_t i;
    for (i = 0; i < SOFT_I2C_DELAY_US * 8; i++) {
        __NOP();
    }
}

/* ==================== 公有函数实现 ==================== */

/**
 * @brief  初始化软件I2C引脚
 */
void Soft_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = SOFT_I2C_SDA_PIN | SOFT_I2C_SCL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    SOFT_I2C_SDA_HIGH();
    SOFT_I2C_SCL_HIGH();
}

/**
 * @brief  产生I2C起始信号
 */
void Soft_I2C_Start(void)
{
    SOFT_I2C_SDA_HIGH();
    SOFT_I2C_SCL_HIGH();
    Soft_I2C_Delay();
    SOFT_I2C_SDA_LOW();
    Soft_I2C_Delay();
    SOFT_I2C_SCL_LOW();
    Soft_I2C_Delay();
}

/**
 * @brief  产生I2C停止信号
 */
void Soft_I2C_Stop(void)
{
    SOFT_I2C_SDA_LOW();
    SOFT_I2C_SCL_HIGH();
    Soft_I2C_Delay();
    SOFT_I2C_SDA_HIGH();
    Soft_I2C_Delay();
}

/**
 * @brief  等待从机应答信号
 */
uint8_t Soft_I2C_WaitAck(void)
{
    uint8_t ack;
    
    SOFT_I2C_SDA_HIGH();
    Soft_I2C_Delay();
    SOFT_I2C_SCL_HIGH();
    Soft_I2C_Delay();
    
    if (SOFT_I2C_SDA_READ()) {
        ack = 1;  // NACK
    } else {
        ack = 0;  // ACK
    }
    
    SOFT_I2C_SCL_LOW();
    Soft_I2C_Delay();
    
    return ack;
}

/**
 * @brief  主机发送应答信号
 */
void Soft_I2C_SendAck(uint8_t ack)
{
    SOFT_I2C_SCL_LOW();
    Soft_I2C_Delay();
    
    if (ack) {
        SOFT_I2C_SDA_HIGH();  // NACK
    } else {
        SOFT_I2C_SDA_LOW();   // ACK
    }
    
    Soft_I2C_Delay();
    SOFT_I2C_SCL_HIGH();
    Soft_I2C_Delay();
    SOFT_I2C_SCL_LOW();
    Soft_I2C_Delay();
}

/**
 * @brief  发送一个字节
 */
void Soft_I2C_SendByte(uint8_t data)
{
    uint8_t i;
    
    for (i = 0; i < 8; i++) {
        SOFT_I2C_SCL_LOW();
        Soft_I2C_Delay();
        
        if (data & 0x80) {
            SOFT_I2C_SDA_HIGH();
        } else {
            SOFT_I2C_SDA_LOW();
        }
        
        data <<= 1;
        Soft_I2C_Delay();
        SOFT_I2C_SCL_HIGH();
        Soft_I2C_Delay();
    }
    
    SOFT_I2C_SCL_LOW();
    Soft_I2C_Delay();
}

/**
 * @brief  接收一个字节
 */
uint8_t Soft_I2C_ReceiveByte(uint8_t ack)
{
    uint8_t i;
    uint8_t data = 0;
    
    SOFT_I2C_SDA_HIGH();
    
    for (i = 0; i < 8; i++) {
        SOFT_I2C_SCL_LOW();
        Soft_I2C_Delay();
        SOFT_I2C_SCL_HIGH();
        Soft_I2C_Delay();
        
        data <<= 1;
        if (SOFT_I2C_SDA_READ()) {
            data |= 0x01;
        }
    }
    
    SOFT_I2C_SCL_LOW();
    Soft_I2C_Delay();
    
    Soft_I2C_SendAck(ack);
    
    return data;
}

/**
 * @brief  写入一个字节到指定设备寄存器
 */
uint8_t Soft_I2C_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
    Soft_I2C_Start();
    
    Soft_I2C_SendByte(dev_addr);
    if (Soft_I2C_WaitAck()) {
        Soft_I2C_Stop();
        return 1;
    }
    
    Soft_I2C_SendByte(reg_addr);
    if (Soft_I2C_WaitAck()) {
        Soft_I2C_Stop();
        return 1;
    }
    
    Soft_I2C_SendByte(data);
    if (Soft_I2C_WaitAck()) {
        Soft_I2C_Stop();
        return 1;
    }
    
    Soft_I2C_Stop();
    
    return 0;
}

/**
 * @brief  从指定设备寄存器读取一个字节
 */
uint8_t Soft_I2C_ReadReg(uint8_t dev_addr, uint8_t reg_addr)
{
    uint8_t data;
    
    Soft_I2C_Start();
    
    Soft_I2C_SendByte(dev_addr);
    if (Soft_I2C_WaitAck()) {
        Soft_I2C_Stop();
        return 0xFF;
    }
    
    Soft_I2C_SendByte(reg_addr);
    if (Soft_I2C_WaitAck()) {
        Soft_I2C_Stop();
        return 0xFF;
    }
    
    Soft_I2C_Start();
    
    Soft_I2C_SendByte(dev_addr | 0x01);
    if (Soft_I2C_WaitAck()) {
        Soft_I2C_Stop();
        return 0xFF;
    }
    
    data = Soft_I2C_ReceiveByte(1);  // NACK
    
    Soft_I2C_Stop();
    
    return data;
}

/**
 * @brief  从指定设备寄存器连续读取多个字节
 */
uint8_t Soft_I2C_ReadMultiBytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *buffer, uint16_t length)
{
    uint16_t i;
    
    if (buffer == NULL || length == 0) {
        return 1;
    }
    
    Soft_I2C_Start();
    
    Soft_I2C_SendByte(dev_addr);
    if (Soft_I2C_WaitAck()) {
        Soft_I2C_Stop();
        return 1;
    }
    
    Soft_I2C_SendByte(reg_addr);
    if (Soft_I2C_WaitAck()) {
        Soft_I2C_Stop();
        return 1;
    }
    
    Soft_I2C_Start();
    
    Soft_I2C_SendByte(dev_addr | 0x01);
    if (Soft_I2C_WaitAck()) {
        Soft_I2C_Stop();
        return 1;
    }
    
    for (i = 0; i < length; i++) {
        if (i == length - 1) {
            buffer[i] = Soft_I2C_ReceiveByte(1);  // 最后一个字节发送NACK
        } else {
            buffer[i] = Soft_I2C_ReceiveByte(0);  // 其他字节发送ACK
        }
    }
    
    Soft_I2C_Stop();
    
    return 0;
}
