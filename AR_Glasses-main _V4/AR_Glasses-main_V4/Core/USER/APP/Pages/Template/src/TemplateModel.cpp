#include "TemplateModel.h"
#include <string>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include "kml_parse.h"

using namespace Page;

uint32_t TemplateModel::GetData()
{
    return lv_tick_get();
}

inline float deg2rad(float deg) {
    return deg * 3.1415926f / 180.0f;
}

float TemplateModel::distanceMeters(float lat1, float lon1, float lat2, float lon2)
{

    const float dLat = deg2rad(lat2 - lat1);
    const float dLon = deg2rad(lon2 - lon1);

    const float a =
        sin(dLat / 2) * sin(dLat / 2) +
        cos(deg2rad(lat1)) * cos(deg2rad(lat2)) *
        sin(dLon / 2) * sin(dLon / 2);

    const float c = 2.0f * atan2(sqrt(a), sqrt(1.0f - a));
    return 6371000.0f * c;
}


void TemplateModel::KMLParser() {
     //push_back,逐行添加数据
	const char* kml_file_path = "S:/track/DFS.kml";
	const char* parse_file_path = "S:/track/DFS.txt";
	rawCoordinates.clear();
	//int count = kml_parse_coordinates(kml_file_path, rawCoordinates);
	//int count = kml_parse_to_file(kml_file_path,parse_file_path);
	//int count = count_kml_points(kml_file_path);
	int count = kml_filter_to_file(kml_file_path,parse_file_path);
	load_points_from_file(parse_file_path,rawCoordinates,count);
	if (count < 0) {
	        LV_LOG_USER("解析失败！");
	}
	else {
	        LV_LOG_USER("成功解析了 %d 个坐标点", count);
	}
/*
    rawCoordinates.clear();
    struct Local {
        static void add(TemplateModel* m,
            float lat, float lon, float alt)
        {
            //TemplateModel::RawCoordinates rc;
        	RawCoordinates rc;
            rc.latitude = lat;
            rc.longitude = lon;
            rc.altitude = alt;
            m->rawCoordinates.push_back(rc);
        }
    };

    Local::add(this, 23.038644f, 113.377181f, 16.398987f);
    Local::add(this, 23.038639f, 113.377180f, 16.417277f);
    Local::add(this, 23.038574f, 113.377172f, 15.352843f);
    Local::add(this, 23.038520f, 113.377161f, 14.070333f);
    Local::add(this, 23.038457f, 113.377144f, 12.529395f);
    Local::add(this, 23.038445f, 113.377098f, 11.321925f);
    Local::add(this, 23.038454f, 113.377077f, 11.063447f);
    Local::add(this, 23.038465f, 113.377023f, 10.468488f);
    Local::add(this, 23.038498f, 113.376964f, 10.645040f);
    Local::add(this, 23.038475f, 113.377011f, 11.021361f);
    Local::add(this, 23.038460f, 113.377067f, 11.043159f);
    Local::add(this, 23.038459f, 113.377118f, 11.424913f);
    Local::add(this, 23.038443f, 113.377196f, 11.635923f);
    Local::add(this, 23.038417f, 113.377236f, 11.699077f);
    Local::add(this, 23.038392f, 113.377284f, 11.778406f);
    Local::add(this, 23.038371f, 113.377338f, 12.047964f);
    Local::add(this, 23.038353f, 113.377397f, 12.293296f);
    Local::add(this, 23.038335f, 113.377458f, 12.426409f);
    Local::add(this, 23.038319f, 113.377521f, 12.385329f);
    Local::add(this, 23.038303f, 113.377585f, 12.460472f);
    Local::add(this, 23.038287f, 113.377649f, 12.620061f);
    Local::add(this, 23.038274f, 113.377711f, 12.818458f);
    Local::add(this, 23.038258f, 113.377771f, 12.922769f);
    Local::add(this, 23.038244f, 113.377830f, 13.052135f);
    Local::add(this, 23.038230f, 113.377888f, 13.027515f);
    Local::add(this, 23.038216f, 113.377945f, 12.960397f);
    Local::add(this, 23.038202f, 113.378002f, 12.878966f);
    Local::add(this, 23.038188f, 113.378060f, 12.953507f);
    Local::add(this, 23.038172f, 113.378118f, 12.957840f);
    Local::add(this, 23.038158f, 113.378178f, 12.876757f);
    Local::add(this, 23.038143f, 113.378237f, 12.815941f);
    Local::add(this, 23.038127f, 113.378296f, 12.654274f);
    Local::add(this, 23.038113f, 113.378366f, 12.133737f);
    Local::add(this, 23.038110f, 113.378436f, 11.935564f);
    Local::add(this, 23.038111f, 113.378487f, 11.864735f);
    Local::add(this, 23.038105f, 113.378541f, 12.123684f);
    Local::add(this, 23.038095f, 113.378597f, 12.326729f);
    Local::add(this, 23.038078f, 113.378660f, 12.209929f);
    Local::add(this, 23.038061f, 113.378729f, 11.858918f);
    Local::add(this, 23.038041f, 113.378798f, 11.957322f);
    Local::add(this, 23.038024f, 113.378868f, 11.754252f);
    Local::add(this, 23.038006f, 113.378943f, 11.910395f);
    Local::add(this, 23.037994f, 113.379016f, 11.611007f);
    Local::add(this, 23.037982f, 113.379092f, 11.178886f);
    Local::add(this, 23.037970f, 113.379169f, 10.711508f);
    Local::add(this, 23.037957f, 113.379246f, 10.317347f);
    Local::add(this, 23.037942f, 113.379317f, 10.050632f);
    Local::add(this, 23.037910f, 113.379400f, 9.985849f);
    Local::add(this, 23.037893f, 113.379449f, 10.298571f);
    Local::add(this, 23.037891f, 113.379502f, 10.542756f);
    Local::add(this, 23.037912f, 113.379550f, 10.886990f);
    Local::add(this, 23.037958f, 113.379582f, 11.163915f);
    Local::add(this, 23.038019f, 113.379600f, 11.516468f);
    Local::add(this, 23.038093f, 113.379608f, 11.984726f);
    Local::add(this, 23.038171f, 113.379607f, 12.525479f);
    Local::add(this, 23.038252f, 113.379604f, 12.823516f);
    Local::add(this, 23.038334f, 113.379603f, 12.902344f);
    Local::add(this, 23.038415f, 113.379601f, 12.925175f);
    Local::add(this, 23.038495f, 113.379598f, 12.906082f);
    Local::add(this, 23.038575f, 113.379598f, 12.844754f);
    Local::add(this, 23.038653f, 113.379592f, 12.946611f);
    Local::add(this, 23.038732f, 113.379588f, 13.117644f);
    Local::add(this, 23.038811f, 113.379588f, 13.214648f);
    Local::add(this, 23.038890f, 113.379587f, 13.375108f);
    Local::add(this, 23.038969f, 113.379589f, 13.369720f);
    Local::add(this, 23.039048f, 113.379591f, 13.444652f);
    Local::add(this, 23.039128f, 113.379595f, 13.274791f);
    Local::add(this, 23.039209f, 113.379599f, 13.372983f);
    Local::add(this, 23.039291f, 113.379605f, 13.278292f);
    Local::add(this, 23.039373f, 113.379612f, 13.028030f);
    Local::add(this, 23.039456f, 113.379619f, 12.839369f);
    Local::add(this, 23.039539f, 113.379625f, 12.702579f);
    Local::add(this, 23.039624f, 113.379629f, 12.824152f);
    Local::add(this, 23.039708f, 113.379635f, 12.800534f);
    Local::add(this, 23.039789f, 113.379641f, 13.074805f);
    Local::add(this, 23.039869f, 113.379646f, 13.284578f);
    Local::add(this, 23.039947f, 113.379650f, 13.665031f);
    Local::add(this, 23.040023f, 113.379657f, 14.141475f);
    Local::add(this, 23.040097f, 113.379667f, 14.705165f);
    Local::add(this, 23.040169f, 113.379679f, 15.081788f);
    Local::add(this, 23.040241f, 113.379693f, 15.430800f);
    Local::add(this, 23.040315f, 113.379696f, 15.567893f);
    Local::add(this, 23.040392f, 113.379687f, 15.493492f);
    Local::add(this, 23.040471f, 113.379676f, 15.585473f);
    Local::add(this, 23.040551f, 113.379663f, 15.873905f);
    Local::add(this, 23.040633f, 113.379652f, 16.032273f);
    Local::add(this, 23.040714f, 113.379643f, 15.787503f);
    Local::add(this, 23.040795f, 113.379637f, 15.340342f);
    Local::add(this, 23.040877f, 113.379627f, 15.274185f);
    Local::add(this, 23.040961f, 113.379617f, 15.296958f);
    Local::add(this, 23.041047f, 113.379606f, 15.316698f);
    Local::add(this, 23.041133f, 113.379594f, 15.464643f);
    Local::add(this, 23.041219f, 113.379581f, 15.578763f);
    Local::add(this, 23.041303f, 113.379565f, 15.308834f);
    Local::add(this, 23.041385f, 113.379545f, 14.811877f);
    Local::add(this, 23.041467f, 113.379523f, 14.145225f);
    Local::add(this, 23.041548f, 113.379501f, 13.413350f);
    Local::add(this, 23.041628f, 113.379476f, 12.674681f);
    Local::add(this, 23.041707f, 113.379450f, 11.815751f);
    Local::add(this, 23.041783f, 113.379420f, 11.227893f);
    Local::add(this, 23.041858f, 113.379392f, 10.806070f);
    Local::add(this, 23.041935f, 113.379383f, 10.466837f);
    Local::add(this, 23.042009f, 113.379398f, 10.304728f);
    Local::add(this, 23.042077f, 113.379440f, 10.483819f);
    Local::add(this, 23.042136f, 113.379503f, 10.828834f);
    Local::add(this, 23.042184f, 113.379581f, 11.381598f);
    Local::add(this, 23.042226f, 113.379668f, 12.018031f);
    Local::add(this, 23.042267f, 113.379755f, 12.549839f);
    Local::add(this, 23.042308f, 113.379843f, 12.905181f);
    Local::add(this, 23.042351f, 113.379929f, 13.194474f);
    Local::add(this, 23.042396f, 113.380011f, 13.367313f);
    Local::add(this, 23.042442f, 113.380089f, 13.584557f);
    Local::add(this, 23.042487f, 113.380167f, 13.712724f);
    Local::add(this, 23.042530f, 113.380246f, 13.806918f);
    Local::add(this, 23.042573f, 113.380325f, 13.897156f);
    Local::add(this, 23.042616f, 113.380404f, 14.000611f);
    Local::add(this, 23.042657f, 113.380485f, 14.129617f);
    Local::add(this, 23.042697f, 113.380566f, 14.316050f);
    Local::add(this, 23.042738f, 113.380645f, 14.517201f);
    Local::add(this, 23.042779f, 113.380725f, 14.750116f);
    Local::add(this, 23.042821f, 113.380805f, 14.993023f);
    Local::add(this, 23.042861f, 113.380884f, 15.271313f);
    Local::add(this, 23.042900f, 113.380964f, 15.388987f);
    Local::add(this, 23.042937f, 113.381044f, 15.567825f);
    Local::add(this, 23.042972f, 113.381123f, 15.748157f);
    Local::add(this, 23.043007f, 113.381205f, 15.880881f);
    Local::add(this, 23.043040f, 113.381289f, 16.110877f);
    Local::add(this, 23.043054f, 113.381334f, 16.177026f);
    Local::add(this, 23.043079f, 113.381421f, 16.358234f);
    Local::add(this, 23.043107f, 113.381507f, 16.567214f);
    Local::add(this, 23.043137f, 113.381590f, 16.632355f);
    Local::add(this, 23.043167f, 113.381672f, 16.616021f);
    Local::add(this, 23.043195f, 113.381752f, 16.570241f);
    Local::add(this, 23.043225f, 113.381833f, 16.509883f);
    Local::add(this, 23.043256f, 113.381913f, 16.669468f);
    Local::add(this, 23.043290f, 113.381989f, 16.780833f);
    Local::add(this, 23.043327f, 113.382066f, 16.885871f);
    Local::add(this, 23.043360f, 113.382145f, 17.170929f);
    Local::add(this, 23.043389f, 113.382228f, 17.423831f);
    Local::add(this, 23.043416f, 113.382313f, 17.729590f);
    Local::add(this, 23.043441f, 113.382401f, 17.893723f);
    Local::add(this, 23.043463f, 113.382491f, 18.127423f);
    Local::add(this, 23.043484f, 113.382583f, 18.172995f);
    Local::add(this, 23.043503f, 113.382674f, 18.272879f);
    Local::add(this, 23.043520f, 113.382754f, 18.410044f);
    Local::add(this, 23.043535f, 113.382827f, 18.479390f);
    Local::add(this, 23.043546f, 113.382901f, 18.436659f);
    Local::add(this, 23.043556f, 113.382979f, 18.290243f);
    Local::add(this, 23.043567f, 113.383061f, 18.068205f);
    Local::add(this, 23.043578f, 113.383142f, 17.914947f);
    Local::add(this, 23.043589f, 113.383225f, 17.898714f);
    Local::add(this, 23.043600f, 113.383310f, 18.063357f);
    Local::add(this, 23.043608f, 113.383394f, 18.311451f);
    Local::add(this, 23.043616f, 113.383480f, 18.427789f);
    Local::add(this, 23.043625f, 113.383566f, 18.602131f);
    Local::add(this, 23.043634f, 113.383653f, 18.922284f);
    Local::add(this, 23.043640f, 113.383739f, 19.196083f);
    Local::add(this, 23.043645f, 113.383827f, 19.461388f);
    Local::add(this, 23.043649f, 113.383914f, 19.643389f);
    Local::add(this, 23.043651f, 113.384000f, 19.844373f);
    Local::add(this, 23.043653f, 113.384087f, 20.135363f);
    Local::add(this, 23.043653f, 113.384174f, 20.237561f);
    Local::add(this, 23.043653f, 113.384261f, 20.371698f);
    Local::add(this, 23.043652f, 113.384347f, 20.388901f);
    Local::add(this, 23.043650f, 113.384432f, 20.558558f);
    Local::add(this, 23.043648f, 113.384515f, 20.646553f);
    Local::add(this, 23.043646f, 113.384597f, 20.782917f);
    Local::add(this, 23.043644f, 113.384680f, 20.966972f);
    Local::add(this, 23.043639f, 113.384762f, 20.997068f);
    Local::add(this, 23.043634f, 113.384842f, 20.827510f);
    Local::add(this, 23.043629f, 113.384921f, 20.929138f);
    Local::add(this, 23.043620f, 113.385001f, 20.904294f);
    Local::add(this, 23.043611f, 113.385085f, 20.678080f);
    Local::add(this, 23.043602f, 113.385168f, 20.482274f);
    Local::add(this, 23.043592f, 113.385254f, 20.393697f);
    Local::add(this, 23.043581f, 113.385339f, 20.434748f);
    Local::add(this, 23.043571f, 113.385428f, 20.503683f);
    Local::add(this, 23.043562f, 113.385518f, 20.653653f);
    Local::add(this, 23.043551f, 113.385608f, 20.785532f);
    Local::add(this, 23.043539f, 113.385699f, 20.915123f);
    Local::add(this, 23.043529f, 113.385791f, 21.067501f);
    Local::add(this, 23.043521f, 113.385882f, 21.387540f);
    Local::add(this, 23.043515f, 113.385976f, 21.788315f);
    Local::add(this, 23.043508f, 113.386069f, 22.161928f);
    Local::add(this, 23.043503f, 113.386162f, 22.413964f);
    Local::add(this, 23.043498f, 113.386254f, 22.662486f);
    Local::add(this, 23.043490f, 113.386345f, 22.911464f);
    Local::add(this, 23.043479f, 113.386437f, 23.175639f);
    Local::add(this, 23.043468f, 113.386528f, 23.447217f);
    Local::add(this, 23.043457f, 113.386619f, 23.737750f);
    Local::add(this, 23.043447f, 113.386709f, 24.083602f);
    Local::add(this, 23.043439f, 113.386799f, 24.532626f);
    Local::add(this, 23.043433f, 113.386888f, 25.158402f);
    Local::add(this, 23.043424f, 113.386977f, 25.435625f);
    Local::add(this, 23.043414f, 113.387067f, 25.739907f);
    Local::add(this, 23.043405f, 113.387158f, 25.783530f);
    Local::add(this, 23.043396f, 113.387247f, 25.896996f);
    Local::add(this, 23.043386f, 113.387337f, 25.909433f);
    Local::add(this, 23.043379f, 113.387425f, 25.913340f);
    Local::add(this, 23.043375f, 113.387516f, 25.869575f);
    Local::add(this, 23.043369f, 113.387608f, 25.819260f);
    Local::add(this, 23.043363f, 113.387699f, 25.851091f);
    Local::add(this, 23.043357f, 113.387789f, 25.877117f);
    Local::add(this, 23.043353f, 113.387881f, 25.848116f);
    Local::add(this, 23.043349f, 113.387972f, 25.917862f);
    Local::add(this, 23.043347f, 113.388062f, 25.895684f);
    Local::add(this, 23.043346f, 113.388149f, 25.870174f);
    Local::add(this, 23.043346f, 113.388235f, 25.831562f);
    Local::add(this, 23.043347f, 113.388320f, 25.816586f);
    Local::add(this, 23.043347f, 113.388403f, 25.706858f);
*/

}

