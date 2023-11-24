#include "Utility.h"
//#include "CarDetector.h"
#include "ExperimentalDetector.h"

//#define DISPLAY_SPEED

constexpr auto g_ORTHO_CODE = "ortho";
constexpr auto g_ROAD_NUM = 4;
constexpr auto g_PROC_IMGSZ = 50;

static int app()
{
	static constexpr auto VIDEO_CODE = "hiru";
	//static constexpr auto VIDEO_CODE = "yugata";

	GuiHandler::Initialize();
	ResourceProvider::Init(g_ROAD_NUM, VIDEO_CODE, g_ORTHO_CODE);
	CarDetector car_detector(L"", cv::Size(g_PROC_IMGSZ, g_PROC_IMGSZ));
	GuiHandler::SetVideoResource(std::format("resources/{}/input.mp4", VIDEO_CODE));
	GuiHandler::SetRenderer(car_detector.CreateRenderer());

	while (GuiHandler::EventPoll())
	{
		//GuiHandler::UpdateBackgroundFrame();
#ifdef DISPLAY_SPEED
		const auto start = std::chrono::high_resolution_clock::now();
#endif
		if (GuiHandler::IsRunning())
		{
			const cv::Mat frame = GuiHandler::GetFrame();
			car_detector.Run(frame);
		}

		if (GuiHandler::MouseDraggedL())
		{
			const auto drag_rect = GuiHandler::GetDragRect();
			car_detector.SetDetectedCar(drag_rect);
		}
		car_detector.SetDistOutputFlag();
		GuiHandler::Render();

#ifdef DISPLAY_SPEED
		const auto end = std::chrono::high_resolution_clock::now();
		std::cout << std::format("elapsed: {} [ms]\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
#endif
	}
	cv::destroyAllWindows();
	std::cout << "end....." << std::endl;

	return 0;
}

static int experiment()
{
	static constexpr auto VIDEO_CODE = "kitani";
	//static constexpr auto EXPERIMENTAL_ID = 0;
	//static constexpr auto EXPERIMENTAL_ID = 1;
	//static constexpr auto EXPERIMENTAL_ID = 2;
	//static constexpr auto EXPERIMENTAL_ID = 3;
	//static constexpr auto EXPERIMENTAL_ID = 4;
	static constexpr auto EXPERIMENTAL_ID = 5;

	std::vector<std::vector<std::string>> start_time_list =
	{
		{"2023-11-05 09:58:27.000", "2023-11-05 09:58:26.999"},
		{"2023-11-05 10:11:01.999", "2023-11-05 10:11:02.000"},
		{"2023-11-05 10:17:29.999", "2023-11-05 10:17:29.999"},
		{"2023-11-05 10:26:39.000", "2023-11-05 10:26:38.999"},
		{"2023-11-05 10:44:52.999", "2023-11-05 10:44:53.000"},
		{"2023-11-06 15:28:51.000", "2023-11-06 15:28:51.000"},
	};

	struct ExperimentalStartData
	{
		int64_t start_frame_count = 0;
		std::unordered_map<std::string, cv::Rect> car_pos_hash{};
	} experimental_start_data;

	switch (EXPERIMENTAL_ID)
	{
	case 0:
		experimental_start_data.start_frame_count = 89;
		experimental_start_data.car_pos_hash["jimny"] = cv::Rect(475, 661, 18, 17);
		experimental_start_data.car_pos_hash["levorg"] = cv::Rect(482, 701, 17, 20);
		break;
	case 1:
		experimental_start_data.start_frame_count = 139;
		experimental_start_data.car_pos_hash["jimny"] = cv::Rect(611, 447, 13, 16);
		experimental_start_data.car_pos_hash["levorg"] = cv::Rect(611, 425, 17, 15);
		break;
	case 2:
		experimental_start_data.start_frame_count = 115;
		experimental_start_data.car_pos_hash["jimny"] = cv::Rect(473, 654, 17, 16);
		experimental_start_data.car_pos_hash["levorg"] = cv::Rect(484, 717, 20, 17);
		break;
	case 3:
		experimental_start_data.start_frame_count = 126;
		experimental_start_data.car_pos_hash["jimny"] = cv::Rect(610, 437, 14, 16);
		experimental_start_data.car_pos_hash["levorg"] = cv::Rect(615, 407, 14, 14);
		break;
	case 4:
		experimental_start_data.start_frame_count = 78;
		experimental_start_data.car_pos_hash["jimny"] = cv::Rect(986, 650, 21, 21);
		experimental_start_data.car_pos_hash["levorg"] = cv::Rect(959, 700, 24, 21);
		break;
	case 5:
		experimental_start_data.start_frame_count = 86;
		experimental_start_data.car_pos_hash["jimny"] = cv::Rect(1178, 338, 12, 15);
		experimental_start_data.car_pos_hash["levorg"] = cv::Rect(1159, 323, 14, 14);
		break;
	default:
		break;
	}

	GuiHandler::Initialize();
	ResourceProvider::Init(g_ROAD_NUM, std::format("{}{}", VIDEO_CODE, EXPERIMENTAL_ID), g_ORTHO_CODE);
	GuiHandler::SetVideoResource(std::format("resources/{}{}/input.mp4", VIDEO_CODE, EXPERIMENTAL_ID));
	ExperimentalDetector car_detector(GuiHandler::GetFPS() * 0.2,
		{ "jimny", "levorg" }, start_time_list[EXPERIMENTAL_ID], EXPERIMENTAL_ID, L"", cv::Size(g_PROC_IMGSZ, g_PROC_IMGSZ));
	GuiHandler::SetRenderer(car_detector.CreateRenderer());

	while (GuiHandler::EventPoll())
	{
		//GuiHandler::UpdateBackgroundFrame();
#ifdef DISPLAY_SPEED
		const auto start = std::chrono::high_resolution_clock::now();
#endif
		if (experimental_start_data.start_frame_count == GuiHandler::GetFrameCount())
		{
			car_detector.SetDetectedCar(experimental_start_data.car_pos_hash["jimny"]);
			car_detector.SetDetectedCar(experimental_start_data.car_pos_hash["levorg"]);
		}

		if (GuiHandler::IsRunning())
		{
			const cv::Mat frame = GuiHandler::GetFrame();
			car_detector.Run(frame);
		}

		GuiHandler::Render();

#ifdef DISPLAY_SPEED
		const auto end = std::chrono::high_resolution_clock::now();
		std::cout << std::format("elapsed: {} [ms]\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
#endif
	}
	cv::destroyAllWindows();
	std::cout << "end....." << std::endl;

	return 0;
}

int main()
{
	//return app();
	return experiment();
}