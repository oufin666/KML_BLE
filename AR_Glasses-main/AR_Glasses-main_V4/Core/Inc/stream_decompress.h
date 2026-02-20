/**
 * 流式解压缩模块头文件
 */

#ifndef __STREAM_DECOMPRESS_H
#define __STREAM_DECOMPRESS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 流式解压函数
 * 
 * @param compressed_file 压缩文件路径
 * @param decompressed_file 解压后文件路径
 * @return 0=成功, 其他=失败
 */
int stream_decompress_file(const char* compressed_file, const char* decompressed_file);

#ifdef __cplusplus
}
#endif

#endif /* __STREAM_DECOMPRESS_H */

