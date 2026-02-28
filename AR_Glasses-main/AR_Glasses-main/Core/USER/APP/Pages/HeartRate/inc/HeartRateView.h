#ifndef __HEARTRATE_VIEW_H
#define __HEARTRATE_VIEW_H

#include "Page.h"

namespace Page
{

    class HeartRateView
    {
    public:
        void SetHeart(lv_obj_t* root);

    public:
        lv_obj_t* bg_box;
        lv_obj_t* label_heart;
        lv_obj_t* icon_heart;

    private:
        lv_obj_t* show_icon(lv_obj_t* parent, const void* img_src, lv_color_t color);

    };

}

#endif // !__VIEW_H