float TemplateModel::CalcDistance(std::vector<RawCoordinates>& points, int index_begin, int index_end) {
    const int n = static_cast<int>(points.size());
    if (points.size() < 2 || (index_end != -1 && index_end - index_begin < 2) || n - index_begin < 2) return 0.0f;

    if (index_end == -1)index_end = n;

    float sum = 0.0f;
    for (int i = index_begin + 1; i < index_end; ++i) {
        sum += distanceMeters(points[i - 1].latitude, points[i - 1].longitude, points[i].latitude, points[i].longitude);
    }
    return (sum/1000.0f);
};

int TemplateModel::CalcAscent(std::vector<RawCoordinates>& points, int index_begin, int index_end) {
    const int n = static_cast<int>(points.size());
    if (points.size() < 2 || (index_end != -1 && index_end - index_begin < 2) || n - index_begin < 2) return 0.0;

    if (index_end == -1)index_end = n;

    float sum = 0.0f;
    float tangle = 0.1328f;

    for (int i = index_begin + 1; i < index_end; ++i) {
        float x = distanceMeters(points[i - 1].latitude, points[i - 1].longitude, points[i].latitude, points[i].longitude);
        float y = points[i].altitude - points[i - 1].altitude;

        if ((x != 0.0f) && (y / x >= tangle)) {
            sum += y;
        }
    }

    return int(sum);
}

