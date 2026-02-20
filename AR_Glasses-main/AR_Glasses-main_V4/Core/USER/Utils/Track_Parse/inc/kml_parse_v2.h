/*
 * kml_parse_v2.h
 *
 *  Created on: Jan 27, 2026
 *      Author: wkt98
 */

#ifndef USER_UTILS_TRACK_PARSE_INC_KML_PARSE_V2_H_
#define USER_UTILS_TRACK_PARSE_INC_KML_PARSE_V2_H_

#ifdef __cplusplus
extern "C" {
#endif



#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <ctype.h>
#include <math.h>
#include "ff.h"      // 引入 FATFS 核心头文件
#include "FreeRTOS.h"
#include "task.h"

/* ================== 一、系统配置与常量定义 ================== */


/* 读取缓冲区大小：
 * 4KB 是常见的磁盘块大小，能保证较高的 I/O 效率。
 * 如果内存极度紧张，可降至 512 字节，但会增加 SD 卡/Flash 的读写次数。
 */
 #define READ_BUF_SIZE       4096

 /* LineString 内存缓存点数：
  * 为了避免每解析一个点就写一次文件（导致 I/O 瓶颈），
  * 我们在内存中积攒 128 个点（约 128 * 12 = 1.5KB）后再批量写入。
  */
#define LINE_CACHE_SIZE     128

  /* 缓冲区限制：防止恶意的超长标签或数字导致内存溢出 */
#define TAG_BUF_SIZE        64      // 足够容纳 "LineString", "coordinates" 等
#define NUM_BUF_SIZE        32      // 足够容纳高精度浮点数如 "-123.12345678"

#define MAX_CP_COUNT        64   //最大支持CP点个数
/* 判定重合的阈值 (度)，2e-4f约为 25-32m米 */
#define SPLIT_THRESHOLD     2e-4f

#define  MIN_SEGMENT_COUNT 200 //每个分段的最小点数数量

#define MAX_POINTS_PER_SEGMENT 3000 //每个分段的最大点数数量


/* [移植修改] 路径适配为嵌入式文件系统格式 (假设挂载点为 0:) */
#define FILE_POINT_NAME     "0:/track/point_temp.bin"
#define FILE_LINE_NAME      "0:/track/line_temp.bin"
#define FILE_LINE_FILTERED_NAME "0:/track/line_temp_filter.bin"

/* ================== 二、数据结构定义 ================== */

typedef enum {
    KML_OK = 0,
    KML_ERR_FILE_OPEN,    // 文件打开失败
    KML_ERR_IO_READ,      // 读取错误
    KML_ERR_IO_WRITE,     // 写入错误 (如磁盘满)
    KML_ERR_SEEK,         // 文件定位错误 (fgetpos/fsetpos)
    KML_ERR_BAD_FORMAT,   // KML 格式严重错误
    KML_ERR_INTERNAL,      // 内部逻辑错误 (如 malloc 失败)
    KML_ERR_NO_DATA      //无有效数据
} KmlError;



/* * 解析器状态机枚举
 * 分为三类：
 * 1. 语法动作状态：正在读标签名、属性等。
 * 2. 逻辑上下文状态：当前在 Point 还是 LineString 内部。
 * 3. 专用数据解析状态：处理 coordinates 数值流。
 */
 typedef enum {
    // --- 动作状态 (Action States) ---
    STATE_IDLE,                 // 空闲，寻找下一个 '<'
    STATE_TAG_NAME,             // 正在读取标签名 (如 "Point")
    STATE_ATTR_SKIP,            // 正在跳过属性 (如 id="123")，直到遇到 '>'
    STATE_TAG_END_NAME,         // 正在读取结束标签名 (如 "Point" 在 </Point> 中)

    // --- 逻辑上下文 (Context States) ---
    STATE_CTX_ROOT,             // 根层级
    STATE_CTX_PLACEMARK,        // 在 <Placemark> ... </Placemark> 内部
    STATE_CTX_POINT,            // 在 <Point> ... </Point> 内部
    STATE_CTX_LINE,             // 在 <LineString> ... </LineString> 内部

    // --- 数据解析 (Data Parsing) ---
    STATE_COORD_PARSE           // 核心状态：正在 <coordinates>...</coordinates> 内部解析数字
} ParseState;


/* * 紧凑的坐标点结构体
 * 大小：12 字节
 * 对齐：在 32 位系统上自然对齐
 */
 typedef struct {
    float lon; // 经度
    float lat; // 纬度
    float alt; // 海拔
} GeoPoint;

typedef struct {
    ParseState curr_state;
    ParseState logic_ctx;
    ParseState logic_ctx_prev;

    char tag_buf[TAG_BUF_SIZE];
    uint16_t tag_len;
    char num_buf[NUM_BUF_SIZE];
    uint16_t num_len;

    GeoPoint temp_point;
    uint8_t coord_comp_idx;
    uint8_t has_valid_num;
    uint8_t is_gx_coord;

    /* [移植修改] fpos_t -> FSIZE_t */
    FSIZE_t line_len_pos;
    uint32_t line_point_cnt;

    GeoPoint line_cache[LINE_CACHE_SIZE];
    uint16_t cache_idx;

    /* [移植修改] FILE* -> FIL 结构体实体 (避免二次malloc) */
    FIL fp_point;
    FIL fp_line;

    KmlError error;
    uint32_t valid_point_cnt;

    GeoPoint cp_list[MAX_CP_COUNT];
    uint16_t cp_count;
    uint8_t cp_list_visit[MAX_CP_COUNT];

    /* [移植修改] fpos_t -> FSIZE_t */
    FSIZE_t global_header_pos;
    FSIZE_t segment_header_pos;

    uint32_t total_segments;
    uint32_t curr_seg_points;

} KmlCtx;


/* ================== 全局存储区 ================== */
/* * 说明：根据要求定义全局数组
 * +1 是为了保留一个哨兵位或防止极端情况下的溢出
 */
extern GeoPoint CPX_Points[MAX_POINTS_PER_SEGMENT + 1];

KmlError parse_kml_file(const char* input_file);
KmlError filter_kml_lines(const char* src_file_path, const char* dst_file_path);
int32_t load_segment_to_global_buffer(const char* file_path, uint32_t seg_index);


#ifdef __cplusplus
}
#endif

#endif /* USER_UTILS_TRACK_PARSE_INC_KML_PARSE_V2_H_ */
