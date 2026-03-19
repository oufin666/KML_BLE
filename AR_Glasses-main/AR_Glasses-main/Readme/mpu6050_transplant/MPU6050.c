#include "MPU6050.h"

#define MPU6050_ADDRESS		0xD0		//MPU6050的I2C从机地址

/**
  * 函    数：MPU6050等待事件
  * 参    数：同I2C_CheckEvent
  * 返 回 值：无
  */
void MPU6050_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
{
	uint32_t Timeout;
	Timeout = 10000;									//给定超时计数时间
	while (I2C_CheckEvent(I2Cx, I2C_EVENT) != SUCCESS)	//循环等待指定事件
	{
		Timeout --;										//等待时，计数值自减
		if (Timeout == 0)								//自减到0后，等待超时
		{
			/*超时的错误处理代码，可以添加到此处*/
			break;										//跳出等待，不等了
		}
	}
}

/**
  * 函    数：MPU6050写寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MPU6050手册的寄存器描述
  * 参    数：Data 要写入寄存器的数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成起始条件
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C2, MPU6050_ADDRESS, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
	I2C_SendData(I2C2, RegAddress);											//硬件I2C发送寄存器地址
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTING);			//等待EV8
	
	I2C_SendData(I2C2, Data);												//硬件I2C发送数据
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
	
	I2C_GenerateSTOP(I2C2, ENABLE);											//硬件I2C生成终止条件
}

/**
  * 函    数：MPU6050读寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MPU6050手册的寄存器描述
  * 返 回 值：读取寄存器的数据，范围：0x00~0xFF
  */
uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	
	I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成起始条件
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C2, MPU6050_ADDRESS, I2C_Direction_Transmitter);	//硬件I2C发送从机地址，方向为发送
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);	//等待EV6
	
	I2C_SendData(I2C2, RegAddress);											//硬件I2C发送寄存器地址
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED);				//等待EV8_2
	
	I2C_GenerateSTART(I2C2, ENABLE);										//硬件I2C生成重复起始条件
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT);					//等待EV5
	
	I2C_Send7bitAddress(I2C2, MPU6050_ADDRESS, I2C_Direction_Receiver);		//硬件I2C发送从机地址，方向为接收
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);		//等待EV6
	
	I2C_AcknowledgeConfig(I2C2, DISABLE);									//在接收最后一个字节之前提前将应答失能
	I2C_GenerateSTOP(I2C2, ENABLE);											//在接收最后一个字节之前提前申请停止条件
	
	MPU6050_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED);				//等待EV7
	Data = I2C_ReceiveData(I2C2);											//接收数据寄存器
	
	I2C_AcknowledgeConfig(I2C2, ENABLE);									//将应答恢复为使能，为了不影响后续可能产生的读取多字节操作
	
	return Data;
}

/**
  * 函    数：MPU6050初始化
  * 参    数：无
  * 返 回 值：无
  */
void MPU6050_Init(void)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);		//开启I2C2的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PB10和PB11引脚初始化为复用开漏输出
	
	/*I2C初始化*/
	I2C_InitTypeDef I2C_InitStructure;						//定义结构体变量
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;				//模式，选择为I2C模式
	I2C_InitStructure.I2C_ClockSpeed = 50000;				//时钟速度，选择为50KHz
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;		//时钟占空比，选择Tlow/Thigh = 2
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;				//应答，选择使能
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;	//应答地址，选择7位，从机模式下才有效
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;				//自身地址，从机模式下才有效
	I2C_Init(I2C2, &I2C_InitStructure);						//将结构体变量交给I2C_Init，配置I2C2
	
	/*I2C使能*/
	I2C_Cmd(I2C2, ENABLE);									//使能I2C2，开始运行
	
	/*MPU6050寄存器初始化，需要对照MPU6050手册的寄存器描述配置，此处仅配置了部分重要的寄存器*/
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);				//电源管理寄存器1，取消睡眠模式，选择时钟源为X轴陀螺仪
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);				//电源管理寄存器2，保持默认值0，所有轴均不待机
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x09);				//采样率分频寄存器，配置采样率
	MPU6050_WriteReg(MPU6050_CONFIG, 0x06);					//配置寄存器，配置DLPF
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);			//陀螺仪配置寄存器，选择满量程为±2000°/s
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18);			//加速度计配置寄存器，选择满量程为±16g
}


/**
  * 函    数：MPU6050获取ID号
  * 参    数：无
  * 返 回 值：MPU6050的ID号
  */
