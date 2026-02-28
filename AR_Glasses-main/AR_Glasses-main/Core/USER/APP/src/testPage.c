#include "testPage.h"
#include "lvgl.h"
#include <stdio.h>
#include "BLE.h"

#ifndef LV_SYMBOL_HEART
#define LV_SYMBOL_HEART "\xEF\x80\x84" /* 对应 FontAwesome 的心形编码 */
#endif

// 全局变量：用于滑块值更新
static lv_obj_t *slider_label;

// 静态变量：保存控件指针，只创建一次
static lv_obj_t *time_label = NULL;
static lv_obj_t *date_label = NULL;
static lv_obj_t *lat_label = NULL;
static lv_obj_t *lon_label = NULL;
static lv_obj_t *test_label = NULL;
static lv_obj_t *speed_label = NULL;
static lv_obj_t *direction_label = NULL;

/**
 * @brief 按钮点击事件回调（弹出提示）
 */
static void btn_click_event_cb(lv_event_t *e) {
	lv_obj_t *btn = lv_event_get_target(e);
	lv_obj_t *toast = lv_label_create(lv_scr_act());
	lv_label_set_text(toast, "Button Clicked!");
	lv_obj_set_style_text_align(toast, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_size(toast, 180, 40);
	lv_obj_align(toast, LV_ALIGN_CENTER, 0, 0);
	// 3秒后自动删除提示
	/*
	 lv_timer_t *timer = lv_timer_create(
	 [](lv_timer_t *t) { lv_obj_del(lv_timer_get_user_data(t)); },
	 3000, toast
	 );*/
}

/**
 * @brief 滑块值变化事件回调（更新标签显示）
 */
static void slider_value_change_cb(lv_event_t *e) {
	lv_obj_t *slider = lv_event_get_target(e);
	int32_t value = lv_slider_get_value(slider);
	// 更新标签显示当前滑块值
	lv_label_set_text_fmt(slider_label, "Brightness: %d%%", value);
}

/**
 * @brief 创建ST7789 240*240屏幕的LVGL测试页面
 */
void lvgl_st7789_test_page_create(void) {
	// 1. 获取/创建默认编码器组（用于控件交互）
	lv_group_t *default_group = lv_group_get_default();
	if (default_group == NULL) {
		default_group = lv_group_create();
		lv_group_set_default(default_group);
	}

	// 2. 创建顶部标题标签（适配240*240屏幕，居中顶部）
	lv_obj_t *title_label = lv_label_create(lv_scr_act());
	lv_label_set_text(title_label, "LVGL Test Page");
	lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0); // 适配屏幕的字体大小
	lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_size(title_label, 220, 30); // 宽度不超过屏幕（240）
	lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 10); // 顶部中间，偏移10px

	// 3. 创建开关控件（类似用户示例，适配240*240屏幕尺寸）
	lv_obj_t *switch_obj = lv_switch_create(lv_scr_act());
	lv_obj_set_size(switch_obj, 100, 60); // 尺寸适配240屏幕，避免过大
	lv_obj_align(switch_obj, LV_ALIGN_TOP_MID, 0, 50); // 标题下方，偏移50px
	lv_group_add_obj(default_group, switch_obj); // 绑定编码器

	// 4. 创建交互按钮（带点击事件）
	lv_obj_t *test_btn = lv_btn_create(lv_scr_act());
	lv_obj_set_size(test_btn, 80, 40); // 适配屏幕的按钮尺寸
	lv_obj_align(test_btn, LV_ALIGN_TOP_MID, 0, 120); // 开关下方，偏移120px
	// 按钮文本标签
	lv_obj_t *btn_label = lv_label_create(test_btn);
	lv_label_set_text(btn_label, "Click Me");
	lv_obj_center(btn_label);
	// 绑定点击事件
	lv_obj_add_event_cb(test_btn, btn_click_event_cb, LV_EVENT_CLICKED, NULL);
	lv_group_add_obj(default_group, test_btn); // 绑定编码器

	// 5. 创建滑块控件（控制示例：亮度）
	lv_obj_t *brightness_slider = lv_slider_create(lv_scr_act());
	lv_obj_set_size(brightness_slider, 180, 20); // 宽度适配屏幕（240）
	lv_obj_align(brightness_slider, LV_ALIGN_TOP_MID, 0, 170); // 按钮下方，偏移170px
	lv_slider_set_range(brightness_slider, 0, 100); // 范围：0~100%
	lv_slider_set_value(brightness_slider, 50, LV_ANIM_ON); // 默认值50%
	// 滑块值显示标签
	slider_label = lv_label_create(lv_scr_act());
	lv_label_set_text_fmt(slider_label, "Brightness: 50%%");
	lv_obj_set_size(slider_label, 120, 20);
	lv_obj_align_to(slider_label, brightness_slider, LV_ALIGN_BOTTOM_MID, 0,
			10); // 滑块下方
	// 绑定值变化事件
	lv_obj_add_event_cb(brightness_slider, slider_value_change_cb,
			LV_EVENT_VALUE_CHANGED, NULL);
	lv_group_add_obj(default_group, brightness_slider); // 绑定编码器

	// 6. 创建复选框控件（示例：使能背光）
	lv_obj_t *checkbox_obj = lv_checkbox_create(lv_scr_act());
	lv_checkbox_set_text(checkbox_obj, " Enable Backlight");
	lv_obj_set_size(checkbox_obj, 150, 30);
	lv_obj_align(checkbox_obj, LV_ALIGN_TOP_MID, 0, 210); // 滑块下方，适配240屏幕底部
	//   lv_checkbox_set_checked(checkbox_obj, true); // 默认勾选
	lv_group_add_obj(default_group, checkbox_obj); // 绑定编码器
}

