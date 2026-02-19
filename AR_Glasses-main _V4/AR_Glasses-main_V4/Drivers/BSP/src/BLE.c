#include <string.h>
#include "main.h"

// 串口句柄（根据实际使用的串口修改，此处以USART1为例）
//extern UART_HandleTypeDef huart1;


const uint8_t MIN_HR = 50;
const uint8_t MAX_HR = 220;

    // --- 二阶滤波器的内部状态变量 ---
    // 使用 Direct Form I 或 II 的差分方程:
    // y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
float x1, x2; // x[n-1], x[n-2] (输入历史)
float y1, y2; // y[n-1], y[n-2] (输出历史)

    // --- 滤波器系数 (由采样率和截止频率决定) ---
float a1, a2;
float b0, b1, b2;

    // 上一次有效的输出值 (用于在丢弃数据时保持输出)
uint8_t lastValidOutput;

    // 标记是否是第一次运行
bool isFirstRun;

// 全局变量定义
#define BLUETOOTH_MAC "fc7b93c337dc"  // 目标蓝牙模块MAC地址（无冒号）
#define HEART_RATE_DATA_LEN 4         // 心率广播包长度（16 4E 01 03 → 4字节）
#define MAX_CONNECT_RETRY 10          // 最大连接重试次数
#define HEART_RATE_HEADER 0x16        // 心率包头部标识（第一位固定为0x16）
#define DEVICE_NAME "57732-1"

// 逐字节接收相关变量（状态机管理）
typedef enum {
    RX_STATE_IDLE,        // 空闲态：等待检测0x16起始字节
    RX_STATE_RECEIVING    // 接收态：已检测到起始字节，接收剩余3字节
} Rx_StateTypeDef;

Rx_StateTypeDef rx_state = RX_STATE_IDLE; // 接收状态机
uint8_t rx_byte;                          // 单字节接收缓冲区
uint8_t hr_data_buf[HEART_RATE_DATA_LEN]; // 完整心率包缓冲区
uint8_t rx_count = 0;                     // 已接收字节计数
uint8_t heart_rate_val = 0;               // 解析后的心率值（十进制）
uint8_t connect_response_buf[256];         // 蓝牙模块响应接收缓冲区


void param_init() {
        // 重置状态
        x1 = x2 = 0.0f;
        y1 = y2 = 0.0f;
        isFirstRun = true;
        lastValidOutput = 0;

        // --- 计算二阶巴特沃斯低通滤波器系数 ---
        // 这里的数学推导基于双线性变换
 //       float omega = 2.0f * 3.14159265f * cutoffFreq / sampleRate;
        float sn = 0.5878;
        float cs = 0.8090;
        float alpha1 = sn / (2.0f * 0.707f); // 0.707 是 Q 值 (1/sqrt(2))

        float a0 = 1.0f + alpha1;

        // 计算系数并归一化 (除以 a0)
        b0 = (1.0f - cs) / 2.0f / a0;
        b1 = (1.0f - cs) / a0;
        b2 = (1.0f - cs) / 2.0f / a0;
        a1 = (-2.0f * cs) / a0;
        a2 = (1.0f - alpha1) / a0;
}


/*********************************************************************
 * @function  : AT_Send_Command
 * @brief     : 向蓝牙模块发送AT指令（带回车换行结束符，符合手册4.1节规范）
 * @param     : cmd: AT指令字符串; len: 指令长度
 * @retval    : HAL状态值
 ********************************************************************/
HAL_StatusTypeDef AT_Send_Command(uint8_t *cmd, uint16_t len)
{
    // 发送前清空响应缓冲区
    memset(connect_response_buf, 0, sizeof(connect_response_buf));
    // 发送AT指令（超时500ms）
    return HAL_UART_Transmit(&huart1, cmd, len, 500);
}

/*********************************************************************
 * @function  : Check_Connect_Response
 * @brief     : 校验蓝牙模块连接响应，判断是否连接成功
 * @param     : none
 * @retval    : 1-连接成功，0-连接失败
 ********************************************************************/
