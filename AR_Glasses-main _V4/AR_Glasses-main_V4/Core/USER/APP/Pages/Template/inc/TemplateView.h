#ifndef __TEMPLATE_VIEW_H
#define __TEMPLATE_VIEW_H

#include "Page.h"


namespace Page
{

    class TemplateView
    {
    public:
        void InitChart(lv_obj_t* root);

        struct
        {
            lv_obj_t* background;
            lv_obj_t* line_gap;
        } layout;

        struct {
            lv_obj_t* background;

            lv_obj_t* label_altitude_m;
            lv_obj_t* label_altitude_text;
            lv_obj_t* label_distance_km;
            lv_obj_t* label_distance_text;

            lv_obj_t* label_distance;
            lv_obj_t* label_altitude;

            lv_obj_t* chart;
            lv_chart_series_t* ser;

            lv_obj_t* label_altitude_min;
            lv_obj_t* label_altitude_max;
            lv_obj_t* label_distance_start;
            lv_obj_t* label_distance_end;
            
            lv_obj_t* line_dive_left;
            lv_obj_t* line_dive_right;
            lv_obj_t* line_dive_top;
            lv_obj_t* dotted_line;
            lv_obj_t* line_dive_bottom;

            lv_obj_t* line_pin;
            lv_coord_t x;

        }segmentChart;

        struct {
            lv_obj_t* background;
            lv_obj_t* chart;
            lv_obj_t* label_altitude_min;
            lv_obj_t* label_altitude_max;
            lv_obj_t* label_distance_start;
            lv_obj_t* label_distance_end;
        }routeChart;

        struct {
            lv_obj_t* label_latitude;
            lv_obj_t* label_longitude;
            lv_obj_t* label_debug;

        }gpsDatas;
    };

}

#endif // !__VIEW_H
