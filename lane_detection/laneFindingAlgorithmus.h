#include <opencv2/opencv.hpp>
#include "image_preprocessing/ImagePreProcessing.h"


struct LaneLines {
    float posSlopeMean = -1.0f; // Initialize with neg values for firsttime check
    double xInterceptPosMean = -1.0;
    float negSlopeMean = -1.0f;
    double xInterceptNegMean = -1.0;
};

struct InterceptAndSlope {
    double xIntercept;
    double slope;
};

enum LaneDetectionResult {
    NO_LINE = 0,
    ONE_LINE = 1,
    TWO_LINES = 2
};



class StrassenFinder
{
    public:
        bool find_lanes(cv::Mat inputImage);
        //Before calling this functions find_lanes has to be called
        cv::Mat get_mask();
        cv::Mat get_maskedImage();
        cv::Mat get_edgesImage();
        cv::Mat get_houghLinesImage();
        cv::Mat get_laneLineImage();
        cv::Mat get_filteredHoughLinesImage();

        double get_distance_to_street();
        double get_steering_angle();


    private:
        ImagePreProcessing imagePreProcessor;

        bool houghLines(cv::Mat maskedImage, cv::Mat originalImage, std::vector<cv::Vec4i>& lines);
        LaneDetectionResult averageLaneLines(std::vector<cv::Vec4i> lines, cv::Mat originalImage);
        int calculate_steering_dir(LaneLines laneLines, cv::Mat originalImage);

        double median(std::vector<double> vec);

        cv::Mat mask;
        cv::Mat maskedImage;
        cv::Mat edgesImage;
        cv::Mat laneLineImage;
        cv::Mat houghLinesImage;
        cv::Mat filterdHoughLinesImage;

        int distance_to_street;
        double steering_angle;

};