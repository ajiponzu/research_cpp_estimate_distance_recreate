#include "../ExperimentalDetector.h"

static double get_milliseconds_from_datetime_string(const GpsData& gps_data)
{
	std::regex re(R"((\d{4})-(\d{2})-(\d{2}) (\d{2}):(\d{2}):(\d{2})\.(\d{3}))");
	std::smatch match_ret;
	std::regex_match(gps_data.time, match_ret, re);

	double time = std::stod(std::format("0.{}", match_ret[7].str()));

	return time;
}

static double get_seconds_from_datetime_string(const GpsData& gps_data)
{
	std::regex re(R"((\d{4})-(\d{2})-(\d{2}) (\d{2}):(\d{2}):(\d{2})\.(\d{3}))");
	std::smatch match_ret;
	std::regex_match(gps_data.time, match_ret, re);

	// 5: minute, 6: seconds, 7: millliseconds
	double time = std::stod(match_ret[5].str()) * 60.0 +
		std::stod(std::format("{}.{}", match_ret[6].str(), match_ret[7].str()));

	return time;
}

static double calc_delta_time_abs(const GpsData& prev, const GpsData& next)
{
	const auto prev_seconds = get_seconds_from_datetime_string(prev);
	const auto next_seconds = get_seconds_from_datetime_string(next);

	return std::abs(next_seconds - prev_seconds);
}

static cv::Point2f extract_xy_point(const cv::Point3f& point)
{
	return cv::Point2f(point.x, point.y);
}

static cv::Point calc_rect_bottom_center(const cv::Rect& rect)
{
	const auto bl = rect.tl() + cv::Point(0, rect.height);

	return Func::Img::calc_line_center(bl, rect.br());
}

void ExperimentalCar::Init()
{
	std::ifstream ifs(std::format("resources/{}/{}_velocity{}.csv", m_carName, m_carName, m_experimentalId));
	if (!ifs)
	{
		std::cerr << "experimental data is not found" << std::endl;
		return;
	}

	std::string file_line;
	std::getline(ifs, file_line); // ヘッダ読み飛ばし
	bool skip_flag = true;

	while (std::getline(ifs, file_line))
	{
		GpsData gps_data;

		std::istringstream csv_data(file_line);
		std::string csv_data_buf;

		std::getline(csv_data, csv_data_buf, ',');
		gps_data.time = csv_data_buf;

		if (gps_data.time == m_experimentStartTime)
			skip_flag = false;

		if (skip_flag)
			continue;

		std::getline(csv_data, csv_data_buf, ',');
		gps_data.longitude = std::stod(csv_data_buf);

		std::getline(csv_data, csv_data_buf, ',');
		gps_data.latitude = std::stod(csv_data_buf);

		std::getline(csv_data, csv_data_buf, ',');
		gps_data.velocity = std::stod(csv_data_buf);

		std::getline(csv_data, csv_data_buf, ',');
		gps_data.angle = std::stod(csv_data_buf);

		m_gpsDataList.push_back(gps_data);
	}

	m_ofstream.open(std::format("outputs/experiments/experiment_{}{}.csv", m_carName, m_experimentalId));
	m_ofstream << "frame_number,time,car_name,alt_x,alt_y,ortho_x,ortho_y,correct_x,correct_y,transformed_error,speed,speed_correct,speed_error" << std::endl;
}

void ExperimentalCar::CalcSpeedError()
{
	m_speedError = std::abs(std::abs(m_speed) - m_gpsDataList[m_evaluateCount].velocity);
}

void ExperimentalCar::CalcTransedError()
{
	const auto geo_ref_point =
		cv::Point2d(m_gpsDataList[m_evaluateCount].longitude, m_gpsDataList[m_evaluateCount].latitude)
		- ResourceProvider::GetOrthoGeoInf().geo_org_pos;

	const auto converted_point = cv::Point2d(
		geo_ref_point.x / ResourceProvider::GetOrthoGeoInf().convert_ratio.x,
		geo_ref_point.y / ResourceProvider::GetOrthoGeoInf().convert_ratio.y
	);

	m_transedError = (static_cast<cv::Point2f>(converted_point) - extract_xy_point(m_curPointTransed))
		* ResourceProvider::GetOrthoGeoInf().meter_ratio;
	m_correctPosition = converted_point;
}

bool ExperimentalCar::Tracking(const cv::Mat& frame)
{
	const auto ret = super::Tracking(frame);

	const auto tracking_count = GuiHandler::GetFrameCount() - m_startFrameCount;
	const auto data_exist_frame_count = std::floor(m_dataSpanFrameCount * m_evaluateCount);

	m_calculateFlag = false;
	if (ret && tracking_count == data_exist_frame_count)
	{
		double delta_time_abs = 0.0;
		if (m_evaluateCount == 0)
		{
			delta_time_abs = get_milliseconds_from_datetime_string(m_gpsDataList[0]);
			m_calculateFlag = true;
		}
		else
		{
			delta_time_abs = calc_delta_time_abs(m_gpsDataList[m_evaluateCount - 1], m_gpsDataList[m_evaluateCount]);
			m_calculateFlag = delta_time_abs <= 0.201;
		}

		if (m_calculateFlag)
		{
			std::cout
				<< std::format("{}: [{} / ({})]---{}",
					m_carName, GuiHandler::GetFrameCount(), GuiHandler::GetAllFrameNum(), m_gpsDataList.at(m_evaluateCount).time)
				<< std::endl;
			CalcSpeedError();
			CalcTransedError();

			const auto car_bottom = calc_rect_bottom_center(m_shape);
			m_ofstream << std::format("{},{},{},{},{},{},{},{},{},{},{},{},{}\n",
				GuiHandler::GetFrameCount(), m_gpsDataList[m_evaluateCount].time, m_carName, car_bottom.x, car_bottom.y,
				m_curPointTransed.x, m_curPointTransed.y, m_correctPosition.x, m_correctPosition.y, cv::norm(m_transedError),
				std::abs(m_speed), m_gpsDataList[m_evaluateCount].velocity, m_speedError);
			m_evaluateCount++;
		}
	}

	return ret;
}

void ExperimentalCar::DrawOnImage(cv::Mat& img) const
{
	super::DrawOnImage(img);
}

void ExperimentalCar::DrawOnOrtho(cv::Mat& ortho) const
{
	super::DrawOnOrtho(ortho);
	if (m_carName == "jimny")
		cv::circle(ortho, m_correctPosition, 5, cv::Scalar(255, 255, 0), -1);
	else if (m_carName == "levorg")
		cv::circle(ortho, m_correctPosition, 5, cv::Scalar(255, 0, 255), -1);
}