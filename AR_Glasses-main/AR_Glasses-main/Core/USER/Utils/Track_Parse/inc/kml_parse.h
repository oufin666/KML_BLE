/*
 * kml_parse.h
 *
 *  Created on: Jan 7, 2026
 *      Author: wkt98
 */

#include "main.h"
#include "track_param.h"
#include <vector>

#ifndef USER_UTILS_TRACK_PARSE_KML_PARSE_H_
#define USER_UTILS_TRACK_PARSE_KML_PARSE_H_


int kml_parse_coordinates(const char* filepath, std::vector<RawCoordinates>& out_track);
int kml_parse_to_file(const char* in_filepath, const char* out_filepath);
int count_kml_points(const char* filepath);
int kml_filter_to_file(const char *in_filepath, const char *out_filepath);
int load_points_from_file(const char* filepath, std::vector<RawCoordinates>& out_vector, int point_count);
#endif /* USER_UTILS_TRACK_PARSE_KML_PARSE_H_ */
