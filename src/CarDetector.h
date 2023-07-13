#pragma once
#include "Utility.h"

class DetectedCar
{
private:
	uint64_t m_startFrameCount = 0;
	uint64_t m_curFrameCount = 0;
	double m_movingDistance = 0.0;
	double m_speed = 0.0;
	cv::Rect m_shape;
	cv::Rect m_detectionArea;
	cv::Point2f m_bodyDirection;
	cv::Point3f m_startPointTransed;
	cv::Point3f m_prevPointTransed;
	cv::Point3f m_curPointTransed;
	cv::Point3f m_curPointTransedAnother;
	cv::Mat m_img;
	cv::Mat m_detectionAreaImg;
	bool m_initialized = false;
	int64_t m_id = -1;

	void Init();
	void UpdateDetectionArea(const cv::Mat& frame);
	bool Matching();
	void UpdatePosition(const cv::Point& car_pos);
	void CalcMovingDistance();
	void CalcSpeed();
public:
	DetectedCar() = default;
	DetectedCar(const cv::Rect& shape, const cv::Rect& detection_area, const int64_t& id)
		: m_shape(shape)
		, m_detectionArea(detection_area)
		, m_id(id)
	{
		Init();
	}
	bool operator==(const DetectedCar& other) const = default;

	bool Tracking(const cv::Mat& frame);

	const double& GetSpeed() const { return m_speed; }
	const int64_t& GetId() const { return m_id; }
	const bool IsInitialized() const { return m_initialized; }

	void DrawOnImage(cv::Mat& img) const;
	void DrawOnOrtho(cv::Mat& ortho) const;

	static double CalcCarDistance(const DetectedCar& front_car, const DetectedCar& back_car);
};

class CarDetector
{
private:

	class ThisRenderer : public Renderer
	{
	private:
		std::shared_ptr<CarDetector> m_ptrDetector;

		void Render(cv::Mat& img) override;

		void DrawDetections(cv::Mat& img);
	public:
		ThisRenderer() = delete;
		ThisRenderer(CarDetector* ptr) : m_ptrDetector(ptr) {}
		bool operator==(const ThisRenderer& other) const = delete;
	};

	std::array<DetectedCar, 2> m_detectedCars;
	std::array<cv::VideoWriter, 2> m_videoWriters;
	std::ofstream m_outputCsvStream;
	int64_t m_newCarId = 0;
	uint64_t m_emptyCarId = 0;
	double m_carDistMeter = 0.0;

public:
	CarDetector() = delete;
	CarDetector(const std::wstring& model_path = L"", const cv::Size& proc_imgsz = cv::Size(640, 640));

	ThisRenderer* CreateRenderer() { return new ThisRenderer(this); }

	void Run(const cv::Mat& img);
	void SetDetectedCar(const cv::Rect& rect);

	bool operator==(const CarDetector& other) const = delete;
};