uint8_t Check_Connect_Response(void)
{
    // 接收模块响应（超时1000ms，匹配模块响应时长）
    HAL_UART_Receive(&huart1, connect_response_buf, sizeof(connect_response_buf), 8000);

    // 校验响应是否包含"+Connected>>"（DX-BT36连接成功的典型响应）
    if(strstr((char*)connect_response_buf, "+Connected>>") != NULL &&
       strstr((char*)connect_response_buf, BLUETOOTH_MAC) != NULL)
    {
        return 1; // 连接成功
    }
    return 0; // 连接失败
}

/*********************************************************************
 * @function  : Bluetooth_Connect_Specified_MAC
 * @brief     : 连接指定MAC地址的蓝牙模块（带10次重试+连接成功校验）
 * @param     : none
 * @retval    : HAL状态值（HAL_OK表示最终连接成功，HAL_ERROR表示重试10次失败）
 ********************************************************************/
HAL_StatusTypeDef Bluetooth_Connect_Specified_MAC(void)
{
    uint8_t at_cmd[32];
    uint16_t cmd_len;
    uint8_t retry_count = 0; // 重试计数器
    uint8_t connect_success = 0; // 连接成功标志

    // 循环重试连接，直到成功或达到最大次数
    while(retry_count < MAX_CONNECT_RETRY && !connect_success)
    {
        retry_count++; // 重试次数+1

        // 构建AT指令：AT+CONA<MAC地址>\r\n（手册格式：AT+CONA<mac>）
        cmd_len = sprintf((char*)at_cmd, "AT+CONA%s\r\n", BLUETOOTH_MAC);

        // 发送连接指令
        if(AT_Send_Command(at_cmd, cmd_len) == HAL_OK)
        {
            // 校验模块响应
            connect_success = Check_Connect_Response();
            connect_success = 1;
        }

        // 未成功则延时后重试（避免频繁发送指令）
        if(!connect_success)
        {
            HAL_Delay(500); // 重试间隔500ms
        }
    }

    // 返回最终状态：成功返回HAL_OK，失败返回HAL_ERROR
    return connect_success ? HAL_OK : HAL_ERROR;
}

HAL_StatusTypeDef Bluetooth_Connect_Specified_Name(void)
{
    uint8_t at_cmd[32];
    uint16_t cmd_len;
    uint8_t connect_success = 1; // 连接成功标志
    	cmd_len = sprintf((char*)at_cmd, "AT+FILTER%s\r\n", DEVICE_NAME);
    	if(AT_Send_Command(at_cmd, cmd_len) == HAL_OK)
    	{
    	      HAL_Delay(500);
    	 }

    	cmd_len = sprintf((char*)at_cmd, "AT+AUTOCONN%s\r\n", "1");
        // 发送连接指令
        if(AT_Send_Command(at_cmd, cmd_len) == HAL_OK)
        {
            // 校验模块响应
//            connect_success = Check_Connect_Response();
        	HAL_Delay(8000);

        }

        // 未成功则延时后重试（避免频繁发送指令）
        if(!connect_success)
        {
            HAL_Delay(500); // 重试间隔500ms
        }


    // 返回最终状态：成功返回HAL_OK，失败返回HAL_ERROR
    return connect_success ? HAL_OK : HAL_ERROR;
}

/*********************************************************************
 * @function  : HeartRate_Parser
 * @brief     : 解析完整的心率数据包（仅合法包更新心率值）
 * @param     : data: 完整的4字节心率数据包缓冲区
 * @retval    : 十进制心率值（非法包返回0）
 ********************************************************************/
