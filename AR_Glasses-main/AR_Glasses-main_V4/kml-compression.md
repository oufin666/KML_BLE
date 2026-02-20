# KML文件压缩算法说明

## 算法概述

这是一种**坐标数据压缩算法**，专门针对KML文件中的坐标数据设计，具有以下特点：
- 只压缩坐标数据，保留其他内容的原始UTF-8编码
- 坐标数据使用0xfe作为标识，包含坐标数量和每个坐标的详细信息
- 前2字节为压缩标识（'BL'，即0x42 0x4C）
- 适用于资源受限的硬件平台
- 坐标精度保持6位小数，确保与原始数据一致

## 压缩数据结构

1. **压缩标识**（前2字节）：`0x42 0x4C`（'BL'）
2. **压缩内容**：
   - 普通字符：UTF-8编码的字节序列
     - 单字节：0x00-0x7F
     - 双字节：0xC0-0xDF 后跟 0x80-0xBF
     - 三字节：0xE0-0xEF 后跟两个 0x80-0xBF
     - 四字节：0xF0-0xF7 后跟三个 0x80-0xBF
   - **坐标数据块**（以0xfe开头）：
     - 0xfe：坐标数据标识
     - 坐标数量（2字节，小端序）：低字节在前，高字节在后
     - 每个坐标数据（12字节，小端序）：
       - 经度（4字节，int32，乘以10^6）
       - 纬度（4字节，int32，乘以10^6）
       - 高度（4字节，int32，乘以10^6）

## 压缩算法实现（TypeScript）

