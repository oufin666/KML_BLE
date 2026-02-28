/*
 * kml_parse.cpp
 *
 *  Created on: Jan 7, 2026
 *      Author: wkt98
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lvgl.h"
#include "track_param.h"
#include "kml_parse.h"

/* 解析器配置 */
#define KML_READ_CHUNK_SIZE  512   // 读取块大小
#define MAX_NUM_BUFFER       32    // 浮点数字符串缓冲
#define MIN_COORD_COUNT      2     // 至少需要2个坐标（经度+纬度）
#define TARGET_POINT_COUNT  2000
typedef enum {
	STATE_SEARCH_TAG,      // 寻找 <coordinates>
	STATE_PARSE_CONTENT,   // 解析内容
	STATE_FOUND_EXIT       // 备用状态
} ParserState_t;

typedef struct {
	double sum_lon; // 使用 double 防止累加精度丢失
	double sum_lat;
	double sum_alt;
	int count;
} PointAccumulator;

/**
 * @brief 使用 LVGL FS API 解析 KML
 */
int kml_parse_coordinates(const char *filepath,
		std::vector<RawCoordinates> &out_track) {
	lv_fs_file_t file;
	lv_fs_res_t res;

	// 1. 打开文件 (LVGL API)
	// 注意：LVGL 路径通常需要驱动器前缀，如 "S:/path/to/file.kml"
	res = lv_fs_open(&file, filepath, LV_FS_MODE_RD);
	if (res != LV_FS_RES_OK) {
		LV_LOG_USER("Error opening KML file: %s (Res: %d)", filepath, res);
		return -1;
	}

	// [优化] 预分配内存
	out_track.clear();
	out_track.reserve(2000);

	char buffer[KML_READ_CHUNK_SIZE];
	uint32_t bytes_read = 0; // LVGL read 接口通常使用 uint32_t
	ParserState_t state = STATE_SEARCH_TAG;

	char num_buf[MAX_NUM_BUFFER];
	int num_idx = 0;
	int tuple_idx = 0; // 0=Lon, 1=Lat, 2=Alt

	const char *TAG_START = "<gx:coord>";
	const char *TAG_END = "</gx:coord>";
	int match_idx = 0;

	// [新增] 临时变量
	RawCoordinates temp_point = { 0.0f, 0.0f, 0.0f };

	while (1) {
		// 2. 读取文件 (LVGL API)
		res = lv_fs_read(&file, buffer, KML_READ_CHUNK_SIZE, &bytes_read);

		// 错误检查
		if (res != LV_FS_RES_OK) {
			LV_LOG_USER("Error reading KML file (Res: %d)", res);
			lv_fs_close(&file);
			return -1;
		}

		// EOF 检查：如果读取到的字节数为 0，说明文件结束
		if (bytes_read == 0) {
			break;
		}

		for (uint32_t i = 0; i < bytes_read; i++) {
			char c = buffer[i];

			if (state == STATE_SEARCH_TAG) {
				if (c == TAG_START[match_idx]) {
					match_idx++;
					if (TAG_START[match_idx] == '\0') {
						state = STATE_PARSE_CONTENT;
						match_idx = 0;
						tuple_idx = 0;
						num_idx = 0;
						// 重置临时点
						temp_point = { 0.0f, 0.0f, 0.0f };
					}
				} else {
					match_idx = 0;
					if (c == TAG_START[0])
						match_idx = 1;
				}
			} else if (state == STATE_PARSE_CONTENT) {
				// A. 检查结束标签
				if (c == TAG_END[match_idx]) {
					match_idx++;
					if (TAG_END[match_idx] == '\0') {
						state = STATE_SEARCH_TAG;
						match_idx = 0;

						// 处理 buffer 中残留的最后一个数字（通常是海拔）
						if (num_idx > 0) {
							num_buf[num_idx] = '\0';
							float val = strtof(num_buf, NULL);
							num_idx = 0;

							if (tuple_idx == 0)
								temp_point.longitude = val;
							else if (tuple_idx == 1)
								temp_point.latitude = val;
							else if (tuple_idx == 2)
								temp_point.altitude = val;

							// 凑齐了经纬度，推入 vector
							// 注意：这里保留了你原来代码的逻辑，必须 tuple_idx == 2 (即读完3个数) 才 push
							// 如果有的数据没有海拔，这里可能需要改为 >= 1
							if (tuple_idx == 2) {
								out_track.push_back(temp_point);
							}
						}
						continue;
					}
				} else {
					match_idx = 0;
				}

				// B. 解析数字
				if ((c >= '0' && c <= '9') || c == '-' || c == '.' || c == 'e'
						|| c == 'E') {
					if (num_idx < MAX_NUM_BUFFER - 1) {
						num_buf[num_idx++] = c;
					}
				}
				// C. 分隔符处理 (空格, 换行, 逗号)
				else if (c == ',' || c == ' ' || c == '\n' || c == '\r'
						|| c == '\t') {
					if (num_idx > 0) {
						num_buf[num_idx] = '\0'; // 封口
						float val = strtof(num_buf, NULL);
						num_idx = 0;

						// 存入临时变量
						if (tuple_idx == 0)
							temp_point.longitude = val;
						else if (tuple_idx == 1)
							temp_point.latitude = val;
						else if (tuple_idx == 2)
							temp_point.altitude = val;

						if (c == ',' || c == ' ') {
							tuple_idx++;
						}
						/* // 保留你的注释逻辑
						 else {
						 // 换行符或其他空白，通常意味着一个点的结束
						 if (tuple_idx >= 1) {
						 out_track.push_back(temp_point);
						 temp_point.altitude = 0.0f;
						 }
						 tuple_idx = 0;
						 }
						 */
					}
				}
			}
		}
		if (out_track.size() > 3900) {
			break;
		}
	}

	// 3. 关闭文件 (LVGL API)
	lv_fs_close(&file);

	// 处理文件末尾可能的残留数据
	if (state == STATE_PARSE_CONTENT && num_idx > 0) {
		num_buf[num_idx] = '\0';
		float val = strtof(num_buf, NULL);

		if (tuple_idx == 0)
			temp_point.longitude = val;
		else if (tuple_idx == 1)
			temp_point.latitude = val;
		else if (tuple_idx == 2)
			temp_point.altitude = val;

		if (tuple_idx >= 1) {
			out_track.push_back(temp_point);
		}
	}

	return out_track.size();
}

