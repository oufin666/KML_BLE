#ifndef __TEMPLATE_PRESENTER_H
#define __TEMPLATE_PRESENTER_H

#include "TemplateView.h"
#include "TemplateModel.h"

namespace Page
{

class Template : public PageBase
{
public:
    typedef struct
    {
        uint16_t time;
        lv_color_t color;
    } Param_t;

public:
    Template();
    virtual ~Template();

    virtual void onCustomAttrConfig();
    virtual void onViewLoad();
    virtual void onViewDidLoad();
    virtual void onViewWillAppear();
    virtual void onViewDidAppear();
    virtual void onViewWillDisappear();
    virtual void onViewDidDisappear();
    virtual void onViewUnload();
    virtual void onViewDidUnload();

    void updateCounter();                                                   //测试用，间隔X秒，按顺序自动传入测试数据中的经纬度信息
    void RefreshElevation(float latitude, float longititude, int pCounts); //测试用

private:
    void Update();
    void AttachEvent(lv_obj_t* obj);
    static void onTimerUpdate(lv_timer_t* timer);
    static void onEvent(lv_event_t* event);

    void ShowChart(int index_begin = 0, int index_end = -1);      //显示任意区域的海拔图数据
    void ShowPin(int index_pos);                                //移动标记线到对应点的位置

    void UpdateChart(float latitude, float longititude, int pCounts);            // 更新图表数据                             //更新图表数据
    void InitChart(float latitude, float longititude, int pCounts);                // 初始化数据

private:
    TemplateView View;
    TemplateModel Model;
    lv_timer_t* timer;
};

}

#endif
