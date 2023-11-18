#include "../ExperimentalDetector.h"

static double get_milliseconds_from_datetime_string(const GpsData& gps_data)
{
	std::regex re(R"(^(\d+)-(\d+)-(\d+)T(\d+):(\d+):(\d+)\.(\d+)\+09:00$)");
	std::smatch match_ret;
	std::regex_match(gps_data.time, match_ret, re);

	double time = std::stod(std::format("0.{}", match_ret[7].str()));

	return time;
}

static double get_seconds_from_datetime_string(const GpsData& gps_data)
{
	std::regex re(R"(^(\d+)-(\d+)-(\d+)T(\d+):(\d+):(\d+)\.(\d+)\+09:00$)");
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
}

void ExperimentalCar::CalcSpeedError()
{
	//if (m_carName == "jimny")
		//if (m_carName == "levorg")
	std::cout << std::format("{}: [{}]---{}", m_carName, GuiHandler::GetFrameCount(), m_gpsDataList.at(m_evaluateCount).time) << std::endl;
}

void ExperimentalCar::CalcTransedError()
{
	//cv::Point2d point(3950.671749355036, -102769.58450223645);
	//cv::Point2d point(4124.100393123896, -102586.16094288687);
	//const auto diff = (point - ResourceProvider::GetOrthoGeoInf().geo_org_pos);
	//cv::Point result_point(diff.x / ResourceProvider::GetOrthoGeoInf().convert_ratio.x, diff.y / ResourceProvider::GetOrthoGeoInf().convert_ratio.y);
	//	std::cout << "*" << std::endl;
	//	std::cout << ResourceProvider::GetOrthoGeoInf().geo_org_pos << std::endl;
	//	std::cout << diff << std::endl;
	//	std::cout << result_point << std::endl;
	//	std::cout << "*" << std::endl;
	//cv::circle(ortho, result_point, 5,
	//	cv::Scalar(0, 255, 0), -1);
}

bool ExperimentalCar::Tracking(const cv::Mat& frame)
{
	const auto ret = super::Tracking(frame);

	const auto tracking_count = GuiHandler::GetFrameCount() - m_startFrameCount;
	const auto data_exist_frame_count = std::floor(m_dataSpanFrameCount * m_passedSpanCount);

	if (ret && tracking_count == data_exist_frame_count)
	{
		bool calculate_flag = false;

		double delta_time_abs = 0.0;
		if (m_evaluateCount == 0)
		{
			delta_time_abs = get_milliseconds_from_datetime_string(m_gpsDataList[0]);
			calculate_flag = (m_carName == "jimny" && delta_time_abs == 0.000) || (m_carName == "levorg" && delta_time_abs == 0.999);
		}
		else
		{
			delta_time_abs = calc_delta_time_abs(m_gpsDataList[m_evaluateCount - 1], m_gpsDataList[m_evaluateCount]);
			calculate_flag = delta_time_abs <= m_skipSpanCount * 0.201;
		}

		if (calculate_flag)
		{
			CalcSpeedError();
			CalcTransedError();
			m_evaluateCount++;
			m_skipSpanCount = 1;
		}
		else
			m_skipSpanCount++;

		m_passedSpanCount++;
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
}