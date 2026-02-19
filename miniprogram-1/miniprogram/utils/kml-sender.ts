/**
 * KML 分包发送模块
 * 适配 BLE 分包限制，整段 KML 发送完后追加结束符序列 0xFF 0xFF 0xFF
 */

import {
  writeBLECharacteristicValue,
  findWritableCharacteristic,
  createBLEConnection
} from './ble'

/** BLE 单次写入最大字节数，适配 MTU 限制（230 字节适配资料推荐值） */
export const BLE_CHUNK_SIZE = 230

/** KML 发送结束符序列 */
export const KML_END_MARKER = [0xFF, 0xFF, 0xFF]

/**
 * CRC32 计算函数
 */
export function calculateCRC32(buffer: ArrayBuffer): number {
  // CRC32 表
  const crcTable: number[] = []
  for (let i = 0; i < 256; i++) {
    let crc = i
    for (let j = 0; j < 8; j++) {
      crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0)
    }
    crcTable[i] = crc
  }
  
  let crc = 0xFFFFFFFF
  const view = new Uint8Array(buffer)
  
  for (let i = 0; i < view.length; i++) {
    crc = (crc >> 8) ^ crcTable[(crc & 0xFF) ^ view[i]]
  }
  
  // 使用无符号右移确保结果是正数
  return (crc ^ 0xFFFFFFFF) >>> 0
}

/**
 * 读取 KML 文件内容并计算 CRC32
 */
export async function readKmlFileWithCRC(filePath: string, compress: boolean = false): Promise<{ buffer: ArrayBuffer; crc32: number }> {
  const buffer = await readKmlFileAsArrayBuffer(filePath, compress)
  const crc32 = calculateCRC32(buffer)
  return { buffer, crc32 }
}

export interface SendProgress {
  sentBytes: number
  totalBytes: number
  percent: number
  currentChunk: number
  totalChunks: number
}

export interface KmlSendOptions {
  /** 设备 ID */
  deviceId: string
  /** 分包大小，默认 20 字节 */
  chunkSize?: number
  /** 每包之间的延迟 ms，部分 BLE 栈需要 */
  writeDelayMs?: number
  /** 进度回调 */
  onProgress?: (progress: SendProgress) => void
}

/**
 * 将 ArrayBuffer 按指定大小分块
 */
export function chunkArrayBuffer(
  buffer: ArrayBuffer,
  chunkSize: number
): ArrayBuffer[] {
  const chunks: ArrayBuffer[] = []
  const view = new Uint8Array(buffer)
  for (let i = 0; i < view.length; i += chunkSize) {
    chunks.push(view.slice(i, i + chunkSize).buffer)
  }
  return chunks
}

/** 校验结果：用于在不连设备时验证分包与结束符是否正确 */
export interface VerifyResult {
  ok: boolean
  message: string
  totalBytes: number
  chunkCount: number
  lastByteIs0x04: boolean
  /** 前 64 字节十六进制预览（便于对比） */
  headHex: string
  /** 最后 32 字节十六进制预览（应含 ... FF FF FF） */
  tailHex: string
}

/**
 * 生成「将要通过 BLE 发送的完整载荷」= KML 内容 + 结束符序列 0xFF 0xFF 0xFF
 */
export function getPayloadToSend(
  buffer: ArrayBuffer,
  chunkSize: number = BLE_CHUNK_SIZE
): { payload: ArrayBuffer; chunkCount: number; totalBytes: number } {
  const contentChunks = chunkArrayBuffer(buffer, chunkSize)
  const endChunk = new Uint8Array(KML_END_MARKER).buffer
  const totalBytes = buffer.byteLength + KML_END_MARKER.length
  const chunkCount = contentChunks.length + 1
  const payload = new Uint8Array(totalBytes)
  payload.set(new Uint8Array(buffer), 0)
  payload.set(KML_END_MARKER, payload.length - KML_END_MARKER.length)
  return { payload: payload.buffer, chunkCount, totalBytes }
}

