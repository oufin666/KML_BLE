// 简单的测试脚本，用于验证解压缩功能
import { compressText, decompress } from './miniprogram/utils/kml-sender';

// 测试KML内容
const testKml = `<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
  <Document>
    <name>Test KML File</name>
    <Placemark>
      <name>Point 1</name>
      <Point>
        <coordinates>116.404,39.915,0</coordinates>
      </Point>
    </Placemark>
  </Document>
</kml>`;

console.log('测试解压缩功能...');
console.log('原始KML大小:', testKml.length, '字节');

// 压缩
const compressed = compressText(testKml);
console.log('压缩后大小:', compressed.byteLength, '字节');

// 统计压缩文件中0xfe的个数
const compressedView = new Uint8Array(compressed);
let compressedFeCount = 0;
for (let i = 0; i < compressedView.length; i++) {
  if (compressedView[i] === 0xfe) {
    compressedFeCount++;
  }
}

// 统计原文件中0xfe的个数
const originalView = new TextEncoder().encode(testKml);
let originalFeCount = 0;
for (let i = 0; i < originalView.length; i++) {
  if (originalView[i] === 0xfe) {
    originalFeCount++;
  }
}

// 解压缩
const decompressed = decompress(compressed);
const decompressedText = new TextDecoder('utf-8').decode(new Uint8Array(decompressed));
console.log('解压缩后大小:', decompressedText.length, '字节');

// 验证解压缩结果
console.log('\n原始文件前50个字符:', testKml.substring(0, 50));
console.log('解压缩后前50个字符:', decompressedText.substring(0, 50));

if (testKml === decompressedText) {
  console.log('\n✓ 解压缩成功，与原始文件完全一致');
} else {
  console.log('\n✗ 解压缩失败，与原始文件不一致');
  
  // 找出第一个不同的字符
  for (let i = 0; i < Math.min(testKml.length, decompressedText.length); i++) {
    if (testKml[i] !== decompressedText[i]) {
      console.log(`第一个不同的字符在位置 ${i}:`);
      console.log(`原始: '${testKml[i]}' (ASCII: ${testKml.charCodeAt(i)})`);
      console.log(`解压缩: '${decompressedText[i]}' (ASCII: ${decompressedText.charCodeAt(i)})`);
      break;
    }
  }
}

// 输出0xfe个数
console.log('\n压缩文件0xfe的个数:', compressedFeCount);
console.log('原文件0xfe的个数:', originalFeCount);

// 计算压缩率
const compressionRatio = ((1 - compressed.byteLength / testKml.length) * 100).toFixed(2);
console.log('\n压缩率:', compressionRatio, '%');
if (parseFloat(compressionRatio) >= 50) {
  console.log('✓ 压缩率达到要求（≥50%）');
} else {
  console.log('✗ 压缩率未达到要求（<50%）');
}