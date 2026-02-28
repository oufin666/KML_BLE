/*
 * PageFactory.h
 *
 *  Created on: Dec 26, 2025
 *      Author: wkt98
 */

#ifndef USER_APP_PAGES_INC_PAGEFACTORY_H_
#define USER_APP_PAGES_INC_PAGEFACTORY_H_

#include "PageBase.h"

class PageFactory
{
public:

    virtual PageBase* CreatePage(const char* name)
    {
        return nullptr;
    };
};



#endif /* USER_APP_PAGES_INC_PAGEFACTORY_H_ */