```typescript
/**
 * 简单可靠的KML文件压缩算法
 * 只压缩坐标数据，确保解压缩正确
 */
export function compressText(text: string): ArrayBuffer {
  // 预分配足够的空间
  const maxLength = Math.floor(text.length * 1.2) // 预留足够空间
  const compressed = new Uint8Array(maxLength)
  let offset = 0
  
  let i = 0
  while (i < text.length) {
    let matched = false
    
    // 检查是否是坐标数据
    if (text.startsWith('<coordinates>', i)) {
      // 找到对应的</coordinates>标签
      const startTag = '<coordinates>'
      const endTag = '</coordinates>'
      const endIndex = text.indexOf(endTag, i + startTag.length)
      if (endIndex !== -1) {
        // 提取坐标数据
        const coordData = text.substring(i + startTag.length, endIndex)
        // 压缩坐标数据
        const compressedCoords = compressCoordinates(coordData)
        
        // 检查是否有足够的空间
        if (offset + compressedCoords.length > compressed.length) {
          console.error('Compressed buffer overflow!')
          break
        }
        
        // 写入压缩后的坐标数据
        for (const byte of compressedCoords) {
          compressed[offset++] = byte
        }
        // 跳过整个坐标标签
        i = endIndex + endTag.length
        matched = true
      }
    }
    
    // 写入UTF-8编码的字符
    if (!matched) {
      const code = text.charCodeAt(i)
      if (code <= 0x7f) {
        // 单字节
        compressed[offset++] = code
      } else if (code <= 0x7ff) {
        // 双字节
        compressed[offset++] = 0xc0 | (code >> 6)
        compressed[offset++] = 0x80 | (code & 0x3f)
      } else if (code <= 0xffff) {
        // 三字节
        compressed[offset++] = 0xe0 | (code >> 12)
        compressed[offset++] = 0x80 | ((code >> 6) & 0x3f)
        compressed[offset++] = 0x80 | (code & 0x3f)
      } else {
        // 四字节
        compressed[offset++] = 0xf0 | (code >> 18)
        compressed[offset++] = 0x80 | ((code >> 12) & 0x3f)
        compressed[offset++] = 0x80 | ((code >> 6) & 0x3f)
        compressed[offset++] = 0x80 | (code & 0x3f)
      }
      i++
    }
  }
  
  // 裁剪到实际长度
  const result = compressed.slice(0, offset)
  
  // 添加压缩标识
  const prefix = new Uint8Array([0x42, 0x4C]) // 'BL' 标识
  const finalResult = new Uint8Array(prefix.length + result.length)
  finalResult.set(prefix)
  finalResult.set(result, prefix.length)
  
  return finalResult.buffer
}

/**
 * 压缩坐标数据
 * 将坐标数据转换为更紧凑的格式
 */
function compressCoordinates(coordData: string): number[] {
  const result: number[] = []
  
  // 坐标数据格式: "lon,lat,alt lon,lat,alt ..."
  // 先过滤掉空字符串
  const coords = coordData.trim().split(/\s+/).filter(c => c.length > 0)
  
  // 先收集所有有效的坐标数据
  const validCoords: { lon: number; lat: number; alt: number }[] = []
  
  for (const coord of coords) {
    const parts = coord.split(',')
    if (parts.length >= 2) {
      const lon = parseFloat(parts[0])
      const lat = parseFloat(parts[1])
      const alt = parts.length >= 3 ? parseFloat(parts[2]) : 0
      
      // 检查是否是有效的数字
      if (!isNaN(lon) && !isNaN(lat)) {
        validCoords.push({ lon, lat, alt: isNaN(alt) ? 0 : alt })
      }
    }
  }
  
  // 写入坐标数量（使用实际有效的坐标数量）
  result.push(0xfe) // 坐标数据标识
  result.push(validCoords.length & 0xff) // 低字节
  result.push((validCoords.length >> 8) & 0xff) // 高字节
  
  // 压缩每个坐标
  for (const coord of validCoords) {
    // 将经纬度和高度转换为整数（乘以10^6，保留6位小数）
    const lonInt = Math.round(coord.lon * 1000000)
    const latInt = Math.round(coord.lat * 1000000)
    const altInt = Math.round(coord.alt * 1000000)
    
    // 使用DataView来确保正确的字节顺序和符号处理
    const buffer = new ArrayBuffer(12)
    const view = new DataView(buffer)
    view.setInt32(0, lonInt, true) // true for little-endian
    view.setInt32(4, latInt, true) // true for little-endian
    view.setInt32(8, altInt, true) // true for little-endian
    
    const bytes = new Uint8Array(buffer)
    result.push(...bytes)
  }
  
  return result
}
```

## 硬件端解压算法实现建议（C语言）

### 1. 解压函数