/**
 * @brief 解析 KML 并将坐标点直接写入二进制文件
 * * @param in_filepath  KML 源文件路径 (如 "S:/track.kml")
 * @param out_filepath 输出文件路径 (如 "S:/track.bin")。
 * 注意：如果是 "S:/data/track.bin"，请确保 "data" 文件夹已存在，
 * lv_fs_open 通常只负责创建文件，不会自动创建文件夹。
 * @return int 成功写入的点数量，失败返回 -1
 */
int kml_parse_to_file(const char *in_filepath, const char *out_filepath) {
	lv_fs_file_t f_in;
	lv_fs_file_t f_out;
	lv_fs_res_t res;

	// 1. 打开源文件 (只读)
	res = lv_fs_open(&f_in, in_filepath, LV_FS_MODE_RD);
	if (res != LV_FS_RES_OK) {
		LV_LOG_USER("Open Input Failed: %s (Res: %d)", in_filepath, res);
		return -1;
	}

	// 2. 打开输出文件 (只写)
	// LV_FS_MODE_WR 的行为：
	// - 文件不存在 -> 自动创建
	// - 文件已存在 -> 清空内容 (Truncate) 从头写入
	res = lv_fs_open(&f_out, out_filepath, LV_FS_MODE_WR);
	if (res != LV_FS_RES_OK) {
		LV_LOG_USER("Create Output Failed: %s (Res: %d)", out_filepath, res);
		lv_fs_close(&f_in);
		return -1;
	}

	// --- 初始化解析状态 ---
	char buffer[KML_READ_CHUNK_SIZE];
	uint32_t bytes_read = 0;
	uint32_t bytes_written = 0;

	typedef enum {
		STATE_SEARCH_TAG, STATE_PARSE_CONTENT
	} ParserState_t;
	ParserState_t state = STATE_SEARCH_TAG;

	char num_buf[MAX_NUM_BUFFER];
	int num_idx = 0;
	int tuple_idx = 0; // 0=Lon, 1=Lat, 2=Alt

	const char *TAG_START = "<gx:coord>";
	const char *TAG_END = "</gx:coord>";
	int match_idx = 0;

	RawCoordinates temp_point = { 0.0f, 0.0f, 0.0f };
	int total_points = 0;

	// --- 循环读取与解析 ---
	while (1) {
		res = lv_fs_read(&f_in, buffer, KML_READ_CHUNK_SIZE, &bytes_read);
		if (res != LV_FS_RES_OK || bytes_read == 0) {
			break; // 读取结束或出错
		}

		for (uint32_t i = 0; i < bytes_read; i++) {
			char c = buffer[i];

			// 状态机：寻找标签
			if (state == STATE_SEARCH_TAG) {
				if (c == TAG_START[match_idx]) {
					match_idx++;
					if (TAG_START[match_idx] == '\0') {
						state = STATE_PARSE_CONTENT;
						match_idx = 0;
						tuple_idx = 0;
						num_idx = 0;
						// 清零临时变量
						temp_point.longitude = 0.0f;
						temp_point.latitude = 0.0f;
						temp_point.altitude = 0.0f;
					}
				} else {
					match_idx = 0;
					if (c == TAG_START[0])
						match_idx = 1;
				}
			}
			// 状态机：解析内容
			else if (state == STATE_PARSE_CONTENT) {
				// 检查是否结束标签 </gx:coord>
				if (c == TAG_END[match_idx]) {
					match_idx++;
					if (TAG_END[match_idx] == '\0') {
						state = STATE_SEARCH_TAG;
						match_idx = 0;

						// 处理残留数字 (通常是海拔)
						if (num_idx > 0) {
							num_buf[num_idx] = '\0';
							float val = strtof(num_buf, NULL);
							num_idx = 0;
							if (tuple_idx == 0)
								temp_point.longitude = val;
							else if (tuple_idx == 1)
								temp_point.latitude = val;
							else if (tuple_idx == 2)
								temp_point.altitude = val;

							// === 写入文件关键点 ===
							if (tuple_idx == 2) {
								lv_fs_write(&f_out, &temp_point,
										sizeof(RawCoordinates), &bytes_written);
								total_points++;
							}
						}
						continue;
					}
				} else {
					match_idx = 0;
				}

				// 提取数字字符
				if ((c >= '0' && c <= '9') || c == '-' || c == '.' || c == 'e'
						|| c == 'E') {
					if (num_idx < MAX_NUM_BUFFER - 1) {
						num_buf[num_idx++] = c;
					}
				}
				// 处理分隔符 (空格/逗号/换行)
				else if (c == ',' || c == ' ' || c == '\n' || c == '\r'
						|| c == '\t') {
					if (num_idx > 0) {
						num_buf[num_idx] = '\0';
						float val = strtof(num_buf, NULL);
						num_idx = 0;

						if (tuple_idx == 0)
							temp_point.longitude = val;
						else if (tuple_idx == 1)
							temp_point.latitude = val;
						else if (tuple_idx == 2)
							temp_point.altitude = val;

						if (c == ',' || c == ' ')
							tuple_idx++;
					}
				}
			}
		}
	}

	// 处理文件末尾可能的残留数据
	if (state == STATE_PARSE_CONTENT && num_idx > 0) {
		num_buf[num_idx] = '\0';
		float val = strtof(num_buf, NULL);
		if (tuple_idx == 0)
			temp_point.longitude = val;
		else if (tuple_idx == 1)
			temp_point.latitude = val;
		else if (tuple_idx == 2)
			temp_point.altitude = val;

		if (tuple_idx >= 1) {
			lv_fs_write(&f_out, &temp_point, sizeof(RawCoordinates),
					&bytes_written);
			total_points++;
		}
	}

	// 3. 关闭文件
	lv_fs_close(&f_in);
	lv_fs_close(&f_out);

	LV_LOG_USER("Done. %d points written to %s", total_points, out_filepath);
	return total_points;
}

