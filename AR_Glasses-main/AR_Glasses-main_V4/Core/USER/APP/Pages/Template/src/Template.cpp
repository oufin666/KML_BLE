#include "Template.h"
//#include"Utils/FileWrite/FileWrite.h"

using namespace Page;

Template::Template()
    : timer(nullptr)
{
}

Template::~Template()
{

}

/* ---------------------------------------------------------- */

/* -------------- 测试用数据（C++98 方式填充） -------------- */
/*
static std::vector<TemplateModel::RawCoordinates> TestCoordinate;
static struct TestCoordinateInit
{
    static void add(float lat, float lon, float alt)
    {
        TemplateModel::RawCoordinates rc;
        rc.latitude = lat;
        rc.longitude = lon;
        rc.altitude = alt;
        TestCoordinate.push_back(rc);
    }
    TestCoordinateInit()
    {
        add(23.184323f, 113.266400f, 9.014f);
        add(23.183718f, 113.267541f, 10.101f);
        add(23.183450f, 113.268075f, 11.078f);
        add(23.182809f, 113.269273f, 12.505f);
        add(23.181913f, 113.271203f, 17.427f);
        add(23.182001f, 113.272198f, 18.955f);
        add(23.182846f, 113.272349f, 17.847f);
        add(23.183490f, 113.271986f, 16.184f);
        add(23.184104f, 113.272324f, 17.339f);
        add(23.184415f, 113.272790f, 17.345f);
        add(23.184654f, 113.273093f, 18.336f);
        add(23.184756f, 113.273609f, 22.866f);
        add(23.184512f, 113.274102f, 24.671f);
        add(23.184573f, 113.274209f, 24.145f);
        add(23.184561f, 113.273973f, 25.734f);
        add(23.183787f, 113.273743f, 19.917f);
        add(23.182888f, 113.273467f, 19.950f);
        add(23.181711f, 113.273119f, 21.858f);
        add(23.181335f, 113.273313f, 28.674f);
        add(23.180725f, 113.273432f, 20.875f);
        add(23.180304f, 113.273515f, 36.323f);
        add(23.179790f, 113.273412f, 36.369f);
        add(23.179436f, 113.273601f, 29.718f);
        add(23.179443f, 113.274014f, 37.189f);
        add(23.179603f, 113.274420f, 34.470f);
        add(23.179817f, 113.274929f, 33.288f);
        add(23.179945f, 113.275343f, 32.471f);
        add(23.179598f, 113.275585f, 27.372f);
        add(23.178963f, 113.275012f, 23.881f);
        add(23.178532f, 113.274894f, 23.909f);
        add(23.177758f, 113.275005f, 25.338f);
        add(23.177734f, 113.275168f, 32.418f);
        add(23.178198f, 113.275385f, 51.184f);
        add(23.178224f, 113.275858f, 68.117f);
        add(23.178357f, 113.276249f, 83.905f);
        add(23.178679f, 113.276836f, 89.619f);
        add(23.178264f, 113.277517f, 95.651f);
        add(23.178298f, 113.278313f, 97.996f);
        add(23.178453f, 113.279315f, 131.425f);
    }
} _testInit;*/


