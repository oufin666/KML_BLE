#ifndef __TEMPLATE_MODEL_H
#define __TEMPLATE_MODEL_H
#include <string>
#include <vector>
#include "lvgl/lvgl.h"
#include "track_param.h"
#include "main.h"
#include "GPS.h"



namespace Page
{

class TemplateModel
{

public:
    uint32_t TickSave;
    uint32_t GetData();

    //获取经纬度、海拔数据数组
/*    struct RawCoordinates {
        float latitude;
        float longitude;
        float altitude;
    };*/



    std::vector<RawCoordinates> rawCoordinates;

    //标记点
    struct PinCoordinate {

        int index_now;      //当前标记点位于rawCoordinates中的序列
        int pointCounts;  //默认的刷新点数
        bool isInit;    //用于判断是否完成初始化

        PinCoordinate()          // C++98 合法
            : index_now(0), pointCounts(3000), isInit(false)
        {}
    };
    PinCoordinate pinCoordinate;

    //用于分解海拔高度，按照最低，最高
    int min_altitude;
    int max_altitude;
    //全程距离以及对应的爬山
    float Distance;
    int Ascent;

    //解析KML文件，获取其中的经纬度数据
    void KMLParser(); //测试用，210个数据

    //计算N组经纬度之间的距离，以及对应的路程
    float CalcDistance(std::vector<RawCoordinates>& points,int index_begin = 0,int index_end = -1);
    //计算两经纬度之间的距离
    float distanceMeters(float lat1, float lon1, float lat2, float lon2);
    //计算N组海拔数据后的爬升
    int CalcAscent(std::vector<RawCoordinates>& points, int index_begin = 0, int index_end = -1);

    //更新折线图的显示
    void UpdateData(std::vector<RawCoordinates>& points, int index_begin = 0, int index_end = -1);

    //根据GPS点经纬度，获取列表中距离最近的一个,可传入索引匹配计算
    int MatchNearestPoint(std::vector<RawCoordinates>& points, double latitude,double longitude, int index_begin = 0, int index_end = -1);

public:
    //记录GPS的经纬度数据
    struct GPSData_t {
        bool hasLast; //判断是否是第一次传入数据

        float current_latitude;
        float current_longitude;             // 构造函数赋初值

        GPSData_t()                         // C++98 合法
            : hasLast(false), current_latitude(-1), current_longitude(-1)
        {}
    };
    GPSData_t  gpsCoordinate;

    //获取GPS数据用
//    void Init();
//    void Deinit();
    void GetGPS_Info(GPS_Data* info, int gpsDistance = 4);
//    void UpdateGPS(float latitude, float longitude, int gpsDistance = 2);    //测试用，根据GPS经纬度刷新数据,2米刷新一次


private:
 //   Account* account;   //订阅GPS数据
//    static int onEvent(Account* account, Account::EventParam_t* param);
    GPS_Data gpsInfo;  // 存储GPS数据
};

}

#endif
