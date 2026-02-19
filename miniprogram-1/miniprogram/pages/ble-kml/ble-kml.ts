// ble-kml.ts
// BLE KML 分包发送页面

import {
  openBluetoothAdapter,
  closeBluetoothAdapter,
  startBluetoothDevicesDiscovery,
  stopBluetoothDevicesDiscovery,
  createBLEConnection,
  closeBLEConnection,
  findWritableCharacteristic,
  writeBLECharacteristicValue,
  findNotifyCharacteristic,
  enableBLECharacteristicNotify,
  onBLECharacteristicValueChange,
  offBLECharacteristicValueChange
} from '../../utils/ble'
import {
  readKmlFileAsArrayBuffer,
  readKmlFileWithCRC,
  sendKmlBufferToBle,
  verifyChunkedPayload,
  calculateCRC32,
  decompress,
  SendProgress,
  compressText
} from '../../utils/kml-sender'

interface BLEDeviceInfo {
  deviceId: string
  name: string
}

Component({
  data: {
    status: 'idle' as 'idle' | 'scanning' | 'connecting' | 'connected' | 'sending' | 'done' | 'error',
    statusText: '',
    devices: [] as BLEDeviceInfo[],
    selectedDevice: null as BLEDeviceInfo | null,
    kmlFile: null as { path: string; name: string; size: number } | null,
    binFile: null as { path: string; name: string; size: number } | null,
    kmlFileForVerify: null as { path: string; name: string; size: number } | null,
    verifyResult2: null as string | null,
    progress: 0,
    progressText: '',
    speedText: '',
    timeLeftText: '',
    errorMsg: '',
    verifyResult: null as string | null,
    compress: true, // 是否启用压缩
    crcInfo: null as { original: string; compressed: string } | null, // CRC信息
    // 串口助手相关
    serialPort: {
      receiveData: '',
      sendData: '',
      receiveMode: 'text' as 'text' | 'hex',
      sendMode: 'text' as 'text' | 'hex',
      autoScroll: true
    }
  },

  lifetimes: {
    detached() {
      this._cleanup()
    }
  },

  methods: {
    /** 选择 KML 文件 */
    async onChooseKml() {
      try {
        const res = await new Promise<WechatMiniprogram.ChooseMessageFileSuccessCallbackResult>(
          (resolve, reject) => {
            wx.chooseMessageFile({
              count: 1,
              type: 'file',
              extension: ['kml', 'txt'],
              success: resolve,
              fail: reject
            })
          }
        )
        const file = res.tempFiles[0]
        this.setData({
          kmlFile: { path: file.path, name: file.name, size: file.size },
          errorMsg: ''
        })
        this.setData({ verifyResult: null })
        wx.showToast({ title: '已选择 ' + file.name, icon: 'none' })
      } catch (err: unknown) {
        const msg = err && typeof err === 'object' && 'errMsg' in err ? (err as { errMsg: string }).errMsg : '选择文件失败'
        this.setData({ errorMsg: msg })
        wx.showToast({ title: msg, icon: 'none' })
      }
    },

    /** 处理压缩选项变更 */
    onCompressChange(e: WechatMiniprogram.ChangeEvent) {
      this.setData({ compress: e.detail.value.includes('compress') })
    },

    /** 计算CRC32 */
  async onCalculateCRC() {
    console.log('===== onCalculateCRC called =====')
    const { kmlFile } = this.data
    if (!kmlFile) {
      wx.showToast({ title: '请先选择 KML 文件', icon: 'none' })
      return
    }

    try {
      wx.showLoading({ title: '计算CRC中...' })
      console.log('Reading file for CRC calculation...')
      
      // 计算压缩前的CRC32
      const originalBuffer = await readKmlFileAsArrayBuffer(kmlFile.path, false)
      const originalCRC32 = calculateCRC32(originalBuffer)
      const originalCRC32Hex = originalCRC32.toString(16).toUpperCase().padStart(8, '0')
      console.log('Original CRC32:', originalCRC32Hex)
      
      // 计算压缩后的CRC32
      const compressedBuffer = await readKmlFileAsArrayBuffer(kmlFile.path, true)
      const compressedCRC32 = calculateCRC32(compressedBuffer)
      const compressedCRC32Hex = compressedCRC32.toString(16).toUpperCase().padStart(8, '0')
      console.log('Compressed CRC32:', compressedCRC32Hex)
      
      // 更新CRC信息
      this.setData({
        crcInfo: {
          original: originalCRC32Hex,
          compressed: compressedCRC32Hex
        }
      })
      
      console.log('CRC information updated')
      wx.showToast({ title: 'CRC计算完成', icon: 'success' })
    } catch (err: unknown) {
      const msg = this._errMsg(err)
      wx.showToast({ title: msg, icon: 'none' })
      console.error('Error calculating CRC:', err)
    }
  },

  /** 测试解压缩功能 */
  async onTestDecompress() {
    console.log('===== 开始测试解压缩 =====')
    
    try {
      // 首先测试负数坐标
      console.log('\n测试负数坐标: 22.398226,-61.0')
      const testLon = 22.398226
      const testLat = -61.0
      
      const lonInt = Math.round(testLon * 1000000)
      const latInt = Math.round(testLat * 1000000)
      console.log('转换为整数: lonInt =', lonInt, ', latInt =', latInt)
      
      // 压缩
      const buffer = new ArrayBuffer(8)
      const view = new DataView(buffer)
      view.setInt32(0, lonInt, true)
      view.setInt32(4, latInt, true)
      
      // 解压缩
      const lonInt2 = view.getInt32(0, true)
      const latInt2 = view.getInt32(4, true)
      console.log('解压缩整数: lonInt2 =', lonInt2, ', latInt2 =', latInt2)
      
      const lon2 = lonInt2 / 1000000
      const lat2 = latInt2 / 1000000
      console.log('解压缩浮点: lon2 =', lon2, ', lat2 =', lat2)
      
      if (Math.abs(lon2 - testLon) < 0.000001 && Math.abs(lat2 - testLat) < 0.000001) {
        console.log('✓ 负数坐标测试通过')
      } else {
        console.log('✗ 负数坐标测试失败')
        wx.showToast({ title: '负数坐标测试失败', icon: 'none' })
        return
      }
      
      const { kmlFile } = this.data
      
      // 检查是否有选择的KML文件
      if (!kmlFile) {
        wx.showToast({ title: '请先选择KML文件', icon: 'none' })
        return
      }
      
      console.log('\n测试文件:', kmlFile.name)
      
      // 读取文件内容
      const fs = wx.getFileSystemManager()
      const fileContent = await new Promise<string>((resolve, reject) => {
        fs.readFile({
          filePath: kmlFile.path,
          encoding: 'utf8',
          success: (res) => resolve(res.data as string),
          fail: reject
        })
      })
      
      console.log('原始文件长度:', fileContent.length)
      console.log('原始文件前50字符:', fileContent.substring(0, 50))
      
      // 使用实际的压缩算法
            const compressedData = compressText(fileContent)
            console.log('压缩后数据长度:', compressedData.byteLength)
            console.log('压缩率:', ((1 - compressedData.byteLength / fileContent.length) * 100).toFixed(2) + '%')
            
            // 统计压缩文件中0xfe的个数
            const compressedView = new Uint8Array(compressedData)
            let compressedFeCount = 0
            for (let i = 0; i < compressedView.length; i++) {
              if (compressedView[i] === 0xfe) {
                compressedFeCount++
              }
            }
            
            // 统计原文件中0xfe的个数
            const originalBufferForFe = this._stringToArrayBuffer(fileContent)
            const originalView = new Uint8Array(originalBufferForFe)
            let originalFeCount = 0
            for (let i = 0; i < originalView.length; i++) {
              if (originalView[i] === 0xfe) {
                originalFeCount++
              }
            }
            
            console.log('压缩文件0xfe的个数:', compressedFeCount)
            console.log('原文件0xfe的个数:', originalFeCount)
      
      // 解压缩数据
      const decompressedBuffer = decompress(compressedData)
      console.log('解压缩成功，大小:', decompressedBuffer.byteLength)
      
      // 转换为字符串（手动实现，不依赖 TextDecoder）
      const decompressedContent = this._arrayBufferToString(decompressedBuffer)
      console.log('解压缩结果:')
      console.log(decompressedContent.substring(0, 200) + (decompressedContent.length > 200 ? '...' : ''))
      
      // 验证解压缩结果
      console.log('\n=== 验证结果 ===')
      console.log('原始文件长度:', fileContent.length)
      console.log('解压缩文件长度:', decompressedContent.length)
      console.log('内容是否一致:', decompressedContent === fileContent)
      
      // 输出内容比较（前50个字符）
      console.log('原始文件前50字符:', fileContent.substring(0, 50))
      console.log('解压缩文件前50字符:', decompressedContent.substring(0, 50))
      
      // 输出不同的位置
      if (decompressedContent !== fileContent) {
        for (let j = 0; j < Math.min(fileContent.length, decompressedContent.length); j++) {
          if (fileContent[j] !== decompressedContent[j]) {
            console.log('第一个不同位置:', j)
            console.log('原始字符:', fileContent[j], '(', fileContent.charCodeAt(j), ')')
            console.log('解压缩字符:', decompressedContent[j], '(', decompressedContent.charCodeAt(j), ')')
            // 输出更多上下文
            console.log('原始上下文:', fileContent.substring(Math.max(0, j-10), Math.min(fileContent.length, j+10)))
            console.log('解压缩上下文:', decompressedContent.substring(Math.max(0, j-10), Math.min(decompressedContent.length, j+10)))
            break
          }
        }
      }
      
      // 计算CRC32（使用手动实现的UTF-8编码）
      const originalBuffer = this._stringToArrayBuffer(fileContent)
      const originalCRC32 = calculateCRC32(originalBuffer)
      const decompressedCRC32 = calculateCRC32(decompressedBuffer)
      console.log('原始文件CRC32:', originalCRC32.toString(16).toUpperCase().padStart(8, '0'))
      console.log('解压缩文件CRC32:', decompressedCRC32.toString(16).toUpperCase().padStart(8, '0'))
      console.log('CRC32是否一致:', originalCRC32 === decompressedCRC32)
      
      // 验证结果
      if (decompressedContent === fileContent && originalCRC32 === decompressedCRC32) {
        console.log('🎉 测试通过！解压缩算法正确。')
        wx.showToast({ title: '测试通过！解压缩算法正确', icon: 'success' })
      } else if (originalCRC32 === decompressedCRC32) {
        console.log('⚠️  CRC32一致，但内容不一致，可能是换行符或空格差异')
        wx.showToast({ title: 'CRC32一致，但内容可能有格式差异', icon: 'none' })
      } else {
        console.log('❌ 测试失败！解压缩算法有问题。')
        wx.showToast({ title: '测试失败！解压缩算法有问题', icon: 'none' })
      }
      
    } catch (err: unknown) {
      const msg = this._errMsg(err)
      wx.showToast({ title: msg, icon: 'none' })
      console.error('解压缩测试失败:', err)
    }
  },
  
  /** 字符串转 ArrayBuffer（UTF-8） */
  _stringToArrayBuffer(str: string): ArrayBuffer {
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
  },

  /** ArrayBuffer 转字符串（UTF-8） */
  _arrayBufferToString(buffer: ArrayBuffer): string {
    const view = new Uint8Array(buffer)
    const result: string[] = []
    let i = 0
    
    while (i < view.length) {
      const byte = view[i]
      
      if (byte < 0x80) {
        // 单字节
        result.push(String.fromCharCode(byte))
        i++
      } else if (byte < 0xe0) {
        // 双字节
        if (i + 1 >= view.length) break
        const code = ((byte & 0x1f) << 6) | (view[i + 1] & 0x3f)
        result.push(String.fromCharCode(code))
        i += 2
      } else if (byte < 0xf0) {
        // 三字节
        if (i + 2 >= view.length) break
        const code = ((byte & 0x0f) << 12) | ((view[i + 1] & 0x3f) << 6) | (view[i + 2] & 0x3f)
        result.push(String.fromCharCode(code))
        i += 3
      } else {
        // 四字节
        if (i + 3 >= view.length) break
        const code = ((byte & 0x07) << 18) | ((view[i + 1] & 0x3f) << 12) | 
                    ((view[i + 2] & 0x3f) << 6) | (view[i + 3] & 0x3f)
        result.push(String.fromCharCode(code))
        i += 4
      }
    }
    
    return result.join('')
  },

  /** 初始化蓝牙并开始扫描 */
  async onScanStart() {
    try {
      console.log('onScanStart called')
      this.setData({
        status: 'scanning',
        statusText: '正在搜索 BLE 设备...',
        devices: [],
        selectedDevice: null,
        errorMsg: ''
      })
      console.log('Status set to scanning')
      await openBluetoothAdapter()
      console.log('Bluetooth adapter opened')
      await startBluetoothDevicesDiscovery()
      console.log('Bluetooth discovery started')

      const devicesMap = new Map<string, BLEDeviceInfo>()
      const onFound = (res: WechatMiniprogram.OnBluetoothDeviceFoundCallbackResult) => {
        res.devices.forEach((d) => {
          if (d.deviceId && !devicesMap.has(d.deviceId)) {
            devicesMap.set(d.deviceId, {
              deviceId: d.deviceId,
              name: d.name || d.localName || d.deviceId
            })
            this.setData({ devices: Array.from(devicesMap.values()) })
          }
        })
      }
      wx.onBluetoothDeviceFound(onFound)
      ;(this as unknown as { _onFound: typeof onFound })._onFound = onFound

      wx.showToast({ title: '正在搜索设备', icon: 'loading', duration: 10000 })
    } catch (err: unknown) {
      const msg = this._errMsg(err)
      this.setData({ status: 'error', statusText: '', errorMsg: msg })
      wx.showToast({ title: msg, icon: 'none' })
    }
  },

    /** 停止扫描 */
    async onScanStop() {
      try {
        await stopBluetoothDevicesDiscovery()
        wx.offBluetoothDeviceFound((this as unknown as { _onFound?: () => void })._onFound)
        this.setData({ status: 'idle', statusText: '已停止搜索' })
      } catch {
        this.setData({ status: 'idle', statusText: '' })
      }
    },

    /** 选择设备并连接 */
    async onSelectDevice(e: WechatMiniprogram.TouchEvent) {
      const ds = e.currentTarget && e.currentTarget.dataset ? e.currentTarget.dataset as { deviceId?: string; deviceName?: string } : {}
      if (!ds.deviceId) return
      const device: BLEDeviceInfo = {
        deviceId: ds.deviceId,
        name: ds.deviceName || ds.deviceId
      }

      try {
        this.setData({
          status: 'connecting',
          statusText: '正在连接...',
          selectedDevice: device,
          errorMsg: ''
        })
        await stopBluetoothDevicesDiscovery()
        wx.offBluetoothDeviceFound((this as unknown as { _onFound?: () => void })._onFound)
        await createBLEConnection(device.deviceId)
        
        // 启用通知以接收数据
        try {
          const notifyChar = await findNotifyCharacteristic(device.deviceId)
          await enableBLECharacteristicNotify(device.deviceId, notifyChar.serviceId, notifyChar.uuid)
          
          // 监听数据接收
          const onDataReceived = (res: WechatMiniprogram.OnBLECharacteristicValueChangeCallbackResult) => {
            if (res.deviceId === device.deviceId) {
              this._handleDataReceived(res.value)
            }
          }
          onBLECharacteristicValueChange(onDataReceived)
          ;(this as unknown as { _onDataReceived?: typeof onDataReceived })._onDataReceived = onDataReceived
        } catch (notifyErr) {
          console.log('启用通知失败，可能不支持接收数据:', notifyErr)
        }
        
        this.setData({
          status: 'connected',
          statusText: '已连接: ' + device.name
        })
        wx.showToast({ title: '连接成功', icon: 'success' })
      } catch (err: unknown) {
        const msg = this._errMsg(err)
        this.setData({
          status: 'error',
          statusText: '',
          errorMsg: msg
        })
        wx.showToast({ title: msg, icon: 'none' })
      }
    },

    /** 校验发送内容（不连设备，验证分包 + 结束符 0xFF 0xFF 0xFF） */
    async onVerifyPayload() {
      const { kmlFile } = this.data
      if (!kmlFile) {
        wx.showToast({ title: '请先选择 KML 文件', icon: 'none' })
        return
      }
      try {
        const buffer = await readKmlFileAsArrayBuffer(kmlFile.path)
        const result = verifyChunkedPayload(buffer)

        const lines = [
          result.ok ? '✅ 校验通过' : '❌ 校验失败',
          '',
          `总字节: ${result.totalBytes}（KML ${buffer.byteLength} + 结束符 1）`,
          `分包数: ${result.chunkCount}`,
          `末尾为 0xFF 0xFF 0xFF: ${result.endsWithEndMarker ? '是' : '否'}`,
          `末尾为 0x04: ${result.lastByteIs0x04 ? '是' : '否'}`, // 保持兼容
          '',
          '前 64 字节(hex):',
          result.headHex || '(空)',
          '',
          '最后 32 字节(hex):',
          result.tailHex || '(空)'
        ]
        this.setData({ verifyResult: lines.join('\n') })
        wx.showModal({
          title: result.ok ? '校验通过' : '校验失败',
          content: `总字节: ${result.totalBytes}\n分包数: ${result.chunkCount}\n末尾 0xFF 0xFF 0xFF: ${result.endsWithEndMarker ? '是' : '否'}\n末尾 0x04: ${result.lastByteIs0x04 ? '是' : '否'}（兼容显示）\n\n${result.ok ? '分包拼接结果与预期一致，可放心发送。' : result.message}`,
          showCancel: false
        })
      } catch (err: unknown) {
        const msg = this._errMsg(err)
        wx.showToast({ title: msg, icon: 'none' })
        this.setData({ verifyResult: null })
      }
    },

    /** 发送 KML */
    async onSendKml() {
      console.log('===== onSendKml called =====')
      const { kmlFile, selectedDevice, compress } = this.data
      console.log('Current state:', {
        kmlFile: !!kmlFile,
        selectedDevice: !!selectedDevice,
        compress
      })
      if (!kmlFile || !selectedDevice) {
        console.log('Missing required parameters:', {
          kmlFile: !!kmlFile,
          selectedDevice: !!selectedDevice
        })
        wx.showToast({
          title: !kmlFile ? '请先选择 KML 文件' : '请先连接 BLE 设备',
          icon: 'none'
        })
        return
      }
      console.log('All parameters present, starting send process')

      try {
        // 初始化发送计时
        const sendStartTime = Date.now()
        let lastProgressTime = sendStartTime
        let lastSentBytes = 0

        console.log('Setting status to sending')
        this.setData({
          status: 'sending',
          statusText: '正在发送...',
          progress: 0,
          progressText: '',
          speedText: '',
          timeLeftText: '',
          errorMsg: ''
        })
        console.log('Status set to sending')

        // 计算压缩前后的字节数并计算CRC32
        console.log('Reading file with CRC32 calculation...')
        const { buffer, crc32 } = await readKmlFileWithCRC(kmlFile.path, compress)
        const compressedSize = buffer.byteLength
        const crc32Hex = crc32.toString(16).toUpperCase().padStart(8, '0')
        
        console.log('File read complete. Size:', compressedSize, 'CRC32:', crc32Hex)
        
        // 输出压缩文件前20个字节的十六进制表示
        const view = new Uint8Array(buffer)
        const hexBytes = []
        for (let i = 0; i < Math.min(20, view.length); i++) {
          hexBytes.push(view[i].toString(16).padStart(2, '0'))
        }
        console.log('Compressed file first 20 bytes:', hexBytes.join(' '))
        
        // 先更新状态文本，确保CRC32显示
        await new Promise(resolve => {
          this.setData({
            statusText: compress ? `正在发送（压缩后 ${compressedSize.toLocaleString()} 字节，CRC32: ${crc32Hex}）` : `正在发送（CRC32: ${crc32Hex}）`
          }, resolve)
        })
        
        // 延迟一小段时间，确保UI有足够时间更新
        await new Promise(resolve => setTimeout(resolve, 1000))
        
        console.log('Starting send after status update')
        await sendKmlBufferToBle(buffer, {
          deviceId: selectedDevice.deviceId,
          chunkSize: 230, // 适配资料推荐值，确保与波特率115200匹配
          writeDelayMs: 20, // 适配资料推荐的发送间隔20ms
          onProgress: (p: SendProgress) => {
            console.log('onProgress called:', p)
            const now = Date.now()
            const elapsed = now - lastProgressTime
            const bytesSent = p.sentBytes - lastSentBytes
            
            // 计算速度（字节/秒）
            const speed = elapsed > 0 ? Math.round((bytesSent / elapsed) * 1000) : 0
            const speedKbps = (speed / 1024).toFixed(1)
            
            // 计算剩余时间
            const timeElapsedTotal = now - sendStartTime
            const estimatedTotalTime = p.percent > 0 ? Math.round((timeElapsedTotal / p.percent) * 100) : 0
            const timeLeft = Math.max(0, estimatedTotalTime - timeElapsedTotal)
            const timeLeftFormatted = this._formatTime(timeLeft)
            
            // 立即更新UI，移除延迟
            console.log('Updating progress:', p.percent)
            this.setData({
              progress: p.percent,
              progressText: `${p.sentBytes}/${p.totalBytes} 字节 (${p.percent}%)`,
              speedText: `速度: ${speedKbps} KB/s`,
              timeLeftText: `剩余: ${timeLeftFormatted}`
              // 不再更新状态文本，保持CRC32信息显示
            })
            console.log('Progress updated')
            
            // 更新上次记录
            lastProgressTime = now
            lastSentBytes = p.sentBytes
          }
        })

        this.setData({
          status: 'done',
          statusText: compress ? `发送完成（压缩后 ${compressedSize.toLocaleString()} 字节，CRC32: ${crc32Hex}，含结束符 0xFF 0xFF 0xFF）` : `发送完成（CRC32: ${crc32Hex}，含结束符 0xFF 0xFF 0xFF）`,
          progress: 100,
          speedText: '',
          timeLeftText: ''
        })
        wx.showToast({ title: '发送完成', icon: 'success' })
      } catch (err: unknown) {
        const msg = this._errMsg(err)
        this.setData({
          status: 'error',
          statusText: '',
          errorMsg: msg,
          speedText: '',
          timeLeftText: ''
        })
        wx.showToast({ title: msg, icon: 'none' })
      }
    },

    /** 模拟发送 KML（测试进度） */
    async onSimulateSendKml() {
      console.log('===== onSimulateSendKml called =====')
      const { kmlFile, compress } = this.data
      console.log('Current state:', {
        kmlFile: !!kmlFile,
        compress
      })
      if (!kmlFile) {
        console.log('Missing KML file, showing toast')
        wx.showToast({ title: '请先选择 KML 文件', icon: 'none' })
        return
      }
      console.log('All parameters present, starting simulate send process')

      try {
        // 初始化发送计时
        const sendStartTime = Date.now()
        let lastProgressTime = sendStartTime
        let lastSentBytes = 0

        console.log('Setting status to sending (simulated)')
        this.setData({
          status: 'sending',
          statusText: '正在发送（模拟）...',
          progress: 0,
          progressText: '',
          speedText: '',
          timeLeftText: '',
          errorMsg: ''
        })
        console.log('Status set to sending (simulated)')

        // 读取KML文件内容并计算CRC32
        console.log('Reading file with CRC32 calculation for simulation...')
        const { buffer, crc32 } = await readKmlFileWithCRC(kmlFile.path, compress)
        const compressedSize = buffer.byteLength
        const crc32Hex = crc32.toString(16).toUpperCase().padStart(8, '0')
        
        console.log('File read complete. Size:', compressedSize, 'CRC32:', crc32Hex)
        
        // 显示压缩信息和CRC32
        await new Promise(resolve => {
          this.setData({
            statusText: compress ? `正在发送（模拟，压缩后 ${compressedSize.toLocaleString()} 字节，CRC32: ${crc32Hex}）` : `正在发送（模拟，CRC32: ${crc32Hex}）`
          }, resolve)
        })
        
        // 延迟一小段时间，确保UI有足够时间更新
        await new Promise(resolve => setTimeout(resolve, 1000))
        
        console.log('Status text updated for simulation')
        console.log('Compressed size:', compressedSize)
        console.log('CRC32:', crc32Hex)
        console.log('Compression enabled:', compress)
        
        const totalBytes = buffer.byteLength + 1 // 加上结束符
        const chunkSize = 230 // 适配资料推荐值，确保与波特率115200匹配
        const totalChunks = Math.ceil(buffer.byteLength / chunkSize) + 1 // 加上结束符包
        
        console.log('Simulating send for', totalBytes, 'bytes in', totalChunks, 'chunks')

        // 模拟发送过程
        let sentBytes = 0
        for (let i = 0; i < totalChunks; i++) {
          // 计算当前包大小
          let chunkSizeCurrent = chunkSize
          if (i === totalChunks - 1) {
            // 最后一包是结束符
            chunkSizeCurrent = 1
          } else if (i === totalChunks - 2) {
            // 倒数第二包是剩余数据
            chunkSizeCurrent = buffer.byteLength - (i * chunkSize)
          }
          
          // 更新已发送字节数
          sentBytes += chunkSizeCurrent
          const percent = Math.min(100, Math.round((sentBytes / totalBytes) * 100))
          
          // 计算速度和剩余时间
          const now = Date.now()
          const elapsed = now - lastProgressTime
          const bytesSent = sentBytes - lastSentBytes
          const speed = elapsed > 0 ? Math.round((bytesSent / elapsed) * 1000) : 0
          const speedKbps = (speed / 1024).toFixed(1)
          
          const timeElapsedTotal = now - sendStartTime
          const estimatedTotalTime = percent > 0 ? Math.round((timeElapsedTotal / percent) * 100) : 0
          const timeLeft = Math.max(0, estimatedTotalTime - timeElapsedTotal)
          const timeLeftFormatted = this._formatTime(timeLeft)
          
          // 立即更新UI，只更新进度相关信息，保持状态文本不变
          console.log('Updating progress (simulated):', percent)
          this.setData({
            progress: percent,
            progressText: `${sentBytes}/${totalBytes} 字节 (${percent}%)`,
            speedText: `速度: ${speedKbps} KB/s`,
            timeLeftText: `剩余: ${timeLeftFormatted}`
            // 不再更新状态文本，保持CRC32信息显示
          })
          
          // 更新上次记录
          lastProgressTime = now
          lastSentBytes = sentBytes
          
          // 模拟发送延迟，与实际发送参数一致
          await new Promise(resolve => setTimeout(resolve, 20))
        }

        this.setData({
          status: 'done',
          statusText: compress ? `发送完成（压缩后 ${compressedSize.toLocaleString()} 字节，CRC32: ${crc32Hex}，含结束符 0xFF 0xFF 0xFF）` : `发送完成（CRC32: ${crc32Hex}，含结束符 0xFF 0xFF 0xFF）`,
          progress: 100,
          speedText: '',
          timeLeftText: ''
        })
        wx.showToast({ title: '模拟发送完成', icon: 'success' })
      } catch (err: unknown) {
        const msg = this._errMsg(err)
        this.setData({
          status: 'error',
          statusText: '',
          errorMsg: msg,
          speedText: '',
          timeLeftText: ''
        })
        wx.showToast({ title: msg, icon: 'none' })
      }
    },

    /** 格式化时间（毫秒转分:秒） */
    _formatTime(ms: number): string {
      const totalSeconds = Math.floor(ms / 1000)
      const minutes = Math.floor(totalSeconds / 60)
      const seconds = totalSeconds % 60
      return `${minutes}:${seconds.toString().padStart(2, '0')}`
    },

    /** 断开连接 */
    async onDisconnect() {
      console.log('onDisconnect called')
      const { selectedDevice } = this.data
      console.log('Selected device:', selectedDevice)
      if (selectedDevice) {
        try {
          console.log('Closing connection for device:', selectedDevice.deviceId)
          await closeBLEConnection(selectedDevice.deviceId)
          console.log('Connection closed')
        } catch (error) {
          console.log('Error closing connection:', error)
        }
      }
      console.log('Setting data to idle')
      this.setData({
        status: 'idle',
        statusText: '',
        selectedDevice: null,
        progress: 0,
        progressText: '',
        speedText: '',
        timeLeftText: ''
      })
      console.log('Data set to idle')
      console.log('Calling cleanup')
      this._cleanup()
      console.log('Cleanup completed')
      wx.showToast({ title: '已断开连接', icon: 'none' })
      console.log('Disconnect toast shown')
    },

    /** 处理接收到的数据 */
    _handleDataReceived(value: ArrayBuffer) {
      const { serialPort } = this.data
      const uint8Array = new Uint8Array(value)
      
      let dataStr = ''
      if (serialPort.receiveMode === 'hex') {
        // 转换为十六进制
        dataStr = Array.from(uint8Array)
          .map(b => b.toString(16).padStart(2, '0'))
          .join(' ')
      } else {
        // 转换为文本
        try {
          dataStr = new TextDecoder('utf-8').decode(uint8Array)
        } catch {
          dataStr = String.fromCharCode(...uint8Array)
        }
      }
      
      this.setData({
        'serialPort.receiveData': serialPort.receiveData + dataStr + '\n'
      })
    },

    /** 发送串口数据 */
    async onSendSerialData() {
      const { selectedDevice, serialPort } = this.data
      if (!selectedDevice) {
        wx.showToast({ title: '请先连接设备', icon: 'none' })
        return
      }
      
      try {
        const { sendData, sendMode } = serialPort
        if (!sendData.trim()) {
          wx.showToast({ title: '请输入发送数据', icon: 'none' })
          return
        }
        
        let buffer: ArrayBuffer
        if (sendMode === 'hex') {
          // 处理十六进制数据
          const hexStr = sendData.replace(/\s+/g, '')
          const length = hexStr.length
          if (length % 2 !== 0) {
            wx.showToast({ title: '十六进制数据长度必须为偶数', icon: 'none' })
            return
          }
          
          const uint8Array = new Uint8Array(length / 2)
          for (let i = 0; i < length; i += 2) {
            const byte = parseInt(hexStr.substr(i, 2), 16)
            if (isNaN(byte)) {
              wx.showToast({ title: '无效的十六进制数据', icon: 'none' })
              return
            }
            uint8Array[i / 2] = byte
          }
          buffer = uint8Array.buffer
        } else {
          // 处理文本数据
          buffer = new TextEncoder().encode(sendData).buffer
        }
        
        // 发送数据
        const writableChar = await findWritableCharacteristic(selectedDevice.deviceId)
        await writeBLECharacteristicValue(
          selectedDevice.deviceId,
          writableChar.serviceId,
          writableChar.uuid,
          buffer
        )
        
        wx.showToast({ title: '发送成功', icon: 'success' })
      } catch (err: unknown) {
        const msg = this._errMsg(err)
        this.setData({ errorMsg: msg })
        wx.showToast({ title: msg, icon: 'none' })
      }
    },

    /** 清空接收区 */
    onClearReceive() {
      this.setData({ 'serialPort.receiveData': '' })
    },

    /** 清空发送区 */
    onClearSend() {
      this.setData({ 'serialPort.sendData': '' })
    },

    /** 切换接收模式 */
    onReceiveModeChange(e: WechatMiniprogram.TouchEvent) {
      const mode = e.currentTarget && e.currentTarget.dataset ? e.currentTarget.dataset.mode as 'text' | 'hex' : ''
      if (mode) {
        this.setData({ 
          'serialPort.receiveMode': mode,
          'serialPort.receiveData': '' // 切换模式时清空接收区
        })
      }
    },

    /** 切换发送模式 */
    onSendModeChange(e: WechatMiniprogram.TouchEvent) {
      const mode = e.currentTarget && e.currentTarget.dataset ? e.currentTarget.dataset.mode as 'text' | 'hex' : ''
      if (mode) {
        this.setData({ 'serialPort.sendMode': mode })
      }
    },

    /** 处理发送数据输入 */
    onSendDataChange(e: WechatMiniprogram.InputEvent) {
      this.setData({ 'serialPort.sendData': e.detail.value })
    },

    _cleanup() {
      const self = this as unknown as { 
        _onFound?: () => void,
        _onDataReceived?: (res: WechatMiniprogram.OnBLECharacteristicValueChangeCallbackResult) => void
      }
      if (self._onFound) {
        wx.offBluetoothDeviceFound(self._onFound)
        self._onFound = undefined
      }
      if (self._onDataReceived) {
        offBLECharacteristicValueChange(self._onDataReceived)
        self._onDataReceived = undefined
      }
      stopBluetoothDevicesDiscovery().catch(() => {})
      // 注意：不要在这里访问selectedDevice，因为它可能已经被设置为null
      closeBluetoothAdapter().catch(() => {})
    },

    _errMsg(err: unknown): string {
      if (err && typeof err === 'object' && 'errMsg' in err) {
        return (err as { errMsg: string }).errMsg
      }
      return err instanceof Error ? err.message : '未知错误'
    },

    /** 选择 Bin 文件 */
    async onChooseBinFile() {
      try {
        const res = await new Promise<WechatMiniprogram.ChooseMessageFileSuccessCallbackResult>(
          (resolve, reject) => {
            wx.chooseMessageFile({
              count: 1,
              type: 'file',
              success: resolve,
              fail: reject
            })
          }
        )
        const file = res.tempFiles[0]
        console.log('选择的Bin文件:', file)
        this.setData({
          binFile: { path: file.path, name: file.name, size: file.size },
          errorMsg: ''
        })
        wx.showToast({ title: '已选择 ' + file.name + ' (' + (file.size / 1024).toFixed(2) + 'KB)', icon: 'none' })
      } catch (err: unknown) {
        const msg = this._errMsg(err)
        this.setData({ errorMsg: msg })
        wx.showToast({ title: msg, icon: 'none' })
      }
    },

    /** 选择用于校对的 KML 文件 */
    async onChooseKmlForVerify() {
      try {
        const res = await new Promise<WechatMiniprogram.ChooseMessageFileSuccessCallbackResult>(
          (resolve, reject) => {
            wx.chooseMessageFile({
              count: 1,
              type: 'file',
              extension: ['kml', 'txt'],
              success: resolve,
              fail: reject
            })
          }
        )
        const file = res.tempFiles[0]
        this.setData({
          kmlFileForVerify: { path: file.path, name: file.name, size: file.size },
          errorMsg: ''
        })
        wx.showToast({ title: '已选择 ' + file.name, icon: 'none' })
      } catch (err: unknown) {
        const msg = this._errMsg(err)
        this.setData({ errorMsg: msg })
        wx.showToast({ title: msg, icon: 'none' })
      }
    },

    /** 解压 Bin 文件并与 KML 文件校对 */
    async onDecompressAndVerify() {
      const { binFile, kmlFileForVerify } = this.data
      if (!binFile || !kmlFileForVerify) {
        wx.showToast({ title: '请先选择 Bin 文件和 KML 文件', icon: 'none' })
        return
      }

      try {
        wx.showLoading({ title: '解压并校对中...' })
        console.log('===== 开始解压并校对 =====')

        // 读取 Bin 文件内容
        const fs = wx.getFileSystemManager()
        const binContent = await new Promise<ArrayBuffer>((resolve, reject) => {
          fs.readFile({
            filePath: binFile.path,
            success: (res) => {
              console.log('读取Bin文件结果:', res)
              console.log('res.data类型:', typeof res.data)
              console.log('res.data.constructor.name:', res.data && res.data.constructor ? res.data.constructor.name : 'unknown')
              
              if (res.data && (res.data instanceof ArrayBuffer || Object.prototype.toString.call(res.data) === '[object ArrayBuffer]' || res.data.byteLength !== undefined)) {
                console.log('Bin文件是ArrayBuffer，大小:', res.data.byteLength)
                resolve(res.data)
              } else if (typeof res.data === 'string') {
                // 如果返回的是字符串，转换为 ArrayBuffer
                console.log('Bin文件是字符串，长度:', res.data.length)
                const bytes = []
                for (let i = 0; i < res.data.length; i++) {
                  bytes.push(res.data.charCodeAt(i) & 0xff)
                }
                const buffer = new Uint8Array(bytes).buffer
                console.log('转换后ArrayBuffer大小:', buffer.byteLength)
                resolve(buffer)
              } else {
                console.error('无法识别的文件数据类型:', res.data)
                reject(new Error('无法读取Bin文件内容'))
              }
            },
            fail: (err) => {
              console.error('读取Bin文件失败:', err)
              reject(err)
            }
          })
        })

        console.log('Bin 文件大小:', binContent.byteLength)

        // 解压数据
        const decompressedBuffer = decompress(binContent)
        console.log('解压成功，大小:', decompressedBuffer.byteLength)

        // 转换为字符串
        const decompressedContent = this._arrayBufferToString(decompressedBuffer)
        console.log('解压结果前50字符:', decompressedContent.substring(0, 50))

        // 读取 KML 文件内容
        const kmlContent = await new Promise<string>((resolve, reject) => {
          fs.readFile({
            filePath: kmlFileForVerify.path,
            encoding: 'utf8',
            success: (res) => resolve(res.data as string),
            fail: reject
          })
        })

        console.log('KML 文件长度:', kmlContent.length)
        console.log('KML 文件前50字符:', kmlContent.substring(0, 50))

        // 验证解压结果
        console.log('\n=== 验证结果 ===')
        console.log('KML 文件长度:', kmlContent.length)
        console.log('解压文件长度:', decompressedContent.length)
        console.log('内容是否一致:', decompressedContent === kmlContent)

        // 计算CRC32
        const kmlBuffer = this._stringToArrayBuffer(kmlContent)
        const kmlCRC32 = calculateCRC32(kmlBuffer)
        const decompressedCRC32 = calculateCRC32(decompressedBuffer)
        console.log('KML 文件CRC32:', kmlCRC32.toString(16).toUpperCase().padStart(8, '0'))
        console.log('解压文件CRC32:', decompressedCRC32.toString(16).toUpperCase().padStart(8, '0'))
        console.log('CRC32是否一致:', kmlCRC32 === decompressedCRC32)

        // 输出不同的位置
        let firstDiffPos = -1
        if (decompressedContent !== kmlContent) {
          for (let j = 0; j < Math.min(kmlContent.length, decompressedContent.length); j++) {
            if (kmlContent[j] !== decompressedContent[j]) {
              firstDiffPos = j
              console.log('第一个不同位置:', j)
              console.log('KML字符:', kmlContent[j], '(', kmlContent.charCodeAt(j), ')')
              console.log('解压字符:', decompressedContent[j], '(', decompressedContent.charCodeAt(j), ')')
              break
            }
          }
        }

        // 构建验证结果
        const lines = [
          decompressedContent === kmlContent && kmlCRC32 === decompressedCRC32 ? '✅ 校对通过' : '❌ 校对失败',
          '',
          `Bin 文件: ${binFile.name} (${binFile.size} 字节)`,
          `KML 文件: ${kmlFileForVerify.name} (${kmlFileForVerify.size} 字节)`,
          `解压后大小: ${decompressedContent.length} 字节`,
          `内容长度一致: ${kmlContent.length === decompressedContent.length ? '是' : '否'}`,
          `CRC32一致: ${kmlCRC32 === decompressedCRC32 ? '是' : '否'}`,
        ]

        if (firstDiffPos !== -1) {
          lines.push('')
          lines.push(`第一个不同位置: ${firstDiffPos}`)
        }

        this.setData({ verifyResult2: lines.join('\n') })

        if (decompressedContent === kmlContent && kmlCRC32 === decompressedCRC32) {
          console.log('🎉 校对通过！解压结果与KML文件一致。')
          wx.showToast({ title: '校对通过！解压结果与KML文件一致', icon: 'success' })
        } else if (kmlCRC32 === decompressedCRC32) {
          console.log('⚠️  CRC32一致，但内容不一致，可能是换行符或空格差异')
          wx.showToast({ title: 'CRC32一致，但内容可能有格式差异', icon: 'none' })
        } else {
          console.log('❌ 校对失败！解压结果与KML文件不一致。')
          wx.showToast({ title: '校对失败！解压结果与KML文件不一致', icon: 'none' })
        }

      } catch (err: unknown) {
        const msg = this._errMsg(err)
        this.setData({ errorMsg: msg })
        wx.showToast({ title: msg, icon: 'none' })
        console.error('解压校对失败:', err)
      } finally {
        wx.hideLoading()
      }
    }
  }
})
