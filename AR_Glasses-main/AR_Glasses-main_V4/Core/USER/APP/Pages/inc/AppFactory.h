/*
 * AppFactory.h
 *
 *  Created on: Dec 26, 2025
 *      Author: wkt98
 */

#ifndef USER_APP_PAGES_INC_APPFACTORY_H_
#define USER_APP_PAGES_INC_APPFACTORY_H_

#include "PageManager.h"

class AppFactory : public PageFactory
{
public:
    virtual PageBase* CreatePage(const char* name);
private:

};




#endif /* USER_APP_PAGES_INC_APPFACTORY_H_ */