int count_kml_points(const char *filepath) {
	lv_fs_file_t file;
	if (lv_fs_open(&file, filepath, LV_FS_MODE_RD) != LV_FS_RES_OK)
		return -1;

	char buffer[KML_READ_CHUNK_SIZE];
	uint32_t bytes_read = 0;
	int count = 0;

	// 简化的状态机变量
	int match_idx = 0;
	const char *TAG_START = "<gx:coord>";

	while (1) {
		if (lv_fs_read(&file, buffer, KML_READ_CHUNK_SIZE, &bytes_read)
				!= LV_FS_RES_OK || bytes_read == 0) {
			break;
		}

		for (uint32_t i = 0; i < bytes_read; i++) {
			if (buffer[i] == TAG_START[match_idx]) {
				match_idx++;
				if (TAG_START[match_idx] == '\0') {
					// 找到一个完整的 <gx:coord>
					count++;
					match_idx = 0;
				}
			} else {
				match_idx = 0;
				// 回溯检查：如果当前字符是 '<'，可能是下一个标签的开始
				if (buffer[i] == TAG_START[0])
					match_idx = 1;
			}
		}
	}

	lv_fs_close(&file);
	return count;
}

/**
 * @brief 核心算法：分桶平均滤波并写入文件
 */
int kml_filter_to_file(const char *in_filepath, const char *out_filepath) {
	// === Pass 1: 获取总数 ===
	int total_input_points = count_kml_points(in_filepath);
	if (total_input_points <= 0)
		return -1;

	// === 准备 Pass 2 ===
	lv_fs_file_t f_in, f_out;
	if (lv_fs_open(&f_in, in_filepath, LV_FS_MODE_RD) != LV_FS_RES_OK)
		return -1;
	// LV_FS_MODE_WR 会自动创建或覆盖文件
	if (lv_fs_open(&f_out, out_filepath, LV_FS_MODE_WR) != LV_FS_RES_OK) {
		lv_fs_close(&f_in);
		return -1;
	}

	// 计算分桶大小 (bucket_size)
	// 比如 5000 个点分到 2000 个桶，每个桶容纳 2.5 个点（逻辑上）
	float points_per_bucket = (float) total_input_points / TARGET_POINT_COUNT;
	if (points_per_bucket < 1.0f)
		points_per_bucket = 1.0f; // 如果点数少于2000，则不压缩

	// 缓冲区变量
	char buffer[KML_READ_CHUNK_SIZE];
	uint32_t bytes_read = 0;
	uint32_t bytes_written = 0;

	ParserState_t state = STATE_SEARCH_TAG;
	char num_buf[32];
	int num_idx = 0;
	int tuple_idx = 0;
	const char *TAG_START = "<gx:coord>";
	const char *TAG_END = "</gx:coord>";
	int match_idx = 0;

	// === 滤波核心变量 ===
	PointAccumulator acc = { 0.0, 0.0, 0.0, 0 }; // 累加器
	int current_input_index = 0; // 当前正在读取第几个原始点
	int current_bucket_index = 0; // 当前属于第几个输出桶 (0 ~ 1999)
	int saved_points = 0;
	lv_fs_res_t res;
	while (1) {
		res = lv_fs_read(&f_in, buffer, KML_READ_CHUNK_SIZE, &bytes_read);
		if (res != LV_FS_RES_OK || bytes_read == 0)
			break;

		for (uint32_t i = 0; i < bytes_read; i++) {
			char c = buffer[i];

			// 1. 寻找标签
			if (state == STATE_SEARCH_TAG) {
				if (c == TAG_START[match_idx]) {
					match_idx++;
					if (TAG_START[match_idx] == '\0') {
						state = STATE_PARSE_CONTENT;
						match_idx = 0;
						tuple_idx = 0;
						num_idx = 0;
					}
				} else {
					match_idx = 0;
					if (c == TAG_START[0])
						match_idx = 1;
				}
			}
			// 2. 解析内容
			else if (state == STATE_PARSE_CONTENT) {
				if (c == TAG_END[match_idx]) {
					match_idx++;
					if (TAG_END[match_idx] == '\0') {
						state = STATE_SEARCH_TAG;
						match_idx = 0;

						// 解析完一个点的最后一个坐标 (Altitude)
						if (num_idx > 0) {
							num_buf[num_idx] = '\0';
							float val = strtof(num_buf, NULL);
							num_idx = 0;

							if (tuple_idx == 0)
								acc.sum_lon += val;
							else if (tuple_idx == 1)
								acc.sum_lat += val;
							else if (tuple_idx == 2)
								acc.sum_alt += val;

							acc.count++;
							current_input_index++;

							// === 核心逻辑开始 ===

							// 1. 计算当前点应该属于哪个桶 (0 ~ 1999)
							int target_bucket = (int) (current_input_index
									/ points_per_bucket);

							// 2. 如果跳到了下一个桶，说明上一个桶满了，需要结算并写入
							if (target_bucket > current_bucket_index
									&& tuple_idx == 2) {
								if (acc.count > 0) {
									RawCoordinates p_out;
									p_out.longitude = (float) (acc.sum_lon
											/ acc.count);
									p_out.latitude = (float) (acc.sum_lat
											/ acc.count);
									p_out.altitude = (float) (acc.sum_alt
											/ acc.count);

									lv_fs_write(&f_out, &p_out,
											sizeof(RawCoordinates),
											&bytes_written);
									saved_points++;
								}

								// 重置累加器
								acc.sum_lon = 0;
								acc.sum_lat = 0;
								acc.sum_alt = 0;
								acc.count = 0;
								current_bucket_index = target_bucket;
							}

							// 3. 将当前点加入累加器
							// 注意：此处需要用到你在 tuple_idx 阶段解析出的数值
							// 假设 buffer 里的 current_lon/current_lat 是当前解析的值
							// acc.sum_lon += current_lon;
							// acc.sum_lat += current_lat;
							//acc.sum_alt += alt; // 这里是简写，实际逻辑需整合进 strtof 部分
							//acc.count++;

							//current_input_index++;
							// === 核心逻辑结束 ===
						}
						num_idx = 0;
						continue;
					}
				} else {
					match_idx = 0;
				}

				if ((c >= '0' && c <= '9') || c == '-' || c == '.' || c == 'e'
						|| c == 'E') {
					if (num_idx < MAX_NUM_BUFFER - 1) {
						num_buf[num_idx++] = c;
					}
				}
				// C. 分隔符处理 (空格, 换行, 逗号)
				else if (c == ',' || c == ' ' || c == '\n' || c == '\r'
						|| c == '\t') {
					if (num_idx > 0) {
						num_buf[num_idx] = '\0'; // 封口
						float val = strtof(num_buf, NULL);
						num_idx = 0;

						// 存入临时变量
						if (tuple_idx == 0)
							acc.sum_lon += val;
						else if (tuple_idx == 1)
							acc.sum_lat += val;
						else if (tuple_idx == 2)
							acc.sum_alt += val;

						if (c == ',' || c == ' ') {
							tuple_idx++;
						}

					}
				}

			}
		}
	}

	// === 处理最后一个桶 ===
	// 循环结束后，最后一个桶的数据还在累加器里，没写进去
	if (acc.count > 0 && saved_points < TARGET_POINT_COUNT) {
		RawCoordinates p_out;
		p_out.longitude = (float) (acc.sum_lon / acc.count);
		p_out.latitude = (float) (acc.sum_lat / acc.count);
		p_out.altitude = (float) (acc.sum_alt / acc.count);
		lv_fs_write(&f_out, &p_out, sizeof(RawCoordinates), &bytes_written);
		saved_points++;
	}

	lv_fs_close(&f_in);
	lv_fs_close(&f_out);
	return saved_points;
}







