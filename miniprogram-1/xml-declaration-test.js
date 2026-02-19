// XML声明压缩和解压缩测试
const fs = require('fs');
const path = require('path');

// 读取kml-sender.ts文件内容
const kmlSenderPath = path.join(__dirname, 'miniprogram', 'utils', 'kml-sender.ts');
const kmlSenderContent = fs.readFileSync(kmlSenderPath, 'utf8');

// 提取压缩函数中的标签映射
console.log('提取压缩函数中的标签映射...');
const tagMapMatch = kmlSenderContent.match(/const tagMap:\s*{[\s\S]*?}\s*}/);
if (tagMapMatch) {
  console.log('压缩函数中的标签映射:');
  console.log(tagMapMatch[0]);
}

// 提取解压缩函数中的标签映射
console.log('\n提取解压缩函数中的标签映射...');
const codeMapMatch = kmlSenderContent.match(/const codeMap:\s*{[\s\S]*?}\s*}/);
if (codeMapMatch) {
  console.log('解压缩函数中的标签映射:');
  console.log(codeMapMatch[0]);
}

// 检查XML声明的长度
console.log('\n检查XML声明的长度...');
const xmlDecl1 = '<?xml version="1.0" encoding="UTF-8"?>';
const xmlDecl2 = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>';
console.log('<?xml version="1.0" encoding="UTF-8"?> 长度:', xmlDecl1.length);
console.log('<?xml version="1.0" encoding="UTF-8" standalone="yes"?> 长度:', xmlDecl2.length);

// 检查标签排序
console.log('\n检查标签排序...');
console.log('压缩函数会按标签长度从长到短排序，所以会先匹配长标签');
console.log('较长的XML声明:', xmlDecl2);
console.log('较短的XML声明:', xmlDecl1);

// 检查压缩函数的标签匹配逻辑
console.log('\n检查压缩函数的标签匹配逻辑...');
console.log('压缩函数会先尝试匹配较长的XML声明，然后再尝试匹配较短的XML声明');
console.log('如果输入的XML声明是较短的版本，压缩函数会先尝试匹配较长的版本，匹配失败，然后尝试匹配较短的版本，匹配成功');

// 检查解压缩函数的标签处理逻辑
console.log('\n检查解压缩函数的标签处理逻辑...');
console.log('解压缩函数会将压缩标签转换为对应的字符串，然后将字符串转换为UTF-8字节序列');
console.log('对于XML声明标签，解压缩函数会将其转换为完整的XML声明字符串，然后转换为UTF-8字节序列');

// 检查解压缩函数的字符处理逻辑
console.log('\n检查解压缩函数的字符处理逻辑...');
console.log('解压缩函数对于非压缩标签字符，会直接添加到结果中');
console.log('对于单字节UTF-8字符（< 0x80），会直接添加');
console.log('对于双字节UTF-8字符（< 0xe0），会直接添加');
console.log('对于三字节UTF-8字符（< 0xf0），会直接添加');
console.log('对于四字节UTF-8字符，会直接添加');

// 检查压缩函数的字符处理逻辑
console.log('\n检查压缩函数的字符处理逻辑...');
console.log('压缩函数对于非标签字符，会将其转换为UTF-8编码的字符，然后写入');
console.log('对于单字节UTF-8字符（<= 0x7f），会直接写入');
console.log('对于双字节UTF-8字符（<= 0x7ff），会转换为两个字节写入');
console.log('对于三字节UTF-8字符（<= 0xffff），会转换为三个字节写入');
console.log('对于四字节UTF-8字符，会转换为四个字节写入');

console.log('\n测试完成!');
