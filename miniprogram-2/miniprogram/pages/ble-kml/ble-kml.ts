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
  readFileAsArrayBuffer,
  readKmlFileWithCRC,
  readFileWithCRC,
  sendKmlBufferToBle,
  verifyChunkedPayload,
  calculateCRC32,
  SendProgress,
  compressText,
  decompress,
  convertKmlToBin,
  convertFileToBin,
  compressBinFile,
  compareBuffers
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
    kmlFile: null as { path: string; name: string; size: number; type: 'kml' | 'gpx' } | null,

    progress: 0,
    progressText: '',
    speedText: '',
    timeLeftText: '',
    errorMsg: '',
    verifyResult: null as string | null,
    compress: true, // 是否启用转换
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
    /** 选择 KML 或 GPX 文件 */
    async onChooseKml() {
      try {
        const res = await new Promise<WechatMiniprogram.ChooseMessageFileSuccessCallbackResult>(
          (resolve, reject) => {
            wx.chooseMessageFile({
              count: 1,
              type: 'file',
              extension: ['kml', 'gpx', 'txt'],
              success: resolve,
              fail: reject
            })
          }
        )
        const file = res.tempFiles[0]
        // 确定文件类型
        const fileName = file.name.toLowerCase()
        let fileType: 'kml' | 'gpx' = 'kml'
        if (fileName.endsWith('.gpx')) {
          fileType = 'gpx'
        }
        this.setData({
          kmlFile: { path: file.path, name: file.name, size: file.size, type: fileType },
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

    /** 处理转换选项变更 */
    onCompressChange(e: WechatMiniprogram.ChangeEvent) {
      this.setData({ compress: e.detail.value.includes('compress') })
    },

    /** 计算CRC32 */
  async onCalculateCRC() {
    console.log('===== onCalculateCRC called =====')
    const { kmlFile } = this.data
    if (!kmlFile) {
      wx.showToast({ title: '请先选择文件', icon: 'none' })
      return
    }

    try {
      wx.showLoading({ title: '计算CRC中...' })
      console.log('Reading file for CRC calculation...')
      
      // 计算压缩前的CRC32
      const originalBuffer = await readFileAsArrayBuffer(kmlFile.path, false, kmlFile.type)
      const originalCRC32 = calculateCRC32(originalBuffer)
      const originalCRC32Hex = originalCRC32.toString(16).toUpperCase().padStart(8, '0')
      console.log('Original CRC32:', originalCRC32Hex)
      
      // 计算压缩后的CRC32
      const compressedBuffer = await readFileAsArrayBuffer(kmlFile.path, true, kmlFile.type)
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

  /** 测试转换功能 */
  async onTestConvert() {
    console.log('===== 开始测试转换 =====')
    
    try {
      const { kmlFile } = this.data
      
      // 检查是否有选择的文件
      if (!kmlFile) {
        wx.showToast({ title: '请先选择文件', icon: 'none' })
        return
      }
      
      console.log('\n测试文件:', kmlFile.name)
      console.log('文件类型:', kmlFile.type)
      
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
      
      // 步骤1：将文件转换为bin文件（不压缩）
      console.log(`\n步骤1：将${kmlFile.type.toUpperCase()}转换为bin文件（不压缩）`)
      const binBuffer = convertFileToBin(fileContent, kmlFile.type)
      console.log('转换后bin文件大小:', binBuffer.byteLength, '字节')
      
      // 验证转换后的数据格式
      const binView = new Uint8Array(binBuffer)
      console.log('转换后bin文件前16字节:', Array.from(binView.slice(0, 16)).map(b => b.toString(16).padStart(2, '0')).join(' '))
      
      // 步骤2：验证转换后bin文件的正确性
      console.log('\n步骤2：验证转换后bin文件的正确性')
      const convertSuccess = this._validateBinFile(binBuffer)
      console.log('转换是否成功:', convertSuccess ? '✓' : '✗')
      
      // 步骤3：对bin文件进行压缩
      console.log('\n步骤3：对bin文件进行压缩')
      const compressedBuffer = compressBinFile(binBuffer)
      console.log('压缩后文件大小:', compressedBuffer.byteLength, '字节')
      console.log('压缩率:', ((1 - compressedBuffer.byteLength / binBuffer.byteLength) * 100).toFixed(2) + '%')
      
      // 验证压缩后的数据格式
      const compressedView = new Uint8Array(compressedBuffer)
      console.log('压缩后文件前16字节:', Array.from(compressedView.slice(0, 16)).map(b => b.toString(16).padStart(2, '0')).join(' '))
      
      // 步骤4：解压压缩后的文件
      console.log('\n步骤4：解压压缩后的文件')
      const decompressedBuffer = decompress(compressedBuffer)
      console.log('解压后文件大小:', decompressedBuffer.byteLength, '字节')
      
      // 步骤5：验证解压后的文件与转换后的bin文件是否一致
      console.log('\n步骤5：验证解压后的文件与转换后的bin文件是否一致')
      const isSame = compareBuffers(binBuffer, decompressedBuffer)
      console.log('解压后与转换后文件是否一致:', isSame)
      
      // 验证解压是否成功
      const decompressSuccess = isSame
      console.log('解压是否成功:', decompressSuccess ? '✓' : '✗')
      
      if (convertSuccess && decompressSuccess) {
        console.log('🎉 测试通过！转换和压缩算法正确。')
        wx.showToast({ title: '测试通过！转换和压缩算法正确', icon: 'success' })
      } else {
        console.log('❌ 测试失败！')
        if (!convertSuccess) {
          console.log('转换失败：转换后文件格式不正确')
          wx.showToast({ title: '测试失败！转换后文件格式不正确', icon: 'none' })
        } else if (!decompressSuccess) {
          console.log('解压失败：解压后文件与转换后文件不一致')
          wx.showToast({ title: '测试失败！解压后文件与转换后文件不一致', icon: 'none' })
        } else {
          console.log('测试失败：未知错误')
          wx.showToast({ title: '测试失败！未知错误', icon: 'none' })
        }
      }
      
    } catch (err: unknown) {
      const msg = this._errMsg(err)
      wx.showToast({ title: msg, icon: 'none' })
      console.error('转换测试失败:', err)
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
        wx.showToast({ title: '请先选择文件', icon: 'none' })
        return
      }
      try {
        const buffer = await readFileAsArrayBuffer(kmlFile.path, true, kmlFile.type)
        const result = verifyChunkedPayload(buffer)

        const lines = [
          result.ok ? '✅ 校验通过' : '❌ 校验失败',
          '',
          `总字节: ${result.totalBytes}（${kmlFile.type.toUpperCase()} ${buffer.byteLength} + 结束符 1）`,
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
        const { buffer, crc32 } = await readFileWithCRC(kmlFile.path, compress, kmlFile.type)
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

        // 读取文件内容并计算CRC32
        console.log('Reading file with CRC32 calculation for simulation...')
        const { buffer, crc32 } = await readFileWithCRC(kmlFile.path, compress, kmlFile.type)
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

    /**
     * 验证bin文件的正确性
     */
    _validateBinFile(binBuffer: ArrayBuffer): boolean {
      try {
        const view = new Uint8Array(binBuffer)
        let offset = 0
        
        // 检查文件大小是否至少包含总段数字段
        if (view.length < 4) {
          console.log('验证失败：文件太小，缺少总段数字段')
          return false
        }
        
        // 读取总段数
        const totalSegments = this._readUint32(view, offset)
        offset += 4
        
        if (totalSegments < 0) {
          console.log('验证失败：总段数无效')
          return false
        }
        
        // 检查每一段
        for (let i = 0; i < totalSegments; i++) {
          // 检查是否有足够的空间读取段点数
          if (offset + 4 > view.length) {
            console.log('验证失败：文件格式错误，缺少段点数字段')
            return false
          }
          
          // 读取段点数
          const pointCount = this._readUint32(view, offset)
          offset += 4
          
          if (pointCount < 0) {
            console.log('验证失败：段点数无效')
            return false
          }
          
          // 检查是否有足够的空间读取点数据
          const expectedPointDataSize = pointCount * 12 // 每个点12字节
          if (offset + expectedPointDataSize > view.length) {
            console.log('验证失败：文件格式错误，缺少点数据')
            return false
          }
          
          offset += expectedPointDataSize
        }
        
        // 检查是否读取完整个文件
        if (offset !== view.length) {
          console.log('验证失败：文件大小与计算的大小不一致')
          return false
        }
        
        return true
      } catch (error) {
        console.log('验证失败：', error)
        return false
      }
    },

    /**
     * 读取Uint32数据（little-endian）
     */
    _readUint32(buffer: Uint8Array, offset: number): number {
      const view = new DataView(new ArrayBuffer(4))
      for (let i = 0; i < 4; i++) {
        view.setUint8(i, buffer[offset + i])
      }
      return view.getUint32(0, true)
    }

  }
})
