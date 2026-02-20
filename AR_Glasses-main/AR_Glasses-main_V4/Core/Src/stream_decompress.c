/**
 * 流式解压缩模块 - 完全符合小程序压缩格式
 * 根据 kml-compression.md 规范实现
 * 
 * 压缩格式：
 * - 前2字节：0x42 0x4C (BL标识)
 * - 坐标数据：0xFE + 坐标数量(2字节小端) + 每个坐标12字节(lon,lat,alt各4字节int32小端)
 * - 普通字符：UTF-8编码
 */

#include "ff.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// 缓冲区大小定义
#define READ_CHUNK_SIZE 8192   // 8KB读取缓冲区
#define WRITE_CHUNK_SIZE 16384  // 16KB写入缓冲区

/**
 * 从读取缓冲区获取一个字节
 * 如果缓冲区为空，自动从文件读取下一块数据
 */
static int get_byte(FIL* file, uint8_t* buffer, uint32_t* pos, uint32_t* available, 
                     uint32_t* total_read, uint32_t file_size) {
    if (*pos >= *available) {
        // 缓冲区已用完，读取下一块
        if (*total_read >= file_size) {
            return -1; // 文件结束
        }
        
        UINT bytes_read;
        FRESULT res = f_read(file, buffer, READ_CHUNK_SIZE, &bytes_read);
        if (res != FR_OK || bytes_read == 0) {
            return -1; // 读取失败
        }
        
        *total_read += bytes_read;
        *available = bytes_read;
        *pos = 0;
    }
    
    return buffer[(*pos)++];
}

/**
 * 写入数据到输出缓冲区
 * 如果缓冲区满，自动写入文件
 */
static void write_data(FIL* file, uint8_t* buffer, uint32_t* pos, uint32_t* total_written,
                       const void* data, uint32_t len) {
    const uint8_t* src = (const uint8_t*)data;
    
    for (uint32_t i = 0; i < len; i++) {
        if (*pos >= WRITE_CHUNK_SIZE) {
            // 缓冲区满，写入文件
            UINT bw;
            f_write(file, buffer, *pos, &bw);
            *total_written += bw;
            *pos = 0;
        }
        buffer[(*pos)++] = src[i];
    }
}

/**
 * 流式解压函数
 * 
 * @param compressed_file 压缩文件路径
 * @param decompressed_file 解压后文件路径
 * @return 0=成功, 其他=失败
 */
