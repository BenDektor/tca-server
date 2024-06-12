#include <opencv2/opencv.hpp>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <vector>
#include "image_preprocessing/ImagePreProcessing.h"


struct LineProperties {
    float angle;
    float xIntercept;
    cv::Point startPoint;
    cv::Point endPoint;
};


struct Cluster {
    std::vector<LineProperties> lines;
    LineProperties centroid;
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

        bool houghLines(cv::Mat maskedImage, cv::Mat inputImage, std::vector<cv::Vec4i>& lines);
        std::vector<LineProperties> calculateLineProperties(cv::Mat inputImage, std::vector<cv::Vec4i>& lines);
        void clusterLines(std::vector<LineProperties>& lineProps);
        double distance(const LineProperties& line1, const LineProperties& line2);
        bool isOutlier(const LineProperties& line1, const LineProperties& line2, double threshold);

        double median(std::vector<double> vec);        

};