// 压缩测试工具
import { compressText, decompress } from './kml-sender';

/**
 * 测试压缩算法
 * @param testContent 测试内容
 * @returns 测试结果
 */
export function testCompression(testContent: string): {
  success: boolean;
  originalSize: number;
  compressedSize: number;
  decompressedSize: number;
  compressionRatio: number;
  error?: string;
} {
  try {
    console.log('开始测试压缩算法...');
    console.log('原始内容大小:', testContent.length, '字节');
    
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
    const decompressedContent = new TextDecoder('utf-8').decode(decompressedArray);
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
          console.log(`解压缩: '${decompressedContent[i]}' (ASCII: ${decompressedContent.charCodeAt(i)})`);
          break;
        }
      }
    }
    
    return {
      success,
      originalSize: testContent.length,
      compressedSize,
      decompressedSize,
      compressionRatio: parseFloat(compressionRatio)
    };
  } catch (error) {
    console.error('测试过程中出现错误:', error);
    return {
      success: false,
      originalSize: testContent.length,
      compressedSize: 0,
      decompressedSize: 0,
      compressionRatio: 0,
      error: error instanceof Error ? error.message : String(error)
    };
  }
}

/**
 * 生成测试KML内容
 * @returns 测试KML内容
 */
export function generateTestKml(): string {
  return `<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
  <Document>
    <name>Test KML File</name>
    <description>This is a test KML file for compression testing</description>
    
    <Style id="pointStyle">
      <IconStyle>
        <Icon>
          <href>http://maps.google.com/mapfiles/kml/pushpin/red-pushpin.png</href>
        </Icon>
      </IconStyle>
    </Style>
    
    <Placemark>
      <name>Point 1</name>
      <description>First test point</description>
      <styleUrl>#pointStyle</styleUrl>
      <Point>
        <coordinates>116.404,39.915,0</coordinates>
      </Point>
    </Placemark>
    
    <Placemark>
      <name>Line String</name>
      <description>Test line string with multiple points</description>
      <LineString>
        <coordinates>
          116.404,39.915,0
          116.414,39.925,0
          116.424,39.935,0
        </coordinates>
      </LineString>
    </Placemark>
  </Document>
</kml>`;
}

/**
 * 运行完整的压缩测试
 */
export function runFullCompressionTest(): void {
  const testKml = generateTestKml();
  const result = testCompression(testKml);
  
  console.log('\n测试结果总结:');
  console.log('成功:', result.success);
  console.log('原始大小:', result.originalSize, '字节');
  console.log('压缩后大小:', result.compressedSize, '字节');
  console.log('解压缩后大小:', result.decompressedSize, '字节');
  console.log('压缩率:', result.compressionRatio.toFixed(2), '%');
  
  if (result.success) {
    if (result.compressionRatio >= 50) {
      console.log('✓ 测试通过：解压缩成功且压缩率达到要求（≥50%）');
    } else {
      console.log('✗ 测试失败：解压缩成功但压缩率未达到要求（<50%）');
    }
  } else {
    console.log('✗ 测试失败：解压缩失败');
    if (result.error) {
      console.log('错误信息:', result.error);
    }
  }
}
