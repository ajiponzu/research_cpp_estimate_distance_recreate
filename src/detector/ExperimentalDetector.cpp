#include "../ExperimentalDetector.h"

void ExperimentalDetector::Run(const cv::Mat& img)
{
	super::Run(img);
}

void ExperimentalDetector::SetDetectedCar(const cv::Rect& rect)
{
	if (rect.width < 5 || rect.height < 5)
		return;

	const auto car_id = m_emptyCarId % m_detectedCars.size();

	m_detectedCars[car_id].reset(
		new ExperimentalCar(m_carNameList[car_id], m_experimentStartTimeList[car_id],
			m_experimentalId, m_dataSpanFrameCount, rect, m_newCarId));
	m_newCarId++;
	m_emptyCarId++;
}