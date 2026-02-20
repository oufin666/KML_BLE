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
 * PC端兼容的KML文件转换算法
 * 生成与PC端解析器兼容的二进制格式
 */
export function compressText(text: string): ArrayBuffer {
  // 解析KML文件，提取点和线数据
  const { points, lines } = parseKmlContent(text)
  
  // 生成二进制数据
  const binaryData = generateBinaryData(points, lines)
  
  return binaryData.buffer
}

/**
 * 解析KML内容，提取点和线数据
 */
function parseKmlContent(text: string): { points: { lon: number; lat: number; alt: number }[]; lines: { points: { lon: number; lat: number; alt: number }[] }[] } {
  const points: { lon: number; lat: number; alt: number }[] = []
  const lines: { points: { lon: number; lat: number; alt: number }[] }[] = []
  
  // 简单的KML解析，提取Point和LineString数据
  // 注意：这是一个简化的解析器，实际应用中可能需要更复杂的XML解析
  
  // 提取Point数据
  const pointRegex = /<Point[\s\S]*?<coordinates>([\s\S]*?)<\/coordinates>[\s\S]*?<\/Point>/g
  let match
  while ((match = pointRegex.exec(text)) !== null) {
    const coordData = match[1].trim()
    const point = parseCoordinate(coordData)
    if (point) {
      points.push(point)
    }
  }
  
  // 提取LineString数据
  const lineRegex = /<LineString[\s\S]*?<coordinates>([\s\S]*?)<\/coordinates>[\s\S]*?<\/LineString>/g
  while ((match = lineRegex.exec(text)) !== null) {
    const coordData = match[1].trim()
    const linePoints = parseCoordinates(coordData)
    if (linePoints.length > 0) {
      lines.push({ points: linePoints })
    }
  }
  
  return { points, lines }
}

/**
 * 解析单个坐标
 */
function parseCoordinate(coordData: string): { lon: number; lat: number; alt: number } | null {
  const parts = coordData.split(',')
  if (parts.length >= 2) {
    const lon = parseFloat(parts[0])
    const lat = parseFloat(parts[1])
    const alt = parts.length >= 3 ? parseFloat(parts[2]) : 0
    
    if (!isNaN(lon) && !isNaN(lat)) {
      return { lon, lat, alt: isNaN(alt) ? 0 : alt }
    }
  }
  return null
}

/**
 * 解析多个坐标
 */
function parseCoordinates(coordData: string): { lon: number; lat: number; alt: number }[] {
  const result: { lon: number; lat: number; alt: number }[] = []
  
  // 坐标数据格式: "lon,lat,alt lon,lat,alt ..."
  const coords = coordData.split(/\s+/).filter(c => c.length > 0)
  
  for (const coord of coords) {
    const point = parseCoordinate(coord)
    if (point) {
      result.push(point)
    }
  }
  
  return result
}

/**
 * 生成与PC端兼容的二进制数据
 * 格式：线数据（分段格式），与PC端解析器兼容
 */
function generateBinaryData(points: { lon: number; lat: number; alt: number }[], lines: { points: { lon: number; lat: number; alt: number }[] }[]): Uint8Array {
  // 计算总大小
  let totalSize = 0
  
  // 线数据大小：总段数(4字节) + 每段点数(4字节) + 每段点数据(每个点12字节)
  totalSize += 4 // 总段数
  for (const line of lines) {
    // 简单起见，每条LineString作为一个段
    totalSize += 4 // 段点数
    totalSize += line.points.length * 12 // 段点数据
  }
  
  // 分配缓冲区
  const buffer = new Uint8Array(totalSize)
  let offset = 0
  
  // 写入线数据（分段格式，与PC端解析器兼容）
  const totalSegments = lines.length
  writeUint32(buffer, offset, totalSegments)
  offset += 4
  
  for (const line of lines) {
    const pointCount = line.points.length
    writeUint32(buffer, offset, pointCount)
    offset += 4
    
    for (const point of line.points) {
      writeGeoPoint(buffer, offset, point)
      offset += 12
    }
  }
  
  return buffer
}

/**
 * 写入GeoPoint数据
 */
function writeGeoPoint(buffer: Uint8Array, offset: number, point: { lon: number; lat: number; alt: number }): void {
  // 使用Float32Array存储坐标
  const floatBuffer = new ArrayBuffer(12)
  const floatView = new Float32Array(floatBuffer)
  floatView[0] = point.lon
  floatView[1] = point.lat
  floatView[2] = point.alt
  
  // 复制到目标缓冲区
  const uint8View = new Uint8Array(floatBuffer)
  for (let i = 0; i < 12; i++) {
    buffer[offset + i] = uint8View[i]
  }
}

/**
 * 写入Uint32数据
 */
function writeUint32(buffer: Uint8Array, offset: number, value: number): void {
  const view = new DataView(new ArrayBuffer(4))
  view.setUint32(0, value, true) // little-endian
  const uint8View = new Uint8Array(view.buffer)
  for (let i = 0; i < 4; i++) {
    buffer[offset + i] = uint8View[i]
  }
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
