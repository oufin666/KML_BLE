#include "GPS.h"


// 新增：帧接收状态枚举
typedef enum {
    WAITING_FOR_DOLLAR,  // 等待帧头 '$'
    CHECKING_GNRMC,      // 正在检查是否为 "$GNRMC"
    RECEIVING_GNRMC      // 正在接收有效的 "$GNRMC" 帧数据
} FrameReceiveState;

// 新增：全局状态变量，初始化为等待 '$'
volatile FrameReceiveState frame_state = WAITING_FOR_DOLLAR;
// 新增：用于检查帧头的计数器
volatile uint8_t header_check_counter = 0;
// 目标帧头
const char target_header[] = "$GNRMC";


volatile uint8_t uart_rx_buf[GPS_UART_BUF_SIZE] = { 0 };
volatile uint16_t uart_rx_len = 0;
volatile uint8_t uart_frame_ready = 0;
uint8_t gps_rx_temp = 0;
//缓存前一个字符
uint8_t prev_rx_temp = 0;

//串口初始化已经在usart.c里面完成了
//因此这里初始只需要开启接收中断
void GPS_Init(void) {
	HAL_UART_Receive_IT(GPS_UART_HANDLE, &gps_rx_temp, 1);
}

//此函數在串口中断回調函數被調用，用于接收GPS串口數據
void GPS_Read(UART_HandleTypeDef *huart) {
    // 状态机处理
    switch (frame_state) {
        case WAITING_FOR_DOLLAR:
            // 等待接收帧头的第一个字符 '$'
            if (gps_rx_temp == '$') {
                // 检测到 '$'，准备开始检查后续的 "GNRMC"
                frame_state = CHECKING_GNRMC;
                header_check_counter = 0; // 重置检查计数器
                // 将 '$' 存入缓冲区的起始位置
                uart_rx_buf[0] = gps_rx_temp;
                header_check_counter++; // 计数器加1
            }
            // 如果不是 '$'，则忽略该字符，继续等待
            break;

        case CHECKING_GNRMC:
            // 正在检查是否为目标帧头 "$GNRMC"
            // 将当前字符与目标帧头的对应位置进行比较
            if (gps_rx_temp == target_header[header_check_counter]) {
                // 字符匹配，存入缓冲区
                uart_rx_buf[header_check_counter] = gps_rx_temp;
                header_check_counter++;

                // 如果已经匹配了所有6个字符（"$GNRMC"）
                if (header_check_counter == sizeof(target_header) - 1) {
                    // 帧头匹配成功！开始接收后续的数据
                    frame_state = RECEIVING_GNRMC;
                    uart_rx_len = header_check_counter; // 更新缓冲区长度
                }
            } else {
                // 字符不匹配，说明这不是我们想要的帧
                // 重置状态，等待下一个 '$'
                frame_state = WAITING_FOR_DOLLAR;
                uart_rx_len = 0; // 清空缓冲区
            }
            break;

        case RECEIVING_GNRMC:
            // 正在接收一个已确认的 GNRMC 帧
            if (uart_rx_len < GPS_UART_BUF_SIZE - 1) {
                uart_rx_buf[uart_rx_len++] = gps_rx_temp;

                // 检查是否接收到帧尾 "\r\n"
                if (prev_rx_temp == '\r' && gps_rx_temp == '\n') {
                    // 帧接收完整
                    uart_rx_buf[uart_rx_len - 2] = '\0'; // 在 '\r' 的位置添加字符串结束符
                    uart_frame_ready = 1;                // 置位帧就绪标志
                    uart_rx_len = 0;                     // 重置缓冲区长度，准备接收下一帧
                    frame_state = WAITING_FOR_DOLLAR;    // 重置状态机，等待下一个帧头
                }
                prev_rx_temp = gps_rx_temp; // 更新前一个字符，用于检测帧尾
            } else {
                // 缓冲区溢出，这通常不应该发生
                // 采取防御措施：重置所有状态
                uart_rx_len = 0;
                prev_rx_temp = 0;
                frame_state = WAITING_FOR_DOLLAR;
            }
            break;

        default:
            // 意外状态，重置
            frame_state = WAITING_FOR_DOLLAR;
            break;
    }

    // 重新启动中断接收，以接收下一个字符
    HAL_UART_Receive_IT(GPS_UART_HANDLE, &gps_rx_temp, 1);
}

