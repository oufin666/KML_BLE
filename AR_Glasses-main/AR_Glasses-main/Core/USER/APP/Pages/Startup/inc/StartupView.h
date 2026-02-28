/*
 * StartupView.h
 *
 *  Created on: Dec 26, 2025
 *      Author: wkt98
 */

#ifndef USER_APP_PAGES_STARTUP_INC_STARTUPVIEW_H_
#define USER_APP_PAGES_STARTUP_INC_STARTUPVIEW_H_

#include "Page.h"

namespace Page
{

class StartupView
{
public:
    void Create(lv_obj_t* root);
    void Delete();

public:
    struct
    {
        lv_obj_t* cont;
        lv_obj_t* labelLogo;

        lv_anim_timeline_t* anim_timeline;
    } ui;

private:
};

}




#endif /* USER_APP_PAGES_STARTUP_INC_STARTUPVIEW_H_ */
