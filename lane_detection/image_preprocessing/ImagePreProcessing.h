#include <opencv2/opencv.hpp>


class ImagePreProcessing
{
    public:
        cv::Mat convert_to_gray(cv::Mat image);
        cv::Mat gaussianBlur(cv::Mat image);
        cv::Mat canny_edge_detection(cv::Mat image);
        cv::Mat create_mask(cv::Size imageSize);
        cv::Mat create_bigger_mask(cv::Size imageSize);
        cv::Mat apply_mask_to_image(cv::Mat image, cv::Mat mask);


        //TESTING PORPUSE//
        cv::Mat canny_edge_detection(cv::Mat image, int minVal, int maxVal);
        cv::Mat gaussianBlur(cv::Mat image, int kernelSize);
        cv::Mat hough_lines(cv::Mat edgesImage, double rho, double theta, int threshold, double minLineLength, double maxLineGap);
};