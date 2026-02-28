#ifndef SD_OPERA_H
#define SD_OPERA_H

typedef unsigned long DWORD;

#include "ff.h"
#include <stdio.h>
#include "rtc.h"

DWORD fat_GetFatTimeFromRTC();
uint8_t testFatfs();
uint8_t testLVGLFatfs();

#endif
