/*
 * HeartRate.cpp
 *
 *  Created on: Dec 30, 2025
 *      Author: wkt98
 */


#include "HeartRate.h"

using namespace Page;

HeartRate::HeartRate()
{
}

HeartRate::~HeartRate()
{

}

void HeartRate::onCustomAttrConfig()
{
//    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void HeartRate::onViewLoad()
{
//    Model.Init();
//    Model.SetEncoderEnable(false);
    View.SetHeart(_root);
    lv_timer_t* timer = lv_timer_create(onTimer, 1000, this);
}

void HeartRate::onViewDidLoad()
{
}

void HeartRate::onViewWillAppear()
{

}

void HeartRate::onViewDidAppear()
{

}

void HeartRate::onViewWillDisappear()
{

}

void HeartRate::onViewDidDisappear()
{

}

void HeartRate::onViewUnload()
{

}

void HeartRate::onViewDidUnload()
{
}

void HeartRate::Update(){
	lv_label_set_text_fmt(View.label_heart, "%d", Model.getHeartRate());
}

void HeartRate::onTimer(lv_timer_t* timer)
{
	HeartRate* instance = (HeartRate*)timer->user_data;
	instance->Update();

//    instance->_Manager->Replace("Pages/_Template");
}

void HeartRate::onEvent(lv_event_t* event)
{
	HeartRate* instance = (HeartRate*)lv_event_get_user_data(event);
    LV_ASSERT_NULL(instance);

    lv_obj_t* obj = lv_event_get_current_target(event);
    lv_event_code_t code = lv_event_get_code(event);

    if (obj == instance->_root)
    {
        if (code == LV_EVENT_LEAVE)
        {
            //instance->Manager->Pop();
        }
    }
}





