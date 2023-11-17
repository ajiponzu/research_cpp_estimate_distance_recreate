#include "Utility.h"
#include "CarDetector.h"

//#define DISPLAY_SPEED

//constexpr auto g_VIDEO_CODE = "hiru";
//constexpr auto g_VIDEO_CODE = "yugata";
constexpr auto g_VIDEO_CODE = "kitani0";
//constexpr auto g_VIDEO_CODE = "kitani1";
//constexpr auto g_VIDEO_CODE = "kitani2";
//constexpr auto g_VIDEO_CODE = "kitani3";
//constexpr auto g_VIDEO_CODE = "kitani4";
//constexpr auto g_VIDEO_CODE = "kitani5";

constexpr auto g_ORTHO_CODE = "ortho";
constexpr auto g_ROAD_NUM = 4;
constexpr auto g_PROC_IMGSZ = 50;

int main()
{
	GuiHandler::Initialize();
	ResourceProvider::Init(g_ROAD_NUM, g_VIDEO_CODE, g_ORTHO_CODE);
	CarDetector carDetector(L"", cv::Size(g_PROC_IMGSZ, g_PROC_IMGSZ));

	GuiHandler::SetVideoResource(std::format("resources/{}/input.mp4", g_VIDEO_CODE));
	GuiHandler::SetRenderer(carDetector.CreateRenderer());

	while (GuiHandler::EventPoll())
	{
		//GuiHandler::UpdateBackgroundFrame();
#ifdef DISPLAY_SPEED
		const auto start = std::chrono::high_resolution_clock::now();
#endif
		if (GuiHandler::IsRunning())
		{
			const cv::Mat frame = GuiHandler::GetFrame();
			carDetector.Run(frame);
		}

		if (GuiHandler::MouseDraggedL())
		{
			const auto drag_rect = GuiHandler::GetDragRect();
			carDetector.SetDetectedCar(drag_rect);
		}
		carDetector.SetDistOutputFlag();

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