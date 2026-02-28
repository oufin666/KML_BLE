/*
 * Startup.h
 *
 *  Created on: Dec 26, 2025
 *      Author: wkt98
 */

#ifndef USER_APP_PAGES_STARTUP_INC_STARTUP_H_
#define USER_APP_PAGES_STARTUP_INC_STARTUP_H_

#include "StartUpView.h"
#include "StartUpModel.h"

namespace Page
{

class Startup : public PageBase
{
public:

public:
    Startup();
    virtual ~Startup();

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
    static void onTimer(lv_timer_t* timer);
    static void onEvent(lv_event_t* event);

private:
    StartupView View;
    StartupModel Model;
};

}



#endif /* USER_APP_PAGES_STARTUP_INC_STARTUP_H_ */
