#include <opencv2/opencv.hpp>
#include "image_preprocessing/ImagePreProcessing.h"


struct LaneLines {
    float posSlopeMean = -1.0f; // Initialize with neg values for firsttime check
    double xInterceptPosMean = -1.0;
    float negSlopeMean = -1.0f;
    double xInterceptNegMean = -1.0;
};


class StrassenErkennung
{
    public:
        bool get_steering_dir(cv::Mat inputImage, int& steering_direction, bool drawing_enabled=true);
        
        //Before calling this functions get_steering_dir has to be called
        cv::Mat get_mask();
        cv::Mat get_maskedImage();
        cv::Mat get_edgesImage();
        cv::Mat get_houghLinesImage();
        cv::Mat get_laneLineImage();
        cv::Mat get_filteredHoughLinesImage();


    private:
        ImagePreProcessing imagePreProcessor;

        LaneLines lastLaneLine;

        bool houghLines(cv::Mat maskedImage, cv::Mat originalImage, std::vector<cv::Vec4i>& lines, bool drawing_enabled);
        bool averageLaneLines(std::vector<cv::Vec4i> lines, cv::Mat originalImage, LaneLines& lanelines, bool drawing_enabled);
        int calculate_steering_dir(LaneLines laneLines, cv::Mat originalImage);
        void plot_laneLines(LaneLines laneLines, cv::Mat originalImage);

        double median(std::vector<double> vec);

        cv::Mat mask;
        cv::Mat maskedImage;
        cv::Mat edgesImage;
        cv::Mat laneLineImage;
        cv::Mat houghLinesImage;
        cv::Mat filterdHoughLinesImage;
};