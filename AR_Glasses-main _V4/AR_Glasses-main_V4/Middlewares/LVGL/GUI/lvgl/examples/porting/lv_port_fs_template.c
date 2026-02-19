/**
 * @file lv_port_fs_templ.c
 *
 */

/*Copy this file as "lv_port_fs.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_fs_template.h"
#include "../../lvgl.h"

#include "ff.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void fs_init(void);

static void* fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p);
static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf,
		uint32_t btr, uint32_t *br);
static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf,
		uint32_t btw, uint32_t *bw);
static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos,
		lv_fs_whence_t whence);
static lv_fs_res_t fs_size(lv_fs_drv_t *drv, void *file_p, uint32_t *size_p);
static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p);

static void* fs_dir_open(lv_fs_drv_t *drv, const char *path);
static lv_fs_res_t fs_dir_read(lv_fs_drv_t *drv, void *rddir_p, char *fn);
static lv_fs_res_t fs_dir_close(lv_fs_drv_t *drv, void *rddir_p);


/* === 新增函数声明 === */
static lv_fs_res_t fs_remove(lv_fs_drv_t * drv, const char * path);
static lv_fs_res_t fs_rename(lv_fs_drv_t * drv, const char * oldname, const char * newname);


/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_fs_init(void) {
	/*----------------------------------------------------
	 * Initialize your storage device and File System
	 * -------------------------------------------------*/
	fs_init();

	/*---------------------------------------------------
	 * Register the file system interface in LVGL
	 *--------------------------------------------------*/

	/*Add a simple drive to open images*/
	static lv_fs_drv_t fs_drv;
	lv_fs_drv_init(&fs_drv);

	/*Set up fields...*/
	fs_drv.letter = 'S';
	fs_drv.open_cb = fs_open;
	fs_drv.close_cb = fs_close;
	fs_drv.read_cb = fs_read;
	fs_drv.write_cb = fs_write;
	fs_drv.seek_cb = fs_seek;
	fs_drv.tell_cb = fs_tell;

	fs_drv.dir_close_cb = fs_dir_close;
	fs_drv.dir_open_cb = fs_dir_open;
	fs_drv.dir_read_cb = fs_dir_read;

	/* === 注册新增的回调函数 === */
	    fs_drv.remove_cb = fs_remove;
	    fs_drv.rename_cb = fs_rename;

	lv_fs_drv_register(&fs_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your Storage device and File system.*/
static void fs_init(void) {
	/*E.g. for FatFS initialize the SD card and FatFS itself*/

	/*You code here*/
	static FATFS fs;
	FRESULT res = f_mount(&fs, "0:", 1);
	if (res != FR_OK) {
		printf("挂载失败，错误码：%d\n", res);

	}else{
		printf("挂载成功%d", res);
	}
}

/**
 * Open a file
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode      read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return          a file descriptor or NULL on error
 */
static void* fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode) {
	lv_fs_res_t res = LV_FS_RES_NOT_IMP;
	BYTE fatfs_mode = 0;
	/*把LVGL的打开文件的模式和FatFs一一对应好*/
	if (mode == LV_FS_MODE_WR)
		fatfs_mode = FA_WRITE | FA_OPEN_ALWAYS;
	else if (mode == LV_FS_MODE_RD)
		fatfs_mode = FA_READ;
	else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
		fatfs_mode = FA_READ | FA_WRITE | FA_OPEN_ALWAYS;

	/* LVGL 传进来的path已经没有盘符，比如 "test.txt" ,我们需要把盘符补充上去，然后才能给f_open使用*/
	char *full_path = lv_mem_alloc(256);
	if (full_path == NULL)
		return NULL;
	lv_snprintf(full_path, 256, "0:%s", path);  // 注意 path 已经是 "/xxx"
	//printf("fs_open full_path=%s \n",full_path);
	FIL *f = lv_mem_alloc(sizeof(FIL));   // 为每个文件分配空间
	if (f == NULL) {
		printf("lv_mem_alloc failed ! (f == NULL)\r\n");
		return NULL;
	}
	FRESULT fr = f_open(f, full_path, fatfs_mode);
	if (fr == FR_OK) {
		printf("fs_open full_path=%s  OK !\n", full_path);
		return f;
	} else {
		printf("f_open failed: %d, full_path=%s\n", fr, full_path);
		lv_mem_free(f);
		return NULL;
	}
}

/**
 * Close an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p) {
	LV_UNUSED(drv);
	if (FR_OK == f_close((FIL*) file_p)) {
		lv_mem_free(file_p);
		return LV_FS_RES_OK;
	} else {
		lv_mem_free(file_p);
		return LV_FS_RES_UNKNOWN;
	}
}

/**
 * Read data from an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable.
 * @param buf       pointer to a memory block where to store the read data
 * @param btr       number of Bytes To Read
 * @param br        the real number of read bytes (Byte Read)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf,
		uint32_t btr, uint32_t *br) {
	LV_UNUSED(drv);

	FRESULT res = f_read(file_p, buf, btr, (UINT*) br);
	if (res == FR_OK)
		return LV_FS_RES_OK;
	else
		return LV_FS_RES_UNKNOWN;

}

/**
 * Write into a file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable
 * @param buf       pointer to a buffer with the bytes to write
 * @param btr       Bytes To Write
 * @param br        the number of real written bytes (Bytes Written). NULL if unused.
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf,
		uint32_t btw, uint32_t *bw) {
	 LV_UNUSED(drv);
   FRESULT res = f_write(file_p, buf, btw, (UINT *)bw);
   if(res == FR_OK) return LV_FS_RES_OK;
   else return LV_FS_RES_UNKNOWN;

}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open )
 * @param pos       the new position of read write pointer
 * @param whence    tells from where to interpret the `pos`. See @lv_fs_whence_t
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
    /*Add your code here*/
	LV_UNUSED(drv);
    switch(whence) {
        case LV_FS_SEEK_SET:
            f_lseek(file_p, pos);
            break;
        case LV_FS_SEEK_CUR:
            f_lseek(file_p, f_tell((FIL *)file_p) + pos);
            break;
        case LV_FS_SEEK_END:
            f_lseek(file_p, f_size((FIL *)file_p) + pos);
            break;
        default:
            break;
    }
    return LV_FS_RES_OK;
}

