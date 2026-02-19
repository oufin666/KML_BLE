// 简单测试坐标压缩和解压缩
const testCoords = [
  '116.404,39.915,0',
  '22.398226,-61.0,0',
  '-122.5,37.5,0',
  '0,0,0'
];

console.log('===== 测试坐标压缩和解压缩 =====\n');

for (const coord of testCoords) {
  console.log('测试坐标:', coord);
  
  const parts = coord.split(',');
  const lon = parseFloat(parts[0]);
  const lat = parseFloat(parts[1]);
  
  console.log('  解析结果: lon =', lon, ', lat =', lat);
  
  // 转换为整数
  const lonInt = Math.round(lon * 1000000);
  const latInt = Math.round(lat * 1000000);
  
  console.log('  整数值: lonInt =', lonInt, ', latInt =', latInt);
  
  // 压缩
  const buffer = new ArrayBuffer(8);
  const view = new DataView(buffer);
  view.setInt32(0, lonInt, true);
  view.setInt32(4, latInt, true);
  
  const bytes = new Uint8Array(buffer);
  console.log('  压缩字节:', Array.from(bytes).map(b => '0x' + b.toString(16).padStart(2, '0')).join(' '));
  
  // 解压缩
  const buffer2 = new ArrayBuffer(8);
  const bytes2 = new Uint8Array(buffer2);
  bytes2.set(bytes);
  
  const view2 = new DataView(buffer2);
  const lonInt2 = view2.getInt32(0, true);
  const latInt2 = view2.getInt32(4, true);
  
  console.log('  解压整数: lonInt2 =', lonInt2, ', latInt2 =', latInt2);
  
  // 转换回浮点数
  const lon2 = lonInt2 / 1000000;
  const lat2 = latInt2 / 1000000;
  
  console.log('  解压坐标: lon2 =', lon2, ', lat2 =', lat2);
  
  // 格式化
  const formattedLon = lon2.toFixed(6).replace(/0+$/, '').replace(/\.$/, '');
  const formattedLat = lat2.toFixed(6).replace(/0+$/, '').replace(/\.$/, '');
  const result = `${formattedLon},${formattedLat},0`;
  
  console.log('  格式化结果:', result);
  console.log('  是否匹配:', coord === result ? '✓' : '✗');
  console.log('');
}

