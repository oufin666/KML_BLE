#ifndef GPS_H
#define GPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// GNRMC解析结果结构体
typedef struct {
    int16_t hour;         // 时（UTC）
    int16_t minute;       // 分（UTC）
    int16_t second;       // 秒（UTC）
    float latitude;       // 纬度（十进制度，北纬为正）
    float longitude;      // 经度（十进制度，东经为正）
    float speed_km_h;       // 速度（km/h）
    float course;         // 航向角（度）
    int16_t day;          // 日
    int16_t month;        // 月
    int16_t year;        // 年
    char status;          // 定位状态（'A'有效，'V'无效）
    uint8_t valid;        // 解析有效性（1：成功，0：失败）
} GPS_Data;

extern GPS_Data GPS_data_share;
extern uint8_t gps_data_change;
// 串口接收缓冲区配置
#define GPS_UART_BUF_SIZE 256  // 缓冲区大小（需大于最长NMEA语句长度）
extern volatile uint8_t uart_rx_buf[GPS_UART_BUF_SIZE];
extern volatile uint16_t uart_rx_len;
extern volatile uint8_t uart_frame_ready;//当为1时，表示可以开始解析数据了，应该调用GPS_Parse()
extern uint8_t gps_rx_temp;


void GPS_Init(void);
void GPS_Read(UART_HandleTypeDef *huart);
uint8_t GPS_Parse(const char *str, GPS_Data *GPS_data);
uint8_t UART_ReadFrame(uint8_t *buf);
uint8_t GNRMC_Parse(const char *nmea_str, GPS_Data *rmc_data) ;

#ifdef __cplusplus
}
#endif

#endif
