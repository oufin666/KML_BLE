/*
 * StartupModel.h
 *
 *  Created on: Dec 26, 2025
 *      Author: wkt98
 */

#ifndef USER_APP_PAGES_STARTUP_INC_STARTUPMODEL_H_
#define USER_APP_PAGES_STARTUP_INC_STARTUPMODEL_H_

/*
#include "Common/DataProc/DataProc.h"
#include "Common/HAL/HAL.h"
*/
namespace Page
{

class StartupModel
{
public:
    void Init();
    void Deinit();
    void PlayMusic(const char* music);
    void SetEncoderEnable(bool en)
    {
 //       HAL::Encoder_SetEnable(en);
    }
    void SetStatusBarAppear(bool en);

private:
//    Account* account;
};

}




#endif /* USER_APP_PAGES_STARTUP_INC_STARTUPMODEL_H_ */
