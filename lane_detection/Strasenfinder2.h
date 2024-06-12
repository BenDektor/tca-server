#include <opencv2/opencv.hpp>
#include <cmath>
#include <algorithm>
#include <numeric>
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

enum CarPosition {
    MID_OF_STREET = 0, //Two lanelines with different angles
    SIDE_OF_STREET = 1, //Two lanelines with same angle
    NOT_ON_STREET = 2, //Two lanelines with same angle but further away or not on the angle of the car
    NO_STREET = 3 //No lanelines found
};



class StrasenFinder2
{
    public:
        CarPosition determineCarPos(cv::Mat inputImage);    

        cv::Mat blurredImage;
        cv::Mat mask;
        cv::Mat maskedImage;
        cv::Mat edgesImage;
        cv::Mat houghLinesImage;

    private:
        ImagePreProcessing imagePreProcessor;

        cv::Mat preProcessImage(cv::Mat inputImage);

        bool houghLines(cv::Mat maskedImage, cv::Mat originalImage, std::vector<cv::Vec4i>& lines);
        //LaneDetectionResult averageLaneLines(std::vector<cv::Vec4i> lines, cv::Mat originalImage);
        //int calculate_steering_dir(LaneLines laneLines, cv::Mat originalImage);

        double median(std::vector<double> vec);

        

};