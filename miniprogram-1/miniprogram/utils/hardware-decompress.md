# 硬件端解压缩算法

## 算法说明

本文档提供了适用于硬件端（如单片机）的解压缩算法，用于解压缩通过蓝牙发送的KML文件数据。该算法与小程序端使用的标签替换压缩算法兼容。

## 压缩格式

压缩数据格式如下：
1. 前2字节：压缩标识 `0x42 0x4C`（'BL'）
2. 后续字节：压缩数据，包含两种类型的数据：
   - 标签代码（0x80-0x8F）：表示常见的KML标签
   - 普通字节：UTF-8编码的普通字符

## 标签代码映射

| 代码 | 对应标签 |
|------|----------|
| 0x81 | `<kml xmlns="http://www.opengis.net/kml/2.2">` |
| 0x82 | `</kml>` |
| 0x83 | `<Placemark>` |
| 0x84 | `</Placemark>` |
| 0x85 | `<name>` |
| 0x86 | `</name>` |
| 0x87 | `<description>` |
| 0x88 | `</description>` |
| 0x89 | `<Point>` |
| 0x8A | `</Point>` |
| 0x8B | `<coordinates>` |
| 0x8C | `</coordinates>` |
| 0x8D | `<?xml version="1.0" encoding="UTF-8"?>` |
| 0x8E | `\r\n` |
| 0x8F | `\n` |

## C语言实现

```c
#include <stdint.h>
#include <string.h>

// 定义最大解压缩缓冲区大小
// 根据实际硬件内存调整
#define MAX_DECOMPRESSED_SIZE 1024 * 100 // 100KB

// 标签映射表
const char* tag_map[] = {
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

/**
 * 解压缩数据
 * 
 * @param compressed 压缩数据
 * @param compressed_len 压缩数据长度
 * @param decompressed 解压缩缓冲区
 * @param decompressed_len 解压缩缓冲区大小
 * @return 解压缩后的数据长度，失败返回-1
 */
int decompress(const uint8_t* compressed, int compressed_len, uint8_t* decompressed, int decompressed_len) {
    if (compressed_len < 2) {
        return -1; // 数据长度不足
    }
    
    // 检查压缩标识
    if (compressed[0] != 0x42 || compressed[1] != 0x4C) {
        return -1; // 无效的压缩标识
    }
    
    int offset = 2; // 跳过压缩标识
    int decompressed_offset = 0;
    
    while (offset < compressed_len) {
        uint8_t byte = compressed[offset++];
        
        if (byte >= 0x80 && byte <= 0x8F) {
            // 处理标签代码
            int tag_index = byte - 0x80;
            const char* tag = tag_map[tag_index];
            
            if (tag) {
                // 复制标签到解压缩缓冲区
                int tag_len = strlen(tag);
                if (decompressed_offset + tag_len > decompressed_len) {
                    return -1; // 解压缩缓冲区不足
                }
                
                memcpy(&decompressed[decompressed_offset], tag, tag_len);
                decompressed_offset += tag_len;
            } else {
                // 未知标签，作为普通字符处理
                if (decompressed_offset >= decompressed_len) {
                    return -1; // 解压缩缓冲区不足
                }
                decompressed[decompressed_offset++] = byte;
            }
        } else {
            // 处理普通字符
            if (decompressed_offset >= decompressed_len) {
                return -1; // 解压缩缓冲区不足
            }
            decompressed[decompressed_offset++] = byte;
        }
    }
    
    return decompressed_offset;
}

/**
 * 处理接收到的蓝牙数据
 * 
 * @param data 接收到的数据包
 * @param data_len 数据包长度
 */
void process_ble_data(const uint8_t* data, int data_len) {
    static uint8_t compressed_buffer[4096]; // 压缩数据缓冲区
    static int compressed_offset = 0;
    static uint8_t decompressed_buffer[MAX_DECOMPRESSED_SIZE]; // 解压缩缓冲区
    
    // 将接收到的数据添加到压缩缓冲区
    if (compressed_offset + data_len <= sizeof(compressed_buffer)) {
        memcpy(&compressed_buffer[compressed_offset], data, data_len);
        compressed_offset += data_len;
    } else {
        // 压缩缓冲区不足，处理错误
        compressed_offset = 0;
        return;
    }
    
    // 检查是否收到完整的压缩数据（这里简化处理，实际应用中需要根据通信协议判断）
    // 假设这里检测到数据结束
    
    // 解压缩数据
    int decompressed_len = decompress(
        compressed_buffer, 
        compressed_offset, 
        decompressed_buffer, 
        sizeof(decompressed_buffer)
    );
    
    if (decompressed_len > 0) {
        // 解压缩成功，处理解压缩后的数据
        // 例如，保存到文件系统或进行其他处理
        handle_decompressed_data(decompressed_buffer, decompressed_len);
    }
    
    // 重置缓冲区
    compressed_offset = 0;
}

/**
 * 处理解压缩后的数据
 * 
 * @param data 解压缩后的数据
 * @param len 数据长度
 */
void handle_decompressed_data(const uint8_t* data, int len) {
    // 根据实际需求实现
    // 例如，将数据写入文件系统
}
```

## 使用示例

### 初始化

1. 为压缩数据和解压缩数据分配缓冲区
2. 设置蓝牙接收回调函数，处理接收到的数据

### 数据处理流程

1. 蓝牙接收数据并存储到压缩缓冲区
2. 当收到完整的压缩数据后，调用 `decompress` 函数解压缩
3. 处理解压缩后的数据，例如保存到文件系统
4. 重置缓冲区，准备接收下一批数据

## 优化建议

1. **内存优化**：根据实际硬件内存大小调整缓冲区大小
2. **性能优化**：对于频繁使用的标签，可以考虑使用更快的查找方式
3. **错误处理**：增加更详细的错误处理，提高系统稳定性
4. **流式处理**：如果内存有限，可以考虑实现流式解压缩，边接收边解压缩

## 注意事项

1. 确保解压缩缓冲区足够大，能够容纳解压缩后的数据
2. 处理数据时要注意边界情况，避免缓冲区溢出
3. 与小程序端的压缩算法保持兼容，确保标签映射表一致
4. 根据实际硬件性能调整算法细节，确保在有限资源下正常运行