```c
/**
 * 解压缩数据
 * 
 * @param compressed 压缩数据（包含BL标识）
 * @param compressed_len 压缩数据长度
 * @param decompressed 解压缓冲区
 * @param decompressed_len 解压缓冲区大小
 * @return 解压后的数据长度，失败返回-1
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
        
        if (byte == 0xfe) {
            // 处理坐标数据
            if (offset + 2 > compressed_len) {
                return -1; // 数据长度不足
            }
            
            // 读取坐标数量（小端序）
            int coord_count = compressed[offset] | (compressed[offset + 1] << 8);
            offset += 2;
            
            // 写入<coordinates>标签
            const char* start_tag = "<coordinates>";
            int start_tag_len = strlen(start_tag);
            if (decompressed_offset + start_tag_len > decompressed_len) {
                return -1; // 解压缓冲区不足
            }
            memcpy(&decompressed[decompressed_offset], start_tag, start_tag_len);
            decompressed_offset += start_tag_len;
            
            // 处理每个坐标
            for (int i = 0; i < coord_count; i++) {
                if (offset + 12 > compressed_len) {
                    return -1; // 数据长度不足
                }
                
                // 读取坐标数据（小端序）
                int32_t lon_int = *(int32_t*)&compressed[offset];
                int32_t lat_int = *(int32_t*)&compressed[offset + 4];
                int32_t alt_int = *(int32_t*)&compressed[offset + 8];
                offset += 12;
                
                // 转换回浮点数
                double lon = lon_int / 1000000.0;
                double lat = lat_int / 1000000.0;
                double alt = alt_int / 1000000.0;
                
                // 格式化坐标字符串
                char coord_str[64];
                int coord_str_len = snprintf(coord_str, sizeof(coord_str), "%.6f,%.6f,%.6f", lon, lat, alt);
                
                // 去除尾部多余的0，但至少保留一位小数
                char* dot_pos = strchr(coord_str, '.');
                if (dot_pos) {
                    // 从末尾开始查找非零字符
                    char* end_pos = coord_str + coord_str_len - 1;
                    while (end_pos > dot_pos && *end_pos == '0') {
                        end_pos--;
                    }
                    // 如果末尾是小数点，添加一个0
                    if (end_pos == dot_pos) {
                        end_pos++;
                    }
                    // 截断字符串
                    end_pos++;
                    *end_pos = ' '; // 添加空格分隔符
                    coord_str_len = end_pos - coord_str + 1;
                }
                
                // 写入坐标数据
                if (decompressed_offset + coord_str_len > decompressed_len) {
                    return -1; // 解压缓冲区不足
                }
                memcpy(&decompressed[decompressed_offset], coord_str, coord_str_len);
                decompressed_offset += coord_str_len;
            }
            
            // 写入</coordinates>标签
            const char* end_tag = "</coordinates>";
            int end_tag_len = strlen(end_tag);
            if (decompressed_offset + end_tag_len > decompressed_len) {
                return -1; // 解压缓冲区不足
            }
            memcpy(&decompressed[decompressed_offset], end_tag, end_tag_len);
            decompressed_offset += end_tag_len;
        } else if (byte < 0x80) {
            // 处理单字节UTF-8字符
            if (decompressed_offset >= decompressed_len) {
                return -1; // 解压缓冲区不足
            }
            decompressed[decompressed_offset++] = byte;
        } else if (byte >= 0xc0 && byte < 0xe0) {
            // 处理双字节UTF-8字符
            if (offset >= compressed_len) {
                return -1; // 数据长度不足
            }
            if (decompressed_offset + 2 > decompressed_len) {
                return -1; // 解压缓冲区不足
            }
            decompressed[decompressed_offset++] = byte;
            decompressed[decompressed_offset++] = compressed[offset++];
        } else if (byte >= 0xe0 && byte < 0xf0) {
            // 处理三字节UTF-8字符
            if (offset + 1 >= compressed_len) {
                return -1; // 数据长度不足
            }
            if (decompressed_offset + 3 > decompressed_len) {
                return -1; // 解压缓冲区不足
            }
            decompressed[decompressed_offset++] = byte;
            decompressed[decompressed_offset++] = compressed[offset++];
            decompressed[decompressed_offset++] = compressed[offset++];
        } else if (byte >= 0xf0 && byte < 0xf8) {
            // 处理四字节UTF-8字符
            if (offset + 2 >= compressed_len) {
                return -1; // 数据长度不足
            }
            if (decompressed_offset + 4 > decompressed_len) {
                return -1; // 解压缓冲区不足
            }
            decompressed[decompressed_offset++] = byte;
            decompressed[decompressed_offset++] = compressed[offset++];
            decompressed[decompressed_offset++] = compressed[offset++];
            decompressed[decompressed_offset++] = compressed[offset++];
        } else {
            // 处理其他字节（UTF-8续字节等）
            if (decompressed_offset >= decompressed_len) {
                return -1; // 解压缓冲区不足
            }
            decompressed[decompressed_offset++] = byte;
        }
    }
    
    return decompressed_offset;
}
```

### 2. 数据接收和处理流程

