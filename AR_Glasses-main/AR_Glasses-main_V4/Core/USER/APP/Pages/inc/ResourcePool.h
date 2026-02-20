/*
 * ResourcePool.h
 *
 *  Created on: Dec 26, 2025
 *      Author: wkt98
 */

#ifndef USER_APP_PAGES_INC_RESOURCEPOOL_H_
#define USER_APP_PAGES_INC_RESOURCEPOOL_H_

#include "lvgl/lvgl.h"

namespace ResourcePool
{

void Init();
lv_font_t* GetFont(const char* name);
const void* GetImage(const char* name);

}



#endif /* USER_APP_PAGES_INC_RESOURCEPOOL_H_ */
