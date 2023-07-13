#include "../CarDetector.h"

static constexpr auto g_OUTPUT_CSV_PATH = "outputs/distances";
static constexpr auto g_THICKNESS = 2;
static constexpr auto g_FONT_FACE = cv::FONT_HERSHEY_DUPLEX;
static constexpr auto g_FONT_SCALE = 1.5;
static constexpr auto g_CSV_HEADER = "frame, distance";

static constexpr auto g_VIDEO_BASE_PATH = "outputs/trackings";
static const auto g_FOURCC = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
static constexpr auto g_FPS = 30.0;

void CarDetector::Run(const cv::Mat& _img)
{
	auto frame = GuiHandler::GetFrame();

	uint32_t count = 0;
	m_distOutputFlag = true;
	for (auto& car : m_detectedCars)
	{
		if (car.Tracking(frame))
			m_trackingCarIdList[count] = car.GetId();
		else
		{
			car = DetectedCar();
			m_distOutputFlag = false;
			m_trackingCarIdList[count] = -1;
			m_emptyCarId = count;
		}
		count++;
	}

	if (m_distOutputFlag)
	{
		m_carDistMeter = std::min(
			DetectedCar::CalcCarDistance(m_detectedCars[0], m_detectedCars[1]),
			DetectedCar::CalcCarDistance(m_detectedCars[1], m_detectedCars[0]));
	}
	else
	{
		m_distEstimatedFrameCount = 0;
		m_carDistMeter = 0.0;
	}
}

void CarDetector::ThisRenderer::Render(cv::Mat& img)
{
	const auto& road_mask = ResourceProvider::GetRoadMask();
	cv::addWeighted(img, 0.85, road_mask, 0.15, 1.0, img);

	OutputDetections(img);
}

void CarDetector::ThisRenderer::CloseOutputStream()
{
	if (m_ptrDetector->m_distOutputFlag)
		return;

	if (m_videoWriter.isOpened())
		m_videoWriter.release();
#ifdef SHOW_ORTHO
	if (m_orthoVideoWriter.isOpened())
		m_orthoVideoWriter.release();
#endif
	if (m_outputCsvStream.is_open())
		m_outputCsvStream.close();
}

#ifdef SHOW_ORTHO
void CarDetector::ThisRenderer::OutputData(const cv::Mat& img, const cv::Mat& ortho)
{
	if (!m_ptrDetector->m_distOutputFlag)
		return;

	const auto& car_id_list = m_ptrDetector->m_trackingCarIdList;
	const auto& car_dist_meter = m_ptrDetector->m_carDistMeter;
	auto& dist_estimated_frame_count = m_ptrDetector->m_distEstimatedFrameCount;

	if (m_videoWriter.isOpened())
		m_videoWriter.write(img);
	else
		m_videoWriter.open(std::format("{}/output_{}_{}.mp4",
			g_VIDEO_BASE_PATH, car_id_list[0], car_id_list[1]),
			g_FOURCC, g_FPS, img.size());

	if (m_orthoVideoWriter.isOpened())
		m_orthoVideoWriter.write(ortho);
	else
		m_orthoVideoWriter.open(std::format("{}/output_{}_{}_ortho.mp4",
			g_VIDEO_BASE_PATH, car_id_list[0], car_id_list[1]),
			g_FOURCC, g_FPS, ortho.size());

	if (m_outputCsvStream.is_open())
	{
		m_outputCsvStream
			<< std::format("{}, {:.1f}", dist_estimated_frame_count, car_dist_meter)
			<< std::endl;
		dist_estimated_frame_count++;
	}
	else
	{
		m_outputCsvStream.open(std::format("{}/output_{}_{}.csv",
			g_OUTPUT_CSV_PATH, car_id_list[0], car_id_list[1]));
		m_outputCsvStream << g_CSV_HEADER << std::endl;
	}
}
#else
void CarDetector::ThisRenderer::OutputData(const cv::Mat& img)
{
	if (!m_ptrDetector->m_distOutputFlag)
		return;

	const auto& car_id_list = m_ptrDetector->m_trackingCarIdList;
	const auto& car_dist_meter = m_ptrDetector->m_carDistMeter;
	auto& dist_estimated_frame_count = m_ptrDetector->m_distEstimatedFrameCount;

	if (m_videoWriter.isOpened())
		m_videoWriter.write(img);
	else
		m_videoWriter.open(std::format("{}/output_{}_{}.mp4",
			g_VIDEO_BASE_PATH, car_id_list[0], car_id_list[1]),
			g_FOURCC, g_FPS, img.size());

	if (m_outputCsvStream.is_open())
	{
		m_outputCsvStream
			<< std::format("{}, {:.1f}", dist_estimated_frame_count, car_dist_meter)
			<< std::endl;
		dist_estimated_frame_count++;
	}
	else
	{
		m_outputCsvStream.open(std::format("{}/output_{}_{}.csv",
			g_OUTPUT_CSV_PATH, car_id_list[0], car_id_list[1]));
		m_outputCsvStream << g_CSV_HEADER << std::endl;
	}
}
#endif

void CarDetector::ThisRenderer::OutputDetections(cv::Mat& img)
{
	const auto& detected_cars = m_ptrDetector->m_detectedCars;

	if (detected_cars.empty())
		return;

#ifdef SHOW_ORTHO
	auto ortho = ResourceProvider::GetOrthoTif().clone();
	const auto& ortho_road_mask = ResourceProvider::GetOrthoRoadMask();
	cv::addWeighted(ortho, 0.85, ortho_road_mask, 0.15, 1.0, ortho);
#endif

	uint64_t car_idx = 0;
	for (const auto& detected_car : detected_cars)
	{
		detected_car.DrawOnImage(img);
#ifdef SHOW_ORTHO
		detected_car.DrawOnOrtho(ortho);
#endif
		car_idx++;
	}

	if (m_ptrDetector->m_distOutputFlag)
	{
		int base_line = 0;
		const auto distance_txt = std::format("cars_distance: {:.1f} [m]", m_ptrDetector->m_carDistMeter);
		const auto font_size = cv::getTextSize(distance_txt,
			g_FONT_FACE, g_FONT_SCALE, g_THICKNESS, &base_line);
		cv::putText(img, distance_txt, cv::Point(750, 50),
			g_FONT_FACE, g_FONT_SCALE, cv::Scalar(0, 0, 255), g_THICKNESS);
	}

#ifdef SHOW_ORTHO
	cv::resize(ortho, ortho, cv::Size(), 0.5, 0.5);
	cv::imshow("ortho", ortho);
#endif

	CloseOutputStream();

	if (!GuiHandler::IsRunning())
		return;
	if (!m_ptrDetector->m_distOutputFlag)
		return;

#ifdef SHOW_ORTHO
	OutputData(img, ortho);
#else
	OutputData(img);
#endif
}

void CarDetector::SetDetectedCar(const cv::Rect& rect)
{
	if (rect.width < 5 || rect.height < 5)
		return;

	auto detection_area = cv::Rect(0, 0, rect.width + 20, rect.height + 20);
	detection_area.x = std::max(0, rect.x - 10);
	detection_area.y = std::max(0, rect.y - 10);

	m_detectedCars[m_emptyCarId % m_detectedCars.size()]
		= DetectedCar(rect, detection_area, m_newCarId);
	m_newCarId++;
	m_emptyCarId++;
}