```c
/**
 * 处理接收到的蓝牙数据
 * 
 * @param data 接收到的数据包
 * @param data_len 数据包长度
 */
void process_ble_data(const uint8_t* data, int data_len) {
    static uint8_t compressed_buffer[4096]; // 压缩数据缓冲区
    static int compressed_offset = 0;
    static uint8_t decompressed_buffer[8192]; // 解压缓冲区
    
    // 将接收到的数据添加到压缩缓冲区
    if (compressed_offset + data_len <= sizeof(compressed_buffer)) {
        memcpy(&compressed_buffer[compressed_offset], data, data_len);
        compressed_offset += data_len;
    } else {
        // 压缩缓冲区不足，处理错误
        compressed_offset = 0;
        return;
    }
    
    // 检查是否收到结束符序列（0xFF 0xFF 0xFF）
    if (data_len >= 3 && data[data_len - 1] == 0xFF && data[data_len - 2] == 0xFF && data[data_len - 3] == 0xFF) {
        // 解压缩数据（移除结束符序列）
        int decompressed_len = decompress(
            compressed_buffer, 
            compressed_offset - 3, // 减去结束符序列长度
            decompressed_buffer, 
            sizeof(decompressed_buffer)
        );
        
        if (decompressed_len > 0) {
            // 解压缩成功，处理解压缩后的数据
            handle_decompressed_data(decompressed_buffer, decompressed_len);
        }
        
        // 重置缓冲区
        compressed_offset = 0;
    }
}

/**
 * 处理解压缩后的数据
 * 
 * @param data 解压缩后的数据
 * @param len 数据长度
 */
void handle_decompressed_data(const uint8_t* data, int len) {
    // 这里可以处理解压缩后的数据
    // 例如：保存到文件系统、解析KML内容等
    printf("Decompressed data received: %d bytes\n", len);
    // 打印前100个字节用于调试
    for (int i = 0; i < len && i < 100; i++) {
        printf("%c", data[i]);
    }
    printf("\n");
}
```

## 数据传输注意事项

1. **数据包结构**：
   - 压缩数据前2字节为标识（0x42 0x4C）
   - 发送时会在最后添加结束符序列 0xFF 0xFF 0xFF（单独一包）
   - 硬件端解压时应移除结束符序列

2. **CRC32验证**：
   - 小程序端计算压缩数据（不含结束符序列）的CRC32
   - 硬件端应在移除结束符序列后计算CRC32进行验证

3. **内存管理**：
   - 建议为解压缓冲区分配至少原始文件大小的空间
   - 对于大型KML文件，需注意内存使用情况

4. **坐标数据处理**：
   - 硬件端应正确处理小端序的坐标数据
   - 确保坐标精度保持6位小数
   - 处理坐标数据时应考虑符号位（正负坐标）

## 测试建议

1. **使用简单的KML文件测试**：
   ```xml
   <?xml version="1.0" encoding="UTF-8"?>
   <kml xmlns="http://www.opengis.net/kml/2.2">
     <Placemark>
       <name>测试点</name>
       <description>这是一个测试点</description>
       <Point>
         <coordinates>116.404,39.915,0</coordinates>
       </Point>
     </Placemark>
   </kml>
   ```

2. **验证解压后的数据与原始文件是否一致**：
   - 比较文件长度
   - 比较文件内容
   - 比较CRC32值
   - 特别验证坐标数据是否完全一致

3. **测试边界情况**：
   - 空文件
   - 只有标签的文件
   - 包含特殊字符的文件
   - 包含多个坐标点的文件
   - 包含负坐标的文件

4. **确认CRC32验证功能正常工作**：
   - 小程序端计算压缩数据的CRC32
   - 硬件端计算解压后数据的CRC32
   - 比较两个CRC32值是否一致

5. **性能测试**：
   - 测试不同大小的KML文件压缩和解压速度
   - 测试内存使用情况
   - 测试电池消耗情况（对于电池供电的设备）

## 技术支持

如果在实现过程中遇到任何问题，请联系小程序开发团队提供进一步的技术支持。