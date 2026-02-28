/*
 * kml_parse_v2.c
 *
 *  Created on: Jan 27, 2026
 *      Author: wkt98
 */


#include "kml_parse_v2.h"

/* 辅助函数：将 FATFS 错误码转换为业务错误码 */
static KmlError kml_error_map(FRESULT res) {
    switch (res) {
        case FR_OK: return KML_OK;
        case FR_NO_FILE:
        case FR_NO_PATH: return KML_ERR_FILE_OPEN;
        case FR_DENIED:
        case FR_WRITE_PROTECTED: return KML_ERR_IO_WRITE;
        default: return KML_ERR_IO_READ; // 默认归为 IO 错误
    }
}


// 封装分配函数，增加NULL检查和清零（模拟calloc）
static void* kml_malloc(size_t size) {
    void* ptr = pvPortMalloc(size);
    if (ptr) {
        memset(ptr, 0, size); // 强制清零，增加安全性
    } else {
        // 可以在这里加个钩子，记录内存溢出时间
        // printf("Mem Alloc Failed: %d bytes\n", size);
    }
    return ptr;
}

// 封装释放函数
static void kml_free(void* ptr) {
    if (ptr) {
        vPortFree(ptr);
    }
}



/* ================== 三、辅助函数实现 ================== */

static void reset_temp_point(KmlCtx* ctx) {
    ctx->temp_point.lon = 0.0f;
    ctx->temp_point.lat = 0.0f;
    ctx->temp_point.alt = 0.0f;
    ctx->coord_comp_idx = 0;
    ctx->num_len = 0;
    ctx->has_valid_num = 0;
    memset(ctx->num_buf, 0, NUM_BUF_SIZE);
}

static void flush_line_cache(KmlCtx* ctx) {
    if (ctx->cache_idx > 0) {
        UINT bw;
        /* [移植修改] f_write 替代 fwrite */
        FRESULT res = f_write(&ctx->fp_line, ctx->line_cache, sizeof(GeoPoint) * ctx->cache_idx, &bw);
        if (res != FR_OK || bw != sizeof(GeoPoint) * ctx->cache_idx) {
            ctx->error = (res != FR_OK) ? kml_error_map(res) : KML_ERR_IO_WRITE;
        }
        ctx->cache_idx = 0;
    }
}

static void close_current_segment(KmlCtx* ctx) {
    flush_line_cache(ctx);
    if (ctx->error != KML_OK) return;

    /* [移植修改] f_tell 替代 fgetpos */
    FSIZE_t end_pos = f_tell(&ctx->fp_line);

    /* [移植修改] f_lseek 替代 fsetpos */
    if (f_lseek(&ctx->fp_line, ctx->segment_header_pos) != FR_OK) {
        ctx->error = KML_ERR_SEEK; return;
    }

    UINT bw;
    if (f_write(&ctx->fp_line, &ctx->curr_seg_points, sizeof(uint32_t), &bw) != FR_OK || bw != sizeof(uint32_t)) {
        ctx->error = KML_ERR_IO_WRITE; return;
    }

    if (f_lseek(&ctx->fp_line, end_pos) != FR_OK) {
        ctx->error = KML_ERR_SEEK; return;
    }
}

static void start_new_segment(KmlCtx* ctx) {
    /* [移植修改] f_tell 替代 fgetpos */
    ctx->segment_header_pos = f_tell(&ctx->fp_line);

    uint32_t placeholder = 0;
    UINT bw;
    /* [移植修改] f_write */
    if (f_write(&ctx->fp_line, &placeholder, sizeof(uint32_t), &bw) != FR_OK || bw != sizeof(uint32_t)) {
        ctx->error = KML_ERR_IO_WRITE;
        return;
    }

    ctx->curr_seg_points = 0;
    ctx->total_segments++;
}

static int is_match_cp(KmlCtx* ctx, GeoPoint* p) {
    for (int i = 0; i < ctx->cp_count; i++) {
        if (ctx->cp_list_visit[i] == 1 || i == 1) continue;
        float d_lon = fabsf(p->lon - ctx->cp_list[i].lon);
        float d_lat = fabsf(p->lat - ctx->cp_list[i].lat);
        if (d_lon < SPLIT_THRESHOLD && d_lat < SPLIT_THRESHOLD) {
            // printf("匹配第几个点:%d\n", i + 1); // 嵌入式环境通常去掉printf或重定向
            ctx->cp_list_visit[i] = 1;
            return 1;
        }
    }
    return 0;
}

