/*
 * AppFactory.cpp
 *
 *  Created on: Dec 26, 2025
 *      Author: wkt98
 */

#include "AppFactory.h"

#include "../Template/inc/Template.h"
//#include "LiveMap/LiveMap.h"
//#include "Dialplate/Dialplate.h"
//#include "SystemInfos/SystemInfos.h"
#include "../Startup/inc/Startup.h"
#include "../HeartRate/inc/HeartRate.h"


#define APP_CLASS_MATCH(className)\
do{\
    if (strcmp(name, #className) == 0)\
    {\
        return new Page::className;\
    }\
}while(0)

PageBase* AppFactory::CreatePage(const char* name)
{

    APP_CLASS_MATCH(Template);
    /*
    APP_CLASS_MATCH(LiveMap);
    APP_CLASS_MATCH(Dialplate);
    APP_CLASS_MATCH(SystemInfos);*/
    APP_CLASS_MATCH(Startup);
    APP_CLASS_MATCH(HeartRate);

    return nullptr;
}


