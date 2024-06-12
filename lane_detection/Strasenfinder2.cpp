#include "Strasenfinder2.h"

cv::Mat StrasenFinder2::preProcessImage(cv::Mat inputImage) {

    cv::Mat grayImage = imagePreProcessor.convert_to_gray(inputImage);
    blurredImage = imagePreProcessor.gaussianBlur(grayImage);
    edgesImage = imagePreProcessor.canny_edge_detection(grayImage);
    mask = imagePreProcessor.create_bigger_mask(inputImage.size());
    maskedImage = imagePreProcessor.apply_mask_to_image(edgesImage, mask);

    return maskedImage;
}





bool StrasenFinder2::houghLines(cv::Mat maskedImage, cv::Mat originalImage, std::vector<cv::Vec4i>& lines)
{
    bool success;

    lines.clear();

    float rho = 2;
    float pi = 3.14159265358979323846;
    float theta = pi/180;
    float threshold = 45;
    //int minLineLength = 40;
    int minLineLength = 100;
    int maxLineGap = 10;


    cv::HoughLinesP(maskedImage, lines, rho, theta, threshold, minLineLength, maxLineGap);

    std::cout << "HoughLines Total: " << lines.size() << std::endl;

    // Check if we got more than one line
    if (!lines.empty()) {

        // Initialize lines image
        //cv::Mat allLinesIm(maskedImage.size().height, maskedImage.size().width, CV_8UC3, cv::Scalar(0,0,0)); // CV_8UC3 to make it a 3 channel)
        houghLinesImage = originalImage.clone();
        // Loop through lines
        // std::size_t can store the maximum size of a theoretically possible object of any type
        for (size_t i = 0; i != lines.size(); ++i) {

            // Draw line onto image
            cv::line(houghLinesImage, cv::Point((lines)[i][0], (lines)[i][1]),
                        cv::Point((lines)[i][2], (lines)[i][3]), cv::Scalar(0,0,255), 2, 8 );
        }
        return success = true;
    }
    return success = false;   
}



CarPosition StrasenFinder2::determineCarPos(cv::Mat inputImage) {

    cv::Mat preProccessedImage = preProcessImage(inputImage);

    std::vector<cv::Vec4i> houghlines;
    //Find all lines on image
    if(!houghLines(maskedImage, inputImage, houghlines))
    {
        return CarPosition::NO_STREET;
    }
    else
    {
        std::cout << "HoughLines returned success" <<std::endl;
    }
}




int main(){
    
    cv::Mat image = cv::imread("../images/finder_image6.jpeg");

    StrasenFinder2 strassenFinder;
    

    int pos = strassenFinder.determineCarPos(image);

    cv::Mat blurredImage = strassenFinder.blurredImage;
    cv::Mat houghImage = strassenFinder.houghLinesImage;
    cv::Mat edgesImage = strassenFinder.edgesImage;


    cv::imshow("hough", houghImage);
    cv::imshow("edgesImage", edgesImage);
    cv::imshow("blurr", blurredImage);

    cv::waitKey(0); // wait for a key press    
    return 0;
}