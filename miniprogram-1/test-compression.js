// 测试压缩和解压函数
const { compressText, decompress } = require('./miniprogram/utils/kml-sender');

// 测试KML数据
const testKml = `<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
  <Document>
    <Placemark>
      <name>Test Placemark</name>
      <description>This is a test placemark</description>
      <Point>
        <coordinates>116.4074,39.9042,0</coordinates>
      </Point>
    </Placemark>
  </Document>
</kml>`;

console.log('Original KML:');
console.log(testKml);
console.log('\nOriginal length:', testKml.length);

// 压缩数据
const compressedBuffer = compressText(testKml);
console.log('\nCompressed length:', compressedBuffer.byteLength);
console.log('Compression ratio:', ((1 - compressedBuffer.byteLength / testKml.length) * 100).toFixed(2) + '%');

// 解压数据
const decompressedBuffer = decompress(compressedBuffer);
const decompressedText = new TextDecoder('utf-8').decode(decompressedBuffer);
console.log('\nDecompressed KML:');
console.log(decompressedText);
console.log('\nDecompressed length:', decompressedText.length);

// 验证解压是否正确
const isCorrect = testKml === decompressedText;
console.log('\nDecompression result:', isCorrect ? 'SUCCESS' : 'FAILED');

if (!isCorrect) {
  console.log('\nDifferences:');
  for (let i = 0; i < Math.max(testKml.length, decompressedText.length); i++) {
    if (testKml[i] !== decompressedText[i]) {
      console.log(`Position ${i}: Original '${testKml[i]}' (${testKml.charCodeAt(i)}), Decompressed '${decompressedText[i]}' (${decompressedText.charCodeAt(i)})`);
    }
  }
}