/**
 * @brief  解析GPS协议语句
 * @param  str: 待解析的NMEA字符串（如"$GNRMC,...*75"）
 * @param  GPS_data: 解析结果存储结构体
 * @return 1：解析成功，0：解析失败（格式错误/校验失败）
 */
uint8_t GPS_Parse(const char *str, GPS_Data *GPS_data) {
	return GNRMC_Parse(str, GPS_data);
}

/**
 * @brief  获取完整的串口接收帧
 * @param  buf: 输出缓冲区（用于存储完整帧数据）
 * @return 1：成功获取帧，0：无完整帧
 */
uint8_t UART_ReadFrame(uint8_t *buf) {
	uint8_t result = 0;
	if (uart_frame_ready) {
		strncpy((char*) buf, (char*) uart_rx_buf, GPS_UART_BUF_SIZE - 1);
		buf[GPS_UART_BUF_SIZE - 1] = '\0';  // 确保字符串结束
		uart_frame_ready = 0;
		result = 1;
	}
	return result;
}

/**
 * @brief  计算NMEA语句校验和（从$到*之间的字符异或）
 * @param  str: NMEA语句字符串
 * @return 计算得到的校验和（十六进制）
 */
uint8_t NMEA_CheckSum(const char *str) {
	uint8_t checksum = 0;
	str++;  // 跳过'$'
	while (*str != '*' && *str != '\0') {
		checksum ^= (uint8_t) *str++;
	}
	return checksum;
}

/**
 * @brief  解析GNRMC语句
 * @param  nmea_str: 待解析的NMEA字符串（如"$GNRMC,...*75"）
 * @param  rmc_data: 解析结果存储结构体
 * @return 1：解析成功，0：解析失败（格式错误/校验失败）
 */

