#include "HeartRateView.h"
#include <stdarg.h>
#include <stdio.h>

using namespace Page;

void HeartRateView::SetHeart(lv_obj_t* root) {

    bg_box = lv_obj_create(root);
    lv_obj_set_size(bg_box, 240, 240);
    lv_obj_set_style_bg_color(bg_box, lv_color_black(), 0);
    lv_obj_center(bg_box);
    lv_obj_set_style_bg_opa(bg_box, LV_OPA_COVER, 0);

    label_heart = lv_label_create(root);
    lv_label_set_text(label_heart, "000");
    lv_obj_set_style_text_font(label_heart, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_heart, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_heart, LV_ALIGN_BOTTOM_MID, 0, -20);

    icon_heart = show_icon(root, ResourcePool::GetImage("heart"), lv_color_hex(0xff3600));
    lv_obj_align(icon_heart, LV_ALIGN_TOP_MID, 0, 20);
}


lv_obj_t* HeartRateView::show_icon(lv_obj_t* parent, const void* img_src, lv_color_t color)
{
    if (parent == NULL) parent = lv_scr_act();   

    lv_obj_t* img = lv_img_create(parent);     
    lv_img_set_src(img, img_src);               

    lv_obj_set_style_img_recolor(img, color, 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);

    return img;
}
