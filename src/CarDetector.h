#pragma once
#include "Utility.h"

class DetectedCar
{
protected:
	int64_t m_id = -1;
	uint64_t m_startFrameCount = 0;
	uint64_t m_curFrameCount = 0;
	double m_movingDistance = 0.0;
	double m_speed = 0.0;
	cv::Rect m_shape;
	cv::Rect m_detectionArea;
	cv::Point2f m_orthoBodyDirection;
	cv::Point3f m_prevPointTransed;
	cv::Point3f m_curPointTransed;
	cv::Point3f m_curPointTransedAnother;
	cv::Mat m_img;
	cv::Mat m_detectionAreaImg;
	bool m_initialized = false;

	virtual void Init();
	void UpdateDetectionArea(const cv::Mat& frame);
	bool Matching();
	void UpdatePosition(const cv::Point& car_pos);
	void CalcMovingDistance();
	void CalcSpeed();
public:
	DetectedCar() = default;
	DetectedCar(const cv::Rect& shape, const int64_t& id)
		: m_shape(shape)
		, m_id(id)
	{
		Init();
	}
	bool operator==(const DetectedCar& other) const = default;

	virtual bool Tracking(const cv::Mat& frame);

	const double& GetSpeed() const { return m_speed; }
	const int64_t& GetId() const { return m_id; }

	virtual void DrawOnImage(cv::Mat& img) const;
	virtual void DrawOnOrtho(cv::Mat& ortho) const;

	const bool& IsInitialized() const { return m_initialized; }

	static double CalcCarDistance(const DetectedCar& front_car, const DetectedCar& back_car);
};

class CarDetector
{
protected:

	class ThisRenderer : public Renderer
	{
	protected:
		std::shared_ptr<CarDetector> m_ptrDetector;
		cv::VideoWriter m_videoWriter;
		cv::VideoWriter m_orthoVideoWriter;
		std::ofstream m_outputCsvStream;

		virtual void Render(cv::Mat& img) override;

		virtual void CloseOutputStream();
#ifdef SHOW_ORTHO
		virtual void OutputData(const cv::Mat& img, const cv::Mat& ortho);
#else
		virtual void OutputData(const cv::Mat& img);
#endif
		virtual void OutputDetections(cv::Mat& img);
	public:
		ThisRenderer() = delete;
		ThisRenderer(CarDetector* ptr) : m_ptrDetector(ptr) {}
		bool operator==(const ThisRenderer& other) const = delete;
	};

	std::array<std::shared_ptr<DetectedCar>, 2> m_detectedCars;
	int64_t m_newCarId = 0;
	uint64_t m_emptyCarId = 0;
	double m_carDistMeter = 0.0;
	bool m_distOutputFlag = false;
	uint64_t m_distEstimatedFrameCount = 0;

public:
	CarDetector() = delete;
	CarDetector(const std::wstring& model_path = L"", const cv::Size& proc_imgsz = cv::Size(640, 640))
	{
		m_detectedCars[0].reset(new DetectedCar());
		m_detectedCars[1].reset(new DetectedCar());
	}

	ThisRenderer* CreateRenderer() { return new ThisRenderer(this); }

	virtual void Run(const cv::Mat& img);
	virtual void SetDetectedCar(const cv::Rect& rect);
	virtual void SetDistOutputFlag();

	bool operator==(const CarDetector& other) const = delete;
};