uint8_t MPU6050_GetID(void)
{
	return MPU6050_ReadReg(MPU6050_WHO_AM_I);		//返回WHO_AM_I寄存器的值
}

/**
  * 函    数：MPU6050获取数据
  * 参    数：AccX AccY AccZ 加速度计X、Y、Z轴的数据，使用输出参数的形式返回，范围：-32768~32767
  * 参    数：GyroX GyroY GyroZ 陀螺仪X、Y、Z轴的数据，使用输出参数的形式返回，范围：-32768~32767
  * 返 回 值：无
  */
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
	uint8_t DataH, DataL;								//定义数据高8位和低8位的变量
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);		//读取加速度计X轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);		//读取加速度计X轴的低8位数据
	*AccX = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);		//读取加速度计Y轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);		//读取加速度计Y轴的低8位数据
	*AccY = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);		//读取加速度计Z轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);		//读取加速度计Z轴的低8位数据
	*AccZ = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);		//读取陀螺仪X轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);		//读取陀螺仪X轴的低8位数据
	*GyroX = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);		//读取陀螺仪Y轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);		//读取陀螺仪Y轴的低8位数据
	*GyroY = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);		//读取陀螺仪Z轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);		//读取陀螺仪Z轴的低8位数据
	*GyroZ = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
}

/**
  * @brief  等待I2C事件（带超时和错误处理）
  * @param  I2Cx: I2C硬件接口（I2C1/I2C2）
  * @param  I2C_EVENT: 期望的I2C事件（如I2C_EVENT_MASTER_MODE_SELECT）
  * @retval uint8_t: 0-成功 | 1-超时失败
  */
