#include "kml_utils.h"
#include "stm32f4xx_hal.h"
#include "ff.h"
#include "fatfs.h"
#include <string.h>
#include <stdio.h>

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
#define READ_BUF_SIZE 8192
#define OUT_BUF_SIZE 4096

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

// 标签映射表（与小程序端相同）
static const char* tag_map[] = {
    NULL,                           // 0x80 未使用
    "<kml xmlns=\"http://www.opengis.net/kml/2.2\">",  // 0x81
    "</kml>",                       // 0x82
    "<Placemark>",                  // 0x83
    "</Placemark>",                 // 0x84
    "<name>",                       // 0x85
    "</name>",                      // 0x86
    "<description>",                // 0x87
    "</description>",               // 0x88
    "<Point>",                      // 0x89
    "</Point>",                     // 0x8A
    "<coordinates>",                // 0x8B
    "</coordinates>",               // 0x8C
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>",  // 0x8D
    "\r\n",                        // 0x8E
    "\n"                           // 0x8F
};

// 解压函数（基于坐标数据压缩算法）
// 与小程序端的压缩算法完全兼容
uint32_t decompressData(uint8_t* compressedData, uint32_t compressedSize, uint8_t* outputBuffer, uint32_t outputBufferSize) {
  if (compressedSize < 2 || outputBufferSize == 0) {
    return 0;
  }
  
  uint32_t inPos = 0;
  uint32_t outPos = 0;
  
  // 检查压缩标识
  if (compressedData[0] != 0x42 || compressedData[1] != 0x4C) {
    return 0; // 无效的压缩标识
  }
  
  // 跳过压缩标识
  inPos = 2;
  
  while (inPos < compressedSize && outPos < outputBufferSize) {
    uint8_t byte = compressedData[inPos++];
    
    if (byte == 0xfe) {
      // 处理坐标数据
      if (inPos + 2 > compressedSize) {
        return 0; // 数据长度不足
      }
      
      // 读取坐标数量（小端序）
      int coord_count = compressedData[inPos] | (compressedData[inPos + 1] << 8);
      inPos += 2;
      
      // 写入<coordinates>标签
      const char* start_tag = "<coordinates>";
      int start_tag_len = strlen(start_tag);
      if (outPos + start_tag_len > outputBufferSize) {
        return 0; // 解压缓冲区不足
      }
      memcpy(&outputBuffer[outPos], start_tag, start_tag_len);
      outPos += start_tag_len;
      
      // 处理每个坐标
      for (int i = 0; i < coord_count; i++) {
        // 读取坐标数据（12字节）
        if (inPos + 12 > compressedSize) {
          return 0; // 数据长度不足
        }
        
        // 解析坐标数据
        int32_t lon = (int32_t)(compressedData[inPos] | (compressedData[inPos+1]<<8) | (compressedData[inPos+2]<<16) | (compressedData[inPos+3]<<24));
        int32_t lat = (int32_t)(compressedData[inPos+4] | (compressedData[inPos+5]<<8) | (compressedData[inPos+6]<<16) | (compressedData[inPos+7]<<24));
        int32_t alt = (int32_t)(compressedData[inPos+8] | (compressedData[inPos+9]<<8) | (compressedData[inPos+10]<<16) | (compressedData[inPos+11]<<24));
        inPos += 12;
        
        // 转换为浮点数
        double lon_d = lon / 1000000.0;
        double lat_d = lat / 1000000.0;
        double alt_d = alt / 1000000.0;
        
        // 格式化坐标字符串
        char str[96];
        int len = snprintf(str, sizeof(str), "%.6f,%.6f,%.6f", lon_d, lat_d, alt_d);
        
        // 去除尾部多余的0，但至少保留一位小数
        char* dot = strchr(str, '.');
        if (dot) {
          // 从末尾开始查找非零字符
          char* end = str + len - 1;
          while (end > dot && *end == '0') {
            end--;
          }
          // 如果末尾是小数点，需要保留一个0
          if (end == dot) {
            end++;
            *end = '0';  // 写入'0'
          }
          // 截断字符串并添加空格分隔符
          end++;
          *end = ' ';
          len = end - str + 1; // 包含空格
        }
        
        // 写入坐标数据
        if (outPos + len > outputBufferSize) {
          return 0; // 解压缓冲区不足
        }
        memcpy(&outputBuffer[outPos], str, len);
        outPos += len;
      }
      
      // 写入</coordinates>标签
      const char* end_tag = "</coordinates>";
      int end_tag_len = strlen(end_tag);
      if (outPos + end_tag_len > outputBufferSize) {
        return 0; // 解压缓冲区不足
      }
      memcpy(&outputBuffer[outPos], end_tag, end_tag_len);
      outPos += end_tag_len;
      
    } else if (byte < 0x80) {
      // ASCII字符
      outputBuffer[outPos++] = byte;
      
    } else if (byte >= 0x80 && byte <= 0x8F) {
      // 标签映射
      int tag_index = byte - 0x80;
      if (tag_index < sizeof(tag_map)/sizeof(tag_map[0]) && tag_map[tag_index]) {
        const char* tag = tag_map[tag_index];
        int tag_len = strlen(tag);
        if (outPos + tag_len > outputBufferSize) {
          return 0; // 解压缓冲区不足
        }
        memcpy(&outputBuffer[outPos], tag, tag_len);
        outPos += tag_len;
      }
      
    } else if (byte >= 0xC0 && byte < 0xE0) {
      // 双字节UTF-8字符
      if (inPos + 1 > compressedSize) {
        return 0; // 数据长度不足
      }
      if (outPos + 2 > outputBufferSize) {
        return 0; // 解压缓冲区不足
      }
      outputBuffer[outPos++] = byte;
      outputBuffer[outPos++] = compressedData[inPos++];
      
    } else if (byte >= 0xE0 && byte < 0xF0) {
      // 三字节UTF-8字符
      if (inPos + 2 > compressedSize) {
        return 0; // 数据长度不足
      }
      if (outPos + 3 > outputBufferSize) {
        return 0; // 解压缓冲区不足
      }
      outputBuffer[outPos++] = byte;
      outputBuffer[outPos++] = compressedData[inPos++];
      outputBuffer[outPos++] = compressedData[inPos++];
      
    } else if (byte >= 0xF0 && byte < 0xF8) {
      // 四字节UTF-8字符
      if (inPos + 3 > compressedSize) {
        return 0; // 数据长度不足
      }
      if (outPos + 4 > outputBufferSize) {
        return 0; // 解压缓冲区不足
      }
      outputBuffer[outPos++] = byte;
      outputBuffer[outPos++] = compressedData[inPos++];
      outputBuffer[outPos++] = compressedData[inPos++];
      outputBuffer[outPos++] = compressedData[inPos++];
    }
  }
  
  return outPos;
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
 * @brief 修复KML文件中的错误
 * @param decompressed_file 解压后的文件路径
 * @return FRESULT 文件系统操作结果
 */
static FRESULT kml_fix_file_errors(const char *decompressed_file)
{
    FRESULT res;
    FIL decompressed_fd;
    
    // 重新打开文件进行修复
    res = f_open(&decompressed_fd, decompressed_file, FA_READ | FA_WRITE);
    if (res != FR_OK) {
        return res;
    }
    
    uint32_t file_size = f_size(&decompressed_fd);
    uint8_t* fix_buf = (uint8_t*)malloc(8192);
    if (!fix_buf) {
        f_close(&decompressed_fd);
        return FR_NOT_ENOUGH_CORE;
    }
    
    uint32_t pos = 0;
    uint32_t fixed_count = 0;
    
    while (pos < file_size) {
        // 读取一块数据
        f_lseek(&decompressed_fd, pos);
        UINT br;
        uint32_t to_read = (file_size - pos) > 8192 ? 8192 : (file_size - pos);
        res = f_read(&decompressed_fd, fix_buf, to_read, &br);
        if (res != FR_OK || br == 0) {
            break;
        }
        
        // 查找并修复 "</Datta>", "<Datta ", "/Data>", "valulue" 等错误
        for (uint32_t i = 0; i < br - 9; i++) {
            // 检查 "</Datta>"
            if (fix_buf[i] == '<' && fix_buf[i+1] == '/' && 
                fix_buf[i+2] == 'D' && fix_buf[i+3] == 'a' && 
                fix_buf[i+4] == 't' && fix_buf[i+5] == 't' && 
                fix_buf[i+6] == 'a' && fix_buf[i+7] == '>') {
                // 修复：移除多余的 't'
                fix_buf[i+5] = 'a';
                fix_buf[i+6] = '>';
                fix_buf[i+7] = ' ';  // 用空格填充
                
                // 写回文件
                f_lseek(&decompressed_fd, pos + i);
                UINT bw;
                f_write(&decompressed_fd, &fix_buf[i], 8, &bw);
                fixed_count++;
            }
            // 检查 "<Datta " (开始标签)
            else if (fix_buf[i] == '<' && 
                     fix_buf[i+1] == 'D' && fix_buf[i+2] == 'a' && 
                     fix_buf[i+3] == 't' && fix_buf[i+4] == 't' && 
                     fix_buf[i+5] == 'a' && fix_buf[i+6] == ' ') {
                // 修复：移除多余的 't'
                fix_buf[i+4] = 'a';
                fix_buf[i+5] = ' ';
                fix_buf[i+6] = ' ';  // 用空格填充
                
                // 写回文件
                f_lseek(&decompressed_fd, pos + i);
                UINT bw;
                f_write(&decompressed_fd, &fix_buf[i], 7, &bw);
                fixed_count++;
            }
            // 检查 "/Data>" (缺少开头的 '<')
            else if (i > 0 && fix_buf[i] == '/' && 
                     fix_buf[i+1] == 'D' && fix_buf[i+2] == 'a' && 
                     fix_buf[i+3] == 't' && fix_buf[i+4] == 'a' && 
                     fix_buf[i+5] == '>') {
                // 检查前一个字符
                if (fix_buf[i-1] != '<') {
                    // 如果前一个字符是空格或换行，可以替换
                    if (fix_buf[i-1] == ' ' || fix_buf[i-1] == '\n' || fix_buf[i-1] == '\r' || fix_buf[i-1] == '\t') {
                        fix_buf[i-1] = '<';
                        
                        // 写回文件
                        f_lseek(&decompressed_fd, pos + i - 1);
                        UINT bw;
                        f_write(&decompressed_fd, &fix_buf[i-1], 6, &bw);
                        fixed_count++;
                    }
                }
            }
            // 检查 "<valulue>" 或 "</valulue>"
            else if ((fix_buf[i] == '<' && fix_buf[i+1] == 'v') || 
                     (fix_buf[i] == '<' && fix_buf[i+1] == '/' && fix_buf[i+2] == 'v')) {
                uint32_t offset = (fix_buf[i+1] == '/') ? 2 : 1;
                // 检查是否是 "valulue"
                if (fix_buf[i+offset] == 'v' && fix_buf[i+offset+1] == 'a' && 
                    fix_buf[i+offset+2] == 'l' && fix_buf[i+offset+3] == 'u' && 
                    fix_buf[i+offset+4] == 'l' && fix_buf[i+offset+5] == 'u' && 
                    fix_buf[i+offset+6] == 'e' && fix_buf[i+offset+7] == '>') {
                    // 修复：移除多余的 'u'
                    fix_buf[i+offset+4] = 'e';
                    fix_buf[i+offset+5] = '>';
                    fix_buf[i+offset+6] = ' ';  // 用空格填充
                    fix_buf[i+offset+7] = ' ';  // 用空格填充
                    
                    // 写回文件
                    f_lseek(&decompressed_fd, pos + i);
                    UINT bw;
                    f_write(&decompressed_fd, &fix_buf[i], offset + 8, &bw);
                    fixed_count++;
                }
            }
        }
        
        pos += br;
    }
    
    free(fix_buf);
    f_sync(&decompressed_fd);
    f_close(&decompressed_fd);
    
    return FR_OK;
}

/**
 * @brief 解压KML文件
 * @param compressed_file 压缩文件路径
 * @param decompressed_file 解压文件路径
 * @return FRESULT 文件系统操作结果
 */
FRESULT kml_decompress_file(const char *compressed_file, const char *decompressed_file)
{
    FRESULT res;
    FIL read_file;
    
    // 打开压缩文件
    res = f_open(&read_file, compressed_file, FA_OPEN_EXISTING | FA_READ);
    if (res != FR_OK) {
        return res;
    }
    
    // 获取文件大小
    uint32_t file_size = f_size(&read_file);
    uint32_t data_length = file_size;
    
    // 检查文件末尾是否有结束序列 0xFF 0xFF 0xFF
    if (file_size >= 3) {
        f_lseek(&read_file, file_size - 3);
        uint8_t end_check[3];
        UINT bytes_read;
        f_read(&read_file, end_check, 3, &bytes_read);
        if (bytes_read == 3 && end_check[0] == 0xFF && end_check[1] == 0xFF && end_check[2] == 0xFF) {
            data_length -= 3;
        }
        // 重新定位到文件开头
        f_lseek(&read_file, 0);
    }
    
    // 读取并检查压缩标识
    uint8_t header[2];
    UINT bytes_read;
    res = f_read(&read_file, header, 2, &bytes_read);
    if (res != FR_OK || bytes_read != 2) {
        f_close(&read_file);
        return res;
    }
    
    // 检查 'BL' 标识
    if (header[0] != 0x42 || header[1] != 0x4C) {
        f_close(&read_file);
        return FR_INVALID_PARAMETER;
    }
    
    // 打开解压后的文件
    FIL decompressed_fd;
    res = f_open(&decompressed_fd, decompressed_file, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        f_close(&read_file);
        return res;
    }
    
    // 分配缓冲区
    uint8_t* read_buf = (uint8_t*)malloc(READ_BUF_SIZE);
    uint8_t* out_buf = (uint8_t*)malloc(OUT_BUF_SIZE);
    
    if (!read_buf || !out_buf) {
        if (read_buf) free(read_buf);
        if (out_buf) free(out_buf);
        f_close(&read_file);
        f_close(&decompressed_fd);
        return FR_NOT_ENOUGH_CORE;
    }
    
    // 初始化变量
    uint32_t total_out = 0;
    uint32_t out_pos = 0;
    uint32_t file_pos = 2;  // 跳过'BL'标识
    
    // 读取缓冲区状态
    uint32_t buf_start = 0;  // 缓冲区对应的文件位置
    uint32_t buf_len = 0;    // 缓冲区中的有效数据长度
    uint32_t buf_pos = 0;    // 当前在缓冲区中的位置
    
    // 定位到文件开头+2
    f_lseek(&read_file, 2);
    
    // 主解压循环
    while (file_pos < data_length) {
        // 检查是否需要重新填充读取缓冲区
        if (buf_pos >= buf_len) {
            buf_start = file_pos;
            UINT br;
            uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
            res = f_read(&read_file, read_buf, to_read, &br);
            if (res != FR_OK || br == 0) break;
            buf_len = br;
            buf_pos = 0;
        }
        
        uint8_t byte = read_buf[buf_pos++];
        file_pos++;
        
        if (byte == 0xfe) {
            // 先写出输出缓冲区
            if (out_pos > 0) {
                UINT bw;
                f_write(&decompressed_fd, out_buf, out_pos, &bw);
                total_out += bw;
                out_pos = 0;
            }
            
            // 读取坐标数量（2字节）
            uint8_t count_bytes[2];
            
            // 确保缓冲区中有足够数据读取坐标数量
            if (buf_pos + 2 > buf_len) {
                // 缓冲区数据不足，需要从文件读取
                uint32_t remaining = buf_len - buf_pos;
                if (remaining > 0) {
                    count_bytes[0] = read_buf[buf_pos];
                    if (remaining >= 2) {
                        count_bytes[1] = read_buf[buf_pos + 1];
                        buf_pos += 2;
                        file_pos += 2;
                    } else {
                        // 只有1个字节，需要再读1个
                        buf_pos++;
                        file_pos++;
                        // 重新填充缓冲区
                        f_lseek(&read_file, file_pos);
                        UINT br;
                        uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
                        res = f_read(&read_file, read_buf, to_read, &br);
                        if (res != FR_OK || br < 1) break;
                        buf_start = file_pos;
                        buf_len = br;
                        buf_pos = 0;
                        count_bytes[1] = read_buf[buf_pos++];
                        file_pos++;
                    }
                } else {
                    // 缓冲区已空，重新填充
                    f_lseek(&read_file, file_pos);
                    UINT br;
                    uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
                    res = f_read(&read_file, read_buf, to_read, &br);
                    if (res != FR_OK || br < 2) break;
                    buf_start = file_pos;
                    buf_len = br;
                    buf_pos = 0;
                    count_bytes[0] = read_buf[buf_pos++];
                    count_bytes[1] = read_buf[buf_pos++];
                    file_pos += 2;
                }
            } else {
                // 缓冲区中有足够数据
                count_bytes[0] = read_buf[buf_pos++];
                count_bytes[1] = read_buf[buf_pos++];
                file_pos += 2;
            }
            
            // 解析坐标数量
            uint16_t coord_count = count_bytes[0] | (count_bytes[1] << 8);
            
            // 写<coordinates>
            const char* tag = "<coordinates>";
            UINT bw;
            f_write(&decompressed_fd, (uint8_t*)tag, strlen(tag), &bw);
            total_out += bw;
            
            // 处理每个坐标
            for (uint16_t c = 0; c < coord_count; c++) {
                // 读取坐标数据（12字节）
                uint8_t coord_bytes[12];
                
                // 确保缓冲区中有12字节
                if (buf_pos + 12 > buf_len) {
                    // 缓冲区数据不足
                    uint32_t remaining = buf_len - buf_pos;
                    // 先复制缓冲区中剩余的字节
                    for (uint32_t i = 0; i < remaining; i++) {
                        coord_bytes[i] = read_buf[buf_pos++];
                    }
                    file_pos += remaining;
                    
                    // 重新填充缓冲区
                    f_lseek(&read_file, file_pos);
                    UINT br;
                    uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
                    res = f_read(&read_file, read_buf, to_read, &br);
                    if (res != FR_OK || br < (12 - remaining)) break;
                    buf_start = file_pos;
                    buf_len = br;
                    buf_pos = 0;
                    
                    // 复制剩余的字节
                    for (uint32_t i = remaining; i < 12; i++) {
                        coord_bytes[i] = read_buf[buf_pos++];
                    }
                    file_pos += (12 - remaining);
                } else {
                    // 缓冲区中有足够数据
                    for (uint32_t i = 0; i < 12; i++) {
                        coord_bytes[i] = read_buf[buf_pos++];
                    }
                    file_pos += 12;
                }
                
                // 解析坐标数据
                int32_t lon = (int32_t)(coord_bytes[0] | (coord_bytes[1]<<8) | (coord_bytes[2]<<16) | (coord_bytes[3]<<24));
                int32_t lat = (int32_t)(coord_bytes[4] | (coord_bytes[5]<<8) | (coord_bytes[6]<<16) | (coord_bytes[7]<<24));
                int32_t alt = (int32_t)(coord_bytes[8] | (coord_bytes[9]<<8) | (coord_bytes[10]<<16) | (coord_bytes[11]<<24));
                
                // 转换为浮点数
                double lon_d = lon / 1000000.0;
                double lat_d = lat / 1000000.0;
                double alt_d = alt / 1000000.0;
                
                // 格式化坐标字符串
                char str[96];
                int len = snprintf(str, sizeof(str), "%.6f,%.6f,%.6f", lon_d, lat_d, alt_d);
                
                // 去除尾部多余的0，但至少保留一位小数
                char* dot = strchr(str, '.');
                if (dot) {
                    // 从末尾开始查找非零字符
                    char* end = str + len - 1;
                    while (end > dot && *end == '0') {
                        end--;
                    }
                    // 如果末尾是小数点，需要保留一个0
                    if (end == dot) {
                        end++;
                        *end = '0';  // 写入'0'
                    }
                    // 截断字符串并添加空格分隔符
                    end++;
                    *end = ' ';
                    len = end - str + 1; // 包含空格
                }
                
                f_write(&decompressed_fd, (uint8_t*)str, len, &bw);
                total_out += bw;
            }
            
            // 写</coordinates>
            tag = "</coordinates>";
            f_write(&decompressed_fd, (uint8_t*)tag, strlen(tag), &bw);
            total_out += bw;
            
        } else if (byte < 0x80) {
            // ASCII字符
            // 检查输出缓冲区是否快满了
            if (out_pos >= OUT_BUF_SIZE - 1) {
                UINT bw;
                f_write(&decompressed_fd, out_buf, out_pos, &bw);
                total_out += bw;
                out_pos = 0;
            }
            out_buf[out_pos++] = byte;
            
        } else if (byte >= 0xc0 && byte < 0xe0) {
            // 双字节UTF-8字符
            // 检查输出缓冲区是否有足够空间（需要2字节）
            if (out_pos >= OUT_BUF_SIZE - 2) {
                UINT bw;
                f_write(&decompressed_fd, out_buf, out_pos, &bw);
                total_out += bw;
                out_pos = 0;
            }
            out_buf[out_pos++] = byte;
            // 读取第二个字节
            if (buf_pos >= buf_len) {
                // 需要重新填充缓冲区
                f_lseek(&read_file, file_pos);
                UINT br;
                uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
                res = f_read(&read_file, read_buf, to_read, &br);
                if (res != FR_OK || br < 1) break;
                buf_start = file_pos;
                buf_len = br;
                buf_pos = 0;
            }
            out_buf[out_pos++] = read_buf[buf_pos++];
            file_pos++;
            
        } else if (byte >= 0xe0 && byte < 0xf0) {
            // 三字节UTF-8字符
            // 检查输出缓冲区是否有足够空间（需要3字节）
            if (out_pos >= OUT_BUF_SIZE - 3) {
                UINT bw;
                f_write(&decompressed_fd, out_buf, out_pos, &bw);
                total_out += bw;
                out_pos = 0;
            }
            out_buf[out_pos++] = byte;
            // 读取第二和第三个字节
            for (int j = 0; j < 2; j++) {
                if (buf_pos >= buf_len) {
                    // 需要重新填充缓冲区
                    f_lseek(&read_file, file_pos);
                    UINT br;
                    uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
                    res = f_read(&read_file, read_buf, to_read, &br);
                    if (res != FR_OK || br < 1) break;
                    buf_start = file_pos;
                    buf_len = br;
                    buf_pos = 0;
                }
                out_buf[out_pos++] = read_buf[buf_pos++];
                file_pos++;
            }
            
        } else if (byte >= 0xf0 && byte < 0xf8) {
            // 四字节UTF-8字符
            // 检查输出缓冲区是否有足够空间（需要4字节）
            if (out_pos >= OUT_BUF_SIZE - 4) {
                UINT bw;
                f_write(&decompressed_fd, out_buf, out_pos, &bw);
                total_out += bw;
                out_pos = 0;
            }
            out_buf[out_pos++] = byte;
            // 读取第二、第三和第四个字节
            for (int j = 0; j < 3; j++) {
                if (buf_pos >= buf_len) {
                    // 需要重新填充缓冲区
                    f_lseek(&read_file, file_pos);
                    UINT br;
                    uint32_t to_read = (data_length - file_pos) > READ_BUF_SIZE ? READ_BUF_SIZE : (data_length - file_pos);
                    res = f_read(&read_file, read_buf, to_read, &br);
                    if (res != FR_OK || br < 1) break;
                    buf_start = file_pos;
                    buf_len = br;
                    buf_pos = 0;
                }
                out_buf[out_pos++] = read_buf[buf_pos++];
                file_pos++;
            }
            
        } else {
            // UTF-8续字节 (0x80-0xBF) 或其他字节
            // 检查输出缓冲区是否快满了
            if (out_pos >= OUT_BUF_SIZE - 1) {
                UINT bw;
                f_write(&decompressed_fd, out_buf, out_pos, &bw);
                total_out += bw;
                out_pos = 0;
            }
            out_buf[out_pos++] = byte;
        }
    }
    
    // 写出剩余数据
    if (out_pos > 0) {
        UINT bw;
        f_write(&decompressed_fd, out_buf, out_pos, &bw);
        total_out += bw;
    }
    
    // 关闭文件
    f_sync(&decompressed_fd);
    f_close(&decompressed_fd);
    f_close(&read_file);
    
    // 释放缓冲区
    free(read_buf);
    free(out_buf);
    
    // 修复文件错误
    kml_fix_file_errors(decompressed_file);
    
    return FR_OK;
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
    char decompressed_file[] = "0:/kml/decompressed_kml.kml";
    res = kml_decompress_file(compressed_file, decompressed_file);
    if (res != FR_OK) {
        return res;
    }
    #else
    // 关闭解压缩时，直接返回成功
    uint8_t msg[] = "Decompression disabled, skipping...\r\n";
    HAL_UART_Transmit(&huart1, msg, strlen((char*)msg), 100);
    #endif
    
    return res;
}