int stream_decompress_file(const char* compressed_file, const char* decompressed_file) {
    FRESULT res;
    FIL read_file, write_file;
    uint8_t* read_buffer = NULL;
    uint8_t* write_buffer = NULL;
    int result = -1;
    
    // 打开压缩文件
    res = f_open(&read_file, compressed_file, FA_OPEN_EXISTING | FA_READ);
    if (res != FR_OK) {
        uint8_t msg[80];
        sprintf((char*)msg, "Failed to open compressed file: %d\r\n", res);
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
        return -1;
    }
    
    uint32_t file_size = f_size(&read_file);
    uint8_t size_msg[100];
    sprintf((char*)size_msg, "=== Stream Decompression ===\r\nCompressed: %ld bytes\r\n", file_size);
    HAL_UART_Transmit(&huart1, size_msg, strlen((char*)size_msg), 100);
    
    // 分配缓冲区
    read_buffer = (uint8_t*)malloc(READ_CHUNK_SIZE);
    write_buffer = (uint8_t*)malloc(WRITE_CHUNK_SIZE);
    
    if (!read_buffer || !write_buffer) {
        uint8_t msg[] = "Failed to allocate buffers\r\n";
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
        goto cleanup;
    }
    
    uint8_t mem_msg[80];
    sprintf((char*)mem_msg, "Buffers allocated: R=%d, W=%d\r\n", READ_CHUNK_SIZE, WRITE_CHUNK_SIZE);
    HAL_UART_Transmit(&huart1, mem_msg, strlen((char*)mem_msg), 100);
    
    // 打开输出文件
    res = f_open(&write_file, decompressed_file, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        uint8_t msg[80];
        sprintf((char*)msg, "Failed to create output file: %d\r\n", res);
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
        goto cleanup;
    }
    
    // 初始化状态变量
    uint32_t read_pos = 0;
    uint32_t read_available = 0;
    uint32_t total_read = 0;
    uint32_t write_pos = 0;
    uint32_t total_written = 0;
    
    // 读取并验证压缩标识 (0x42 0x4C = "BL")
    int b1 = get_byte(&read_file, read_buffer, &read_pos, &read_available, &total_read, file_size);
    int b2 = get_byte(&read_file, read_buffer, &read_pos, &read_available, &total_read, file_size);
    
    if (b1 < 0 || b2 < 0 || b1 != 0x42 || b2 != 0x4C) {
        uint8_t msg[80];
        sprintf((char*)msg, "Invalid header: 0x%02X 0x%02X\r\n", b1, b2);
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
        f_close(&write_file);
        goto cleanup;
    }
    
    uint8_t header_msg[] = "Valid header (BL)\r\n";
    HAL_UART_Transmit(&huart1, header_msg, strlen((char*)header_msg), 100);
    
    // 流式解压主循环
    uint32_t coord_blocks = 0;
    uint32_t total_coords = 0;
    
    while (total_read < file_size) {
        int byte = get_byte(&read_file, read_buffer, &read_pos, &read_available, &total_read, file_size);
        if (byte < 0) {
            uint8_t eof_msg[80];
            sprintf((char*)eof_msg, "EOF reached at %ld bytes\r\n", total_read);
            HAL_UART_Transmit(&huart1, eof_msg, strlen((char*)eof_msg), 100);
            break; // 文件结束
        }
        
        if (byte == 0xfe) {
            // 坐标数据压缩标记
            int c1 = get_byte(&read_file, read_buffer, &read_pos, &read_available, &total_read, file_size);
            int c2 = get_byte(&read_file, read_buffer, &read_pos, &read_available, &total_read, file_size);
            if (c1 < 0 || c2 < 0) {
                uint8_t err_msg[] = "ERROR: Incomplete coord count\r\n";
                HAL_UART_Transmit(&huart1, err_msg, strlen((char*)err_msg), 100);
                break;
            }
            
            int coord_count = c1 | (c2 << 8);
            coord_blocks++;
            total_coords += coord_count;
            
            uint8_t coord_msg[80];
            sprintf((char*)coord_msg, "Coord block #%ld: %d coords\r\n", coord_blocks, coord_count);
            HAL_UART_Transmit(&huart1, coord_msg, strlen((char*)coord_msg), 100);
            
            // 写入<coordinates>标签
            const char* start_tag = "<coordinates>";
            write_data(&write_file, write_buffer, &write_pos, &total_written, start_tag, strlen(start_tag));
            
            // 处理每个坐标
            for (int i = 0; i < coord_count; i++) {
                // 读取12字节坐标数据
                uint8_t coord_bytes[12];
                int valid = 1;
                for (int j = 0; j < 12; j++) {
                    int b = get_byte(&read_file, read_buffer, &read_pos, &read_available, &total_read, file_size);
                    if (b < 0) {
                        valid = 0;
                        uint8_t err_msg[100];
                        sprintf((char*)err_msg, "ERROR: Incomplete coord at %d/%d\r\n", i, coord_count);
                        HAL_UART_Transmit(&huart1, err_msg, strlen((char*)err_msg), 100);
                        break;
                    }
                    coord_bytes[j] = (uint8_t)b;
                }
                if (!valid) break;
                
                // 解析坐标（小端序）
                int32_t lon_int = (int32_t)(coord_bytes[0] | (coord_bytes[1] << 8) | (coord_bytes[2] << 16) | (coord_bytes[3] << 24));
                int32_t lat_int = (int32_t)(coord_bytes[4] | (coord_bytes[5] << 8) | (coord_bytes[6] << 16) | (coord_bytes[7] << 24));
                int32_t alt_int = (int32_t)(coord_bytes[8] | (coord_bytes[9] << 8) | (coord_bytes[10] << 16) | (coord_bytes[11] << 24));
                
                // 转换为浮点数
                double lon = lon_int / 1000000.0;
                double lat = lat_int / 1000000.0;
                double alt = alt_int / 1000000.0;
                
                // 格式化坐标字符串
                char coord_str[64];
                int len = snprintf(coord_str, sizeof(coord_str), "%.6f,%.6f,%.6f ", lon, lat, alt);
                
                write_data(&write_file, write_buffer, &write_pos, &total_written, coord_str, len);
            }
            
            // 写入</coordinates>标签
            const char* end_tag = "</coordinates>";
            write_data(&write_file, write_buffer, &write_pos, &total_written, end_tag, strlen(end_tag));
            
        } else if (byte < 0x80) {
            // 单字节UTF-8
            uint8_t b = (uint8_t)byte;
            write_data(&write_file, write_buffer, &write_pos, &total_written, &b, 1);
        } else if (byte >= 0xc0 && byte < 0xe0) {
            // 双字节UTF-8
            uint8_t bytes[2];
            bytes[0] = (uint8_t)byte;
            int b = get_byte(&read_file, read_buffer, &read_pos, &read_available, &total_read, file_size);
            if (b < 0) break;
            bytes[1] = (uint8_t)b;
            write_data(&write_file, write_buffer, &write_pos, &total_written, bytes, 2);
        } else if (byte >= 0xe0 && byte < 0xf0) {
            // 三字节UTF-8
            uint8_t bytes[3];
            bytes[0] = (uint8_t)byte;
            int valid = 1;
            for (int i = 1; i < 3; i++) {
                int b = get_byte(&read_file, read_buffer, &read_pos, &read_available, &total_read, file_size);
                if (b < 0) {
                    valid = 0;
                    break;
                }
                bytes[i] = (uint8_t)b;
            }
            if (valid) {
                write_data(&write_file, write_buffer, &write_pos, &total_written, bytes, 3);
            }
        } else if (byte >= 0xf0 && byte < 0xf8) {
            // 四字节UTF-8
            uint8_t bytes[4];
            bytes[0] = (uint8_t)byte;
            int valid = 1;
            for (int i = 1; i < 4; i++) {
                int b = get_byte(&read_file, read_buffer, &read_pos, &read_available, &total_read, file_size);
                if (b < 0) {
                    valid = 0;
                    break;
                }
                bytes[i] = (uint8_t)b;
            }
            if (valid) {
                write_data(&write_file, write_buffer, &write_pos, &total_written, bytes, 4);
            }
        } else {
            // 其他字节（UTF-8续字节等）
            uint8_t b = (uint8_t)byte;
            write_data(&write_file, write_buffer, &write_pos, &total_written, &b, 1);
        }
        
        // 每处理64KB打印一次进度
        if (total_read % 65536 < READ_CHUNK_SIZE) {
            uint8_t progress[60];
            sprintf((char*)progress, "Progress: %ld/%ld bytes\r\n", total_read, file_size);
            HAL_UART_Transmit(&huart1, progress, strlen((char*)progress), 50);
        }
    }
    
    // 写入剩余数据
    if (write_pos > 0) {
        UINT bw;
        f_write(&write_file, write_buffer, write_pos, &bw);
        total_written += bw;
    }
    
    // 同步并关闭输出文件
    f_sync(&write_file);
    f_close(&write_file);
    
    uint8_t success_msg[150];
    sprintf((char*)success_msg, "=== Decompression Complete ===\r\nOutput: %ld bytes\r\nCoord blocks: %ld, Total coords: %ld\r\n", 
            total_written, coord_blocks, total_coords);
    HAL_UART_Transmit(&huart1, success_msg, strlen((char*)success_msg), 100);
    
    result = 0;
    
cleanup:
    if (read_buffer) free(read_buffer);
    if (write_buffer) free(write_buffer);
    f_close(&read_file);
    
    return result;
}

