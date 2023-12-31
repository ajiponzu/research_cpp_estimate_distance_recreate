#include "../ExperimentalDetector.h"

void ExperimentalDetector::Run(const cv::Mat& img)
{
	auto frame = GuiHandler::GetFrame();
	uint32_t count = 0;

	bool file_write_flag = true;

	std::vector<cv::Point2f> correct_position_list;

	for (auto& car : m_detectedCars)
	{
		if (car->IsInitialized())
		{
			const auto experimental_car = dynamic_cast<ExperimentalCar*>(car.get());
			if (experimental_car->Tracking(frame))
			{
				file_write_flag = file_write_flag && experimental_car->DoneCalculatedFlag();
				correct_position_list.push_back(experimental_car->GetCorrectPosition());
			}
			else
			{
				car.reset(new DetectedCar());
				m_distOutputFlag = false;
				m_emptyCarId = count;
				file_write_flag = false;
			}
		}
		else
			file_write_flag = false;

		count++;
	}

	if (!file_write_flag)
		return;

	m_carDistMeter = std::min(
		DetectedCar::CalcCarDistance(*m_detectedCars.at(0), *m_detectedCars.at(1)),
		DetectedCar::CalcCarDistance(*m_detectedCars.at(1), *m_detectedCars.at(0)));

	const auto correct_distance = cv::norm(correct_position_list[0] - correct_position_list[1])
		* ResourceProvider::GetOrthoGeoInf().meter_ratio;

	m_ofstream << std::format("{},{},{},{}\n", GuiHandler::GetFrameCount(), m_carDistMeter, correct_distance, std::abs(m_carDistMeter - correct_distance));
}

void ExperimentalDetector::SetDetectedCarExp(const cv::Rect& rect, const double& body_length)
{
	if (rect.width < 5 || rect.height < 5)
		return;

	const auto car_id = m_emptyCarId % m_detectedCars.size();

	m_detectedCars[car_id].reset(
		new ExperimentalCar(m_carNameList[car_id], m_experimentStartTimeList[car_id],
			m_experimentalId, m_dataSpanFrameCount, rect, m_newCarId, body_length));
	m_newCarId++;
	m_emptyCarId++;
}