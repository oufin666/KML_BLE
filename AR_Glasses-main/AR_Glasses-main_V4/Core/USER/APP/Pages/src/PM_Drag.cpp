/*
 * PM_Drag.cpp
 *
 *  Created on: Dec 26, 2025
 *      Author: wkt98
 */

#include "PageManager.h"
#include "PM_Log.h"
#include <stdlib.h>

#define CONSTRAIN(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

/* The distance threshold to trigger the drag */
#define PM_INDEV_DEF_DRAG_THROW    20

/**
  * @brief  Page drag event callback
  * @param  event: Pointer to event structure
  * @retval None
  */
void PageManager::onRootDragEvent(lv_event_t* event)
{
    lv_event_code_t eventCode = lv_event_get_code(event);

    if (!(eventCode == LV_EVENT_PRESSED || eventCode == LV_EVENT_PRESSING || eventCode == LV_EVENT_RELEASED))
    {
        return;
    }

    lv_obj_t* root = lv_event_get_current_target(event);
    PageBase* base = (PageBase*)lv_event_get_user_data(event);

    if (base == nullptr)
    {
        PM_LOG_ERROR("Page base is NULL");
        return;
    }

    PageManager* manager = base->_Manager;
    LoadAnimAttr_t animAttr;

    if (!manager->GetCurrentLoadAnimAttr(&animAttr))
    {
        PM_LOG_ERROR("Can't get current anim attr");
        return;
    }

    if (eventCode == LV_EVENT_PRESSED)
    {
        if (manager->_AnimState.IsSwitchReq)
        {
            return;
        }

        if (!manager->_AnimState.IsBusy)
        {
            return;
        }

        PM_LOG_INFO("Root anim interrupted");
        lv_anim_del(root, animAttr.setter);
        manager->_AnimState.IsBusy = false;

        /* Temporary showing the bottom page */
        PageBase* bottomPage = manager->GetStackTopAfter();
        lv_obj_clear_flag(bottomPage->_root, LV_OBJ_FLAG_HIDDEN);
    }
    else if (eventCode == LV_EVENT_PRESSING)
    {
        lv_coord_t cur = animAttr.getter(root);

        lv_coord_t max = std::max(animAttr.pop.exit.start, animAttr.pop.exit.end);
        lv_coord_t min = std::min(animAttr.pop.exit.start, animAttr.pop.exit.end);

        lv_point_t offset;
        lv_indev_get_vect(lv_indev_get_act(), &offset);

        if (animAttr.dragDir == ROOT_DRAG_DIR_HOR)
        {
            cur += offset.x;
        }
        else if (animAttr.dragDir == ROOT_DRAG_DIR_VER)
        {
            cur += offset.y;
        }

        animAttr.setter(root, CONSTRAIN(cur, min, max));
    }
    else if (eventCode == LV_EVENT_RELEASED)
    {
        if (manager->_AnimState.IsSwitchReq)
        {
            return;
        }

        lv_coord_t offset_sum = animAttr.push.enter.end - animAttr.push.enter.start;

        lv_coord_t x_predict = 0;
        lv_coord_t y_predict = 0;
        RootGetDragPredict(&x_predict, &y_predict);

        lv_coord_t start = animAttr.getter(root);
        lv_coord_t end = start;

        if (animAttr.dragDir == ROOT_DRAG_DIR_HOR)
        {
            end += x_predict;
            PM_LOG_INFO("Root drag x_predict = %d", end);
        }
        else if (animAttr.dragDir == ROOT_DRAG_DIR_VER)
        {
            end += y_predict;
            PM_LOG_INFO("Root drag y_predict = %d", end);
        }

        if (std::abs(end) > std::abs((int)offset_sum) / 2)
        {
            lv_async_call(onRootAsyncLeave, base);
        }
        else if(end != animAttr.push.enter.end)
        {
            manager->_AnimState.IsBusy = true;

            lv_anim_t a;
            manager->AnimDefaultInit(&a);
            lv_anim_set_user_data(&a, manager);
            lv_anim_set_var(&a, root);
            lv_anim_set_values(&a, start, animAttr.push.enter.end);
            lv_anim_set_exec_cb(&a, animAttr.setter);
            lv_anim_set_ready_cb(&a, onRootDragAnimFinish);
            lv_anim_start(&a);
            PM_LOG_INFO("Root drag anim start");
        }
    }
}

/**
  * @brief  Drag animation end event callback
  * @param  a: Pointer to animation
  * @retval None
  */
void PageManager::onRootDragAnimFinish(lv_anim_t* a)
{
    PageManager* manager = (PageManager*)lv_anim_get_user_data(a);
    PM_LOG_INFO("Root drag anim finish");
    manager->_AnimState.IsBusy = false;

    /* Hide the bottom page */
    PageBase* bottomPage = manager->GetStackTopAfter();
    if (bottomPage)
    {
        lv_obj_add_flag(bottomPage->_root, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
  * @brief  Enable root's drag function
  * @param  root: Pointer to the root object
  * @retval None
  */
void PageManager::RootEnableDrag(lv_obj_t* root)
{
    PageBase* base = (PageBase*)lv_obj_get_user_data(root);
    lv_obj_add_event_cb(
        root,
        onRootDragEvent,
        LV_EVENT_ALL,
        base
    );
    PM_LOG_INFO("Page(%s) Root drag enabled", base->_Name);
}

/**
  * @brief  Asynchronous callback when dragging ends
  * @param  data: Pointer to the base class of the page
  * @retval None
  */
void PageManager::onRootAsyncLeave(void* data)
{
    PageBase* base = (PageBase*)data;
    PM_LOG_INFO("Page(%s) send event: LV_EVENT_LEAVE, need to handle...", base->_Name);
    lv_event_send(base->_root, LV_EVENT_LEAVE, base);
}

/**
  * @brief  Get drag inertia prediction stop point
  * @param  x: x stop point
  * @param  y: y stop point
  * @retval None
  */
void PageManager::RootGetDragPredict(lv_coord_t* x, lv_coord_t* y)
{
    lv_indev_t* indev = lv_indev_get_act();
    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);

    lv_coord_t y_predict = 0;
    lv_coord_t x_predict = 0;

    while (vect.y != 0)
    {
        y_predict += vect.y;
        vect.y = vect.y * (100 - PM_INDEV_DEF_DRAG_THROW) / 100;
    }

    while (vect.x != 0)
    {
        x_predict += vect.x;
        vect.x = vect.x * (100 - PM_INDEV_DEF_DRAG_THROW) / 100;
    }

    *x = x_predict;
    *y = y_predict;
}


