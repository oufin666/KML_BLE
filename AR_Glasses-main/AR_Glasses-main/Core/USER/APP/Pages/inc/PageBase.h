/*
 * PageBase.h
 *
 *  Created on: Dec 26, 2025
 *      Author: wkt98
 */

#ifndef USER_APP_PAGES_INC_PAGEBASE_H_
#define USER_APP_PAGES_INC_PAGEBASE_H_

#include "lvgl/lvgl.h"

/* Generate stash area data */
#define PAGE_STASH_MAKE(data) {&(data), sizeof(data)}

/* Get the data in the stash area */
#define PAGE_STASH_POP(data)  this->StashPop(&(data), sizeof(data))

#define PAGE_ANIM_TIME_DEFAULT 500 //[ms]

#define PAGE_ANIM_PATH_DEFAULT lv_anim_path_ease_out

class PageManager;

class PageBase
{
public:

    /* Page state */
    typedef enum
    {
        PAGE_STATE_IDLE,
        PAGE_STATE_LOAD,
        PAGE_STATE_WILL_APPEAR,
        PAGE_STATE_DID_APPEAR,
        PAGE_STATE_ACTIVITY,
        PAGE_STATE_WILL_DISAPPEAR,
        PAGE_STATE_DID_DISAPPEAR,
        PAGE_STATE_UNLOAD,
        _PAGE_STATE_LAST
    } State_t;

    /* Stash data area */
    typedef struct
    {
        void* ptr;
        uint32_t size;
    } Stash_t;

    /* Page switching animation properties */
    typedef struct
    {
        uint8_t Type;
        uint16_t Time;
        lv_anim_path_cb_t Path;
    } AnimAttr_t;

public:
    lv_obj_t* _root;       // UI root node
    PageManager* _Manager; // Page manager pointer
    const char* _Name;     // Page name
    uint16_t _ID;          // Page ID
    void* _UserData;       // User data pointer

    /* Private data, Only page manager access */
    struct
    {
        bool ReqEnableCache;        // Cache enable request
        bool ReqDisableAutoCache;   // Automatic cache management enable request

        bool IsDisableAutoCache;    // Whether it is automatic cache management
        bool IsCached;              // Cache enable

        Stash_t Stash;              // Stash area
        State_t State;              // Page state

        /* Animation state  */
        struct
        {
            bool IsEnter;           // Whether it is the entering party
            bool IsBusy;            // Whether the animation is playing
            AnimAttr_t Attr;        // Animation properties
        } Anim;
    } priv;

public:
    virtual ~PageBase() {}

    /* Synchronize user-defined attribute configuration */
    virtual void onCustomAttrConfig() {}

    /* Page load start */
    virtual void onViewLoad() {}

    /* Page load end */
    virtual void onViewDidLoad() {}

    /* Page appear animation start */
    virtual void onViewWillAppear() {}

    /* Page appear animation end  */
    virtual void onViewDidAppear() {}

    /* Page disappear animation start */
    virtual void onViewWillDisappear() {}

    /* Page disappear animation end */
    virtual void onViewDidDisappear() {}

    /* Page unload start */
    virtual void onViewUnload() {}

    /* Page unload end */
    virtual void onViewDidUnload() {}

    /* Set whether to manually manage the cache */
    void SetCustomCacheEnable(bool en);

    /* Set whether to enable automatic cache */
    void SetCustomAutoCacheEnable(bool en);

    /* Set custom animation properties  */
    void SetCustomLoadAnimType(
        uint8_t animType,
        uint16_t time = PAGE_ANIM_TIME_DEFAULT,
        lv_anim_path_cb_t path = PAGE_ANIM_PATH_DEFAULT
    );

    /* Pop the data from stash area */
    bool StashPop(void* ptr, uint32_t size);
};



#endif /* USER_APP_PAGES_INC_PAGEBASE_H_ */