void update_gps_display(lv_obj_t *parent, GPS_Data *data) {
	// 初始化控件（只执行一次）
		if (time_label == NULL) {
			lv_obj_t *parent = lv_scr_act();
			time_label = lv_label_create(parent);
			lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 5, 5);

			date_label = lv_label_create(parent);
			lv_obj_align_to(date_label, time_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

			lat_label = lv_label_create(parent);
			lv_obj_align_to(lat_label, date_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

			lon_label = lv_label_create(parent);
			lv_obj_align_to(lon_label, lat_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

			speed_label = lv_label_create(parent);
			lv_obj_align_to(speed_label, lon_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
			direction_label = lv_label_create(parent);
						lv_obj_align_to(direction_label,speed_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
		}

		lv_label_set_text_fmt(time_label, "time: %02d:%02d:%02d", data->hour,
				data->minute, data->second);

		lv_label_set_text_fmt(date_label, "data: %04d-%02d-%02d", data->year,
				data->month, data->day);

		// 经纬度显示

		lv_label_set_text_fmt(lat_label, "lat: %.3f°%c", fabs(data->latitude),
				data->latitude >= 0 ? 'N' : 'S');

		lv_label_set_text_fmt(lon_label, "lon: %.3f°%c", fabs(data->longitude),
				data->longitude >= 0 ? 'E' : 'W');

		lv_label_set_text_fmt(speed_label, "speed: %.1f km/h", data->speed_km_h);
		lv_label_set_text_fmt(direction_label, "direction: %.1f°", data->course);

}

void update_gps_display_forTestUpdateFre(lv_obj_t *parent, GPS_Data *data,
		int testCount) {
	// 初始化控件（只执行一次）
	if (time_label == NULL) {
		lv_obj_t *parent = lv_scr_act();
		time_label = lv_label_create(parent);
		lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 5, 5);

		date_label = lv_label_create(parent);
		lv_obj_align_to(date_label, time_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

		lat_label = lv_label_create(parent);
		lv_obj_align_to(lat_label, date_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

		lon_label = lv_label_create(parent);
		lv_obj_align_to(lon_label, lat_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

		test_label = lv_label_create(parent);
		lv_obj_align_to(test_label, lon_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
	}
	lv_label_set_text_fmt(time_label, "time: %02d:%02d:%02d", data->hour,
			data->minute, data->second);

	lv_label_set_text_fmt(date_label, "data: %04d-%02d-%02d", data->year,
			data->month, data->day);

	// 经纬度显示

	lv_label_set_text_fmt(lat_label, "lat: %.3f°%c", fabs(data->latitude),
			data->latitude >= 0 ? 'N' : 'S');

	lv_label_set_text_fmt(lon_label, "lon: %.3f°%c", fabs(data->longitude),
			data->longitude >= 0 ? 'E' : 'W');

	lv_label_set_text_fmt(test_label, "testCount: %d", testCount);
}




// 假设这是你获取心率的函数声明
// 在实际代码中，你需要包含定义这个函数的头文件


// 定义一个全局或静态的 label 指针，或者通过 user_data 传递
static lv_obj_t * hr_label;

/**
 * @brief 定时器回调函数，用于刷新心率显示
 * @param timer 定时器对象指针
 */
static void hr_update_timer_cb(lv_timer_t * timer)
{
    // 1. 获取心率数据
    int current_hr = Get_Heart_Rate();

    // 2. 更新 Label 的文本
    // 使用 lv_label_set_text_fmt 可以像 printf 一样格式化字符串
    // LV_SYMBOL_HEART 是 LVGL 自带的心形图标
    if(hr_label != NULL) {
        lv_label_set_text_fmt(hr_label, "%s %d BPM", LV_SYMBOL_HEART, current_hr);
    }
}

/**
 * @brief 初始化心率显示界面
 */
void lv_example_hr_display(void)
{
    // --- 1. 创建显示 Label ---
    hr_label = lv_label_create(lv_scr_act()); // 在当前活动屏幕上创建

    // 设置字体 (可选，如果没有大字体，可以使用默认的 &lv_font_montserrat_14)
    // 注意：使用大字体需要在 lv_conf.h 中开启对应的宏
    lv_obj_set_style_text_font(hr_label, &lv_font_montserrat_24, 0);

    // 设置对齐方式：屏幕居中
    lv_obj_align(hr_label, LV_ALIGN_CENTER, 0, 0);

    // 初始化显示内容
    lv_label_set_text(hr_label, "Waiting...");

    // --- 2. 创建定时器 ---
    // 创建一个 LVGL 定时器，每 1000ms (1秒) 调用一次回调函数
    // 这样可以避免频繁刷新导致屏幕闪烁或占用过多 CPU
    lv_timer_create(hr_update_timer_cb, 1000, NULL);
}

