#include "ImagePreProcessing.h"


cv::Mat ImagePreProcessing::convert_to_gray(cv::Mat image)
{
    cv::Mat grayImage;
    cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    return grayImage;
}

cv::Mat ImagePreProcessing::gaussianBlur(cv::Mat image)
{
    int kernelSize = 5; // bigger kernel = more smoothing
    cv::Mat gaussianBlurImage;
    cv::GaussianBlur(image, gaussianBlurImage, cv::Size(kernelSize, kernelSize), 0, 0);
    return gaussianBlurImage;
}

cv::Mat ImagePreProcessing::canny_edge_detection(cv::Mat image)
{
    // finds gradient in x,y direction, gradient direction is perpendicular to edges
    // Define values for edge detection
    int minVal = 240;
    int maxVal = 255;
    cv::Mat edgesImage;

    cv::Canny(image, edgesImage, minVal, maxVal);
    return edgesImage;
}

cv::Mat ImagePreProcessing::create_mask(cv::Size imageSize)
{
    // Create mask to only keep area defined by four corners
    // Black out every area outside area

    // Define masked image
    // Create all black image with same dimensions as original image
    // 3rd arg is CV_<bit-depth>{U|S|F}C(<number_of_channels>), so this is 8bit, unsigned int, channels: 1
    cv::Mat mask = cv::Mat(imageSize, CV_8UC1, cv::Scalar(0)); // CV_8UC3 to make it a 3 channel

    // Define the points for the mask
    // Use cv::Point type for x,y points
    cv::Point p1 = cv::Point(0,imageSize.height); //left bottom edge
    cv::Point p2 = cv::Point(0,imageSize.height -40);
    cv::Point p3 = cv::Point(imageSize.width / 2 - 500, imageSize.height / 3);  //top middle left point
    cv::Point p4 = cv::Point(imageSize.width / 2 + 500, imageSize.height / 3);  //top middle right point
    cv::Point p5 = cv::Point(imageSize.width, imageSize.height -40);
    cv::Point p6 = cv::Point(imageSize.width, imageSize.height); //right bottom edge

    // create vector from array with points we just defined
    cv::Point vertices1[] = {p1,p2,p3,p4,p5,p6};
    std::vector<cv::Point> vertices (vertices1, vertices1 + sizeof(vertices1) / sizeof(cv::Point));

    // Create vector of vectors, add the vertices we defined above
    // (you could add multiple other similar contours to this vector)
    std::vector<std::vector<cv::Point> > verticesToFill;
    verticesToFill.push_back(vertices);

    // Fill in the vertices on the blank image, showing what the mask is
    cv::fillPoly(mask, verticesToFill, cv::Scalar(255,255,255));

    return mask;
}


cv::Mat ImagePreProcessing::create_bigger_mask(cv::Size imageSize)
{
    // Create mask to only keep area defined by four corners
    // Black out every area outside area

    // Define masked image
    // Create all black image with same dimensions as original image
    // 3rd arg is CV_<bit-depth>{U|S|F}C(<number_of_channels>), so this is 8bit, unsigned int, channels: 1
    cv::Mat mask = cv::Mat(imageSize, CV_8UC1, cv::Scalar(0)); // CV_8UC3 to make it a 3 channel

    // Define the points for the mask
    // Use cv::Point type for x,y points
    cv::Point p1 = cv::Point(0, imageSize.height); //left bottom edge
    cv::Point p2 = cv::Point(0, imageSize.height / 4);  //top middle left point
    cv::Point p3 = cv::Point(imageSize.width, imageSize.height /4);  //top middle right point
    cv::Point p4 = cv::Point(imageSize.width, imageSize.height); //right bottom edge

    // create vector from array with points we just defined
    cv::Point vertices1[] = {p1,p2,p3,p4};
    std::vector<cv::Point> vertices (vertices1, vertices1 + sizeof(vertices1) / sizeof(cv::Point));

    // Create vector of vectors, add the vertices we defined above
    // (you could add multiple other similar contours to this vector)
    std::vector<std::vector<cv::Point> > verticesToFill;
    verticesToFill.push_back(vertices);

    // Fill in the vertices on the blank image, showing what the mask is
    cv::fillPoly(mask, verticesToFill, cv::Scalar(255,255,255));

    return mask;
}



cv::Mat ImagePreProcessing::apply_mask_to_image(cv::Mat image, cv::Mat mask)
{
    // create image only where mask and edge Detection image are the same
    // Create masked im, which takes input1, input2, and output. Only keeps where two images overlap
    cv::Mat maskedImage = image.clone();
    cv::bitwise_and(image, mask, maskedImage);
    return maskedImage;
}