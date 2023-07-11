#include "Utility.h"
#include "CarDetector.h"

#undef SHOW_ORTHO

constexpr auto g_video_code = "hiru";
//constexpr auto g_video_code = "yugata";

constexpr auto g_ortho_code = "ortho";
constexpr auto g_road_num = 4;
constexpr auto g_proc_imgsz = 50;

int main()
{
	GuiHandler::Initialize();
	ResourceProvider::Init(g_road_num, g_video_code, g_ortho_code);
	CarDetector carDetector(L"", cv::Size(g_proc_imgsz, g_proc_imgsz));

	GuiHandler::SetVideoResource(std::format("resources/{}/input.mp4", g_video_code));
	GuiHandler::SetRenderer(carDetector.CreateRenderer());

	while (GuiHandler::EventPoll())
	{
		const auto start = std::chrono::high_resolution_clock::now();
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

		GuiHandler::Render();

		const auto end = std::chrono::high_resolution_clock::now();
		//std::cout << std::format("elapsed: {}\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
	}
	cv::destroyAllWindows();
	std::cout << "end....." << std::endl;

	return 0;
}