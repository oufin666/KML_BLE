#include "kml_utils.h"
#include "stm32f4xx_hal.h"
#include "ff.h"
#include "fatfs.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// 外部变量声明
extern FIL SDFile;  // SDFile在fatfs.c中定义
extern FATFS SDFatFS;  // FatFS文件系统对象

// 外部数据缓冲区变量声明（在usart.c中定义）
extern volatile uint8_t transfer_complete;
extern volatile uint8_t app_buffers[2][4096];
extern volatile uint32_t app_buffer_index[2];
extern volatile uint8_t app_buffer_full[2];

// 外部函数声明
extern void UART1_Start_DMA_Reception(void);
extern void Process_DMA_Data(void);

// 定义缓冲区大小常量
#define READ_CHUNK_SIZE 4096
#define WRITE_CHUNK_SIZE 8192

// 动态生成CRC32表（与小程序端相同的算法）
static uint32_t crc_table[256];

// 初始化CRC32表
void init_crc_table(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (uint32_t j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
        }
        crc_table[i] = crc;
    }
}

/**
 * 计算CRC32值（与小程序端一致）
 *
 * @param data 数据缓冲区
 * @param length 数据长度
 * @return CRC32值
 */
uint32_t calculate_crc32(const uint8_t* data, uint32_t length) {
    uint32_t crc = 0xFFFFFFFF;
    
    for (uint32_t i = 0; i < length; i++) {
        crc = (crc >> 8) ^ crc_table[(crc & 0xFF) ^ data[i]];
    }
    
    return crc ^ 0xFFFFFFFF;
}

// 兼容旧的函数名
uint32_t calculateCRC32(uint8_t* data, uint32_t length) {
    #if KML_ENABLE_CRC
    return calculate_crc32(data, length);
    #else
    // CRC校验关闭，返回0
    return 0;
    #endif
}

/**
 * 读取uint32（小端序）
 */