/**
 * 校验分包逻辑是否正确：模拟分包再拼接，应等于 原内容 + 结束符序列 0xFF 0xFF 0xFF
 */
export function verifyChunkedPayload(
  buffer: ArrayBuffer,
  chunkSize: number = BLE_CHUNK_SIZE
): VerifyResult {
  const contentChunks = chunkArrayBuffer(buffer, chunkSize)
  const endChunk = new Uint8Array(KML_END_MARKER)
  const totalBytes = buffer.byteLength + KML_END_MARKER.length
  const chunkCount = contentChunks.length + 1

  const reassembled = new Uint8Array(totalBytes)
  let offset = 0
  for (const chunk of contentChunks) {
    const arr = new Uint8Array(chunk)
    reassembled.set(arr, offset)
    offset += arr.length
  }
  reassembled.set(KML_END_MARKER, offset)

  const expected = new Uint8Array(totalBytes)
  expected.set(new Uint8Array(buffer), 0)
  expected.set(KML_END_MARKER, expected.length - KML_END_MARKER.length)

  let ok = reassembled.length === expected.length
  if (ok) {
    for (let i = 0; i < expected.length; i++) {
      if (reassembled[i] !== expected[i]) {
        ok = false
        break
      }
    }
  }

  // 检查是否以结束符序列结尾
  const endsWithEndMarker = totalBytes >= KML_END_MARKER.length && 
    reassembled.subarray(totalBytes - KML_END_MARKER.length).every((byte, index) => 
      byte === KML_END_MARKER[index]
    )
  const toHex = (arr: Uint8Array, max: number) =>
    Array.from(arr.slice(0, max))
      .map((b) => b.toString(16).padStart(2, '0'))
      .join(' ')

  return {
    ok,
    message: ok
      ? `校验通过：${totalBytes} 字节，${chunkCount} 包，末尾 ${KML_END_MARKER.map(b => '0x' + b.toString(16).toUpperCase()).join(' ')}`
      : `校验失败：拼接结果与预期不一致`,
    totalBytes,
    chunkCount,
    lastByteIs0x04: endsWithEndMarker, // 保持向后兼容
    endsWithEndMarker,
    headHex: toHex(reassembled, 64),
    tailHex: toHex(reassembled.slice(-32), 32)
  }
}

/**
 * 延时函数
 */
function delay(ms: number): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, ms))
}

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

/**
 * 解压缩坐标数据
 */
function decompressCoordinates(data: Uint8Array, offset: number): { text: string; newOffset: number } {
  let pos = offset
  const coords: string[] = []
  
  // 读取坐标数量
  const count = (data[pos + 1] & 0xff) | ((data[pos + 2] & 0xff) << 8)
  pos += 3
  
  // 解压缩每个坐标
  for (let i = 0; i < count; i++) {
    // 使用DataView来确保正确的字节顺序和符号处理
    const buffer = new ArrayBuffer(12)
    const bytes = new Uint8Array(buffer)
    bytes.set(data.subarray(pos, pos + 12))
    const view = new DataView(buffer)
    
    const lonInt = view.getInt32(0, true) // true for little-endian
    const latInt = view.getInt32(4, true) // true for little-endian
    const altInt = view.getInt32(8, true) // true for little-endian
    
    pos += 12
    
    // 转换回浮点数并保持精度
    const lon = lonInt / 1000000
    const lat = latInt / 1000000
    const alt = altInt / 1000000
    
    // 格式化坐标，确保精度和格式正确
    const formattedLon = formatCoordinate(lon)
    const formattedLat = formatCoordinate(lat)
    const formattedAlt = formatCoordinate(alt)
    coords.push(`${formattedLon},${formattedLat},${formattedAlt}`)
  }
  
  return {
    text: coords.join(' '),
    newOffset: pos
  }
}

/**
 * 格式化坐标值，保持与原始KML文件一致的格式
 * 规则：如果是整数，保留一位小数（如 0 -> 0.0）
 *      如果有小数，去除尾部多余的0，但至少保留一位小数
 */