uint8_t HeartRate_Parser(uint8_t *data)
{
    // 1. 空指针/长度校验
    if(data == NULL) return 0;

    // 2. 校验数据包头部（双重校验，确保是合法心率包）
    if(data[0] != HEART_RATE_HEADER)
    {
        return 0; // 非法包，返回0不更新心率值
    }
    // 3. 合法包：提取第二位为心率数据（十六进制转十进制）
//    heart_rate_val = data[1];  // 例：0x4E → 78（十进制）
    uint8_t rawHR = data[1];
    		if (rawHR < MIN_HR || rawHR > MAX_HR) {
                return lastValidOutput;
            }

            // 2. 初始化处理 (防止刚启动时从0突变)
            if (isFirstRun) {
                x1 = x2 = rawHR;
                y1 = y2 = rawHR;
                lastValidOutput = rawHR;
                isFirstRun = false;
                return rawHR;
            }

            // 3. 执行二阶滤波 (Direct Form I)
            // 公式：y[n] = b0*x + b1*x1 + b2*x2 - a1*y1 - a2*y2
            float output = b0 * rawHR + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

            // 4. 更新历史状态
            x2 = x1;
            x1 = rawHR;
            y2 = y1;
            y1 = output;

            // 5. 更新保持值
            lastValidOutput = output;
    				heart_rate_val = (uint8_t)output;
    return heart_rate_val;
}

/*********************************************************************
 * @function  : HAL_UART_RxCpltCallback
 * @brief     : UART接收完成中断回调函数（逐字节接收+状态机解析）
 * @param     : huart: 串口句柄
 * @retval    : none
 ********************************************************************/

void BLE_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)  // 确认是蓝牙模块连接的串口
    {
        // 从串口缓冲区获取接收到的字节
        uint8_t rx_byte = *((uint8_t*)huart->pRxBuffPtr - 1);
        
        switch(rx_state)
        {
            case RX_STATE_IDLE:
                // 空闲态：检测到起始字节0x16，进入接收态
                if(rx_byte == HEART_RATE_HEADER)
                {
                    rx_state = RX_STATE_RECEIVING; // 切换到接收态
                    rx_count = 0;                  // 重置接收计数
                    hr_data_buf[rx_count++] = rx_byte; // 存储起始字节
                }
                break;

            case RX_STATE_RECEIVING:
                // 接收态：存储当前字节，计数+1
                hr_data_buf[rx_count++] = rx_byte;

                // 凑够4字节完整数据包，解析并重置状态机
                if(rx_count >= HEART_RATE_DATA_LEN)
                {
                    HeartRate_Parser(hr_data_buf); // 解析心率值
                    rx_state = RX_STATE_IDLE;      // 回到空闲态
                    rx_count = 0;                  // 重置计数
                    memset(hr_data_buf, 0, sizeof(hr_data_buf)); // 清空数据包缓冲区
                }
                break;

            default:
                // 异常状态：重置为空闲态
                rx_state = RX_STATE_IDLE;
                rx_count = 0;
                break;
        }
    }
}


/*********************************************************************
 * @function  : Bluetooth_Init
 * @brief     : 蓝牙模块初始化（带重试的连接 + 开启逐字节接收）
 * @param     : none
 * @retval    : none
 ********************************************************************/
void Bluetooth_Init(void)
{
	param_init();
    // 1. 连接指定MAC地址的蓝牙模块（带10次重试）
//    HAL_StatusTypeDef connect_status = Bluetooth_Connect_Specified_Name();
	HAL_StatusTypeDef connect_status =  HAL_OK;
    // 2. 仅连接成功后才开启逐字节接收
    if(connect_status == HAL_OK)
    {
        // 初始化接收状态机
        rx_state = RX_STATE_IDLE;
        rx_count = 0;
        memset(hr_data_buf, 0, sizeof(hr_data_buf));

        // 开启UART单字节中断接收（核心修改：每次只接收1字节）
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
    else
    {

        // 连接失败可添加错误处理（如LED闪烁提示）
        // 示例：while(1) { HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); HAL_Delay(500); }
    }
}

/*********************************************************************
 * @function  : Get_Heart_Rate
 * @brief     : 外部调用函数，返回当前解析后的心率值
 * @param     : none
 * @retval    : 十进制心率值（0~255，0可能表示无合法数据）
 ********************************************************************/
uint8_t Get_Heart_Rate(void)
{
    return heart_rate_val;
}