static uint32_t read_uint32_le(const uint8_t* data) {
    return (uint32_t)(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

/**
 * 写入uint32（小端序）
 */
static void write_uint32_le(uint8_t* data, uint32_t value) {
    data[0] = value & 0xFF;
    data[1] = (value >> 8) & 0xFF;
    data[2] = (value >> 16) & 0xFF;
    data[3] = (value >> 24) & 0xFF;
}

/**
 * 读取int32（小端序）
 */
static int32_t read_int32_le(const uint8_t* data) {
    return (int32_t)(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

/**
 * 写入int32（小端序）
 */
static void write_int32_le(uint8_t* data, int32_t value) {
    data[0] = value & 0xFF;
    data[1] = (value >> 8) & 0xFF;
    data[2] = (value >> 16) & 0xFF;
    data[3] = (value >> 24) & 0xFF;
}

/**
 * 解码差分值（按照新的压缩算法）
 * @param bytes 编码的字节数组
 * @param bytes_consumed 输出：消耗的字节数
 * @return 解码后的差值
 */
int32_t decode_delta(const uint8_t* bytes, int* bytes_consumed) {
    int32_t result = 0;
    uint8_t sign = 0;
    int shift = 0;
    int i = 0;
    
    while (1) {
        uint8_t byte = bytes[i];
        
        // 第一个字节：提取符号位
        if (i == 0) {
            sign = (byte & 0x40) >> 6;
        }
        
        // 提取数据位
        uint8_t data_bits = (i == 0) ? (byte & 0x3F) : (byte & 0x7F);
        
        // 累加数据（低位优先）
        result |= ((int32_t)data_bits << shift);
        shift += (i == 0) ? 6 : 7;
        
        i++;
        
        // 检查继续位
        if ((byte & 0x80) == 0) break;
        
        // 防止无限循环
        if (i > 5) break;
    }
    
    *bytes_consumed = i;
    
    // 应用符号位
    return sign ? -result : result;
}

/**
 * 从文件读取指定字节数
 * @return 0=成功, -1=失败
 */
static int read_bytes(FIL* file, uint8_t* buffer, uint32_t len) {
    UINT bytes_read;
    FRESULT res = f_read(file, buffer, len, &bytes_read);
    if (res != FR_OK || bytes_read != len) {
        return -1;
    }
    return 0;
}

/**
 * 写入数据到文件（带缓冲）
 */
static int write_buffered(FIL* file, uint8_t* buffer, uint32_t* pos, 
                          const void* data, uint32_t len) {
    const uint8_t* src = (const uint8_t*)data;
    
    for (uint32_t i = 0; i < len; i++) {
        buffer[(*pos)++] = src[i];
        
        // 缓冲区满，写入文件
        if (*pos >= WRITE_CHUNK_SIZE) {
            UINT bw;
            FRESULT res = f_write(file, buffer, *pos, &bw);
            if (res != FR_OK || bw != *pos) {
                return -1;
            }
            *pos = 0;
        }
    }
    
    return 0;
}

/**
 * 刷新写入缓冲区
 */
static int flush_buffer(FIL* file, uint8_t* buffer, uint32_t* pos) {
    if (*pos > 0) {
        UINT bw;
        FRESULT res = f_write(file, buffer, *pos, &bw);
        if (res != FR_OK || bw != *pos) {
            return -1;
        }
        *pos = 0;
    }
    return 0;
  }
  
/**
 * 流式差分解压函数（按照新的压缩算法）
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
    sprintf((char*)size_msg, "=== Delta Decompression Start ===\r\nCompressed: %lu bytes\r\n", file_size);
    HAL_UART_Transmit(&huart1, size_msg, strlen((char*)size_msg), 100);
      
    // 分配缓冲区
    read_buffer = (uint8_t*)malloc(READ_CHUNK_SIZE);
    write_buffer = (uint8_t*)malloc(WRITE_CHUNK_SIZE);
    
    if (!read_buffer || !write_buffer) {
        uint8_t msg[] = "Failed to allocate memory\r\n";
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
        goto cleanup;
    }
    
    // 打开输出文件
    res = f_open(&write_file, decompressed_file, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        uint8_t msg[80];
        sprintf((char*)msg, "Failed to create output file: %d\r\n", res);
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
        goto cleanup;
    }
    
    // 读取总段数
    uint8_t header[4];
    if (read_bytes(&read_file, header, 4) != 0) {
        uint8_t msg[] = "Failed to read header\r\n";
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
        f_close(&write_file);
        goto cleanup;
    }
    
    uint32_t total_segments = read_uint32_le(header);
    uint8_t seg_msg[80];
    sprintf((char*)seg_msg, "Total segments: %lu\r\n", total_segments);
    HAL_UART_Transmit(&huart1, seg_msg, strlen((char*)seg_msg), 100);
    
    // 写入总段数
    uint32_t write_pos = 0;
    if (write_buffered(&write_file, write_buffer, &write_pos, header, 4) != 0) {
        uint8_t msg[] = "Failed to write header\r\n";
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
        f_close(&write_file);
        goto cleanup;
    }
    
    uint32_t total_points = 0;
    
    // 处理每个段
    for (uint32_t seg = 0; seg < total_segments; seg++) {
        // 读取段点数
        uint8_t count_bytes[4];
        if (read_bytes(&read_file, count_bytes, 4) != 0) {
            uint8_t msg[80];
            sprintf((char*)msg, "Failed to read segment %lu point count\r\n", seg + 1);
            HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
            f_close(&write_file);
            goto cleanup;
        }
        
        uint32_t point_count = read_uint32_le(count_bytes);
        total_points += point_count;
        
        uint8_t point_msg[80];
        sprintf((char*)point_msg, "Segment %lu: %lu points\r\n", seg + 1, point_count);
        HAL_UART_Transmit(&huart1, point_msg, strlen((char*)point_msg), 100);
        
        // 写入段点数
        if (write_buffered(&write_file, write_buffer, &write_pos, count_bytes, 4) != 0) {
            uint8_t msg[] = "Failed to write segment point count\r\n";
            HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
            f_close(&write_file);
            goto cleanup;
        }
        
        if (point_count == 0) continue;
        
        // 读取第一个点（完整坐标12字节）
        uint8_t first_point[12];
        if (read_bytes(&read_file, first_point, 12) != 0) {
            uint8_t msg[] = "Failed to read first point\r\n";
            HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
            f_close(&write_file);
            goto cleanup;
        }
        
        int32_t prev_lon = read_int32_le(first_point);
        int32_t prev_lat = read_int32_le(first_point + 4);
        int32_t prev_alt = read_int32_le(first_point + 8);
        
        // 调试：打印第一个点
        if (seg == 0) {
            uint8_t coord_msg[150];
            sprintf((char*)coord_msg, "First point: lon=%ld (%.6f), lat=%ld (%.6f), alt=%ld\r\n", 
                    prev_lon, prev_lon/1000000.0f, prev_lat, prev_lat/1000000.0f, prev_alt);
            HAL_UART_Transmit(&huart1, coord_msg, strlen((char*)coord_msg), 100);
        }
        
        // 写入第一个点
        if (write_buffered(&write_file, write_buffer, &write_pos, first_point, 12) != 0) {
            uint8_t msg[] = "Failed to write first point\r\n";
            HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
            f_close(&write_file);
            goto cleanup;
        }
        
        // 解压后续点（流式处理）
        for (uint32_t i = 1; i < point_count; i++) {
            uint8_t delta_buffer[6];  // 单个差值最多5字节
            int bytes_consumed;
            
            // 读取并解码经度差值
            int lon_bytes = 0;
            do {
                if (read_bytes(&read_file, &delta_buffer[lon_bytes], 1) != 0) {
                    uint8_t msg[80];
                    sprintf((char*)msg, "Failed to read point %lu lon delta\r\n", i + 1);
                    HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
                    f_close(&write_file);
                    goto cleanup;
                }
                lon_bytes++;
            } while ((delta_buffer[lon_bytes - 1] & 0x80) && lon_bytes < 6);
            
            int32_t delta_lon = decode_delta(delta_buffer, &bytes_consumed);
            
            // 读取并解码纬度差值
            int lat_bytes = 0;
            do {
                if (read_bytes(&read_file, &delta_buffer[lat_bytes], 1) != 0) {
                    uint8_t msg[80];
                    sprintf((char*)msg, "Failed to read point %lu lat delta\r\n", i + 1);
                    HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
                    f_close(&write_file);
                    goto cleanup;
                }
                lat_bytes++;
            } while ((delta_buffer[lat_bytes - 1] & 0x80) && lat_bytes < 6);
            
            int32_t delta_lat = decode_delta(delta_buffer, &bytes_consumed);
            
            // 读取并解码海拔差值
            int alt_bytes = 0;
            do {
                if (read_bytes(&read_file, &delta_buffer[alt_bytes], 1) != 0) {
                    uint8_t msg[80];
                    sprintf((char*)msg, "Failed to read point %lu alt delta\r\n", i + 1);
                    HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
                    f_close(&write_file);
                    goto cleanup;
                }
                alt_bytes++;
            } while ((delta_buffer[alt_bytes - 1] & 0x80) && alt_bytes < 6);
            
            int32_t delta_alt = decode_delta(delta_buffer, &bytes_consumed);
            
            // 计算当前点坐标（累加差值）
            int32_t curr_lon = prev_lon + delta_lon;
            int32_t curr_lat = prev_lat + delta_lat;
            int32_t curr_alt = prev_alt + delta_alt;
            
            // 写入当前点
            uint8_t point_data[12];
            write_int32_le(point_data, curr_lon);
            write_int32_le(point_data + 4, curr_lat);
            write_int32_le(point_data + 8, curr_alt);
            
            if (write_buffered(&write_file, write_buffer, &write_pos, point_data, 12) != 0) {
                uint8_t msg[80];
                sprintf((char*)msg, "Failed to write point %lu\r\n", i + 1);
                HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
                f_close(&write_file);
                goto cleanup;
            }
            
            // 更新前一个点（用于下一次差分计算）
            prev_lon = curr_lon;
            prev_lat = curr_lat;
            prev_alt = curr_alt;
        }
    }
    
    // 刷新写入缓冲区
    if (flush_buffer(&write_file, write_buffer, &write_pos) != 0) {
        uint8_t msg[] = "Failed to flush buffer\r\n";
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
        f_close(&write_file);
        goto cleanup;
    }
    
    // 同步并关闭输出文件
    f_sync(&write_file);
    uint32_t output_size = f_tell(&write_file);
    f_close(&write_file);
    
    uint8_t success_msg[150];
    sprintf((char*)success_msg, "=== Decompression Complete ===\r\nOutput: %lu bytes\r\nTotal points: %lu\r\nCompression ratio: %.1f%%\r\n", 
            output_size, total_points, (float)(file_size * 100) / output_size);
    HAL_UART_Transmit(&huart1, success_msg, strlen((char*)success_msg), 100);
    
    result = 0;
    
cleanup:
    if (read_buffer) free(read_buffer);
    if (write_buffer) free(write_buffer);
    f_close(&read_file);
    
    return result;
}

/**
 * @brief 打开压缩文件并准备接收数据
 * @param compressed_file 压缩文件路径
 * @param sd_file_opened 文件打开标志指针
 * @param kml_transfer_active 传输激活标志指针
 * @return FRESULT 文件系统操作结果
 */
FRESULT kml_open_compressed_file(const char* compressed_file, volatile uint8_t* sd_file_opened, volatile uint8_t* kml_transfer_active) {
    FRESULT res;
    
    // 重置缓冲区，确保开始接收新文件时缓冲区为空
    for (int i = 0; i < 2; i++) {
        app_buffer_full[i] = 0;
        app_buffer_index[i] = 0;
    }
    transfer_complete = 0;
    
    // 打开压缩文件准备接收数据
    uint8_t open_msg2[150];
    sprintf((char*)open_msg2, "Opening %s for compressed data reception...\r\n", compressed_file);
    HAL_UART_Transmit(&huart1, open_msg2, strlen((char*)open_msg2), 100);
    
    res = f_open(&SDFile, compressed_file, FA_CREATE_ALWAYS | FA_WRITE);
    uint8_t open_result_msg[100];
    sprintf((char*)open_result_msg, "f_open result: %d\r\n", res);
    HAL_UART_Transmit(&huart1, open_result_msg, strlen((char*)open_result_msg), 100);
    
    if (res == FR_OK) {
        *sd_file_opened = 1;
        *kml_transfer_active = 1;
        
        uint8_t msg2[150];
        sprintf((char*)msg2, "\r\n=== Ready to receive KML file ===\r\nFile: %s\r\nsd_file_opened=%d, kml_transfer_active=%d\r\n", 
                compressed_file, *sd_file_opened, *kml_transfer_active);
        HAL_UART_Transmit(&huart1, msg2, strlen((char*)msg2), 100);
        uint8_t msg3[] = "Waiting for KML file data...\r\nEnd with 0xFF 0xFF 0xFF to complete transfer\r\n\r\n";
        HAL_UART_Transmit(&huart1, msg3, strlen((char*)msg3), 100);
        
        // 启动DMA接收
        UART1_Start_DMA_Reception();
    } else {
        uint8_t msg[] = "Cannot create file for compressed data reception\r\n";
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
    }
    
    return res;
}

/**
 * @brief 处理数据缓冲区
 * @param sd_file_opened 文件打开标志
 * @param total_written 总写入字节数指针
 * @param last_print_index 上次打印索引指针
 * @return FRESULT 文件系统操作结果
 */
FRESULT kml_process_buffers(volatile uint8_t sd_file_opened, uint32_t* total_written, uint32_t* last_print_index) {
    FRESULT res = FR_OK;
    
    // 处理DMA接收的数据
    Process_DMA_Data();
    
    // 检查并处理满的缓冲区
    for (int i = 0; i < 2; i++) {
        if (app_buffer_full[i] && sd_file_opened) {
            // 缓冲区满，写入SD卡
            uint32_t bytes_to_write = app_buffer_index[i];
            if (bytes_to_write > 0) {
                UINT bytes_written;
                res = kml_write_bin_to_sd(&SDFile, (uint8_t*)app_buffers[i], bytes_to_write, &bytes_written);
                if(res == FR_OK) {
                    *total_written += bytes_written;
                    // 定期打印写入状态，避免过多调试信息
                    if(*total_written - *last_print_index > 409600) {  // 每400KB打印一次，减少干扰
                        *last_print_index = *total_written;
                    }
                    // 重置缓冲区状态
                    app_buffer_full[i] = 0;
                    app_buffer_index[i] = 0;
                } else {
                    // 写入失败，打印详细错误信息
                    uint8_t err_msg[100];
                    sprintf((char*)err_msg, "Compressed data Write error: %d\r\n", res);
                    HAL_UART_Transmit(&huart1, err_msg, strlen((char*)err_msg), 100);
                }
            }
        }
    }
    
    return res;
}

/**
 * @brief 处理传输完成
 * @param compressed_file 压缩文件路径
 * @param actual_written 实际写入字节数
 * @param sd_file_opened 文件打开标志指针
 * @param kml_transfer_active 传输激活标志指针
 * @return FRESULT 文件系统操作结果
 */
FRESULT kml_handle_transfer_complete(const char* compressed_file, uint32_t actual_written, volatile uint8_t* sd_file_opened, volatile uint8_t* kml_transfer_active) {
    FRESULT res;
    
    // 确保所有满的缓冲区都写入了
    for (int i = 0; i < 2; i++) {
        if (app_buffer_full[i]) {
            uint32_t bytes_to_write = app_buffer_index[i];
            if (bytes_to_write > 0) {
                UINT bytes_written;
                res = f_write(&SDFile, (uint8_t*)app_buffers[i], bytes_to_write, &bytes_written);
                if(res == FR_OK) {
                    // 重置缓冲区状态
                    app_buffer_full[i] = 0;
                    app_buffer_index[i] = 0;
                } else {
                    // 最终写入失败
                    uint8_t final_err_msg[100];
                    sprintf((char*)final_err_msg, "Final Compressed Write error: %d\r\n", res);
                    HAL_UART_Transmit(&huart1, final_err_msg, strlen((char*)final_err_msg), 100);
                    break;
                }
            }
        }
    }
    
    // 打印最终写入状态
    uint8_t final_write_msg[100];
    sprintf((char*)final_write_msg, "Final Compressed Write: Transfer complete, Total %ld bytes written\r\n", actual_written);
    HAL_UART_Transmit(&huart1, final_write_msg, strlen((char*)final_write_msg), 100);
    
    // 同步并关闭压缩文件
    f_sync(&SDFile);
    f_close(&SDFile);
    *sd_file_opened = 0;
    
    // 打印压缩文件传输完成信息
    uint8_t msg[120];
    sprintf((char*)msg, "=== Compressed File Transfer Complete ===\r\nFile: %s\r\nTotal Written: %ld bytes\r\n", 
            compressed_file, actual_written);
    HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
    
    // 处理完成的传输（包括解压）
    res = kml_process_completed_transfer(compressed_file, actual_written);
    if (res != FR_OK) {
        uint8_t err_msg[100];
        sprintf((char*)err_msg, "Error processing transfer: %d\r\n", res);
        HAL_UART_Transmit(&huart1, err_msg, strlen((char*)err_msg), 100);
    }
    
    // 重置缓冲区状态
    for (int i = 0; i < 2; i++) {
        app_buffer_full[i] = 0;
        app_buffer_index[i] = 0;
    }
    transfer_complete = 0;
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 不重新打开压缩文件，保留之前的文件供后续检查
    *sd_file_opened = 0;
    uint8_t ready_msg[150];
    sprintf((char*)ready_msg, "Ready for next transfer\r\nPrevious compressed file saved as: %s\r\n", compressed_file);
    HAL_UART_Transmit(&huart1, ready_msg, strlen((char*)ready_msg), 100);
    *kml_transfer_active = 1;
    
    uint8_t msg3[] = "\r\nReady for next transfer\r\n";
    HAL_UART_Transmit(&huart1, msg3, strlen((char*)msg3), 100);
    
    return res;
}

/**
 * @brief 初始化KML系统
 * @return FRESULT 文件系统操作结果
 */
FRESULT kml_init_system(void)
{
    FRESULT res;
    const char* kml_dir = "0:/kml";
    
    // 初始化CRC32表（如果开启CRC校验）
    #if KML_ENABLE_CRC
    init_crc_table();
    #endif
    
    // 尝试挂载文件系统
    res = f_mount(&SDFatFS, "0:", 1);
    if (res != FR_OK) {
        return res;
    }
    
    // 创建kml目录
    res = f_mkdir(kml_dir);
    if (res != FR_OK && res != FR_EXIST) {
        return res;
    }
    
    return FR_OK;
}

/**
 * @brief 写入二进制数据到SD卡
 * @param file 文件指针
 * @param buffer 数据缓冲区
 * @param buffer_size 缓冲区大小
 * @param bytes_written 实际写入字节数
 * @return FRESULT 文件系统操作结果
 */
FRESULT kml_write_bin_to_sd(FIL *file, uint8_t *buffer, uint32_t buffer_size, UINT *bytes_written)
{
    if (!file || !buffer || buffer_size == 0) {
        return FR_INVALID_PARAMETER;
    }
    
    return f_write(file, buffer, buffer_size, bytes_written);
}

/**
 * @brief 处理完成的传输
 * @param compressed_file 压缩文件路径
 * @param total_written 总写入字节数
 * @return FRESULT 文件系统操作结果
 */
FRESULT kml_process_completed_transfer(const char *compressed_file, uint32_t total_written)
{
    FRESULT res = FR_OK;
    
    // 解压文件（如果开启解压缩）
    #if KML_ENABLE_DECOMPRESS
    // 使用新的差分压缩解压算法
    char decompressed_file[] = "0:/kml/decompressed_kml.bin";
    
    uint8_t decompress_start_msg[100];
    sprintf((char*)decompress_start_msg, "Starting delta decompression: %s -> %s\r\n", compressed_file, decompressed_file);
    HAL_UART_Transmit(&huart1, decompress_start_msg, strlen((char*)decompress_start_msg), 100);
    
    // 调用新的差分解压函数
    int result = stream_decompress_file(compressed_file, decompressed_file);
    
    if (result == 0) {
        uint8_t success_msg[] = "Delta decompression successful!\r\n";
        HAL_UART_Transmit(&huart1, success_msg, strlen((char*)success_msg), 100);
        res = FR_OK;
    } else {
        uint8_t error_msg[80];
        sprintf((char*)error_msg, "Delta decompression failed, error code: %d\r\n", result);
        HAL_UART_Transmit(&huart1, error_msg, strlen((char*)error_msg), 100);
        res = FR_INT_ERR;
    }
    #else
    // Decompression disabled, skip
    uint8_t msg[] = "Decompression disabled, skipping...\r\n";
    HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
    #endif
    
    return res;
}

/**
 * @brief 打开JSON文件并准备接收数据（JSON模式）
 * @param json_file JSON文件路径
 * @param sd_file_opened 文件打开状态
 * @param kml_transfer_active 传输激活状态
 * @return FRESULT 文件系统操作结果
 */
FRESULT kml_open_json_file(const char* json_file, volatile uint8_t* sd_file_opened, volatile uint8_t* kml_transfer_active) {
    FRESULT res;
    
    // 重置缓冲区，确保开始接收新文件时缓冲区为空
    for (int i = 0; i < 2; i++) {
        app_buffer_full[i] = 0;
        app_buffer_index[i] = 0;
    }
    transfer_complete = 0;
    
    // 打开JSON文件准备接收数据
    uint8_t open_msg[150];
    sprintf((char*)open_msg, "Opening %s for JSON data reception...\r\n", json_file);
    HAL_UART_Transmit(&huart1, open_msg, strlen((char*)open_msg), 100);
    
    res = f_open(&SDFile, json_file, FA_CREATE_ALWAYS | FA_WRITE);
    uint8_t open_result_msg[100];
    sprintf((char*)open_result_msg, "f_open result: %d\r\n", res);
    HAL_UART_Transmit(&huart1, open_result_msg, strlen((char*)open_result_msg), 100);
    
    if (res == FR_OK) {
        *sd_file_opened = 1;
        *kml_transfer_active = 1;
        
        uint8_t msg[150];
        sprintf((char*)msg, "\r\n=== Ready to receive JSON file ===\r\nFile: %s\r\nsd_file_opened=%d, kml_transfer_active=%d\r\n", 
                json_file, *sd_file_opened, *kml_transfer_active);
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
        uint8_t msg2[] = "Waiting for JSON data...\r\nEnd with 0xFF 0xFF 0xFF to complete transfer\r\n\r\n";
        HAL_UART_Transmit(&huart1, msg2, strlen((char*)msg2), 100);
        
        // 启动DMA接收
        UART1_Start_DMA_Reception();
    } else {
        uint8_t msg[] = "Cannot create file for JSON data reception\r\n";
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
    }
    
    return res;
}

/**
 * @brief 处理JSON传输完成（JSON模式）
 * @param json_file JSON文件路径
 * @param actual_written 实际写入字节数
 * @param sd_file_opened 文件打开标志指针
 * @param kml_transfer_active 传输激活标志指针
 * @return FRESULT 文件系统操作结果
 */
FRESULT kml_handle_json_transfer_complete(const char* json_file, uint32_t actual_written, volatile uint8_t* sd_file_opened, volatile uint8_t* kml_transfer_active) {
    FRESULT res;
    
    // 确保所有满的缓冲区都写入了
    for (int i = 0; i < 2; i++) {
        if (app_buffer_full[i]) {
            uint32_t bytes_to_write = app_buffer_index[i];
            if (bytes_to_write > 0) {
                UINT bytes_written;
                res = f_write(&SDFile, (uint8_t*)app_buffers[i], bytes_to_write, &bytes_written);
                if(res == FR_OK) {
                    // 重置缓冲区状态
                    app_buffer_full[i] = 0;
                    app_buffer_index[i] = 0;
                } else {
                    // 最终写入失败
                    uint8_t final_err_msg[100];
                    sprintf((char*)final_err_msg, "Final JSON Write error: %d\r\n", res);
                    HAL_UART_Transmit(&huart1, final_err_msg, strlen((char*)final_err_msg), 100);
                    break;
                }
            }
        }
    }
    
    // 打印最终写入状态
    uint8_t final_write_msg[100];
    sprintf((char*)final_write_msg, "Final JSON Write: Transfer complete, Total %ld bytes written\r\n", actual_written);
    HAL_UART_Transmit(&huart1, final_write_msg, strlen((char*)final_write_msg), 100);
    
    // 同步并关闭JSON文件
    f_sync(&SDFile);
    f_close(&SDFile);
    *sd_file_opened = 0;
    
    // 打印JSON文件传输完成信息
    uint8_t msg[120];
    sprintf((char*)msg, "=== JSON File Transfer Complete ===\r\nFile: %s\r\nTotal Written: %ld bytes\r\n", 
            json_file, actual_written);
    HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
    
    // 重置缓冲区状态
    for (int i = 0; i < 2; i++) {
        app_buffer_full[i] = 0;
        app_buffer_index[i] = 0;
    }
    transfer_complete = 0;
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    *sd_file_opened = 0;
    uint8_t ready_msg[150];
    sprintf((char*)ready_msg, "Ready for next transfer\r\nPrevious JSON file saved as: %s\r\n", json_file);
    HAL_UART_Transmit(&huart1, ready_msg, strlen((char*)ready_msg), 100);
    *kml_transfer_active = 1;
    
    uint8_t msg2[] = "\r\nReady for next transfer\r\n";
    HAL_UART_Transmit(&huart1, msg2, strlen((char*)msg2), 100);
    
    return FR_OK;
}

/**
 * @brief BLE KML任务主函数
 * @param compressed_file 压缩文件路径（或JSON文件路径，取决于模式）
 * @param sd_file_opened 文件打开标志指针
 * @param kml_transfer_active 传输激活标志指针
 * @param transfer_complete 传输完成标志指针
 * @note 功能：根据KML_ENABLE_JSON_MODE宏定义选择接收模式
 *       - JSON模式：直接保存JSON字符串为.json文件
 *       - BIN模式：接收压缩bin文件并进行解压处理
 */
void kml_ble_task_main(const char* compressed_file, volatile uint8_t* sd_file_opened, volatile uint8_t* kml_transfer_active, volatile uint8_t* transfer_complete) {
    FRESULT res;
    
    // 延时等待系统初始化完成（SD卡已在main中初始化）
    uint8_t wait_msg[] = "Waiting for SD init...\r\n";
    HAL_UART_Transmit(&huart1, wait_msg, strlen((char*)wait_msg), 100);
    vTaskDelay(pdMS_TO_TICKS(500));  // 缩短延迟时间

    // 发送启动消息
    #if KML_ENABLE_JSON_MODE
    uint8_t start_msg[] = "\r\n=== BLE KML Task Started (JSON Mode) ===\r\n";
    #else
    uint8_t start_msg[] = "\r\n=== BLE KML Task Started (BIN Mode) ===\r\n";
    #endif
    HAL_UART_Transmit(&huart1, start_msg, strlen((char*)start_msg), 100);
    
    // 初始化系统
    res = kml_init_system();
    if (res != FR_OK) {
        uint8_t msg[] = "SD mount FAILED! Cannot continue.\r\n";
        HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
        // 进入死循环
        for(;;) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }
    
    uint8_t msg1[] = "SD mounted OK\r\n";
    HAL_UART_Transmit(&huart1, msg1, strlen((char*)msg1), 100);

    // 主循环
    for (;;) {
        // 检查是否需要打开文件
        if (!*sd_file_opened && !*kml_transfer_active) {
            #if KML_ENABLE_JSON_MODE
            // JSON模式：打开JSON文件
            res = kml_open_json_file(compressed_file, sd_file_opened, kml_transfer_active);
            #else
            // BIN模式：打开压缩文件
            res = kml_open_compressed_file(compressed_file, sd_file_opened, kml_transfer_active);
            #endif
            if (res != FR_OK) {
                vTaskDelay(pdMS_TO_TICKS(1000)); // 延迟后重试
            }
        }
        
        // 处理数据缓冲区
        static uint32_t total_written = 0;
        static uint32_t last_print_index = 0;
        if (*kml_transfer_active) {
            kml_process_buffers(*sd_file_opened, &total_written, &last_print_index);
        }
        
        // 检查传输是否完成
        if(*transfer_complete && *sd_file_opened){
            // 保存总写入字节数
            uint32_t actual_written = total_written;
            
            #if KML_ENABLE_JSON_MODE
            // JSON模式：处理JSON传输完成
            kml_handle_json_transfer_complete(compressed_file, actual_written, sd_file_opened, kml_transfer_active);
            #else
            // BIN模式：处理压缩文件传输完成
            kml_handle_transfer_complete(compressed_file, actual_written, sd_file_opened, kml_transfer_active);
            #endif
            
            // 重置总写入计数
            total_written = 0;
            last_print_index = 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}