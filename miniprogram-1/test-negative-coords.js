// 测试负数坐标的压缩和解压缩

// 模拟压缩坐标
function compressCoordinates(coordData) {
  const result = []
  
  // 坐标数据格式: "lon,lat,alt lon,lat,alt ..."
  const coords = coordData.trim().split(/\s+/)
  
  // 写入坐标数量
  result.push(0xfe) // 坐标数据标识
  result.push(coords.length & 0xff) // 低字节
  result.push((coords.length >> 8) & 0xff) // 高字节
  
  // 压缩每个坐标
  for (const coord of coords) {
    const parts = coord.split(',')
    if (parts.length >= 2) {
      const lon = parseFloat(parts[0])
      const lat = parseFloat(parts[1])
      
      console.log('原始坐标:', lon, lat)
      
      // 将经纬度转换为整数（乘以10^6，保留6位小数）
      const lonInt = Math.round(lon * 1000000)
      const latInt = Math.round(lat * 1000000)
      
      console.log('转换为整数:', lonInt, latInt)
      
      // 使用DataView来确保正确的字节顺序和符号处理
      const buffer = new ArrayBuffer(8)
      const view = new DataView(buffer)
      view.setInt32(0, lonInt, true) // true for little-endian
      view.setInt32(4, latInt, true) // true for little-endian
      
      const bytes = new Uint8Array(buffer)
      console.log('压缩后的字节:', Array.from(bytes).map(b => b.toString(16).padStart(2, '0')).join(' '))
      result.push(...bytes)
    }
  }
  
  return result
}

// 模拟解压缩坐标
function decompressCoordinates(data, offset) {
  let pos = offset
  const coords = []
  
  // 读取坐标数量
  const count = (data[pos + 1] & 0xff) | ((data[pos + 2] & 0xff) << 8)
  console.log('坐标数量:', count)
  pos += 3
  
  // 解压缩每个坐标
  for (let i = 0; i < count; i++) {
    // 使用DataView来确保正确的字节顺序和符号处理
    const buffer = new ArrayBuffer(8)
    const bytes = new Uint8Array(buffer)
    bytes.set(data.subarray(pos, pos + 8))
    
    console.log('读取的字节:', Array.from(bytes).map(b => b.toString(16).padStart(2, '0')).join(' '))
    
    const view = new DataView(buffer)
    
    const lonInt = view.getInt32(0, true) // true for little-endian
    const latInt = view.getInt32(4, true) // true for little-endian
    
    console.log('解压缩的整数:', lonInt, latInt)
    
    pos += 8
    
    // 转换回浮点数并保持精度
    const lon = lonInt / 1000000
    const lat = latInt / 1000000
    
    console.log('解压缩的坐标:', lon, lat)
    
    // 格式化坐标，确保精度和格式正确
    const formattedLon = lon.toFixed(6).replace(/(\.\d*?)0+$/, '$1').replace(/\.$/, '')
    const formattedLat = lat.toFixed(6).replace(/(\.\d*?)0+$/, '$1').replace(/\.$/, '')
    coords.push(`${formattedLon},${formattedLat},0`)
  }
  
  return {
    text: coords.join(' '),
    newOffset: pos
  }
}

// 测试负数坐标
console.log('===== 测试负数坐标 =====')
const testCoord = '22.398226,-61.0,0'
console.log('测试坐标:', testCoord)

// 压缩
const compressed = compressCoordinates(testCoord)
console.log('\n压缩后的数据:', compressed.map(b => b.toString(16).padStart(2, '0')).join(' '))

// 解压缩
const compressedArray = new Uint8Array(compressed)
const decompressed = decompressCoordinates(compressedArray, 0)
console.log('\n解压缩后的坐标:', decompressed.text)

// 验证
console.log('\n验证结果:', testCoord === decompressed.text ? '✓ 通过' : '✗ 失败')

