#include "../Utility.h"

#define NOMINMAX
#include <Windows.h>

/* GuiHandlerのstatic変数初期化 */
uint64_t GuiHandler::s_frameCount = 0;
bool GuiHandler::s_isRunning = true;
bool GuiHandler::s_wndUpdate = true;
bool GuiHandler::s_useVideo = false;
bool GuiHandler::s_mouseClickedL = false;
bool GuiHandler::s_mouseDraggedL = false;
std::pair<int, int> GuiHandler::s_clickPoint;
std::pair<int, int> GuiHandler::s_dragPoint;
cv::Mat GuiHandler::s_displayImg{};
cv::Mat GuiHandler::s_currentFrame{};
cv::VideoCapture GuiHandler::s_videoCapture;
std::unique_ptr<Renderer> GuiHandler::s_ptrRenderer = nullptr;
std::unordered_set<int>  GuiHandler::s_keyEventTable{};

std::shared_ptr<cv::BackgroundSubtractor> GuiHandler::s_backgroundCreator;
cv::Mat GuiHandler::s_backgroundFrame;
cv::Mat GuiHandler::s_foregroundFrame;
/* end */

/* global変数 */
static constexpr auto g_WND_NAME = "display";
static constexpr auto g_FRAME_INTERVAl = 1;
static constexpr auto g_BASE_PATH = "outputs/screenshots";
static bool g_mouse_move_l = false;
static cv::Point g_move_point;
/* end */

void GuiHandler::ScreenShot()
{
	std::string path = std::format("{}/result_{}.png", g_BASE_PATH, s_frameCount);
	cv::imwrite(path, s_displayImg);
	path = std::format("{}/frame_{}.png", g_BASE_PATH, s_frameCount);
	cv::imwrite(path, s_currentFrame);
}

void GuiHandler::RecvMouseMsg(int event, int x, int y, int flag, void* callBack)
{
	switch (event) {
	case cv::EVENT_LBUTTONDOWN: // マウス左クリック
		s_clickPoint = std::make_pair(x, y);
		s_mouseClickedL = true;
		break;
	case cv::EVENT_MOUSEMOVE:
		if (!s_mouseClickedL)
			return;
		s_dragPoint = std::make_pair(x, y);
		g_mouse_move_l = true;
		break;
	case cv::EVENT_LBUTTONUP:
		if (!g_mouse_move_l)
		{
			s_mouseClickedL = false;
			return;
		}
		s_dragPoint = std::make_pair(x, y);
		s_mouseDraggedL = true;
		g_mouse_move_l = false;
		s_mouseClickedL = false;
		break;
	case cv::EVENT_RBUTTONDOWN: // マウス右クリック
		s_isRunning = !s_isRunning;
		break;
	default:
		break;
	}
}

void GuiHandler::HandleInputKey(const int& key)
{
	switch (key)
	{
	case 27: // ESCキー
		s_wndUpdate = false;
		break;
	case 32: // SPACE
		break;
	default:
		break;
	}
	s_keyEventTable.insert(key);
}

void GuiHandler::ClearEventFlags()
{
	s_mouseDraggedL = false;
	s_keyEventTable.clear();
}

void GuiHandler::Initialize()
{
	// FreeConsole(); // 本番環境でのコンソール非表示関数

	cv::namedWindow(g_WND_NAME, cv::WindowFlags::WINDOW_FULLSCREEN);
	cv::setMouseCallback(g_WND_NAME, RecvMouseMsg);
	cv::moveWindow(g_WND_NAME, 0, 0);

	s_backgroundCreator = cv::createBackgroundSubtractorKNN();
}

bool GuiHandler::EventPoll()
{
	ClearEventFlags();
	HandleInputKey(cv::waitKey(g_FRAME_INTERVAl));

	if (s_useVideo && s_isRunning) // ビデオ使用時かつ再生中
	{
		s_videoCapture.read(s_currentFrame);
		if (s_currentFrame.empty())
		{
			s_wndUpdate = false;
			return s_wndUpdate;
		}
		s_frameCount++;
	}
	s_currentFrame.copyTo(s_displayImg);

	if (!s_videoCapture.isOpened() && s_useVideo)
		s_wndUpdate = false;

	return s_wndUpdate;
}

void GuiHandler::Render()
{
	if (s_ptrRenderer)
		s_ptrRenderer->Render(s_displayImg);

	if (GetKeyEvent((int)(' ')))
		ScreenShot();

	if (g_mouse_move_l)
	{
		const auto& [tx, ty] = s_clickPoint;
		const auto& [bx, by] = s_dragPoint;
		cv::rectangle(s_displayImg, cv::Rect(tx, ty, bx - tx, by - ty), cv::Scalar(0, 0, 255), 2);
	}

	cv::imshow(g_WND_NAME, s_displayImg);
}

void GuiHandler::SetVideoResource(const std::string& path)
{
	if (s_videoCapture.open(path))
	{
		const int wid = (int)s_videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
		const int high = (int)s_videoCapture.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
		s_currentFrame = cv::Mat(high, wid, CV_8UC3);
		s_displayImg = cv::Mat(high, wid, CV_8UC3);

		s_useVideo = true;
	}
}

void GuiHandler::SetRenderer(Renderer* ptrRenderer)
{
	s_ptrRenderer.reset(ptrRenderer);
}

double GuiHandler::GetFPS()
{
	if (!s_useVideo)
		return 0.0;

	return s_videoCapture.get(cv::CAP_PROP_FPS);
}

double GuiHandler::GetSPF()
{
	if (!s_useVideo)
		return 0.0;

	return 1.0 / GetFPS();
}

const std::pair<int, int>& GuiHandler::GetClickPoint()
{
	std::cout << std::format("click: ({}, {})", s_clickPoint.first, s_clickPoint.second) << std::endl;
	return s_clickPoint;
}

const std::pair<int, int>& GuiHandler::GetDragPoint()
{
	std::cout << std::format("drag: ({}, {})", s_dragPoint.first, s_dragPoint.second) << std::endl;
	return s_dragPoint;
}

cv::Rect GuiHandler::GetDragRect()
{
	const auto& [cl_x, cl_y] = s_clickPoint;
	const auto& [dr_x, dr_y] = s_dragPoint;

	const int rx = std::min(cl_x, dr_x);
	const int ry = std::min(cl_y, dr_y);
	const int width = std::abs(cl_x - dr_x);
	const int height = std::abs(cl_y - dr_y);

	return cv::Rect(rx, ry, width, height);
}

void GuiHandler::UpdateBackgroundFrame()
{
	s_backgroundCreator->apply(s_currentFrame, s_foregroundFrame);
	s_backgroundCreator->getBackgroundImage(s_backgroundFrame);
}