/* ================== 四、核心逻辑：坐标数值处理 ================== */

static void commit_number(KmlCtx* ctx) {
    if (!ctx->has_valid_num) return;
    ctx->num_buf[ctx->num_len] = '\0';
    float val = strtof(ctx->num_buf, NULL);

    switch (ctx->coord_comp_idx) {
    case 0: ctx->temp_point.lon = val; break;
    case 1: ctx->temp_point.lat = val; break;
    case 2: ctx->temp_point.alt = val; break;
    default: break;
    }

    ctx->coord_comp_idx++;
    ctx->num_len = 0;
    ctx->has_valid_num = 0;
}

static void commit_point(KmlCtx* ctx) {
    commit_number(ctx);
    if (ctx->coord_comp_idx < 2) {
        reset_temp_point(ctx);
        return;
    }

    if (ctx->logic_ctx_prev == STATE_CTX_POINT) {
        UINT bw;
        /* [移植修改] f_write */
        if (f_write(&ctx->fp_point, &ctx->temp_point, sizeof(GeoPoint), &bw) != FR_OK || bw != sizeof(GeoPoint)) {
            ctx->error = KML_ERR_IO_WRITE;
        }
        if (ctx->cp_count < MAX_CP_COUNT) {
            ctx->cp_list[ctx->cp_count++] = ctx->temp_point;
        }
    }
    else if (ctx->logic_ctx_prev == STATE_CTX_LINE) {
        int need_split = is_match_cp(ctx, &ctx->temp_point);

        if (ctx->cache_idx < LINE_CACHE_SIZE) {
            ctx->line_cache[ctx->cache_idx++] = ctx->temp_point;
            ctx->curr_seg_points++;
        }
        else {
            ctx->error = KML_ERR_INTERNAL;
            return;
        }

        if (ctx->cache_idx >= LINE_CACHE_SIZE) flush_line_cache(ctx);

        if (need_split && ctx->curr_seg_points > MIN_SEGMENT_COUNT) {
            close_current_segment(ctx);
            if (ctx->error != KML_OK) return;
            start_new_segment(ctx);
            /* 注意：此处需防止 cache_idx 越界，但在 flush 后 idx 为 0，安全 */
            ctx->line_cache[ctx->cache_idx++] = ctx->temp_point;
            ctx->curr_seg_points++;
        }
    }
    reset_temp_point(ctx);
}

/* ================== 五、核心逻辑：XML 标签处理 ================== */

static void handle_tag_open(KmlCtx* ctx) {
    if (ctx->tag_len == 0) return;
    ctx->tag_buf[ctx->tag_len] = '\0';
    char* tag = ctx->tag_buf;

    if (tag[0] == '?' || tag[0] == '!') return;

    if (strcmp(tag, "Placemark") == 0) {
        ctx->logic_ctx = STATE_CTX_PLACEMARK;
    }
    else if (ctx->logic_ctx == STATE_CTX_PLACEMARK) {
        if (strcmp(tag, "Point") == 0) {
            ctx->logic_ctx = STATE_CTX_POINT;
        }
        else if (strcmp(tag, "LineString") == 0 || strcmp(tag, "Track") == 0  || strcmp(tag, "gx:Track") == 0) {
            ctx->logic_ctx = STATE_CTX_LINE;
            if (ctx->total_segments == 0) {
                /* [移植修改] f_tell 替代 fgetpos */
                ctx->global_header_pos = f_tell(&ctx->fp_line);

                uint32_t zero = 0;
                UINT bw;
                /* [移植修改] f_write */
                if (f_write(&ctx->fp_line, &zero, sizeof(uint32_t), &bw) != FR_OK || bw != sizeof(uint32_t)) {
                    ctx->error = KML_ERR_IO_WRITE;
                    return;
                }
                start_new_segment(ctx);
            }
        }
    }

    if (strcmp(tag, "coordinates") == 0 || strcmp(tag, "gx:coord") == 0 || strcmp(tag, "coord") == 0) {
        if (ctx->logic_ctx == STATE_CTX_POINT || ctx->logic_ctx == STATE_CTX_LINE) {
            ctx->logic_ctx_prev = ctx->logic_ctx;
            ctx->curr_state = STATE_COORD_PARSE;
            reset_temp_point(ctx);

            if (strcmp(tag, "coord" ) == 0 || strcmp(tag, "gx:coord") == 0) {
                ctx->is_gx_coord = 1;
            } else {
                ctx->is_gx_coord = 0;
            }
        }
    }
}

