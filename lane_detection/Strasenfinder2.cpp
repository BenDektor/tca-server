#include "Strasenfinder2.h"

cv::Mat StrasenFinder2::preProcessImage(cv::Mat inputImage) {

    cv::Mat grayImage = imagePreProcessor.convert_to_gray(inputImage);
    blurredImage = imagePreProcessor.gaussianBlur(grayImage);
    edgesImage = imagePreProcessor.canny_edge_detection(grayImage);
    mask = imagePreProcessor.create_bigger_mask(inputImage.size());
    maskedImage = imagePreProcessor.apply_mask_to_image(edgesImage, mask);

    return maskedImage;
}



bool StrasenFinder2::houghLines(cv::Mat maskedImage, cv::Mat inputImage, std::vector<cv::Vec4i>& lines)
{
    bool success;

    lines.clear();

    float rho = 1;
    float theta = CV_PI /180;
    float threshold = 75;
    int minLineLength = 200;
    int maxLineGap = 70;
    cv::HoughLinesP(maskedImage, lines, rho, theta, threshold, minLineLength, maxLineGap);
    std::cout << "HoughLines Total: " << lines.size() << std::endl;
    
    // Check if we got more than one line
    if (!lines.empty()) {
        houghLinesImage = inputImage.clone();
        // Loop through lines
        for (size_t i = 0; i != lines.size(); ++i) {

            // Draw line onto image
            cv::line(houghLinesImage, cv::Point((lines)[i][0], (lines)[i][1]),
                        cv::Point((lines)[i][2], (lines)[i][3]), cv::Scalar(0,0,255), 2, 8 );
        }
        return success = true;
    }
    return success = false;   
}

// Calculate angles, x-intercepts, and line coordinates
std::vector<LineProperties> StrasenFinder2::calculateLineProperties(cv::Mat inputImage, std::vector<cv::Vec4i>& lines) {
    std::vector<LineProperties> lineProps;
    for (const auto& line : lines) {
        // Calculate angle of the line
        float angle = std::atan2(line[3] - line[1], line[2] - line[0]);
        // Calculate x-intercept
        if(!tan(angle)){
            continue; //skip for null division (horizontal lines)
        }
        float xIntercept = line[0] - (line[1] - inputImage.rows) / tan(angle);
        // Store line start and end points
        cv::Point startPoint(line[0], line[1]);
        cv::Point endPoint(line[2], line[3]);

        LineProperties lineProperty = {angle, xIntercept, startPoint, endPoint};

         // Print line properties
        std::cout << "Angle: " << lineProperty.angle << " X Intercept: " << lineProperty.xIntercept << " Start Point: " << lineProperty.startPoint << " End Point: " << lineProperty.endPoint << " " << tan(angle)<<std::endl;

        lineProps.push_back(lineProperty);
    }
    return lineProps;
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

        std::vector<LineProperties> lineproperties = calculateLineProperties(inputImage, houghlines);
        
        // Perform k-means clustering
        auto [cluster1, cluster2] = kMeansClustering(lineproperties);
    }
}






int main(){
    
    cv::Mat image = cv::imread("../images/test_image8.jpg");

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