/**
 * Give the position of the read write pointer
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable.
 * @param pos_p     pointer to to store the result
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    /*Add your code here*/
	LV_UNUSED(drv);
    *pos_p = f_tell((FIL *)file_p);
    return LV_FS_RES_OK;
}


/**
 * Initialize a 'lv_fs_dir_t' variable for directory reading
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to a directory
 * @return          pointer to the directory read descriptor or NULL on error
 */
static void * fs_dir_open(lv_fs_drv_t * drv, const char * path)
{
	 LV_UNUSED(drv);
    DIR * d = lv_mem_alloc(sizeof(DIR));
    if(d == NULL) return NULL;

	/*Make the path relative to the current directory (the projects root folder)*/
	char *real_path = lv_mem_alloc(256);
	if(real_path == NULL) return NULL;
    lv_snprintf(real_path, 256, "0:%s", path);
    FRESULT res = f_opendir(d, real_path);
    if(res != FR_OK) {
        lv_mem_free(d);
        d = NULL;
    }
    return d;
}

/**
 * Read the next filename form a directory.
 * The name of the directories will begin with '/'
 * @param drv       pointer to a driver where this function belongs
 * @param rddir_p   pointer to an initialized 'lv_fs_dir_t' variable
 * @param fn        pointer to a buffer to store the filename
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_read(lv_fs_drv_t * drv, void * rddir_p, char * fn)
{
	LV_UNUSED(drv);
    FRESULT res;
    FILINFO fno;
    fn[0] = '\0';
    do {
        res = f_readdir(rddir_p, &fno);
        if(res != FR_OK) return LV_FS_RES_UNKNOWN;

        if(fno.fattrib & AM_DIR) {
            fn[0] = '/';
            strcpy(&fn[1], fno.fname);
        }
        else strcpy(fn, fno.fname);

    } while(strcmp(fn, "/.") == 0 || strcmp(fn, "/..") == 0);

    return LV_FS_RES_OK;
}

/**
 * Close the directory reading
 * @param drv       pointer to a driver where this function belongs
 * @param rddir_p   pointer to an initialized 'lv_fs_dir_t' variable
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_close(lv_fs_drv_t * drv, void * rddir_p)
{
	LV_UNUSED(drv);
	if(FR_OK == f_closedir(rddir_p))
	{
		lv_mem_free(rddir_p);
		return LV_FS_RES_OK;
	}
    else
	{
		lv_mem_free(rddir_p);
		return LV_FS_RES_UNKNOWN;
	}
}



/* === 新增函数实现 === */

/**
 * Delete a file
 * @param drv       pointer to a driver where this function belongs
 * @param path      path of the file to delete
 * @return          LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_remove(lv_fs_drv_t * drv, const char * path)
{
    LV_UNUSED(drv);
    char *real_path = lv_mem_alloc(256);
    if(real_path == NULL) return LV_FS_RES_OUT_OF_MEM;

    // 补全盘符 "0:"
    lv_snprintf(real_path, 256, "0:%s", path);

    FRESULT res = f_unlink(real_path);
    lv_mem_free(real_path);

    if(res == FR_OK) return LV_FS_RES_OK;
    else return LV_FS_RES_UNKNOWN;
}

/**
 * Rename a file
 * @param drv       pointer to a driver where this function belongs
 * @param oldname   path to the file to rename
 * @param newname   path with the new name
 * @return          LV_FS_RES_OK or any error from 'fs_res_t'
 */
static lv_fs_res_t fs_rename(lv_fs_drv_t * drv, const char * oldname, const char * newname)
{
    LV_UNUSED(drv);

    // 分配旧路径内存
    char *real_old_path = lv_mem_alloc(256);
    if(real_old_path == NULL) return LV_FS_RES_OUT_OF_MEM;

    // 分配新路径内存
    char *real_new_path = lv_mem_alloc(256);
    if(real_new_path == NULL) {
        lv_mem_free(real_old_path);
        return LV_FS_RES_OUT_OF_MEM;
    }

    // 补全盘符
    lv_snprintf(real_old_path, 256, "0:%s", oldname);
    lv_snprintf(real_new_path, 256, "0:%s", newname);

    FRESULT res = f_rename(real_old_path, real_new_path);

    lv_mem_free(real_old_path);
    lv_mem_free(real_new_path);

    if(res == FR_OK) return LV_FS_RES_OK;
    else return LV_FS_RES_UNKNOWN;
}


#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
