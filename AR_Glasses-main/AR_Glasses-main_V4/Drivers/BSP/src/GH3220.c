#include "GH3220.h"

// SDA方向切换（输出/输入）
void GH3220_I2C_SDA_Output(void) {
	GPIO_InitTypeDef gpio = { 0 };
	gpio.Pin = HR_SDA_Pin;
	gpio.Mode = GPIO_MODE_OUTPUT_OD;  // 开漏输出（I2C标准要求）
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(HR_SDA_GPIO_Port, &gpio);
}

void GH3220_I2C_SDA_Input(void) {
	GPIO_InitTypeDef gpio = { 0 };
	gpio.Pin = HR_SDA_Pin;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(HR_SDA_GPIO_Port, &gpio);
}

// SDA/SCL电平控制
void GH3220_I2C_SDA_Set(GU8 level) {
	HAL_GPIO_WritePin(HR_SDA_GPIO_Port, HR_SDA_Pin,
			level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void GH3220_I2C_SCL_Set(GU8 level) {
	HAL_GPIO_WritePin(HR_SCL_GPIO_Port, HR_SCL_Pin,
			level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

GU8 GH3220_I2C_SDA_Get() {
	if (HAL_GPIO_ReadPin(HR_SDA_GPIO_Port, HR_SDA_Pin) == GPIO_PIN_RESET) {
		return 0;
	} else {
		return 1;
	}
}

// 时序延时（每次延时2us）
void GH3220_I2C_Delay(void) {
	delay_us(2);
}

// 起始信号：SCL高时拉低SDA
void GH3220_I2C_Start(void) {
	GH3220_I2C_SDA_Output();
	GH3220_I2C_SCL_Set(1);
	GH3220_I2C_SDA_Set(1);
	GH3220_I2C_Delay();
	GH3220_I2C_SDA_Set(0);  // 在SCL高时拉低SDA产生起始条件
	GH3220_I2C_Delay();
	GH3220_I2C_SCL_Set(0);
	GH3220_I2C_Delay();
}

// 停止信号：SCL高时拉高SDA
void GH3220_I2C_Stop(void) {
	GH3220_I2C_SDA_Output();
	GH3220_I2C_SCL_Set(0);
	GH3220_I2C_SDA_Set(0);
	GH3220_I2C_Delay();
	GH3220_I2C_SCL_Set(1);
	GH3220_I2C_Delay();
	GH3220_I2C_SDA_Set(1);
	GH3220_I2C_Delay();
}

// 发送1字节并检测应答（应答为0表示成功）
//返回1表示发送成功
GU8 GH3220_I2C_SendByte(GU8 byte) {
	GU8 i;
	GH3220_I2C_SDA_Output();
	for (i = 0; i < 8; i++) {
		GH3220_I2C_SCL_Set(0);
		GH3220_I2C_Delay();
		GH3220_I2C_SDA_Set((byte & 0x80) ? 1 : 0);  // 从高位到低位发送
		byte <<= 1;
		GH3220_I2C_Delay();
		GH3220_I2C_SCL_Set(1);
		GH3220_I2C_Delay();
	}
	GH3220_I2C_SCL_Set(0);
	GH3220_I2C_SDA_Input();  // 切换为输入，读取应答
	GH3220_I2C_Delay();
	GH3220_I2C_SCL_Set(1);
	GH3220_I2C_Delay();
	GU8 ack = GH3220_I2C_SDA_Get();
	GH3220_I2C_SCL_Set(0);
	GH3220_I2C_SDA_Output();  // 恢复输出模式
	return (ack == 0);  // 应答为低电平表示成功
}

// 接收1字节并发送应答
GU8 GH3220_I2C_ReceiveByte(GU8 ack) {
	GU8 byte = 0;
	GH3220_I2C_SDA_Input();
	for (GU8 i = 0; i < 8; i++) {
		GH3220_I2C_SCL_Set(1);
		GH3220_I2C_Delay();
		byte <<= 1;
		if (GH3220_I2C_SDA_Get()) {
			byte |= 0x01;
		}
		GH3220_I2C_SCL_Set(0);
		GH3220_I2C_Delay();
	}
	GH3220_I2C_SDA_Output();
	GH3220_I2C_SDA_Set(ack ? 0 : 1); // 应答：0，非应答：1
	GH3220_I2C_SCL_Set(1);
	GH3220_I2C_Delay();
	GH3220_I2C_SCL_Set(0);
	GH3220_I2C_Delay();
	GH3220_I2C_SDA_Set(1);
	return byte;
}

/* i2c interface */
/**
 * @fn     void hal_gh3220_i2c_init(void)
 *
 * @brief  hal i2c init for gh3220
 *
 * @attention   None
 *
 * @param[in]   None
 * @param[out]  None
 *
 * @return  None
 */
void hal_gh3220_i2c_init() {
	hal_gh3220_set(); //置位RESET引脚
	HAL_Delay(8); //延时8ms
}

/**
 * @fn     uint8_t hal_gh3220_i2c_write(uint8_t device_id, const uint8_t write_buffer[], uint16_t length)
 *
 * @brief  hal i2c write for gh3220
 *
 * @attention   device_id is 8bits, if platform i2c use 7bits addr, should use (device_id >> 1)
 *
 * @param[in]   device_id       device addr
 * @param[in]   write_buffer    write data buffer
 * @param[in]   length          write data len
 * @param[out]  None
 *
 * @return  status
 * @retval  #1      return successfully
 * @retval  #0      return error
 */
GU8 hal_gh3220_i2c_write(GU8 device_id, const GU8 write_buffer[], GU16 length) {
	GH3220_I2C_Start();
	//
	if (!GH3220_I2C_SendByte(device_id)) {
		GH3220_I2C_Stop();
		return 0;
	}
	// 发送寄存器地址和数据
	for (GU16 i = 0; i < length; i++) {
		if (!GH3220_I2C_SendByte(write_buffer[i])) {
			GH3220_I2C_Stop();
			return 0;
		}
	}
	GH3220_I2C_Stop();
	return 1;
}

/**
 * @fn     uint8_t hal_gh3220_i2c_read(uint8_t device_id, const uint8_t write_buffer[], uint16_t write_length,
 *                            uint8_t read_buffer[], uint16_t read_length)
 *
 * @brief  hal i2c read for gh3220
 *
 * @attention   device_id is 8bits, if platform i2c use 7bits addr, should use (device_id >> 1)
 *
 * @param[in]   device_id       device addr
 * @param[in]   write_buffer    write data buffer
 * @param[in]   write_length    write data len
 * @param[in]   read_length     read data len
 * @param[out]  read_buffer     pointer to read buffer
 *
 * @return  status
 * @retval  #1      return successfully
 * @retval  #0      return error
 */
GU8 hal_gh3220_i2c_read(GU8 device_id, const GU8 write_buffer[],
		GU16 write_length, GU8 read_buffer[], GU16 read_length) {
	GU8 i2c_write_addr = (device_id >> 1) << 1;  // 转换为7位地址并设置写位（最低位0）
	GU8 i2c_read_addr = (device_id >> 1) << 1 | 0x01;  // 设置读位（最低位1）

	// 1. 启动I2C并发送写地址
	GH3220_I2C_Start();
	if (!GH3220_I2C_SendByte(i2c_write_addr)) {
		GH3220_I2C_Stop();
		return 0;
	}

	// 2. 发送写缓冲区数据（如寄存器地址）
	for (GU16 i = 0; i < write_length; i++) {
		if (!GH3220_I2C_SendByte(write_buffer[i])) {
			GH3220_I2C_Stop();
			return 0;
		}
	}

	// 3. 重复起始信号，切换为读操作
	GH3220_I2C_Start();
	if (!GH3220_I2C_SendByte(i2c_read_addr)) {
		GH3220_I2C_Stop();
		return 0;
	}

	// 4. 读取数据（最后1字节发送非应答）
	for (GU16 i = 0; i < read_length; i++) {
		if (i != read_length - 1) {
			read_buffer[i] = GH3220_I2C_ReceiveByte(1);  //回复ACK信号
		} else {
			read_buffer[i] = GH3220_I2C_ReceiveByte(0);  //回复NACK信号
		}
	}

	// 5. 停止I2C
	GH3220_I2C_Stop();
	return 1;
}

void hal_gh3220_set(void) {
	HAL_GPIO_WritePin(HR_RESET_GPIO_Port, HR_RESET_Pin, GPIO_PIN_SET);
}

void hal_gh3220_reset(void) {
	HAL_GPIO_WritePin(HR_RESET_GPIO_Port, HR_RESET_Pin, GPIO_PIN_RESET);
}

//配置中断引脚为上升沿触发
void hal_gh3220_int_init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = HR_INT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(HR_INT_GPIO_Port, &GPIO_InitStruct);
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}
