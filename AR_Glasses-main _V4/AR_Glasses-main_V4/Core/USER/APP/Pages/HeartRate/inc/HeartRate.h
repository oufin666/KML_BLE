/*
 * HeartRate.h
 *
 *  Created on: Dec 30, 2025
 *      Author: wkt98
 */

#ifndef USER_APP_PAGES_HEARTRATE_INC_HEARTRATE_H_
#define USER_APP_PAGES_HEARTRATE_INC_HEARTRATE_H_





#include "HeartRateView.h"
#include "HeartRateModel.h"

namespace Page
{

class HeartRate : public PageBase
{
public:

public:
	HeartRate();
    virtual ~HeartRate();

    virtual void onCustomAttrConfig();
    virtual void onViewLoad();
    virtual void onViewDidLoad();
    virtual void onViewWillAppear();
    virtual void onViewDidAppear();
    virtual void onViewWillDisappear();
    virtual void onViewDidDisappear();
    virtual void onViewUnload();
    virtual void onViewDidUnload();

private:
    void Update();
    static void onTimer(lv_timer_t* timer);
    static void onEvent(lv_event_t* event);

private:
    HeartRateView View;
    HeartRateModel Model;
};

}








#endif /* USER_APP_PAGES_HEARTRATE_INC_HEARTRATE_H_ */