static void handle_tag_close(KmlCtx* ctx) {
    ctx->tag_buf[ctx->tag_len] = '\0';
    char* tag = ctx->tag_buf;

    if (strcmp(tag, "Placemark") == 0) {
        ctx->logic_ctx = STATE_CTX_ROOT;
    }
    else if (strcmp(tag, "Point") == 0 || strcmp(tag, "LineString") == 0 ||
             strcmp(tag, "gx:Track") == 0 || strcmp(tag, "Track") == 0) {
        ctx->logic_ctx = STATE_CTX_PLACEMARK;
    }
}

/* ================== 六、主解析循环入口 ================== */

KmlError parse_kml_file(const char* input_file) {
    KmlCtx* ctx = (KmlCtx*)kml_malloc(sizeof(KmlCtx));
    if (!ctx) return KML_ERR_INTERNAL;

    uint8_t* read_buf = NULL;
    UINT n_read; // [移植修改] 替换 size_t，FATFS 使用 UINT
    int is_self_closing = 0;
    KmlError final_err = KML_OK;

  //  memset(ctx, 0, sizeof(KmlCtx));
    ctx->curr_state = STATE_IDLE;
    ctx->logic_ctx = STATE_CTX_ROOT;

    /* [移植修改] FIL 对象定义 */
    FIL fp_in_struct;
    FIL* fp_in = &fp_in_struct;
    FRESULT res;

    /* [移植修改] f_open (FA_READ) */
    res = f_open(fp_in, input_file, FA_READ);
    if (res != FR_OK) { kml_free(ctx); return kml_error_map(res); }

    /* [移植修改] f_open (FA_WRITE | FA_CREATE_ALWAYS) */
    res = f_open(&ctx->fp_point, FILE_POINT_NAME, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        f_close(fp_in); kml_free(ctx); return kml_error_map(res);
    }

    res = f_open(&ctx->fp_line, FILE_LINE_NAME, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        f_close(fp_in); f_close(&ctx->fp_point); kml_free(ctx); return kml_error_map(res);
    }

    read_buf = (uint8_t*)kml_malloc(READ_BUF_SIZE);
    if (!read_buf) {
        ctx->error = KML_ERR_INTERNAL;
        goto cleanup;
    }

    /* [移植修改] f_read 替代 fread，n_read 由参数传出 */
    while (f_read(fp_in, read_buf, READ_BUF_SIZE, &n_read) == FR_OK && n_read > 0) {
        for (UINT i = 0; i < n_read; i++) {
            if (ctx->error != KML_OK) goto cleanup;

            char c = read_buf[i];

            // --- 状态机代码保持不变 ---
            if (ctx->curr_state == STATE_COORD_PARSE) {
                if (c == '<') {
                    if (ctx->has_valid_num) commit_point(ctx);
                    ctx->logic_ctx = ctx->logic_ctx_prev;
                    ctx->curr_state = STATE_TAG_NAME;
                    ctx->tag_len = 0;
                    is_self_closing = 0;
                }
                else if (c == ',' || isspace((int)c)) {
                    commit_number(ctx);
                    if (isspace((int)c)) {
                        if (!ctx->is_gx_coord) {
                            if (ctx->coord_comp_idx > 0) commit_point(ctx);
                        }
                    }
                }
                else {
                    if (ctx->num_len < NUM_BUF_SIZE - 1) {
                        if ((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E') {
                            ctx->num_buf[ctx->num_len++] = c;
                            ctx->has_valid_num = 1;
                        }
                    }
                }
                continue;
            }

            switch (ctx->curr_state) {
            case STATE_IDLE:
                if (c == '<') {
                    ctx->curr_state = STATE_TAG_NAME;
                    ctx->tag_len = 0;
                    is_self_closing = 0;
                }
                break;
            case STATE_TAG_NAME:
                if (c == '>') {
                    if (is_self_closing) handle_tag_close(ctx);
                    else handle_tag_open(ctx);
                    if (ctx->curr_state != STATE_COORD_PARSE) ctx->curr_state = STATE_IDLE;
                }
                else if (isspace((int)c)) {
                    handle_tag_open(ctx);
                    ctx->curr_state = STATE_ATTR_SKIP;
                }
                else if (c == '/') {
                    if (ctx->tag_len == 0) ctx->curr_state = STATE_TAG_END_NAME;
                    else is_self_closing = 1;
                }
                else {
                    if (ctx->tag_len < TAG_BUF_SIZE - 1) ctx->tag_buf[ctx->tag_len++] = c;
                }
                break;
            case STATE_ATTR_SKIP:
                if (c == '>') {
                    if (is_self_closing) handle_tag_close(ctx);
                    if (ctx->curr_state != STATE_COORD_PARSE) ctx->curr_state = STATE_IDLE;
                }
                else if (c == '/') is_self_closing = 1;
                break;
            case STATE_TAG_END_NAME:
                if (c == '>') {
                    handle_tag_close(ctx);
                    ctx->curr_state = STATE_IDLE;
                }
                else if (!isspace((int)c)) {
                    if (ctx->tag_len < TAG_BUF_SIZE - 1) ctx->tag_buf[ctx->tag_len++] = c;
                }
                break;
            default:
                ctx->curr_state = STATE_IDLE;
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));// 每处理 4KB 数据，让出 1ms 给系统调
    }

    // --- 结束处理 ---
    if (ctx->curr_seg_points > MIN_SEGMENT_COUNT) {
        close_current_segment(ctx);
    }

    /* [移植修改] 回填总段数 */
    if (ctx->error == KML_OK) {
        FSIZE_t end_pos = f_tell(&ctx->fp_line);
        if (f_lseek(&ctx->fp_line, ctx->global_header_pos) == FR_OK) {
            UINT bw;
            f_write(&ctx->fp_line, &ctx->total_segments, sizeof(uint32_t), &bw);
            f_lseek(&ctx->fp_line, end_pos);
        } else {
            ctx->error = KML_ERR_SEEK;
        }
    }

cleanup:
    if (ctx) final_err = ctx->error;
    else final_err = KML_ERR_INTERNAL;

    if (read_buf) kml_free(read_buf);

    /* [移植修改] f_close */
    f_close(fp_in);

    if (ctx) {
        f_close(&ctx->fp_point);
        f_close(&ctx->fp_line);
        kml_free(ctx);
        ctx = NULL;
    }

    /* [移植修改] f_unlink 替代 remove */
    if (final_err != KML_OK) {
        f_unlink(FILE_POINT_NAME);
        f_unlink(FILE_LINE_NAME);
    }

    return final_err;
}

KmlError filter_kml_lines(const char* src_file_path, const char* dst_file_path) {
    /* [移植修改] 定义FIL对象 */
    FIL src_file, dst_file;
    FIL *fp_src = &src_file;
    FIL *fp_dst = &dst_file;

    KmlError ret_error = KML_OK;
    UINT br, bw; // 读写计数变量

    if (f_open(fp_src, src_file_path, FA_READ) != FR_OK) return KML_ERR_FILE_OPEN;
    if (f_open(fp_dst, dst_file_path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
        f_close(fp_src); return KML_ERR_FILE_OPEN;
    }

    uint16_t write_idx;
    uint32_t total_segments = 0;
    GeoPoint* read_cache = NULL;
    GeoPoint* write_cache = NULL;

    /* [移植修改] f_read */
    if (f_read(fp_src, &total_segments, sizeof(uint32_t), &br) != FR_OK || br != sizeof(uint32_t)) {
        ret_error = KML_ERR_IO_READ; goto error_cleanup;
    }
    /* [移植修改] f_write */
    if (f_write(fp_dst, &total_segments, sizeof(uint32_t), &bw) != FR_OK || bw != sizeof(uint32_t)) {
        ret_error = KML_ERR_IO_WRITE; goto error_cleanup;
    }

    read_cache = (GeoPoint*)kml_malloc(sizeof(GeoPoint) * LINE_CACHE_SIZE);
    write_cache = (GeoPoint*)kml_malloc(sizeof(GeoPoint) * LINE_CACHE_SIZE);
    if (!read_cache || !write_cache) {
        ret_error = KML_ERR_INTERNAL; goto error_cleanup;
    }

    write_idx = 0;

    for (uint32_t i = 0; i < total_segments; i++) {
        uint32_t src_count = 0;
        if (f_read(fp_src, &src_count, sizeof(uint32_t), &br) != FR_OK || br != sizeof(uint32_t)) {
            ret_error = KML_ERR_IO_READ; goto error_cleanup;
        }

        FSIZE_t segment_head_pos = f_tell(fp_dst);

        uint32_t placeholder = 0;
        if (f_write(fp_dst, &placeholder, sizeof(uint32_t), &bw) != FR_OK || bw != sizeof(uint32_t)) {
            ret_error = KML_ERR_IO_WRITE; goto error_cleanup;
        }

        uint32_t dst_count = 0;
        write_idx = 0;

        if (src_count <= MAX_POINTS_PER_SEGMENT) {
            /* 场景 A: 直接复制 */
            uint32_t remain = src_count;
            while (remain > 0) {
                uint16_t chunk = (remain > LINE_CACHE_SIZE) ? LINE_CACHE_SIZE : (uint16_t)remain;

                /* [移植修改] 这里的chunk是元素个数，f_read需要字节数 */
                if (f_read(fp_src, read_cache, sizeof(GeoPoint) * chunk, &br) != FR_OK || br != sizeof(GeoPoint) * chunk) {
                    ret_error = KML_ERR_IO_READ; goto error_cleanup;
                }

                if (write_idx + chunk > LINE_CACHE_SIZE) {
                    if (f_write(fp_dst, write_cache, sizeof(GeoPoint) * write_idx, &bw) != FR_OK || bw != sizeof(GeoPoint) * write_idx) {
                        ret_error = KML_ERR_IO_WRITE; goto error_cleanup;
                    }
                    write_idx = 0;
                }

                memcpy(&write_cache[write_idx], read_cache, chunk * sizeof(GeoPoint));
                write_idx += chunk;
                remain -= chunk;
            }
            dst_count = src_count;
        }
        else {
            /* 场景 B: 均匀抽稀 */
            if (MAX_POINTS_PER_SEGMENT <= 1) {
                ret_error = KML_ERR_INTERNAL; goto error_cleanup;
            }

            float step = (float)(src_count - 1) / (float)(MAX_POINTS_PER_SEGMENT - 1);
            float next_sample_idx = 0.0f;
            uint32_t processed = 0;
            // uint8_t is_last_point_processed = 0; // 保留原逻辑变量但消除编译警告（如果不用）

            while (processed < src_count) {
                uint32_t remain = src_count - processed;
                uint16_t chunk = (remain > LINE_CACHE_SIZE) ? LINE_CACHE_SIZE : (uint16_t)remain;

                if (f_read(fp_src, read_cache, sizeof(GeoPoint) * chunk, &br) != FR_OK || br != sizeof(GeoPoint) * chunk) {
                    ret_error = KML_ERR_IO_READ; goto error_cleanup;
                }

                for (uint16_t k = 0; k < chunk; k++) {
                    uint32_t global_idx = processed + k;
                    int is_last_point = (global_idx == src_count - 1);

                    if (is_last_point || (float)global_idx >= next_sample_idx) {
                        if (dst_count <= MAX_POINTS_PER_SEGMENT) {
                            write_cache[write_idx++] = read_cache[k];
                            dst_count++;
                            if (!is_last_point) next_sample_idx += step;
                        }

                        if (write_idx >= LINE_CACHE_SIZE) {
                            if (f_write(fp_dst, write_cache, sizeof(GeoPoint) * write_idx, &bw) != FR_OK || bw != sizeof(GeoPoint) * write_idx) {
                                ret_error = KML_ERR_IO_WRITE; goto error_cleanup;
                            }
                            write_idx = 0;
                        }
                    }
                }
                processed += chunk;
            }
        }

        if (write_idx > 0) {
            if (f_write(fp_dst, write_cache, sizeof(GeoPoint) * write_idx, &bw) != FR_OK || bw != sizeof(GeoPoint) * write_idx) {
                ret_error = KML_ERR_IO_WRITE; goto error_cleanup;
            }
            write_idx = 0;
        }

        FSIZE_t end_pos = f_tell(fp_dst);
        if (f_lseek(fp_dst, segment_head_pos) != FR_OK) { ret_error = KML_ERR_SEEK; goto error_cleanup; }
        if (f_write(fp_dst, &dst_count, sizeof(uint32_t), &bw) != FR_OK || bw != sizeof(uint32_t)) {
            ret_error = KML_ERR_IO_WRITE; goto error_cleanup;
        }
        if (f_lseek(fp_dst, end_pos) != FR_OK) { ret_error = KML_ERR_SEEK; goto error_cleanup; }
    }

    kml_free(read_cache); kml_free(write_cache);
    read_cache = NULL; write_cache = NULL;
    f_close(fp_src); f_close(fp_dst);

    /* [移植修改] f_unlink / f_rename */
    if (f_unlink(src_file_path) != FR_OK) {
        ret_error = KML_ERR_IO_WRITE; goto error_cleanup_no_src_dst;
    }
    if (f_rename(dst_file_path, src_file_path) != FR_OK) {
        ret_error = KML_ERR_IO_WRITE; goto error_cleanup_no_src_dst;
    }

    return KML_OK;

error_cleanup:
    if (read_cache) kml_free(read_cache);
    if (write_cache) kml_free(write_cache);
    f_close(fp_src); f_close(fp_dst);

error_cleanup_no_src_dst:
    f_unlink(dst_file_path);
    return ret_error;
}






/* ================== 全局存储区 ================== */
/* * 说明：根据要求定义全局数组
 * +1 是为了保留一个哨兵位或防止极端情况下的溢出
 */
GeoPoint CPX_Points[MAX_POINTS_PER_SEGMENT + 1];

/* ================== 函数实现 ================== */

/**
 * @brief  读取指定索引段的数据到全局数组 Points 中
 * @param  file_path 文件路径 (如 "0:/track/line_temp.bin")
 * @param  seg_index 目标段的索引 (从 0 开始)
 * @return int32_t   成功返回读取的点数(>=0)，失败返回负数错误码
 */
int32_t load_segment_to_global_buffer(const char* file_path, uint32_t seg_index) {
    FRESULT res;
    UINT br;
    int32_t ret_val = -1; // 默认返回错误
    uint32_t total_segs = 0;
    uint32_t pts_in_seg = 0;

    // 1. 动态分配 FIL 对象 (防止栈溢出，约 560 字节)
    FIL* fil = (FIL*)kml_malloc(sizeof(FIL));
    if (!fil) {
        // printf("Error: OOM for FIL\n");
        return KML_ERR_INTERNAL;
    }

    // 2. 打开文件
    res = f_open(fil, file_path, FA_READ);
    if (res != FR_OK) {
        kml_free(fil);
        return kml_error_map(res);
    }

    // 3. 读取总段数
    res = f_read(fil, &total_segs, sizeof(uint32_t), &br);
    if (res != FR_OK || br != sizeof(uint32_t)) {
        goto cleanup;
    }

    // 4. 索引校验
    if (seg_index >= total_segs) {
        // printf("Error: Segment index %u out of range (Total: %u)\n", seg_index, total_segs);
        ret_val = KML_ERR_NO_DATA;
        goto cleanup;
    }

    // 5. 遍历跳过前面的段 (Seek 操作)
    // 这里的逻辑必须顺序读取每个段的头(点数)，然后 f_lseek 跳过数据体
    for (uint32_t i = 0; i < seg_index; i++) {
        // 读取当前段的点数
        res = f_read(fil, &pts_in_seg, sizeof(uint32_t), &br);
        if (res != FR_OK || br != sizeof(uint32_t)) {
            goto cleanup;
        }

        // 计算需要跳过的字节偏移量
        FSIZE_t jump_offset = (FSIZE_t)pts_in_seg * sizeof(GeoPoint);

        // 执行跳转 (FATFS f_lseek 需要绝对地址)
        res = f_lseek(fil, f_tell(fil) + jump_offset);
        if (res != FR_OK) {
            goto cleanup;
        }
    }

    // 6. 读取目标段的头部 (点数)
    res = f_read(fil, &pts_in_seg, sizeof(uint32_t), &br);
    if (res != FR_OK || br != sizeof(uint32_t)) {
        goto cleanup;
    }

    // 7. 安全性检查：防止缓冲区溢出
    if (pts_in_seg > MAX_POINTS_PER_SEGMENT) {
        // printf("Warning: Segment size (%u) > Buffer (%u), clamping.\n", pts_in_seg, MAX_POINTS_PER_SEGMENT);
        pts_in_seg = MAX_POINTS_PER_SEGMENT; // 截断读取
    }

    // 8. 读取实际点数据到全局数组
    // 直接读取整个数组块，效率最高
    res = f_read(fil, CPX_Points, pts_in_seg * sizeof(GeoPoint), &br);
    if (res != FR_OK || br != pts_in_seg * sizeof(GeoPoint)) {
        goto cleanup;
    }

    // 成功，设置返回值
    ret_val = (int32_t)pts_in_seg;

cleanup:
    f_close(fil);
    kml_free(fil);
    return ret_val;
}



