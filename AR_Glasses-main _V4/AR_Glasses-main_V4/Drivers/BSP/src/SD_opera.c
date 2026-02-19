#include "SD_opera.h"

DWORD fat_GetFatTimeFromRTC() {
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) == HAL_OK) {
		HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
		WORD date = ((2000 + sDate.Year - 1980) << 9) | (sDate.Month << 5)
				| sDate.Date;
		WORD time = (sTime.Hours << 11) | (sTime.Minutes << 5)
				| (sTime.Seconds >> 1);

		DWORD fatTime = ((DWORD) date << 16) | time;
		return fatTime;
	} else {
		return 0;
	}
}

uint8_t testLVGLFatfs() {
	lv_fs_file_t f;
	lv_fs_res_t res;
	uint32_t read_num;
	char read_buffer[100] = { 0 };  // 确保缓冲区足够容纳读取内容
	res = lv_fs_open(&f, "S:/img/27.png", LV_FS_MODE_RD);
	if (res != LV_FS_RES_OK) {
		printf("打开读取文件失败，错误码：%d\n", res);
//		f_mount(NULL, "0:", 0);
		return -1;
	}


	res = lv_fs_read(&f, read_buffer, 8, &read_num);
	if (res != LV_FS_RES_OK || read_num != 8) {
		printf("读取数据失败，错误码：%d，实际读取字节数：%d\n", res, read_num);
		f_close(&f);
//		f_mount(NULL, "0:", 0);
		return -1;
	}
}

uint8_t testFatfs() {
	FATFS fs;
	FIL file, read_file;
	FRESULT res;
	UINT bytes_written, bytes_read;
	const char *data = "Test LVGL Hello 你好";
	char read_buffer[100] = { 0 };  // 确保缓冲区足够容纳读取内容
	// 1. 挂载文件系统
	res = f_mount(&fs, "0:", 1);
	if (res != FR_OK) {
		printf("挂载失败，错误码：%d\n", res);
		return -1;
	}

	// 2. 创建并打开文件
	res = f_open(&file, "0:/test.txt", FA_CREATE_ALWAYS | FA_WRITE);
	if (res != FR_OK) {
		printf("创建文件失败，错误码：%d\n", res);
		f_mount(NULL, "0:", 0);
		return -1;
	}

	// 3. 写入数据
	res = f_write(&file, data, strlen(data), &bytes_written);
	if (res != FR_OK || bytes_written != strlen(data)) {
		printf("写入数据失败，错误码：%d\n", res);
		f_close(&file);
		f_mount(NULL, "0:", 0);
		return -1;
	}

	// 4. 关闭文件并卸载
	res = f_close(&file);
	if (res != FR_OK) {
		printf("关闭文件失败，错误码：%d\n", res);
	} else {
		printf("写入成功！\n");
	}

	// 4. 打开文件读取
	res = f_open(&read_file, "0:/test.txt", FA_READ);
	if (res != FR_OK) {
		printf("打开读取文件失败，错误码：%d\n", res);
		f_mount(NULL, "0:", 0);
		return -1;
	}

	// 5. 读取数据
	res = f_read(&read_file, read_buffer, strlen(data), &bytes_read);
	if (res != FR_OK || bytes_read != strlen(data)) {
		printf("读取数据失败，错误码：%d，实际读取字节数：%d\n", res, bytes_read);
		f_close(&read_file);
		f_mount(NULL, "0:", 0);
		return -1;
	}

	// 6. 验证内容一致性
	if (strcmp(read_buffer, data) == 0) {
		printf("读取成功，内容一致：%s\n", read_buffer);
	} else {
		printf("读取内容不一致！写入：%s，读取：%s\n", data, read_buffer);
	}

	// 关闭读取文件并卸载
	res = f_close(&read_file);
	if (res != FR_OK) {
		printf("关闭读取文件失败，错误码：%d\n", res);
	}
	f_mount(NULL, "0:", 0);

	return 0;
}