function formatCoordinate(value: number): string {
  // 先转换为固定6位小数
  let str = value.toFixed(6)
  
  // 去除尾部的0，但至少保留一位小数
  // 例如：-61.000000 -> -61.0
  //      22.398226 -> 22.398226
  //      0.000000 -> 0.0
  str = str.replace(/(\.\d*?)0+$/, '$1')
  
  // 如果小数点后没有数字了，添加一个0
  // 例如：-61. -> -61.0
  if (str.endsWith('.')) {
    str += '0'
  }
  
  return str
}

/**
 * 读取 KML 文件内容为 ArrayBuffer（UTF-8）
 * 使用 encoding: 'utf8' 保证各平台返回一致，再转成 UTF-8 字节
 */
export function readKmlFileAsArrayBuffer(filePath: string, compress: boolean = false): Promise<ArrayBuffer> {
  return new Promise((resolve, reject) => {
    const fs = wx.getFileSystemManager()
    fs.readFile({
      filePath,
      encoding: 'utf8',
      success: (res) => {
        const data = res.data
        if (typeof data === 'string') {
          if (compress) {
            // 使用压缩算法
            console.log('Compressing file, original length:', data.length)
            const compressedBuffer = compressText(data)
            console.log('Compressed length:', compressedBuffer.byteLength)
            console.log('Compression ratio:', ((1 - compressedBuffer.byteLength / data.length) * 100).toFixed(2) + '%')
            
            // 统计压缩后文件中0xfe的个数
            const view = new Uint8Array(compressedBuffer)
            let feCount = 0
            for (let i = 0; i < view.length; i++) {
              if (view[i] === 0xfe) {
                feCount++
              }
            }
            console.log('Number of 0xfe in compressed file:', feCount)
            
            resolve(compressedBuffer)
          } else {
            const originalBuffer = stringToUtf8ArrayBuffer(data)
            console.log('Original buffer length:', originalBuffer.byteLength)
            resolve(originalBuffer)
          }
          return
        }
        if (data instanceof ArrayBuffer) {
          resolve(data)
          return
        }
        reject(new Error('无法读取文件内容'))
      },
      fail: (err) => reject(err)
    })
  })
}

/**
 * 字符串转 UTF-8 ArrayBuffer
 * 兼容不支持 TextEncoder 的环境
 */
function stringToUtf8ArrayBuffer(str: string): ArrayBuffer {
  // 手动实现 UTF-8 编码，不依赖 TextEncoder
  let length = 0
  for (let i = 0; i < str.length; i++) {
    const code = str.charCodeAt(i)
    if (code <= 0x7f) {
      length += 1
    } else if (code <= 0x7ff) {
      length += 2
    } else if (code <= 0xffff) {
      length += 3
    } else {
      length += 4
    }
  }
  
  const buffer = new ArrayBuffer(length)
  const view = new Uint8Array(buffer)
  let offset = 0
  
  for (let i = 0; i < str.length; i++) {
    const code = str.charCodeAt(i)
    if (code <= 0x7f) {
      view[offset++] = code
    } else if (code <= 0x7ff) {
      view[offset++] = 0xc0 | (code >> 6)
      view[offset++] = 0x80 | (code & 0x3f)
    } else if (code <= 0xffff) {
      view[offset++] = 0xe0 | (code >> 12)
      view[offset++] = 0x80 | ((code >> 6) & 0x3f)
      view[offset++] = 0x80 | (code & 0x3f)
    } else {
      view[offset++] = 0xf0 | (code >> 18)
      view[offset++] = 0x80 | ((code >> 12) & 0x3f)
      view[offset++] = 0x80 | ((code >> 6) & 0x3f)
      view[offset++] = 0x80 | (code & 0x3f)
    }
  }
  
  return buffer
}

/**
 * 解压缩数据
 * 用于硬件端解压缩
 */
