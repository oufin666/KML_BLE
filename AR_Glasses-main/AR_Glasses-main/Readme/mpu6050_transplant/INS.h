/*
 * INS.h
 * 惯性导航相关的头文件
 * Created on: 2025-01-07
 * Author: OUYQ
*/
#ifndef INS_H_
#define INS_H_

#include <math.h>
#include "drv_icm42688.h"
#include "Filter.h" 

void INS_init(void);
void INS_update(void);

#endif /* INS_H_ */