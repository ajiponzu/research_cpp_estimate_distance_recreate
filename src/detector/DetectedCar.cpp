#include "../CarDetector.h"

static constexpr auto g_KILO_RATIO = 0.001;
static constexpr auto g_FONT_FACE = cv::FONT_HERSHEY_DUPLEX;
static constexpr auto g_FONT_SCALE = 0.5;
static constexpr auto g_THICKNESS = 1;
static constexpr auto g_ORTHO_METER = 0.2;
static constexpr auto g_CAR_BODY_ESTIMATED_METER = 4.2;
static constexpr auto g_HOUR_SECOND_RATIO = 3600;
static constexpr auto g_MATCHING_THR = 0.1;

static float get_dsm_z(const cv::Point& point)
{
	return ResourceProvider::GetOrthoDsm().at<float>(point);
}

static cv::Point3f get_transed_point(const cv::Point& point)
{
	return ResourceProvider::GetTransedPointsMap().at<cv::Point3f>(point);
}

static cv::Point2f get_white_lane_dir(const cv::Point& point)
{
	return ResourceProvider::GetLaneDirectionsMap().at<cv::Point2f>(point);
}

static float get_white_lane_ratio(const cv::Point& point)
{
	return ResourceProvider::GetLaneRatiosMap().at<float>(point);
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

void DetectedCar::Init()
{
	const auto car_center = calc_rect_bottom_center(m_shape);
	m_curPointTransed = m_startPointTransed = get_transed_point(car_center);
	m_startFrameCount = m_curFrameCount = GuiHandler::GetFrameCount();
	m_img = GuiHandler::GetFrame()(m_shape);
	m_initialized = true;
}

void DetectedCar::UpdateDetectionArea(const cv::Mat& frame)
{
	auto detection_area = cv::Rect(0, 0, m_shape.width + 20, m_shape.height + 20);
	detection_area.x = std::max(0, m_shape.x - 10);
	detection_area.y = std::max(0, m_shape.y - 10);
	m_detectionArea = detection_area;
	m_detectionAreaImg = frame(m_detectionArea).clone();
}

bool DetectedCar::Matching()
{
	cv::Mat match_results;
	double max_val = 0.0;
	cv::Point max_point;
	cv::matchTemplate(m_detectionAreaImg, m_img, match_results, cv::TM_CCOEFF_NORMED);
	cv::minMaxLoc(match_results, nullptr, &max_val, nullptr, &max_point);

	if (max_val < g_MATCHING_THR)
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
	m_bodyDirection = get_white_lane_dir(car_pos);
	const auto car_body_direction_pixel = g_CAR_BODY_ESTIMATED_METER * m_bodyDirection / g_ORTHO_METER;
	const auto another_point = static_cast<cv::Point>(extract_xy_point(m_curPointTransed) - car_body_direction_pixel);
	m_curPointTransedAnother = convert_xy_into_xyz(another_point, get_dsm_z(another_point));
}

void DetectedCar::CalcMovingDistance()
{
	const auto position_delta_vec = m_curPointTransed - m_prevPointTransed;
	const auto orthographic_vec = Func::Img::calc_orthographic_vec(m_bodyDirection, extract_xy_point(position_delta_vec));
	const auto position_delta_kilometer = cv::norm(orthographic_vec) * g_ORTHO_METER * g_KILO_RATIO;
	m_movingDistance += position_delta_kilometer;
}

void DetectedCar::CalcSpeed()
{
	CalcMovingDistance();

	const auto tracking_frame_count = (m_curFrameCount - m_startFrameCount);
	const auto delta_kilometer_per_frame = m_movingDistance * g_HOUR_SECOND_RATIO * GuiHandler::GetFPS(); // 1時間あたりのフレーム数と移動距離の積
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
	const auto speed_txt = std::format("{:.1f} [km/h]", m_speed);
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
	cv::circle(ortho, static_cast<cv::Point>(extract_xy_point(m_startPointTransed)), 5,
		cv::Scalar((m_id % 2 + 1) * 50.0, 255, (m_id % 2 + 1) * 50.0), -1);
	cv::circle(ortho, static_cast<cv::Point>(extract_xy_point(m_curPointTransed)), 5,
		cv::Scalar(0, (m_id % 2 + 1) * 50.0, 255), -1);
	cv::circle(ortho, static_cast<cv::Point>(extract_xy_point(m_curPointTransedAnother)), 5,
		cv::Scalar(255, 0, (m_id % 2 + 1) * 50.0), -1);
}

double DetectedCar::CalcCarDistance(const DetectedCar& front_car, const DetectedCar& back_car)
{
	const auto distance = calc_car_distance(front_car.m_curPointTransed, back_car.m_curPointTransedAnother);

	return distance * g_ORTHO_METER;
}