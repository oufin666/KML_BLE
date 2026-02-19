// XML声明测试
const fs = require('fs');
const path = require('path');

// 读取kml-sender.ts文件内容
const kmlSenderPath = path.join(__dirname, 'miniprogram', 'utils', 'kml-sender.ts');
const kmlSenderContent = fs.readFileSync(kmlSenderPath, 'utf8');

// 检查XML声明的标签映射
console.log('检查XML声明的标签映射...');

// 查找压缩函数中的XML声明映射
const compressionXmlMatch = kmlSenderContent.match(/'\?xml version="1\.0" encoding="UTF-8"\?>'\s*:\s*0x([0-9a-fA-F]+)/);
const compressionXmlStandaloneMatch = kmlSenderContent.match(/'\?xml version="1\.0" encoding="UTF-8" standalone="yes"\?>'\s*:\s*0x([0-9a-fA-F]+)/);

console.log('压缩函数中的XML声明映射:');
console.log('<?xml version="1.0" encoding="UTF-8"?> -> 0x' + (compressionXmlMatch ? compressionXmlMatch[1] : '未找到'));
console.log('<?xml version="1.0" encoding="UTF-8" standalone="yes"?> -> 0x' + (compressionXmlStandaloneMatch ? compressionXmlStandaloneMatch[1] : '未找到'));

// 查找解压缩函数中的XML声明映射
const decompressionXmlMatch = kmlSenderContent.match(/0x([0-9a-fA-F]+)\s*:\s*'\?xml version="1\.0" encoding="UTF-8"\?>'/);
const decompressionXmlStandaloneMatch = kmlSenderContent.match(/0x([0-9a-fA-F]+)\s*:\s*'\?xml version="1\.0" encoding="UTF-8" standalone="yes"\?>'/);

console.log('\n解压缩函数中的XML声明映射:');
console.log('0x' + (decompressionXmlMatch ? decompressionXmlMatch[1] : '未找到') + ' -> <?xml version="1.0" encoding="UTF-8"?>');
console.log('0x' + (decompressionXmlStandaloneMatch ? decompressionXmlStandaloneMatch[1] : '未找到') + ' -> <?xml version="1.0" encoding="UTF-8" standalone="yes"?>');

// 检查标签映射是否一致
if (compressionXmlMatch && decompressionXmlMatch) {
  console.log('\nXML声明映射一致性检查:');
  console.log('压缩映射代码:', '0x' + compressionXmlMatch[1]);
  console.log('解压缩映射代码:', '0x' + decompressionXmlMatch[1]);
  console.log('映射一致:', compressionXmlMatch[1] === decompressionXmlMatch[1] ? '是' : '否');
}

if (compressionXmlStandaloneMatch && decompressionXmlStandaloneMatch) {
  console.log('\nXML声明(带standalone)映射一致性检查:');
  console.log('压缩映射代码:', '0x' + compressionXmlStandaloneMatch[1]);
  console.log('解压缩映射代码:', '0x' + decompressionXmlStandaloneMatch[1]);
  console.log('映射一致:', compressionXmlStandaloneMatch[1] === decompressionXmlStandaloneMatch[1] ? '是' : '否');
}

// 检查是否有重复的标签映射
console.log('\n检查是否有重复的标签映射...');

// 提取所有标签映射
const tagMapMatches = kmlSenderContent.match(/'([^']+)'\s*:\s*0x([0-9a-fA-F]+)/g);
const codeMapMatches = kmlSenderContent.match(/0x([0-9a-fA-F]+)\s*:\s*'([^']+)'/g);

if (tagMapMatches) {
  const tagMap = {};
  let hasDuplicateTags = false;
  
  for (const match of tagMapMatches) {
    const [, tag, code] = match.match(/'([^']+)'\s*:\s*0x([0-9a-fA-F]+)/);
    if (tagMap[tag]) {
      console.log('重复的标签映射:', tag, '-> 0x' + code, '(已存在: 0x' + tagMap[tag] + ')');
      hasDuplicateTags = true;
    } else {
      tagMap[tag] = code;
    }
  }
  
  if (!hasDuplicateTags) {
    console.log('压缩函数中没有重复的标签映射');
  }
}

if (codeMapMatches) {
  const codeMap = {};
  let hasDuplicateCodes = false;
  let hasDuplicateTags = false;
  const tagCounts = {};
  
  for (const match of codeMapMatches) {
    const [, code, tag] = match.match(/0x([0-9a-fA-F]+)\s*:\s*'([^']+)'/);
    if (codeMap[code]) {
      console.log('重复的代码映射:', '0x' + code, '->', tag, '(已存在:', codeMap[code], ')');
      hasDuplicateCodes = true;
    } else {
      codeMap[code] = tag;
    }
    
    // 检查重复的标签
    if (tagCounts[tag]) {
      tagCounts[tag]++;
      hasDuplicateTags = true;
    } else {
      tagCounts[tag] = 1;
    }
  }
  
  if (!hasDuplicateCodes) {
    console.log('解压缩函数中没有重复的代码映射');
  }
  
  if (hasDuplicateTags) {
    console.log('\n解压缩函数中重复的标签:');
    for (const [tag, count] of Object.entries(tagCounts)) {
      if (count > 1) {
        console.log(tag, ':', count, '次');
      }
    }
  } else {
    console.log('解压缩函数中没有重复的标签');
  }
}

console.log('\n测试完成!');
