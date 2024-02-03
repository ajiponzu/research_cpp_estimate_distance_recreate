#include "../CarDetector.h"

static constexpr auto g_FONT_FACE = cv::FONT_HERSHEY_DUPLEX;
static constexpr auto g_FONT_SCALE = 0.6;
static constexpr auto g_THICKNESS = 2;

static float get_dsm_z(const cv::Point& point)
{
	return ResourceProvider::GetOrthoDsm().at<float>(point);
}

static cv::Point3f get_transed_point(const cv::Point& point)
{
	return ResourceProvider::GetTransedPointsMap().at<cv::Point3f>(point);
}

static cv::Point2f get_white_lane_dir_altitude(const cv::Point& point)
{
	const auto& lane_inf = ResourceProvider::GetLaneDirectionsMap().at<cv::Vec4f>(point);
	return cv::Point2f(lane_inf[0], lane_inf[1]);
}

static cv::Point2f get_white_lane_dir_ortho(const cv::Point& point)
{
	const auto& lane_inf = ResourceProvider::GetLaneDirectionsMap().at<cv::Vec4f>(point);
	return cv::Point2f(lane_inf[2], lane_inf[3]);
}

static bool isnot_on_mask(const cv::Point& point)
{
	return !Func::Img::is_on_mask(ResourceProvider::GetRoadMask(), point);
}

static cv::Point2f extract_xy_point(const cv::Point3f& point)
{
	return cv::Point2f(point.x, point.y);
}
static cv::Point3f convert_xy_into_xyz(const cv::Point2f& point, const float& z = 0.0f)
{
	return cv::Point3f(point.x, point.y, z);
}

static cv::Point calc_rect_bottom_center(const cv::Rect& rect)
{
	const auto bl = rect.tl() + cv::Point(0, rect.height);

	return Func::Img::calc_line_center(bl, rect.br());
}

static double calc_car_distance(const cv::Point3f& pt1, const cv::Point3f& pt2)
{
	const auto delta_xy = extract_xy_point(pt1 - pt2);
	const auto delta_z = (pt1 - pt2).z;

	return cv::norm(convert_xy_into_xyz(delta_xy, delta_z));
}

static double calc_vetors_angle(const cv::Point2f& vec1, const cv::Point2f& vec2, const bool& is_degree)
{
	auto rotate_angle = std::acos(vec1.dot(vec2) / (cv::norm(vec1) * cv::norm(vec2)));
	if (is_degree)
		rotate_angle *= 180.0 / std::numbers::pi;

	return rotate_angle;
}

void DetectedCar::Init()
{
	const auto car_center = calc_rect_bottom_center(m_shape);
	m_curPointTransed = get_transed_point(car_center);
	m_startFrameCount = m_curFrameCount = GuiHandler::GetFrameCount();
	m_img = GuiHandler::GetFrame()(m_shape);
	m_initialized = true;
	m_orthoBodyDirection = get_white_lane_dir_ortho(car_center);
}

void DetectedCar::UpdateDetectionArea(const cv::Mat& frame)
{
	static constexpr auto MATCHING_RANGE = 10;

	auto detection_area = cv::Rect(0, 0, m_shape.width + MATCHING_RANGE, m_shape.height + MATCHING_RANGE);
	const auto center_of_m_shape = Func::Img::calc_rect_center(m_shape);
	detection_area.x = std::max(0, center_of_m_shape.x - detection_area.width / 2);
	detection_area.y = std::max(0, center_of_m_shape.y - detection_area.height / 2);
	m_detectionArea = detection_area;
	m_detectionAreaImg = frame(m_detectionArea).clone();
}

bool DetectedCar::Matching()
{
	static constexpr auto MATCHING_THR = 0.1;

	cv::Mat match_results;
	double max_val = 0.0;
	cv::Point max_point;
	cv::matchTemplate(m_detectionAreaImg, m_img, match_results, cv::TM_CCOEFF_NORMED);
	cv::minMaxLoc(match_results, nullptr, &max_val, nullptr, &max_point);

	if (max_val < MATCHING_THR)
		return false;

	auto result_location = m_detectionArea + max_point;
	result_location.width = m_shape.width;
	result_location.height = m_shape.height;
	m_shape = result_location;

	return true;
}