uint8_t MPU6050_WaitEvent_oyq(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
{
    uint32_t Timeout = 100000;  // 增大超时计数（根据实际时钟调整）
    
    while (I2C_CheckEvent(I2Cx, I2C_EVENT) != SUCCESS) {
        if (Timeout-- == 0) {
            I2C_GenerateSTOP(I2Cx, ENABLE);  // 超时后强制终止通信
            return 1;  // 返回错误码
        }
    }
    return 0;  // 成功
}

/**
  * @brief  向MPU6050写入寄存器数据
  * @param  I2Cx: I2C硬件接口（I2C1/I2C2）
  * @param  RegAddress: 寄存器地址
  * @param  Data: 要写入的数据
  * @retval uint8_t: 0-成功 | 1-失败
  */
uint8_t MPU6050_WriteReg_oyq(I2C_TypeDef* I2Cx, uint8_t RegAddress, uint8_t Data)
{
    /* 1. 发送起始条件 */
    I2C_GenerateSTART(I2Cx, ENABLE);
    if (MPU6050_WaitEvent_oyq(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)) return 1;

    /* 2. 发送设备地址（写模式） */
    I2C_Send7bitAddress(I2Cx, MPU6050_ADDRESS, I2C_Direction_Transmitter);
    if (MPU6050_WaitEvent_oyq(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) return 1;

    /* 3. 发送寄存器地址 */
    I2C_SendData(I2Cx, RegAddress);
    if (MPU6050_WaitEvent_oyq(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return 1;

    /* 4. 发送数据 */
    I2C_SendData(I2Cx, Data);
    if (MPU6050_WaitEvent_oyq(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return 1;

    /* 5. 发送停止条件 */
    I2C_GenerateSTOP(I2Cx, ENABLE);
    return 0;  // 成功
}

/**
  * @brief  从MPU6050读取寄存器数据
  * @param  I2Cx: I2C硬件接口（I2C1/I2C2）
  * @param  RegAddress: 寄存器地址
  * @retval uint8_t: 读取的数据（0xFF表示失败）
  */
uint8_t MPU6050_ReadReg_oyq(I2C_TypeDef* I2Cx, uint8_t RegAddress)
{
    uint8_t Data = 0xFF;  // 默认错误值

    /* 1. 发送起始条件（写模式） */
    I2C_GenerateSTART(I2Cx, ENABLE);
    if (MPU6050_WaitEvent_oyq(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)) return Data;

    /* 2. 发送设备地址（写模式） */
    I2C_Send7bitAddress(I2Cx, MPU6050_ADDRESS, I2C_Direction_Transmitter);
    if (MPU6050_WaitEvent_oyq(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) return Data;

    /* 3. 发送寄存器地址 */
    I2C_SendData(I2Cx, RegAddress);
    if (MPU6050_WaitEvent_oyq(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return Data;

    /* 4. 发送重复起始条件（切换为读模式） */
    I2C_GenerateSTART(I2Cx, ENABLE);
    if (MPU6050_WaitEvent_oyq(I2Cx, I2C_EVENT_MASTER_MODE_SELECT)) return Data;

    /* 5. 发送设备地址（读模式） */
    I2C_Send7bitAddress(I2Cx, MPU6050_ADDRESS, I2C_Direction_Receiver);
    if (MPU6050_WaitEvent_oyq(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) return Data;

    /* 6. 配置非应答+停止条件（单字节读取） */
    I2C_AcknowledgeConfig(I2Cx, DISABLE);
    I2C_GenerateSTOP(I2Cx, ENABLE);

    /* 7. 等待数据接收完成 */
    if (MPU6050_WaitEvent_oyq(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) == 0) {
        Data = I2C_ReceiveData(I2Cx);  // 读取数据
    }

    /* 8. 恢复应答使能（为后续操作做准备） */
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
    return Data;
}

uint8_t MPU6050_hard_Init_oyq(I2C_TypeDef* I2Cx, GPIO_TypeDef* GPIOx, uint16_t SCL_Pin, uint16_t SDA_Pin)
{
    /* 1. 参数检查（严格模式）*/
    if (!IS_I2C_ALL_PERIPH(I2Cx)) return 1;
    if (!IS_GPIO_ALL_PERIPH(GPIOx)) return 1;
    if ((SCL_Pin == 0) || (SDA_Pin == 0)) return 1;

    /* 2. 使能时钟（动态适配）*/
    uint32_t RCC_APB1Periph = (I2Cx == I2C1) ? RCC_APB1Periph_I2C1 : RCC_APB1Periph_I2C2;
    uint32_t RCC_APB2Periph_GPIOx = 0;
    
    if (GPIOx == GPIOB) RCC_APB2Periph_GPIOx = RCC_APB2Periph_GPIOB;
    else if (GPIOx == GPIOF) RCC_APB2Periph_GPIOx = RCC_APB2Periph_GPIOF;
    else return 1;  // 不支持的GPIO端口

    RCC_APB1PeriphClockCmd(RCC_APB1Periph, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOx, ENABLE);

    /* 3. GPIO初始化 */
    GPIO_InitTypeDef GPIO_InitStruct = {
        .GPIO_Pin = SCL_Pin | SDA_Pin,
        .GPIO_Mode = GPIO_Mode_AF_OD,
        .GPIO_Speed = GPIO_Speed_50MHz
    };
    GPIO_Init(GPIOx, &GPIO_InitStruct);

    /* 4. I2C初始化（带超时检查）*/
    I2C_InitTypeDef I2C_InitStruct = {
        .I2C_Mode = I2C_Mode_I2C,
        .I2C_ClockSpeed = 50000,
        .I2C_DutyCycle = I2C_DutyCycle_2,
        .I2C_Ack = I2C_Ack_Enable,
        .I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit,
        .I2C_OwnAddress1 = 0x00
    };
    I2C_Init(I2Cx, &I2C_InitStruct);
    I2C_Cmd(I2Cx, ENABLE);

    /* 5. MPU6050寄存器初始化（验证设备是否在线）*/
    if (MPU6050_WriteReg_oyq(I2Cx, MPU6050_PWR_MGMT_1, 0x01) != 0) return 1;
    Delay_ms(10);  // 等待唤醒稳定

    /* 6. 验证设备ID */
    uint8_t id = MPU6050_ReadReg_oyq(I2Cx, MPU6050_WHO_AM_I);
    if (id != 0x68) return 1;  // ID不匹配

    /* 7. 配置传感器参数 */
    uint8_t config_ok = 0;
    config_ok |= MPU6050_WriteReg_oyq(I2Cx, MPU6050_PWR_MGMT_2, 0x00);
    config_ok |= MPU6050_WriteReg_oyq(I2Cx, MPU6050_SMPLRT_DIV, 0x09);
    config_ok |= MPU6050_WriteReg_oyq(I2Cx, MPU6050_CONFIG, 0x06);
    config_ok |= MPU6050_WriteReg_oyq(I2Cx, MPU6050_GYRO_CONFIG, 0x18);  // ±2000°/s
    config_ok |= MPU6050_WriteReg_oyq(I2Cx, MPU6050_ACCEL_CONFIG, 0x18); // ±16g

    return (config_ok == 0) ? 0 : 1;  // 全部寄存器写入成功返回0
}

