#include "ImagePreProcessing.h"


cv::Mat ImagePreProcessing::convert_to_gray(cv::Mat image)
{
    cv::Mat grayImage;
    cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    return grayImage;
}

cv::Mat ImagePreProcessing::gaussianBlur(cv::Mat image)
{
    int kernelSize = 11; // bigger kernel = more smoothing
    cv::Mat gaussianBlurImage;
    cv::GaussianBlur(image, gaussianBlurImage, cv::Size(kernelSize, kernelSize), 0, 0);
    return gaussianBlurImage;
}

cv::Mat ImagePreProcessing::canny_edge_detection(cv::Mat image)
{
    // finds gradient in x,y direction, gradient direction is perpendicular to edges
    // Define values for edge detection
    int minVal = 60;
    int maxVal = 150;
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

cv::Mat ImagePreProcessing::gaussianBlur(cv::Mat image, int kernelSize) {
    cv::Mat gaussianBlurImage;
    cv::GaussianBlur(image, gaussianBlurImage, cv::Size(kernelSize, kernelSize), 0, 0);
    return gaussianBlurImage;
}

cv::Mat ImagePreProcessing::canny_edge_detection(cv::Mat image, int minVal, int maxVal) {
    cv::Mat edgesImage;
    cv::Canny(image, edgesImage, minVal, maxVal);
    return edgesImage;
}


//TESTING PORPUSE//

void onTrackbarChange(int, void* userdata) {
    // Retrieve the user data
    std::tuple<ImagePreProcessing*, cv::Mat, int*, int*, int*, int*, int*, int*>* data = static_cast<std::tuple<ImagePreProcessing*, cv::Mat, int*, int*, int*, int*, int*, int*>*>(userdata);
    ImagePreProcessing* processor = std::get<0>(*data);
    cv::Mat image = std::get<1>(*data);
    int* minVal = std::get<2>(*data);
    int* maxVal = std::get<3>(*data);
    int* kernelSize = std::get<4>(*data);
    int* houghThreshold = std::get<5>(*data);
    int* minLineLength = std::get<6>(*data);
    int* maxLineGap = std::get<7>(*data);

    // Ensure kernelSize is a positive odd number
    if (*kernelSize % 2 == 0) {
        (*kernelSize)++;
    }
    if (*kernelSize <= 0) {
        *kernelSize = 1;
    }

    // Default values for HoughLinesP parameters
    double rho = 1;
    double theta = CV_PI / 180;

    // Process the image
    cv::Mat grayImage = processor->convert_to_gray(image);
    cv::Mat blurredImage = processor->gaussianBlur(grayImage, *kernelSize);
    cv::Mat edgesImage = processor->canny_edge_detection(blurredImage, *minVal, *maxVal);
    cv::Mat linesImage = processor->hough_lines(edgesImage, rho, theta, *houghThreshold, *minLineLength, *maxLineGap);

    // Show the result
    cv::imshow("Edges", edgesImage);
    cv::imshow("Lines", linesImage);
}

cv::Mat ImagePreProcessing::hough_lines(cv::Mat edgesImage, double rho, double theta, int threshold, int minLineLength, int maxLineGap) {
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edgesImage, lines, rho, theta, threshold, minLineLength, maxLineGap);
    cv::Mat lineImage = cv::Mat::zeros(edgesImage.size(), CV_8UC3);
    for (const auto& line : lines) {
        cv::line(lineImage, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
    }
    return lineImage;
}


int main() {
    // Read the image
    std::string imagePath = "../size14.jpg"; // Change to your image path
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Error: Could not open image" << std::endl;
        return -1;
    }

    // Initialize the processor
    ImagePreProcessing processor;

    // Parameters for trackbars
    int minVal = 240;
    int maxVal = 255;
    int kernelSize = 5;
    int houghThreshold = 75;
    int minLineLength = 200;
    int maxLineGap = 70;

    // Default values for HoughLinesP parameters
    double rho = 1;
    double theta = CV_PI / 180;

    // Create windows
    cv::namedWindow("Edges", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Lines", cv::WINDOW_AUTOSIZE);

    // Create trackbars
    cv::createTrackbar("Min Val", "Edges", &minVal, 255, onTrackbarChange, new std::tuple<ImagePreProcessing*, cv::Mat, int*, int*, int*, int*, int*, int*>(&processor, image, &minVal, &maxVal, &kernelSize, &houghThreshold, &minLineLength, &maxLineGap));
    cv::createTrackbar("Max Val", "Edges", &maxVal, 255, onTrackbarChange, new std::tuple<ImagePreProcessing*, cv::Mat, int*, int*, int*, int*, int*, int*>(&processor, image, &minVal, &maxVal, &kernelSize, &houghThreshold, &minLineLength, &maxLineGap));
    cv::createTrackbar("Kernel Size", "Edges", &kernelSize, 31, onTrackbarChange, new std::tuple<ImagePreProcessing*, cv::Mat, int*, int*, int*, int*, int*, int*>(&processor, image, &minVal, &maxVal, &kernelSize, &houghThreshold, &minLineLength, &maxLineGap));
    cv::createTrackbar("Threshold", "Lines", &houghThreshold, 200, onTrackbarChange, new std::tuple<ImagePreProcessing*, cv::Mat, int*, int*, int*, int*, int*, int*>(&processor, image, &minVal, &maxVal, &kernelSize, &houghThreshold, &minLineLength, &maxLineGap));
    cv::createTrackbar("Min Line Len", "Lines", &minLineLength, 300, onTrackbarChange, new std::tuple<ImagePreProcessing*, cv::Mat, int*, int*, int*, int*, int*, int*>(&processor, image, &minVal, &maxVal, &kernelSize, &houghThreshold, &minLineLength, &maxLineGap));
    cv::createTrackbar("Max Line Gap", "Lines", &maxLineGap, 300, onTrackbarChange, new std::tuple<ImagePreProcessing*, cv::Mat, int*, int*, int*, int*, int*, int*>(&processor, image, &minVal, &maxVal, &kernelSize, &houghThreshold, &minLineLength, &maxLineGap));

    // Initial call to display the image
    onTrackbarChange(0, new std::tuple<ImagePreProcessing*, cv::Mat, int*, int*, int*, int*, int*, int*>(&processor, image, &minVal, &maxVal, &kernelSize, &houghThreshold, &minLineLength, &maxLineGap));

    // Wait for the user to press a key
    cv::waitKey(0);
    return 0;
}