void DetectedCar::UpdatePosition(const cv::Point& car_pos)
{
	m_prevPointTransed = m_curPointTransed;
	m_curPointTransed = get_transed_point(car_pos);

	// (白線方向 / [m / pix]) * 車体の全長[m] = 車体の他面位置[pix^2]
	m_orthoBodyDirection = get_white_lane_dir_ortho(car_pos);

	const auto car_body_direction_pixel = m_bodyLength * m_orthoBodyDirection / ResourceProvider::GetOrthoGeoInf().meter_ratio;
	const auto another_point = static_cast<cv::Point>(extract_xy_point(m_curPointTransed) - car_body_direction_pixel);
	m_curPointTransedAnother = convert_xy_into_xyz(another_point, get_dsm_z(another_point));
}

void DetectedCar::CalcMovingDistance()
{
	static constexpr auto KILO_RATIO = 0.001;

	const auto position_delta_vec = m_curPointTransed - m_prevPointTransed;
	const auto orthographic_vec = Func::Img::calc_orthographic_vec(m_orthoBodyDirection, extract_xy_point(position_delta_vec));
	const auto position_delta_kilometer = cv::norm(orthographic_vec) * ResourceProvider::GetOrthoGeoInf().meter_ratio * KILO_RATIO;
	if (orthographic_vec.dot(m_orthoBodyDirection) < 0)
		m_movingDistance -= position_delta_kilometer;
	else
		m_movingDistance += position_delta_kilometer;
}

void DetectedCar::CalcSpeed()
{
	static constexpr auto HOUR_SECOND_RATIO = 3600;

	CalcMovingDistance();

	const auto tracking_frame_count = (m_curFrameCount - m_startFrameCount);
	const auto delta_kilometer_per_frame = m_movingDistance * HOUR_SECOND_RATIO * GuiHandler::GetFPS(); // 1時間あたりのフレーム数と移動距離の積
	m_speed = delta_kilometer_per_frame / tracking_frame_count; // 1時間x(FPS)フレームでの移動距離 / 移動フレーム数
}

bool DetectedCar::Tracking(const cv::Mat& frame)
{
	if (!m_initialized)
		return false;

	m_curFrameCount = GuiHandler::GetFrameCount();
	UpdateDetectionArea(frame);

	const auto matching_result = Matching();
	const auto car_pos = calc_rect_bottom_center(m_shape);
	if (!matching_result || isnot_on_mask(car_pos))
		return false;

	UpdatePosition(car_pos);
	CalcSpeed();

	return true;
}

void DetectedCar::DrawOnImage(cv::Mat& img) const
{
	int base_line = 0;
	const auto speed_txt = std::format("[ID {}] {:.1f} [km/h]", m_id, std::abs(m_speed));
	const auto font_size = cv::getTextSize(speed_txt,
		g_FONT_FACE, g_FONT_SCALE, g_THICKNESS, &base_line);

	cv::rectangle(img, m_shape, cv::Scalar(0, 0, 255), 2);
	cv::rectangle(img,
		cv::Point(m_shape.tl().x, m_shape.tl().y - font_size.height - 5),
		cv::Point(m_shape.tl().x + font_size.width, m_shape.tl().y),
		cv::Scalar(0, 0, 255), -1);
	cv::putText(img, speed_txt,
		cv::Point(m_shape.tl().x, m_shape.tl().y - 5),
		g_FONT_FACE, g_FONT_SCALE, cv::Scalar(255, 255, 255), g_THICKNESS);

#ifdef _DEBUG
	if (GuiHandler::IsRunning())
		std::cout << std::format("car_{}_speed: {:.1f} [km/h]", m_id, m_speed) << std::endl;
#endif
}

void DetectedCar::DrawOnOrtho(cv::Mat& ortho) const
{
	cv::circle(ortho, static_cast<cv::Point>(extract_xy_point(m_curPointTransed)), 5,
		cv::Scalar(0, 0, 255), -1);
	cv::circle(ortho, static_cast<cv::Point>(extract_xy_point(m_curPointTransedAnother)), 5,
		cv::Scalar(255, 0, 0), -1);
}

double DetectedCar::CalcCarDistance(const DetectedCar& front_car, const DetectedCar& back_car)
{
	const auto distance = calc_car_distance(front_car.m_curPointTransed, back_car.m_curPointTransedAnother);

	return distance * ResourceProvider::GetOrthoGeoInf().meter_ratio;
}