# BLE KML 发送小程序
通过蓝牙低功耗(BLE)发送KML文件的微信小程序，支持文件压缩、解压校对等功能。

## 项目概览
本项目提供了一套完整的KML文件通过BLE传输到硬件设备的解决方案，包括文件选择、压缩、分包发送、设备管理和数据校对等功能。

## 整体流程

### KML发送流程
1. **文件选择**：用户选择本地KML文件
2. **文件解析**：解析KML文件，提取坐标数据
3. **数据压缩**：使用差分压缩算法压缩坐标数据
4. **BLE连接**：扫描并连接目标BLE设备
5. **分包发送**：将压缩数据分包（230字节/包）发送
6. **发送完成**：发送结束符 `0xFF 0xFF 0xFF` 标记传输完成

### 解压校对流程
1. **文件选择**：用户选择Bin文件和对应的KML文件
2. **Bin解压**：解压Bin文件得到坐标数据
3. **KML解析**：解析KML文件得到原始坐标数据
4. **数据校对**：比较解压后的数据与原始数据
5. **结果展示**：显示校对结果，包括差异和CRC32校验值

## 功能
- **KML文件发送**：支持选择、压缩、分包发送，实时显示进度
- **BLE设备管理**：扫描、连接、自动重连
- **串口助手**：文本/十六进制模式，收发数据
- **Bin文件解压校对**：解压Bin文件并与KML文件校对

## 使用
### 发送KML
1. 选择KML文件
2. 扫描并连接BLE设备
3. 点击"发送KML"按钮

### 解压校对
1. 选择Bin文件
2. 选择KML文件（用于校对）
3. 点击"解压并校对"按钮

## 技术
- **压缩算法**：针对KML坐标数据优化的差分压缩算法
- **分包机制**：230字节/包，发送间隔20ms，适配MTU限制
- **结束符**：自动追加 `0xFF 0xFF 0xFF`
- **校对方式**：内容比较 + CRC32校验

## 架构说明

### 核心模块
1. **主页面** (`pages/ble-kml/`)：用户交互界面，包含文件选择、BLE控制和操作按钮
2. **BLE模块** (`utils/ble.ts`)：处理BLE设备扫描、连接和数据传输
3. **KML处理模块** (`utils/kml-sender.ts`)：负责KML文件解析、数据压缩和发送逻辑
4. **压缩测试模块** (`utils/compression-test.ts`)：用于测试压缩算法性能和准确性

### 函数调用关系
```
主页面
  ├── 选择KML文件 → kml-sender.ts:parseKmlFile()
  ├── 扫描设备 → ble.ts:startScan()
  ├── 连接设备 → ble.ts:connectDevice()
  ├── 发送KML → kml-sender.ts:sendKmlFile()
  │   ├── 压缩数据 → kml-sender.ts:compressData()
  │   ├── 分包发送 → ble.ts:sendData()
  │   └── 发送结束符 → ble.ts:sendData()
  └── 解压校对 → compression-test.ts:decompressAndVerify()
      ├── 解压Bin → compression-test.ts:decompressBin()
      ├── 解析KML → kml-sender.ts:parseKmlFile()
      └── 比较数据 → compression-test.ts:compareData()
```

## 项目结构
```
miniprogram-2/
├── miniprogram/
│   ├── pages/ble-kml/    # 主页面
│   └── utils/           # 工具模块
│       ├── ble.ts       # BLE通信模块
│       ├── kml-sender.ts # KML处理模块
│       └── compression-test.ts # 压缩测试模块
├── Readme/
│   ├── Readme.md        # 项目说明
│   └── 压缩算法说明.md    # 压缩算法详细说明
└── package.json         # 项目配置
```

## 关键函数说明

### BLE模块 (`ble.ts`)
- `startScan()`：开始扫描BLE设备
- `connectDevice(deviceId)`：连接指定BLE设备
- `sendData(data)`：发送数据到BLE设备
- `disconnectDevice()`：断开BLE连接

### KML处理模块 (`kml-sender.ts`)
- `parseKmlFile(file)`：解析KML文件，提取坐标数据
- `compressData(coordinates)`：压缩坐标数据
- `sendKmlFile(file, deviceId)`：发送KML文件到BLE设备

### 压缩测试模块 (`compression-test.ts`)
- `decompressBin(binFile)`：解压Bin文件
- `compareData(original, decompressed)`：比较原始数据和解压后数据
- `decompressAndVerify(binFile, kmlFile)`：解压并校对数据

## 注意事项
- 确保BLE设备在可扫描范围内
- 发送大文件时请保持小程序在前台运行
- 解压校对时需要同时提供Bin文件和对应的KML文件
- 蓝牙传输可能受环境干扰，如发送失败可重试
