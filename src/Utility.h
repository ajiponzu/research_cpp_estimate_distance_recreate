#pragma once

#include "utility/Functions.h"

class ResourceProvider
{
private:
	friend class GuiHandler;

	static cv::Mat s_orthoDsm;
	static cv::Mat s_orthoTif;
	static cv::Mat s_orthoRoadMask;
	static cv::Mat s_transedPointsMap;
	static cv::Mat s_laneDirectionsMap;
	static cv::Mat s_roadMask;
	static Func::GeoCvt::OrthoGeoInf s_orthoGeoInf;

public:
	static void Init(const int& road_num, const std::string& video_code, const std::string& ortho_code);

	static const cv::Mat& GetOrthoDsm() { return s_orthoDsm; }
	static const cv::Mat& GetOrthoTif() { return s_orthoTif; }
	static const cv::Mat& GetOrthoRoadMask() { return s_orthoRoadMask; }
	static const cv::Mat& GetTransedPointsMap() { return s_transedPointsMap; }
	static const cv::Mat& GetLaneDirectionsMap() { return s_laneDirectionsMap; }
	static const cv::Mat& GetRoadMask() { return s_roadMask; }
	static const Func::GeoCvt::OrthoGeoInf& GetOrthoGeoInf() { return s_orthoGeoInf; }

	ResourceProvider() = delete;
	bool operator==(const ResourceProvider& other) const = delete;
};

class Renderer
{
private:
	friend class GuiHandler;
protected:
	virtual void Render(cv::Mat& img) = 0;
public:
	Renderer() = default;
	bool operator==(const Renderer& other) const = delete;
};

class GuiHandler
{
private:
	static uint64_t s_frameCount;
	static bool s_isRunning;
	static bool s_wndUpdate;
	static bool s_useVideo;
	static bool s_mouseClickedL;
	static bool s_mouseDraggedL;
	static std::pair<int, int> s_clickPoint;
	static std::pair<int, int> s_dragPoint;
	static cv::Mat s_displayImg;
	static cv::Mat s_currentFrame;
	static cv::VideoCapture s_videoCapture;
	static std::unique_ptr<Renderer> s_ptrRenderer;
	static std::unordered_set<int> s_keyEventTable;

	static std::shared_ptr<cv::BackgroundSubtractor> s_backgroundCreator;
	static cv::Mat s_backgroundFrame;
	static cv::Mat s_foregroundFrame;

	// スクリーンショットを撮影する
	static void ScreenShot();
	// マウスイベントを処理する
	static void RecvMouseMsg(int event, int x, int y, int flag, void* callBack);
	// キーボード入力を処理する
	static void HandleInputKey(const int& key);
	// イベントフラグをすべてクリアする
	static void ClearEventFlags();
public:
	// GUI機能を初期化する
	static void Initialize();
	// GUIイベントを処理する・アプリケーションの継続を通知する
	static bool EventPoll();
	// 毎フレーム描画する
	// (*)呼び出さないと表示ウィンドウに変更が適用されないので注意
	static void Render();
	// 経過フレームカウントを取得する
	static const auto& GetFrameCount() { return s_frameCount; }
	// 左クリックイベントの発生通知を取得する
	static const bool& MouseClickedL() { return s_mouseClickedL; }
	// 左ドラッグイベントの発生通知を取得する
	static const bool& MouseDraggedL() { return s_mouseDraggedL; }
	// 処理継続を確認する
	static const bool& IsRunning() { return s_isRunning; }
	// 現在のフレーム画像を取得する
	static cv::Mat GetFrame() { return s_currentFrame.clone(); }
	// ビデオリソースの読み込みと初期化
	static void SetVideoResource(const std::string& path);
	// レンダラーリソースの設定
	static void SetRenderer(Renderer* ptrRenderer);
	// ビデオリソースがある場合にそのビデオのFPSを取得する
	static double GetFPS();
	static double GetSPF();
	static uint64_t GetAllFrameNum();
	// クリックした座標を取得する
	static const std::pair<int, int>& GetClickPoint();
	// ドラッグ終了時のカーソル座標を取得する
	static const std::pair<int, int>& GetDragPoint();
	// ドラッグで作成した矩形を取得する
	static cv::Rect GetDragRect();
	// 指定したキーのイベントを取得する
	static bool GetKeyEvent(const int& keyCode) { return s_keyEventTable.contains(keyCode); }

	static void UpdateBackgroundFrame();

	static const cv::Mat& GetBackgroundFrame() { return s_backgroundFrame; }
	static const cv::Mat& GetForegroundFrame() { return s_foregroundFrame; }

	GuiHandler() = delete;
	bool operator==(const GuiHandler& other) const = delete;
};