void TemplateModel::UpdateData(std::vector<RawCoordinates>& points, int index_begin, int index_end) {
    const int n = static_cast<int>(points.size());;
    if (points.size() < 2 || (index_end != -1 && index_end - index_begin < 2) || n - index_begin < 2) return;

    if (index_end == -1)index_end = n;

    Distance = 0.0f;
    Ascent = 0;

    min_altitude = points[index_begin].altitude;
    max_altitude = points[index_begin].altitude;

    for (uint32_t i = index_begin; i < index_end; i++) {
        if (points[i].altitude < min_altitude) min_altitude = points[i].altitude;
        if (points[i].altitude > max_altitude) max_altitude = points[i].altitude;
    }

    Distance = CalcDistance(points, index_begin, index_end);
    Ascent = CalcAscent(points, index_begin, index_end);
};

int TemplateModel::MatchNearestPoint(std::vector<RawCoordinates>& points, double latitude, double longitude, int index_begin, int index_end) {

    //初始化判断
    const int n = static_cast<int>(points.size());
    if (points.size() < 2 || (index_end != -1 && index_end - index_begin < 2)) return -1;
    //限制阈值，避免索引溢出
    if (index_end == -1|| index_end>n)index_end = n;
    if (index_begin < 0)index_begin = 0;

    //储存距离的列表
    int iListSize = index_end - index_begin + 1;
    std::vector<float>list(iListSize);

    //计算各自两点之间的距离
    for (int i = index_begin; i < index_end; i++) {
        list[i- index_begin] = distanceMeters(latitude, longitude, points[i].latitude, points[i].longitude);
    }

    //找寻距离最近点的数据
    int indexPin = 0;
    float min_distance = list[0];

    for (int i = 0; i < iListSize -1; ++i) {
        if (list[i] < min_distance) {
            min_distance = list[i];
            indexPin = i;
        }
    }

    if (list[indexPin] < 10.0f) {
       return indexPin + index_begin;
    }
    else {
        return -1;
    }

}
/*
void TemplateModel::UpdateGPS(float latitude, float longitude, int gpsDistance) {

    // 第一次进来：没有上一帧
    if (!gpsCoordinate.hasLast)
    {
        gpsCoordinate.current_latitude = latitude;
        gpsCoordinate.current_longitude = longitude;

        gpsCoordinate.hasLast = true;
        return;   // 关键：第一次不做距离/匹配计算
    }

    float deltaDistance = distanceMeters(gpsCoordinate.current_latitude, gpsCoordinate.current_longitude, latitude, longitude);

    //更新当前的经纬度数据
    if (deltaDistance >= gpsDistance) {

        gpsCoordinate.current_latitude = latitude;
        gpsCoordinate.current_longitude = longitude;
    }

}*/