//优化版本，当超出数组时候，只显示剩余数组
void Template::ShowChart(int index_begin, int index_end) {

    if (index_end == -1) {
        int size_points = Model.rawCoordinates.size();

        Model.UpdateData(Model.rawCoordinates, 0, size_points - 1);
        lv_label_set_text_fmt(View.segmentChart.label_altitude_max, "%d", Model.max_altitude);
        lv_label_set_text_fmt(View.segmentChart.label_altitude_min, "%d", Model.min_altitude);
        lv_label_set_text_fmt(View.segmentChart.label_distance_end, "%.2f", Model.Distance);
        lv_label_set_text_fmt(View.segmentChart.label_distance, "%.2f", Model.Distance);
        lv_label_set_text_fmt(View.segmentChart.label_altitude, "%d", Model.Ascent);

        const uint32_t point_cnt = static_cast<uint32_t>(size_points);

        lv_coord_t minv = Model.rawCoordinates[index_begin].altitude, maxv = Model.rawCoordinates[index_begin].altitude;
        for (uint32_t i = 1; i < point_cnt; i++) {
            if (Model.rawCoordinates[i].altitude < minv) minv = Model.rawCoordinates[i].altitude;
            if (Model.rawCoordinates[i].altitude > maxv) maxv = Model.rawCoordinates[i].altitude;
        }
        lv_coord_t margin = (maxv - minv) / 10 + 1;

        lv_chart_set_range(View.segmentChart.chart, LV_CHART_AXIS_PRIMARY_Y, minv - margin, maxv + margin);
        lv_chart_set_point_count(View.segmentChart.chart, point_cnt);

        for (uint32_t i = 0; i < point_cnt; i++) {
            lv_chart_set_next_value(View.segmentChart.chart, View.segmentChart.ser, Model.rawCoordinates[i].altitude);
        }
        lv_chart_refresh(View.segmentChart.chart);
    }

    if (index_end >= Model.rawCoordinates.size()) {

        index_end = Model.rawCoordinates.size() - 1;

        Model.UpdateData(Model.rawCoordinates, index_begin, index_end);
        lv_label_set_text_fmt(View.segmentChart.label_altitude_max, "%d", Model.max_altitude);
        lv_label_set_text_fmt(View.segmentChart.label_altitude_min, "%d", Model.min_altitude);
        lv_label_set_text_fmt(View.segmentChart.label_distance_end, "%.f", Model.Distance);
        lv_label_set_text_fmt(View.segmentChart.label_distance, "%.f", Model.Distance);
        lv_label_set_text_fmt(View.segmentChart.label_altitude, "%d", Model.Ascent);

        const uint32_t point_cnt = static_cast<uint32_t>(index_end - index_begin);

        lv_coord_t minv = Model.rawCoordinates[index_begin].altitude, maxv = Model.rawCoordinates[index_begin].altitude;
        for (uint32_t i = index_begin; i < index_end; i++) {
            if (Model.rawCoordinates[i].altitude < minv) minv = Model.rawCoordinates[i].altitude;
            if (Model.rawCoordinates[i].altitude > maxv) maxv = Model.rawCoordinates[i].altitude;
        }
        lv_coord_t margin = (maxv - minv) / 10 + 1;

        lv_chart_set_range(View.segmentChart.chart, LV_CHART_AXIS_PRIMARY_Y, minv - margin, maxv + margin);
        lv_chart_set_point_count(View.segmentChart.chart, point_cnt);

        lv_chart_set_all_value(View.segmentChart.chart, View.segmentChart.ser, LV_CHART_POINT_NONE);

        for (uint32_t i = index_begin; i < index_end; i++) {
            lv_chart_set_next_value(View.segmentChart.chart, View.segmentChart.ser, Model.rawCoordinates[i].altitude);
        }
        lv_chart_refresh(View.segmentChart.chart);
    }

    else {

        Model.UpdateData(Model.rawCoordinates, index_begin, index_end);
        lv_label_set_text_fmt(View.segmentChart.label_altitude_max, "%d", Model.max_altitude);
        lv_label_set_text_fmt(View.segmentChart.label_altitude_min, "%d", Model.min_altitude);
        lv_label_set_text_fmt(View.segmentChart.label_distance_end, "%.2f", Model.Distance);
        lv_label_set_text_fmt(View.segmentChart.label_distance, "%.2f", Model.Distance);
        lv_label_set_text_fmt(View.segmentChart.label_altitude, "%d", Model.Ascent);

        const uint32_t point_cnt = static_cast<uint32_t>(index_end - index_begin);

        lv_coord_t minv = Model.rawCoordinates[index_begin].altitude, maxv = Model.rawCoordinates[index_begin].altitude;
        for (uint32_t i = index_begin; i < index_end; i++) {
            if (Model.rawCoordinates[i].altitude < minv) minv = Model.rawCoordinates[i].altitude;
            if (Model.rawCoordinates[i].altitude > maxv) maxv = Model.rawCoordinates[i].altitude;
        }
        lv_coord_t margin = (maxv - minv) / 10 + 1;

        lv_chart_set_range(View.segmentChart.chart, LV_CHART_AXIS_PRIMARY_Y, minv - margin, maxv + margin);
        lv_chart_set_point_count(View.segmentChart.chart, point_cnt);

        lv_chart_set_all_value(View.segmentChart.chart, View.segmentChart.ser, LV_CHART_POINT_NONE);

        for (uint32_t i = index_begin; i < index_end; i++) {
            lv_chart_set_next_value(View.segmentChart.chart, View.segmentChart.ser, Model.rawCoordinates[i].altitude);
        }
        lv_chart_refresh(View.segmentChart.chart);
    }

};