/**
 * @brief 从二进制文件中读取轨迹点到 vector
 * @param filepath 文件路径 (如 "S:/track.bin")
 * @param out_vector 用于接收数据的 vector
 * @param point_count 用于接收数据的 vector的容量大小
 * @return int 成功读取的点数，-1 表示文件打开失败
 */
int load_points_from_file(const char* filepath, std::vector<RawCoordinates>& out_vector, int point_count) {
    lv_fs_file_t file;
    lv_fs_res_t res;

    // 1. 打开文件 (只读模式)
    res = lv_fs_open(&file, filepath, LV_FS_MODE_RD);
    if (res != LV_FS_RES_OK) {
        LV_LOG_USER("Open file failed: %s (Res: %d)", filepath, res);
        return -1;
    }


    if (point_count == 0) {
        lv_fs_close(&file);
        return 0;
    }

    // 3. [关键步骤] 预分配内存
    // 注意：这将直接从系统堆 (System Heap) 申请 point_count * 12 字节
    // 如果是 2000 个点，约需要 24KB RAM。
    // 请确保你的 .ld 文件中 _Min_Heap_Size > 24KB，否则这里会崩溃！
    out_vector.clear();
    out_vector.reserve(point_count);

    // 4. 读取数据
    RawCoordinates temp_point;
    uint32_t bytes_read = 0;

    // 方法 A：逐点读取 (更安全，栈占用小)
    for (size_t i = 0; i < point_count; i++) {
        res = lv_fs_read(&file, &temp_point, sizeof(RawCoordinates), &bytes_read);

        // 确保读满了 12 字节
        if (res == LV_FS_RES_OK && bytes_read == sizeof(RawCoordinates)) {
            out_vector.push_back(temp_point);
        } else {
            break; // 文件读取异常或结束
        }
    }


    // 5. 关闭文件
    lv_fs_close(&file);

    LV_LOG_USER("Loaded %d points into vector.", out_vector.size());
    return out_vector.size();
}







