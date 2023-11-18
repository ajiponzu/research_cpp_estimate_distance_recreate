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
	static constexpr auto EXPERIMENTAL_ID = 3;
	//static constexpr auto EXPERIMENTAL_ID = 4;
	//static constexpr auto EXPERIMENTAL_ID = 5;

	std::vector<std::vector<std::string>> start_time_list =
	{
		{"", ""},
		{"", ""},
		{"", ""},
		{"2023-11-05T10:26:39.000+09:00", "2023-11-05T10:26:38.999+09:00"},
		{"", ""},
		{"", ""},
	};

	struct ExperimentalStartData
	{
		int64_t start_frame_count = 0;
		std::unordered_map<std::string, cv::Rect> car_pos_hash{};
	} experimental_start_data;

	experimental_start_data.start_frame_count = 126;
	experimental_start_data.car_pos_hash["jimny"] = cv::Rect(610, 437, 14, 16);
	experimental_start_data.car_pos_hash["levorg"] = cv::Rect(615, 407, 14, 14);

	GuiHandler::Initialize();
	ResourceProvider::Init(g_ROAD_NUM, std::format("{}{}", VIDEO_CODE, EXPERIMENTAL_ID), g_ORTHO_CODE);
	GuiHandler::SetVideoResource(std::format("resources/{}{}/input.mp4", VIDEO_CODE, EXPERIMENTAL_ID));
	//ExperimentalDetector car_detector(GuiHandler::GetFPS(), { "jimny", "levorg" },
	//	start_time_list[EXPERIMENTAL_ID], EXPERIMENTAL_ID, L"", cv::Size(g_PROC_IMGSZ, g_PROC_IMGSZ));
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

int main()
{
	//return app();
	return experiment();
}