void Template::ShowPin(int index_pos) {
    lv_obj_set_x(View.segmentChart.line_pin, View.segmentChart.x + index_pos);
}

void Template::InitChart(float latitude, float longititude, int pCounts) {
    //如果未能识别到GPS数据，则使用默认经纬度
    if (latitude == -1 || longititude == -1) {
        Model.pinCoordinate.index_now = 0;//使用默认点坐标
        ShowChart(Model.pinCoordinate.index_now * Model.pinCoordinate.pointCounts, (Model.pinCoordinate.index_now + 1) * Model.pinCoordinate.pointCounts - 1);
        ShowPin(0);
    }
    else {
        //获取当前经纬度最近的经纬度坐标
        int iTemp = Model.MatchNearestPoint(Model.rawCoordinates, latitude, longititude);


        //如果最近一点的距离仍然过远，则使用起始点为默认位置
        if (iTemp == -1) {
            Model.pinCoordinate.index_now = 0;//使用默认点坐标
            ShowChart(Model.pinCoordinate.index_now * Model.pinCoordinate.pointCounts, (Model.pinCoordinate.index_now + 1) * Model.pinCoordinate.pointCounts - 1);
            ShowPin(0);
        }

        else {

            int iQuotient = iTemp / Model.pinCoordinate.pointCounts;   //获取商

            int iRemainder = iTemp % Model.pinCoordinate.pointCounts;  //获取余数
            int iMax = (iQuotient + 1) * Model.pinCoordinate.pointCounts;         //判断是否超过点数


            ShowChart(iQuotient * Model.pinCoordinate.pointCounts, (iQuotient + 1) * Model.pinCoordinate.pointCounts - 1);

            if (iMax <= Model.rawCoordinates.size()) {
                ShowPin(iRemainder);
            }
            else {
                int iRemainderAll = ((float(iRemainder) / (Model.rawCoordinates.size() % Model.pinCoordinate.pointCounts)) * 210);
                ShowPin(iRemainderAll);
            }

            //更新标记点数据
            Model.pinCoordinate.index_now = iTemp;                        //获取定位的序列号
            Model.pinCoordinate.isInit = true;
        }

    }

}

void Template::UpdateChart(float latitude, float longititude, int pCounts) {

    //获取新索引值
    //int iTemp = Model.MatchNearestPoint(Model.rawCoordinates, latitude, longititude,Model.pinCoordinate.index_now-20, Model.pinCoordinate.index_now + 20);
    //测试用
    int iTemp = Model.MatchNearestPoint(Model.rawCoordinates, latitude, longititude);

    //判断是否需要刷新图表数据
    if (iTemp == Model.pinCoordinate.index_now || iTemp == -1) {
        //Test1.WriteDebugText("UpdateChart:default points");
        return;
    }
    else {
        int iQuotient = iTemp / Model.pinCoordinate.pointCounts;   //获取商

        int iRemainder = iTemp % Model.pinCoordinate.pointCounts;  //获取余数
        int iMax = (iQuotient + 1) * Model.pinCoordinate.pointCounts;         //判断是否超过点数

        ShowChart(iQuotient * Model.pinCoordinate.pointCounts, (iQuotient + 1) * Model.pinCoordinate.pointCounts - 1);

        if (iMax <= Model.rawCoordinates.size()) {
            ShowPin(iRemainder);
            //Test1.WriteDebugText("UpdateChart:IN MAX");
        }
        else {
            int iRemainderAll = ((float(iRemainder) / (Model.rawCoordinates.size() % Model.pinCoordinate.pointCounts)) * 210);
            ShowPin(iRemainderAll);
        }

        //更新标记点数据
        Model.pinCoordinate.index_now = iTemp;                        //获取定位的序列号
        Model.pinCoordinate.isInit = true;
    }


}