export function decompress(data: ArrayBuffer): ArrayBuffer {
  const view = new Uint8Array(data)
  let offset = 0
  
  // 检查压缩标识（只在开头检查一次）
  if (view[0] === 0x42 && view[1] === 0x4C) {
    // 'BL' 标识，跳过
    offset = 2
  } else if (view[0] === 0x1F && view[1] === 0x8B) {
    // 旧的 gzip 标识，跳过
    offset = 2
  }
  
  const decompressed: number[] = []
  
  while (offset < view.length) {
    const byte = view[offset]
    
    // 首先检查是否是坐标数据标识
    if (byte === 0xfe) {
      // 坐标数据，使用专门的解压缩函数
      const { text, newOffset } = decompressCoordinates(view, offset)
      offset = newOffset
      // 写入<coordinates>标签
      const startTag = '<coordinates>'
      for (let j = 0; j < startTag.length; j++) {
        decompressed.push(startTag.charCodeAt(j))
      }
      // 将解压缩后的坐标数据转换为UTF-8字节序列
      for (let i = 0; i < text.length; i++) {
        const code = text.charCodeAt(i)
        if (code <= 0x7f) {
          decompressed.push(code)
        } else if (code <= 0x7ff) {
          decompressed.push(0xc0 | (code >> 6))
          decompressed.push(0x80 | (code & 0x3f))
        } else if (code <= 0xffff) {
          decompressed.push(0xe0 | (code >> 12))
          decompressed.push(0x80 | ((code >> 6) & 0x3f))
          decompressed.push(0x80 | (code & 0x3f))
        } else {
          decompressed.push(0xf0 | (code >> 18))
          decompressed.push(0x80 | ((code >> 12) & 0x3f))
          decompressed.push(0x80 | ((code >> 6) & 0x3f))
          decompressed.push(0x80 | (code & 0x3f))
        }
      }
      // 写入</coordinates>标签
      const endTag = '</coordinates>'
      for (let j = 0; j < endTag.length; j++) {
        decompressed.push(endTag.charCodeAt(j))
      }
    } else if (byte < 0x80) {
      // 单字节UTF-8字符（ASCII字符）
      decompressed.push(byte)
      offset++
    } else if (byte >= 0xc0 && byte < 0xe0) {
      // 双字节UTF-8字符
      if (offset + 1 < view.length) {
        decompressed.push(byte)
        decompressed.push(view[offset + 1])
        offset += 2
      } else {
        // 数据不足，作为普通字符处理
        decompressed.push(byte)
        offset++
      }
    } else if (byte >= 0xe0 && byte < 0xf0) {
      // 三字节UTF-8字符
      if (offset + 2 < view.length) {
        decompressed.push(byte)
        decompressed.push(view[offset + 1])
        decompressed.push(view[offset + 2])
        offset += 3
      } else {
        // 数据不足，作为普通字符处理
        decompressed.push(byte)
        offset++
      }
    } else if (byte >= 0xf0 && byte < 0xf8) {
      // 四字节UTF-8字符
      if (offset + 3 < view.length) {
        decompressed.push(byte)
        decompressed.push(view[offset + 1])
        decompressed.push(view[offset + 2])
        decompressed.push(view[offset + 3])
        offset += 4
      } else {
        // 数据不足，作为普通字符处理
        decompressed.push(byte)
        offset++
      }
    } else {
      // 0x80-0xbf 范围的字节（UTF-8 continuation bytes）
      // 这些字节不应该单独出现，但为了容错，直接复制
      decompressed.push(byte)
      offset++
    }
  }
  
  // 转换为 ArrayBuffer
  const result = new Uint8Array(decompressed.length)
  for (let i = 0; i < decompressed.length; i++) {
    result[i] = decompressed[i]
  }
  
  return result.buffer
}

/**
 * 将 KML 的 ArrayBuffer 分包发送到 BLE 设备
 * 最后追加结束符序列 0xFF 0xFF 0xFF
 */
