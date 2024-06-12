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
};