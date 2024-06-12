#include <opencv2/opencv.hpp>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <vector>
#include "../image_preprocessing/ImagePreProcessing.h"


struct LineProperties {
    float angle;
    float xIntercept;
    cv::Point startPoint;
    cv::Point endPoint;

    // Define the == operator
    bool operator==(const LineProperties& other) const {
        return angle == other.angle &&
               xIntercept == other.xIntercept &&
               startPoint == other.startPoint &&
               endPoint == other.endPoint;
    }
};



enum StreetLaneStatus {
    NO_LANE = 0, //Two lanelines with different angles
    ONE_LANE = 1, //Two lanelines with same angle
    TWO_LANES = 2, //Two lanelines with same angle but further away or not on the angle of the car
};

struct StreetLaneResult {
    StreetLaneStatus status;
    std::vector<std::vector<LineProperties>> groups;
};



class StrasenFinder2
{
    public:
        StreetLaneResult find_streetLanes(cv::Mat inputImage);    

        cv::Mat blurredImage;
        cv::Mat mask;
        cv::Mat maskedImage;
        cv::Mat edgesImage;
        cv::Mat houghLinesImage;

    private:
        ImagePreProcessing imagePreProcessor;

        cv::Mat preProcessImage(cv::Mat inputImage);

        bool houghLinesAlgo(cv::Mat maskedImage, cv::Mat inputImage, std::vector<cv::Vec4i>& lines);
        std::vector<LineProperties> calculateLineProperties(cv::Mat inputImage, std::vector<cv::Vec4i>& lines);
        double median(std::vector<double> vec);

        std::vector<std::vector<LineProperties>> groupLines(const std::vector<LineProperties>& lines);
        float calculateAverageXIntercept(const std::vector<LineProperties>& group);
        
};