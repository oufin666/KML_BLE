// 简单测试脚本
const fs = require('fs');
const path = require('path');

// 模拟微信小程序环境中的Uint8Array和ArrayBuffer
if (typeof Uint8Array === 'undefined') {
  global.Uint8Array = require('typedarray').Uint8Array;
}

if (typeof ArrayBuffer === 'undefined') {
  global.ArrayBuffer = require('typedarray').ArrayBuffer;
}

// 读取kml-sender.ts文件内容并转换为JavaScript
const kmlSenderPath = path.join(__dirname, 'miniprogram', 'utils', 'kml-sender.ts');
const kmlSenderContent = fs.readFileSync(kmlSenderPath, 'utf8');

// 简单的TypeScript到JavaScript转换
let jsContent = kmlSenderContent
  .replace(/export function/g, 'function')
  .replace(/export const/g, 'const')
  .replace(/: ArrayBuffer/g, '')
  .replace(/: string/g, '')
  .replace(/: number/g, '')
  .replace(/: boolean/g, '')
  .replace(/: any/g, '')
  .replace(/: { [^}]+ }/g, '')
  .replace(/=> ArrayBuffer/g, '')
  .replace(/=> string/g, '')
  .replace(/=> number/g, '')
  .replace(/=> boolean/g, '')
  .replace(/=> any/g, '')
  .replace(/=> { [^}]+ }/g, '')
  .replace(/\/\*\*[\s\S]*?\*\//g, '') // 删除JSDoc注释
  .replace(/\/\/.*$/gm, ''); // 删除单行注释

// 写入临时JavaScript文件
const tempJsPath = path.join(__dirname, 'temp-kml-sender.js');
fs.writeFileSync(tempJsPath, jsContent);

// 导入函数
const { compressText, decompress, textToUtf8, utf8ToText } = require('./temp-kml-sender');

// 测试内容
const testContent = `<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
  <Document>
    <name>Test KML</name>
    <description>This is a test KML file</description>
    <Placemark>
      <name>Point 1</name>
      <Point>
        <coordinates>116.404,39.915,0</coordinates>
      </Point>
    </Placemark>
  </Document>
</kml>`;

console.log('开始测试压缩和解压缩功能...');
console.log('原始内容大小:', testContent.length, '字节');

try {
  // 压缩
  const compressedBuffer = compressText(testContent);
  const compressedSize = compressedBuffer.byteLength;
  console.log('压缩后大小:', compressedSize, '字节');
  
  // 计算压缩率
  const compressionRatio = ((1 - compressedSize / testContent.length) * 100).toFixed(2);
  console.log('压缩率:', compressionRatio, '%');
  
  // 解压缩
  const decompressedBuffer = decompress(compressedBuffer);
  const decompressedArray = new Uint8Array(decompressedBuffer);
  
  // 将Uint8Array转换为字符串
  let decompressedContent = '';
  for (let i = 0; i < decompressedArray.length; i++) {
    decompressedContent += String.fromCharCode(decompressedArray[i]);
  }
  
  const decompressedSize = decompressedContent.length;
  console.log('解压缩后大小:', decompressedSize, '字节');
  
  // 验证解压缩结果
  const success = testContent === decompressedContent;
  console.log('解压缩结果:', success ? '成功' : '失败');
  
  if (!success) {
    // 找出第一个不同的字符
    for (let i = 0; i < Math.min(testContent.length, decompressedContent.length); i++) {
      if (testContent[i] !== decompressedContent[i]) {
        console.log(`第一个不同的字符在位置 ${i}:`);
        console.log(`原始: '${testContent[i]}' (ASCII: ${testContent.charCodeAt(i)})`);
        console.log(`解压缩: '${decompressedContent[i]}' (ASCII: ${decompressedArray[i]})`);
        break;
      }
    }
  }
  
  console.log('\n测试结果总结:');
  console.log('成功:', success);
  console.log('压缩率:', compressionRatio, '%');
  
  if (success) {
    if (parseFloat(compressionRatio) >= 50) {
      console.log('✓ 测试通过：解压缩成功且压缩率达到要求（≥50%）');
    } else {
      console.log('✗ 测试失败：解压缩成功但压缩率未达到要求（<50%）');
    }
  } else {
    console.log('✗ 测试失败：解压缩失败');
  }
  
} catch (error) {
  console.error('测试过程中出现错误:', error);
  console.error('错误堆栈:', error.stack);
} finally {
  // 清理临时文件
  if (fs.existsSync(tempJsPath)) {
    fs.unlinkSync(tempJsPath);
  }
}