void Template::RefreshElevation(float latitude, float longititude, int pCounts) {

    if (latitude == -1 || longititude == -1) {
        InitChart(0, pCounts, pCounts);
        lv_label_set_text_fmt(View.gpsDatas.label_latitude, "%.f", Model.gpsCoordinate.current_latitude);
        lv_label_set_text_fmt(View.gpsDatas.label_longitude, "%.f", Model.gpsCoordinate.current_longitude);
        lv_label_set_text_fmt(View.gpsDatas.label_debug, "Init");
    }

    if (Model.pinCoordinate.isInit == false) {
        InitChart(latitude, longititude, pCounts);
        lv_label_set_text_fmt(View.gpsDatas.label_latitude, "%.f", Model.gpsCoordinate.current_latitude);
        lv_label_set_text_fmt(View.gpsDatas.label_longitude, "%.f", Model.gpsCoordinate.current_longitude);
        lv_label_set_text_fmt(View.gpsDatas.label_debug, "Init");
    }

    else
    {
        UpdateChart(latitude, longititude, pCounts);

        lv_label_set_text_fmt(View.gpsDatas.label_latitude, "%.f", Model.gpsCoordinate.current_latitude);
        lv_label_set_text_fmt(View.gpsDatas.label_longitude, "%.f", Model.gpsCoordinate.current_longitude);
        lv_label_set_text_fmt(View.gpsDatas.label_debug, "Update");
    }
}


static int counter = 0;
static lv_timer_t* counter_timer = nullptr;
static int counter_chart = 0;

static void counter_timer_cb(lv_timer_t* timer) {
    Template* instance = (Template*)timer->user_data;
    instance->updateCounter();
}

int i = 0;
/*
void Template::updateCounter() {

    counter++;
    if (counter >= 210) {
        counter = 0;
        i += 1;
    }

    if (i < TestCoordinate.size()) {
        RefreshElevation(TestCoordinate[i].latitude, TestCoordinate[i].longitude, Model.pinCoordinate.pointCounts);
    }
    else {
        counter_chart = 0;
        i = 0;
        RefreshElevation(TestCoordinate[i].latitude, TestCoordinate[i].longitude, Model.pinCoordinate.pointCounts);
    }

}*/

void Template::onCustomAttrConfig()
{
    LV_LOG_USER(__func__);
    SetCustomCacheEnable(true);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_OVER_BOTTOM, 1000, lv_anim_path_bounce);
}


void Template::onViewLoad()
{
    LV_LOG_USER(__func__);
    AttachEvent(_root);

    View.InitChart(_root);
    Model.KMLParser();

    //counter_timer = lv_timer_create(counter_timer_cb, 20, this);

}

void Template::onViewDidLoad()
{
    LV_LOG_USER(__func__);
}

void Template::onViewWillAppear()
{
    LV_LOG_USER(__func__);
    Param_t param;
    param.color = lv_color_white();
    param.time = 1000;

    PAGE_STASH_POP(param);

    lv_obj_set_style_bg_color(_root, param.color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(_root, LV_OPA_COVER, LV_PART_MAIN);

    timer = lv_timer_create(onTimerUpdate, param.time, this);

    //初始化数据
 //   Model.Init();

}

void Template::onViewDidAppear()
{
    LV_LOG_USER(__func__);
}

void Template::onViewWillDisappear()
{
    LV_LOG_USER(__func__);
}

void Template::onViewDidDisappear()
{
    LV_LOG_USER(__func__);
    lv_timer_del(timer);
}

void Template::onViewUnload()
{
    LV_LOG_USER(__func__);
}

void Template::onViewDidUnload()
{
    LV_LOG_USER(__func__);
}

void Template::AttachEvent(lv_obj_t* obj)
{
    lv_obj_set_user_data(obj, this);
    lv_obj_add_event_cb(obj, onEvent, LV_EVENT_ALL, this);
}

void Template::Update()
{
	GPS_Data gpsInfo;
    Model.GetGPS_Info(&gpsInfo);

    RefreshElevation(Model.gpsCoordinate.current_latitude, Model.gpsCoordinate.current_longitude, Model.pinCoordinate.pointCounts);

}

void Template::onTimerUpdate(lv_timer_t* timer)
{
    Template* instance = (Template*)timer->user_data;

    instance->Update();
}

void Template::onEvent(lv_event_t* event)
{
    Template* instance = (Template*)lv_event_get_user_data(event);
    LV_ASSERT_NULL(instance);

    lv_obj_t* obj = lv_event_get_current_target(event);
    lv_event_code_t code = lv_event_get_code(event);

    if (obj == instance->_root)
    {
        if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LEAVE)
        {
            instance->_Manager->Pop();
        }
    }
}
