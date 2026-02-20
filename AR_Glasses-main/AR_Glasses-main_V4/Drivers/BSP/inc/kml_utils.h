#ifndef KML_UTILS_H
#define KML_UTILS_H

#include "ff.h"

// 功能开关宏定义
#define KML_ENABLE_CRC          0   // 1: 开启CRC校验, 0: 关闭CRC校验
#define KML_ENABLE_DECOMPRESS   0   // 1: 开启解压缩, 0: 关闭解压缩

// 函数声明
/**
 * @brief 初始化KML系统（被StartBLEKMLTask调用）
 * @return FRESULT 文件系统操作结果
 * @note 功能：初始化CRC32表（如果启用）、挂载SD卡文件系统、创建kml目录
 */
FRESULT kml_init_system(void);
FRESULT kml_write_bin_to_sd(FIL *file, uint8_t *buffer, uint32_t buffer_size, UINT *bytes_written);
FRESULT kml_decompress_file(const char *compressed_file, const char *decompressed_file);
FRESULT kml_process_completed_transfer(const char *compressed_file, uint32_t total_written);
/**
 * @brief 打开压缩文件并准备接收数据（被StartBLEKMLTask调用）
 * @param compressed_file 压缩文件路径
 * @param sd_file_opened 文件打开状态
 * @param kml_transfer_active 传输激活状态
 * @return FRESULT 文件系统操作结果
 * @note 功能：重置缓冲区状态、打开文件、启动DMA接收
 */
FRESULT kml_open_compressed_file(const char* compressed_file, volatile uint8_t* sd_file_opened, volatile uint8_t* kml_transfer_active);
/**
 * @brief 处理数据缓冲区（被StartBLEKMLTask调用）
 * @param sd_file_opened 文件打开状态
 * @param total_written 总写入字节数
 * @param last_print_index 上次打印索引
 * @return FRESULT 文件系统操作结果
 * @note 功能：处理DMA接收的数据、将满缓冲区写入SD卡、更新写入计数
 */
FRESULT kml_process_buffers(volatile uint8_t sd_file_opened, uint32_t* total_written, uint32_t* last_print_index);
/**
 * @brief 处理传输完成的操作（被StartBLEKMLTask调用）
 * @param compressed_file 压缩文件路径
 * @param actual_written 实际写入字节数
 * @param sd_file_opened 文件打开状态
 * @param kml_transfer_active 传输激活状态
 * @return FRESULT 文件系统操作结果
 * @note 功能：确保所有数据写入SD卡、关闭压缩文件、解压处理（如果启用）、重置缓冲区状态
 */
FRESULT kml_handle_transfer_complete(const char* compressed_file, uint32_t actual_written, volatile uint8_t* sd_file_opened, volatile uint8_t* kml_transfer_active);

// CRC相关函数声明
void init_crc_table(void);
uint32_t calculate_crc32(const uint8_t* data, uint32_t length);
uint32_t calculateCRC32(uint8_t* data, uint32_t length);

// 解压函数声明
uint32_t decompressData(uint8_t* compressedData, uint32_t compressedSize, uint8_t* outputBuffer, uint32_t outputBufferSize);

#endif /* KML_UTILS_H */
