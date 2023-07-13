#include "../CarDetector.h"

static bool g_dist_output_flag = false;
static uint64_t g_distance_estimate_frame_count = 0;

static constexpr auto g_OUTPUT_CSV_PATH = "outputs/distances";
static constexpr auto g_THICKNESS = 2;
static constexpr auto g_FONT_FACE = cv::FONT_HERSHEY_DUPLEX;
static constexpr auto g_FONT_SCALE = 1.5;
static constexpr auto g_ESTIMATED_FRAME_SPAN = 5;
static constexpr auto g_CSV_HEADER = "frame, distance";

static constexpr auto g_FPS = 30.0;
static const auto g_FOURCC = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
static const auto g_VIDEO_SIZE = cv::Size(1920, 1080);
static const auto g_ORTHO_VIDEO_SIZE = cv::Size(2119, 2577);
static constexpr auto g_VIDEO_BASE_PATH = "outputs/trackings";

enum class VideoIdx : uint32_t
{
	VIDEO = 0,
	ORTHO,
};

CarDetector::CarDetector(const std::wstring& model_path, const cv::Size& proc_imgsz)
{
	for (auto& video_writer : m_videoWriters)
		video_writer = cv::VideoWriter();
}

void CarDetector::Run(const cv::Mat& _img)
{
	auto frame = GuiHandler::GetFrame();

	uint32_t count = 0;
	int64_t car_id_list[] = { -1, -1 };
	bool initialized_flag = true;
	for (auto& car : m_detectedCars)
	{
		if (car.Tracking(frame))
			car_id_list[count] = car.GetId();
		else
		{
			car = DetectedCar();
			m_emptyCarId = count;
			initialized_flag = false;
		}
		count++;
	}

	if (initialized_flag)
	{
		m_carDistMeter = std::min(
			DetectedCar::CalcCarDistance(m_detectedCars[0], m_detectedCars[1]),
			DetectedCar::CalcCarDistance(m_detectedCars[1], m_detectedCars[0]));

		if (!g_dist_output_flag)
		{
			g_distance_estimate_frame_count = 0;
			g_dist_output_flag = true;
			m_videoWriters[std::underlying_type_t<VideoIdx>(VideoIdx::VIDEO)]
				.open(std::format("{}/output_{}_{}.mp4",
					g_VIDEO_BASE_PATH, car_id_list[0], car_id_list[1]),
					g_FOURCC, g_FPS, g_VIDEO_SIZE);
#ifdef SHOW_ORTHO
			m_videoWriters[std::underlying_type_t<VideoIdx>(VideoIdx::ORTHO)]
				.open(std::format("{}/output_{}_{}_ortho.mp4",
					g_VIDEO_BASE_PATH, car_id_list[0], car_id_list[1]),
					g_FOURCC, g_FPS, g_ORTHO_VIDEO_SIZE);
#endif
			m_outputCsvStream.open(std::format("{}/output_{}_{}.csv",
				g_OUTPUT_CSV_PATH, car_id_list[0], car_id_list[1]));
			m_outputCsvStream << g_CSV_HEADER << std::endl;
		}

		if (GuiHandler::GetFrameCount() % g_ESTIMATED_FRAME_SPAN == 0)
		{
			m_outputCsvStream
				<< std::format("{}, {:.1f}", g_distance_estimate_frame_count, m_carDistMeter)
				<< std::endl;
			g_distance_estimate_frame_count += g_ESTIMATED_FRAME_SPAN;
		}
	}
	else
	{
		m_carDistMeter = 0.0;
		g_dist_output_flag = false;
		auto& video_writer = m_videoWriters[std::underlying_type_t<VideoIdx>(VideoIdx::VIDEO)];
		if (video_writer.isOpened())
			video_writer.release();
#ifdef SHOW_ORTHO
		auto& video_writer_ortho = m_videoWriters[std::underlying_type_t<VideoIdx>(VideoIdx::ORTHO)];
		if (video_writer_ortho.isOpened())
			video_writer_ortho.release();
#endif
		if (m_outputCsvStream.is_open())
			m_outputCsvStream.close();
	}
}

void CarDetector::ThisRenderer::Render(cv::Mat& img)
{
	const auto& road_mask = ResourceProvider::GetRoadMask();
	cv::addWeighted(img, 0.85, road_mask, 0.15, 1.0, img);

	DrawDetections(img);
}

void CarDetector::ThisRenderer::DrawDetections(cv::Mat& img)
{
	const auto& detected_cars = m_ptrDetector->m_detectedCars;

	if (detected_cars.empty())
		return;

#ifdef SHOW_ORTHO
	auto ortho = ResourceProvider::GetOrthoTif().clone();
	const auto& ortho_road_mask = ResourceProvider::GetOrthoRoadMask();
	cv::addWeighted(ortho, 0.85, ortho_road_mask, 0.15, 1.0, ortho);
#endif

	uint64_t car_id = 0;
	for (const auto& detected_car : detected_cars)
	{
		detected_car.DrawOnImage(img);
#ifdef SHOW_ORTHO
		detected_car.DrawOnOrtho(ortho);
#endif
#ifdef _DEBUG
		std::cout << std::format("car_{}_speed: {:.1f} [km/h]", car_id, detected_car.GetSpeed()) << std::endl;
#endif
		car_id++;
	}

	if (!GuiHandler::IsRunning())
		return;

	if (g_dist_output_flag)
	{
		int base_line = 0;
		const auto distance_txt = std::format("cars_distance: {:.1f} [m]", m_ptrDetector->m_carDistMeter);
		const auto font_size = cv::getTextSize(distance_txt,
			g_FONT_FACE, g_FONT_SCALE, g_THICKNESS, &base_line);

		cv::putText(img, distance_txt, cv::Point(750, 50),
			g_FONT_FACE, g_FONT_SCALE, cv::Scalar(0, 0, 255), g_THICKNESS);
	}

	auto& video_writers = m_ptrDetector->m_videoWriters;
	auto& video_writer = video_writers[std::underlying_type_t<VideoIdx>(VideoIdx::VIDEO)];
	if (video_writer.isOpened())
		video_writer.write(img);
#ifdef SHOW_ORTHO
	auto& video_writer_ortho = video_writers[std::underlying_type_t<VideoIdx>(VideoIdx::ORTHO)];
	if (video_writer_ortho.isOpened())
		video_writer_ortho.write(ortho);
	cv::resize(ortho, ortho, cv::Size(), 0.5, 0.5);
	cv::imshow("ortho", ortho);
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