uint8_t GNRMC_Parse(const char *nmea_str, GPS_Data *rmc_data) {
    // 入参校验
    if (nmea_str == NULL || rmc_data == NULL) {
        return 0;
    }

    // 1. 检查语句头和最小长度
    if (strlen(nmea_str) < 18 || strncmp(nmea_str, "$GNRMC", 6) != 0) {
        return 0;
    }

    // 2. 验证校验和
    char *star_ptr = strchr(nmea_str, '*');
    if (star_ptr == NULL || (star_ptr - nmea_str) >= (int)strlen(nmea_str) - 1) {
        return 0; // 无校验和或校验和为空
    }
    uint8_t calc_checksum = NMEA_CheckSum(nmea_str);
    uint8_t recv_checksum = (uint8_t)strtol(star_ptr + 1, NULL, 16);
    if (calc_checksum != recv_checksum) {
        return 0; // 校验和不匹配
    }

    // 初始化解析结果
    memset(rmc_data, 0, sizeof(GPS_Data));
    rmc_data->valid = 0;

    // 3. 复制字符串用于分割（避免修改原数据）
    char buf[GPS_UART_BUF_SIZE];
    size_t nmea_len = strlen(nmea_str);
    if (nmea_len >= sizeof(buf)) {
        return 0; // 缓冲区不足
    }
    strncpy(buf, nmea_str, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    // 4. 使用线程安全的strtok_r分割字段
    char *saveptr; // 用于记录分割位置（线程安全）
    char *token = strtok_r(buf, ",*", &saveptr);
    int field_idx = 0;

    while (token != NULL) {
        switch (field_idx) {
            case 0:  // $GNRMC（跳过）
                break;

            case 1:  // 时间（hhmmss.sss）
                if (strlen(token) >= 6) {
                    float second;
                    int hour, minute;
                    // 严格校验格式解析结果
                    if (sscanf(token, "%2d%2d%f", &hour, &minute, &second) == 3) {
                        rmc_data->hour = hour+8;
                        rmc_data->minute = minute;
                        rmc_data->second = (uint8_t)(second + 0.5f); // 四舍五入
                    } else {
                        return 0; // 时间格式错误
                    }
                } else {
                    return 0; // 时间字段过短
                }
                break;

            case 2:  // 定位状态（A/V）
                if (token[0] == 'A' || token[0] == 'V') {
                    rmc_data->status = token[0];
                } else {
                    return 0; // 状态无效
                }
                break;

            case 3:  // 纬度（ddmm.mmmm）
                if (strlen(token) > 0) {
                    float lat_deg_min;
                    if (sscanf(token, "%f", &lat_deg_min) == 1) {
                        int deg = (int)(lat_deg_min / 100);
                        float min = lat_deg_min - deg * 100.0f;
                        if (min >= 0.0f && min < 60.0f) { // 校验分的合理性
                            rmc_data->latitude = deg + min / 60.0f;
                        } else {
                            return 0; // 纬度分无效
                        }
                    } else {
                        return 0; // 纬度格式错误
                    }
                } else {
                    return 0; // 纬度字段为空
                }
                break;

            case 4:  // 纬度半球（N/S）
                if (token[0] == 'S') {
                    rmc_data->latitude = -rmc_data->latitude;
                } else if (token[0] != 'N') {
                    return 0; // 半球标识无效
                }
                break;

            case 5:  // 经度（dddmm.mmmm）
                if (strlen(token) > 0) {
                    float lon_deg_min;
                    if (sscanf(token, "%f", &lon_deg_min) == 1) {
                        int deg = (int)(lon_deg_min / 100);
                        float min = lon_deg_min - deg * 100.0f;
                        if (min >= 0.0f && min < 60.0f) { // 校验分的合理性
                            rmc_data->longitude = deg + min / 60.0f;
                        } else {
                            return 0; // 经度分无效
                        }
                    } else {
                        return 0; // 经度格式错误
                    }
                } else {
                    return 0; // 经度字段为空
                }
                break;

            case 6:  // 经度半球（E/W）
                if (token[0] == 'W') {
                    rmc_data->longitude = -rmc_data->longitude;
                } else if (token[0] != 'E') {
                    return 0; // 半球标识无效
                }
                break;

            case 7:  // 速度（节）
            { // 用大括号解决C89变量声明问题
                float speed_kn;
                if (sscanf(token, "%f", &speed_kn) == 1) {
                    rmc_data->speed_km_h = speed_kn * 1.852f; // 转换为km/h
                } else {
                    return 0; // 速度格式错误
                }
            }
                break;

            case 8:  // 航向（度）
                if (sscanf(token, "%f", &rmc_data->course) != 1) {
                    return 0; // 航向格式错误
                }
                break;

            case 9:  // 日期（ddmmyy）
                if (strlen(token) >= 6) {
                    int dd, mm, yy;
                    if (sscanf(token, "%2d%2d%2d", &dd, &mm, &yy) == 3) {
                        // 简单校验日期合理性
                        if (dd >= 1 && dd <= 31 && mm >= 1 && mm <= 12) {
                            rmc_data->day = dd;
                            rmc_data->month = mm;
                            rmc_data->year = 2000 + yy;
                        } else {
                            return 0; // 日期无效
                        }
                    } else {
                        return 0; // 日期格式错误
                    }
                } else {
                    return 0; // 日期字段过短
                }
                break;

            default:
                break; // 忽略多余字段
        }

        // 继续分割下一个字段
        token = strtok_r(NULL, ",*", &saveptr);
        field_idx++;
    }

    // 检查是否解析到了必要的字段（至少到日期字段）
    if (field_idx < 10) {
        return 0;
    }

    // 标记解析有效性（定位有效时才为1）
    rmc_data->valid = (rmc_data->status == 'A') ? 1 : 0;
    return 1;
}

