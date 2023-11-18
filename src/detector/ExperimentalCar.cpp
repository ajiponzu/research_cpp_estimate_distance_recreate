#include "../ExperimentalDetector.h"

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
	std::cout << "speed\n";
}

void ExperimentalCar::CalcTransedError()
{
	std::cout << "transed\n";
}

bool ExperimentalCar::Tracking(const cv::Mat& frame)
{
	const auto ret = super::Tracking(frame);
	if (ret)
	{
		CalcSpeedError();
		CalcTransedError();
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