export async function sendKmlBufferToBle(
  buffer: ArrayBuffer,
  options: KmlSendOptions
): Promise<void> {
  console.log('sendKmlBufferToBle called with buffer length:', buffer.byteLength)
  const {
    deviceId,
    chunkSize = 230, // 按照表格设置分包大小
    writeDelayMs = 20, // 按照表格设置发送间隔
    onProgress
  } = options

  // 分包：KML 内容
  console.log('Chunking buffer with size:', chunkSize)
  const contentChunks = chunkArrayBuffer(buffer, chunkSize)
  // 结束符单独一包
  const endChunk = new Uint8Array(KML_END_MARKER).buffer
  const allChunks = [...contentChunks, endChunk]
  const totalBytes = buffer.byteLength + KML_END_MARKER.length
  let sentBytes = 0
  let currentChunkIndex = 0

  console.log('Total chunks to send:', allChunks.length)
  console.log('Total bytes to send:', totalBytes)

  // 主发送循环，支持重连后续传
  while (currentChunkIndex < allChunks.length) {
    try {
      console.log('Finding writable characteristic for device:', deviceId)
      const characteristic = await findWritableCharacteristic(deviceId)
      console.log('Found writable characteristic:', characteristic)

      // 发送当前及后续数据包
      for (let i = currentChunkIndex; i < allChunks.length; i++) {
        // 每发送10个包，暂停一段时间，避免连接超时
        if (i > 0 && i % 10 === 0) {
          console.log('Pausing for 500ms to avoid connection timeout')
          await delay(500)
        }

        const chunk = allChunks[i]
        console.log('Sending chunk', i + 1, 'of', allChunks.length, 'size:', chunk.byteLength)
        
        // 添加错误重试机制
        let retries = 3
        let sent = false
        
        while (retries > 0 && !sent) {
          try {
            await writeBLECharacteristicValue(
              deviceId,
              characteristic.serviceId,
              characteristic.uuid,
              chunk
            )
            console.log('Chunk', i + 1, 'sent successfully')
            sent = true
          } catch (error: any) {
            console.warn('Error sending chunk', i + 1, ':', error)
            
            // 检查是否是连接错误
            if (error.errMsg && error.errMsg.includes('no connection')) {
              console.warn('Connection lost, will attempt to reconnect')
              throw error // 抛出错误以触发重连
            }
            
            retries--
            if (retries > 0) {
              console.log('Retrying...', retries, 'attempts left')
              await delay(writeDelayMs * 2) // 重试时增加延迟
            } else {
              throw error // 重试失败，抛出错误
            }
          }
        }

        sentBytes += chunk.byteLength
        currentChunkIndex = i + 1 // 更新当前位置

        if (onProgress) {
          console.log('Calling onProgress with sentBytes:', sentBytes, 'totalBytes:', totalBytes)
          onProgress({
            sentBytes,
            totalBytes,
            percent: Math.round((sentBytes / totalBytes) * 100),
            currentChunk: i + 1,
            totalChunks: allChunks.length
          })
          console.log('onProgress called successfully')
        }

        // 包间延迟，避免 BLE 栈溢出
        if (writeDelayMs > 0 && i < allChunks.length - 1) {
          console.log('Waiting for', writeDelayMs, 'ms before sending next chunk')
          await delay(writeDelayMs)
        }
      }
      
      // 所有数据包发送完成
      console.log('All chunks sent successfully')
      return
      
    } catch (error: any) {
      console.warn('Error during send, attempting to reconnect:', error)
      
      // 检查是否是连接错误
      if (error.errMsg && error.errMsg.includes('no connection')) {
        console.log('Attempting to reconnect...')
        
        try {
          // 尝试重新连接
          await createBLEConnection(deviceId)
          console.log('Reconnected successfully, resuming send from chunk', currentChunkIndex + 1)
          
          // 延迟一段时间，确保连接稳定
          await delay(1000)
        } catch (reconnectError) {
          console.error('Failed to reconnect:', reconnectError)
          throw new Error('连接断开且重连失败，请重新手动连接设备')
        }
      } else {
        // 其他错误，直接抛出
        throw error
      }
    }
  }
}
