#include "../Utility.h"

cv::Mat ResourceProvider::s_orthoDsm;
cv::Mat ResourceProvider::s_orthoTif;
cv::Mat ResourceProvider::s_orthoRoadMask;
cv::Mat ResourceProvider::s_transedPointsMap;
cv::Mat ResourceProvider::s_laneDirectionsMap;
cv::Mat ResourceProvider::s_laneRatiosMap;
cv::Mat ResourceProvider::s_roadMask;
Func::GeoCvt::OrthoGeoInf ResourceProvider::s_orthoGeoInf;

void ResourceProvider::Init(const int& road_num, const std::string& video_code, const std::string& ortho_code)
{
	const auto ortho_dem_path = std::format("resources/{}/dsm.tif", ortho_code);
	const auto ortho_tif_path = std::format("resources/{}/ortho.tif", ortho_code);
	const auto ortho_road_mask_path = std::format("resources/{}/road_mask.png", ortho_code);
	const auto video_transed_map_path = std::format("resources/{}/transed_points_map.tif", video_code);
	const auto video_lanes_inf_map_path = std::format("resources/{}/lanes_inf_map.tif", video_code);

	s_orthoDsm = Func::GeoCvt::get_float_tif(ortho_dem_path);

	const auto& [ortho_tif, ortho_geo_inf] = Func::GeoCvt::get_multicolor_mat(ortho_tif_path);
	s_orthoTif = ortho_tif.clone();
	s_orthoGeoInf = ortho_geo_inf;

	s_orthoRoadMask = cv::imread(ortho_road_mask_path);

	auto transed_points_map = Func::GeoCvt::get_float_tif(video_transed_map_path);
	auto lanes_inf_map = Func::GeoCvt::get_float_tif(video_lanes_inf_map_path);

	std::vector<cv::Mat> splited_mat_list;
	cv::split(transed_points_map, splited_mat_list);
	cv::merge(std::vector{ splited_mat_list[0], splited_mat_list[1], splited_mat_list[2] }, transed_points_map);
	splited_mat_list[3].convertTo(s_roadMask, CV_8U);
	cv::cvtColor(s_roadMask, s_roadMask, cv::COLOR_GRAY2BGR);
	s_transedPointsMap = transed_points_map.clone();

	cv::split(lanes_inf_map, splited_mat_list);
	cv::merge(std::vector{ splited_mat_list[0], splited_mat_list[1] }, s_laneDirectionsMap);
	s_laneRatiosMap = splited_mat_list[2].clone();
}