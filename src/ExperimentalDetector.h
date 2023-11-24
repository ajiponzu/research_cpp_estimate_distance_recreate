#pragma once
#include "CarDetector.h"

struct GpsData
{
	std::string time = "";
	double longitude = 0.0;
	double latitude = 0.0;
	double velocity = 0.0;
	double angle = 0.0;
};

class ExperimentalCar :
	public DetectedCar
{
protected:
	using super = DetectedCar;

	int64_t m_evaluateCount = 0;
	int64_t m_experimentalId;
	double m_dataSpanFrameCount;
	double m_speedError;
	cv::Point2f m_correctPosition;
	cv::Point2f m_transedError;
	std::string m_carName;
	std::string m_experimentStartTime;
	std::vector<GpsData> m_gpsDataList;
	std::ofstream m_ofstream;
	bool m_calculateFlag;

	virtual void Init() override;

	void CalcSpeedError();
	void CalcTransedError();

public:
	ExperimentalCar(const std::string& car_name, const std::string& experiment_start_time,
		const int64_t& experimental_id, const double& data_span_frame_count,
		const cv::Rect& shape, const int64_t& id)
		: DetectedCar(shape, id),
		m_carName(car_name),
		m_experimentStartTime(experiment_start_time),
		m_experimentalId(experimental_id),
		m_dataSpanFrameCount(data_span_frame_count)
	{
		Init();
	}

	bool operator==(const ExperimentalCar& other) const = default;

	virtual bool Tracking(const cv::Mat& frame) override;

	const double& GetSpeedError() const { return m_speedError; }
	const cv::Point2f& GetTransedError() const { return m_transedError; }
	const cv::Point2f& GetCorrectPosition() const { return m_correctPosition; }
	const bool DoneCalculatedFlag() const { return m_calculateFlag; }

	virtual void DrawOnImage(cv::Mat& img) const;
	virtual void DrawOnOrtho(cv::Mat& ortho) const;
};

class ExperimentalDetector :
	public CarDetector
{
protected:
	using super = CarDetector;

	double m_dataSpanFrameCount;
	int64_t m_experimentalId;
	std::vector<std::string> m_carNameList;
	std::vector<std::string> m_experimentStartTimeList;
	std::ofstream m_ofstream;

public:
	ExperimentalDetector() = delete;
	ExperimentalDetector(const double& data_span_frame_count, const std::vector<std::string>& car_name_list,
		const std::vector<std::string>& experiment_start_time_list, const int64_t& experimental_id,
		const std::wstring& model_path = L"", const cv::Size& proc_imgsz = cv::Size(640, 640))
		: CarDetector(model_path, proc_imgsz),
		m_dataSpanFrameCount(data_span_frame_count),
		m_carNameList(car_name_list),
		m_experimentStartTimeList(experiment_start_time_list),
		m_experimentalId(experimental_id)
	{
		m_distOutputFlag = true;
		m_ofstream.open(std::format("outputs/experiments/experiment_distance{}.csv", m_experimentalId));
		m_ofstream << "distance,correct_distance,distance_error" << std::endl;
	}

	virtual void Run(const cv::Mat& img);
	virtual void SetDetectedCar(const cv::Rect& rect);

	bool operator==(const ExperimentalDetector& other) const = delete;
};