/*
void TemplateModel::Init()
{
    account = new Account("TemplateModel", DataProc::Center(), 0, this);
    account->Subscribe("GPS");        // 订阅GPS数据
    account->Subscribe("SysConfig");  // 订阅系统配置
    account->SetEventCallback(onEvent);
}

void TemplateModel::Deinit()
{
    if (account)
    {
        delete account;
        account = NULL;
    }
}

int TemplateModel::onEvent(Account* account, Account::EventParam_t* param)
{
    if (param->event != Account::EVENT_PUB_PUBLISH)
    {
        return Account::RES_UNSUPPORTED_REQUEST;
    }

    if (strcmp(param->tran->ID, "GPS") != 0
        || param->size != sizeof(HAL::GPS_Info_t))
    {
        return Account::RES_PARAM_ERROR;
    }

    TemplateModel* instance = (TemplateModel*)account->UserData;
    memcpy(&(instance->gpsInfo), param->data_p, param->size);

    return Account::RES_OK;
}*/
//里面自带判断GPS数据是否要刷新，Current是当前的经纬度数据，根据间隔gpsDistance判断是否需要更新数据


static std::vector<RawCoordinates> TestCoordinate;
static struct TestCoordinateInit
{
    static void add(float lat, float lon, float alt)
    {
        RawCoordinates rc;
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
} _testInit;





void TemplateModel::GetGPS_Info(GPS_Data* info, int gpsDistance)
{
	/*
	static int i = 0;
	gpsCoordinate.current_latitude = TestCoordinate[i].latitude;
	gpsCoordinate.current_longitude = TestCoordinate[i].longitude;
	i++;*/

    memset(info, 0, sizeof(GPS_Data));
    // 拉取GPS数据

	if (xSemaphoreTake(gps_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {

		if(gps_data_change == 1){
			gps_data_change = 0;
			if(GPS_data_share.valid == 0){
				info->longitude = 116.39127;
				info->latitude = 39.90621;
				return;
			} else
		    {
		        // 第一次进来：没有上一帧
		        if (!gpsCoordinate.hasLast)
		        {
		            gpsCoordinate.current_latitude = info->latitude;
		            gpsCoordinate.current_longitude = info->longitude;

		            gpsCoordinate.hasLast = true;
		            return;   // 关键：第一次不做距离/匹配计算
		        }

		        float deltaDistance = distanceMeters(gpsCoordinate.current_latitude, gpsCoordinate.current_longitude, info->latitude, info->longitude);

		        //更新当前的经纬度数据
		        if (deltaDistance >= gpsDistance) {
		            gpsCoordinate.current_latitude = info->latitude;
		            gpsCoordinate.current_longitude = info->longitude;
		        }
		    }

		}
			// 释放互斥锁（必须执行，否则其他任务会阻塞）
		xSemaphoreGive(gps_mutex);
	}
	/*
    if (account->Pull("GPS", info, sizeof(HAL::GPS_Info_t)) != Account::RES_OK)
    {
        return;
    }*/

    /* 当数据无效的时候使用默认初始定位的经纬度数据 */
	/*
    if (!info->isVaild)
    {
        DataProc::SysConfig_Info_t sysConfig;
        if (account->Pull("SysConfig", &sysConfig, sizeof(sysConfig)) == Account::RES_OK)
        {
            info->longitude = sysConfig.longitude;
            info->latitude = sysConfig.latitude;

        }
    }*/
/*
    else
    {
        // 第一次进来：没有上一帧
        if (!gpsCoordinate.hasLast)
        {
            gpsCoordinate.current_latitude = info->latitude;
            gpsCoordinate.current_longitude = info->longitude;

            gpsCoordinate.hasLast = true;
            return;   // 关键：第一次不做距离/匹配计算
        }

        float deltaDistance = distanceMeters(gpsCoordinate.current_latitude, gpsCoordinate.current_longitude, info->latitude, info->longitude);

        //更新当前的经纬度数据
        if (deltaDistance >= gpsDistance) {
            gpsCoordinate.current_latitude = info->latitude;
            gpsCoordinate.current_longitude = info->longitude;
        }
    }*/
}
