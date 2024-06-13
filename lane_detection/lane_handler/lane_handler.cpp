#include "lane_handler.h"
#include <iostream>


// Function to calculate average LineProperties from a vector of LineProperties
LineProperties LaneHandler::calculateAverageLineProperties(const std::vector<LineProperties>& lines) {
    // Initialize accumulators for average calculation
    float avgAngle = 0.0f;
    float avgXIntercept = 0.0f;

    // Calculate sums for angle and xIntercept
    for (const auto& line : lines) {
        avgAngle += line.angle;
        avgXIntercept += line.xIntercept;
    }

    // Calculate averages for angle and xIntercept
    avgAngle /= lines.size();
    avgXIntercept /= lines.size();

    // Calculate the start and end points based on the new logic
    cv::Mat image = strassenFinder.houghLinesImage;

    cv::Point avgStartPoint(static_cast<int>(avgXIntercept), image.rows); // (x = xIntercept, y = image.height)
    int endPointY = image.rows / 5;
    int endPointX = static_cast<int>(avgXIntercept + (endPointY - image.rows) / std::tan(avgAngle)); // Calculate x based on angle

    cv::Point avgEndPoint(endPointX, endPointY);

    // Create and return the average LineProperties
    LineProperties avgLineProperties;
    avgLineProperties.angle = avgAngle;
    avgLineProperties.xIntercept = avgXIntercept;
    avgLineProperties.startPoint = avgStartPoint;
    avgLineProperties.endPoint = avgEndPoint;

    return avgLineProperties;
}

void printLineProperties(const LineProperties& lineProps) {
    std::cout << "Angle: " << lineProps.angle << ", ";
    std::cout << "X Intercept: " << lineProps.xIntercept << ", ";
    std::cout << "Start Point: (" << lineProps.startPoint.x << ", " << lineProps.startPoint.y << "), ";
    std::cout << "End Point: (" << lineProps.endPoint.x << ", " << lineProps.endPoint.y << ")" << std::endl;
}

void drawAverageLine(cv::Mat& image, const LineProperties& line) {
    cv::Point pt1(line.startPoint);
    cv::Point pt2(line.endPoint);
    cv::line(image, pt1, pt2, cv::Scalar(0, 255, 0), 2); // Green color line with thickness 2
}


// Function to check if the car is on the street
bool LaneHandler::checkCarOnStreet(const LineProperties& line1, const LineProperties& line2) {
    int imageWidth = strassenFinder.houghLinesImage.cols;
    // Check if line1 xIntercept is within the image width range
    if (line1.xIntercept >= 0 && line1.xIntercept <= imageWidth) {
        return true;
    }
    // Check if line2 is different from a default-constructed LineProperties and within the image width range
    LineProperties defaultLine;
    if (line2.xIntercept != defaultLine.xIntercept && line2.xIntercept >= 0 && line2.xIntercept <= imageWidth) {
        return true;
    }
    return false;
}

int LaneHandler::calculateSteeringDir(const LineProperties& line1, const LineProperties& line2){
    LineProperties combinedAvgLine = calculateAverageLineProperties({line1, line2});
    
    int offset_to_middle = (strassenFinder.houghLinesImage.cols / 2) - combinedAvgLine.endPoint.x 
    return offset_to_middle;
};


CarPosition LaneHandler::handleTwoLanes(const std::vector<LineProperties>& lineGroup1, const std::vector<LineProperties>& lineGroup2){
    LineProperties avgLineGroup1 = calculateAverageLineProperties(lineGroup1);
    LineProperties avgLineGroup2 = calculateAverageLineProperties(lineGroup2);


    std::cout << "Average Line Properties for Group 1:" << std::endl;
    printLineProperties(avgLineGroup1);

    std::cout << "Average Line Properties for Group 2:" << std::endl;
    printLineProperties(avgLineGroup2);

    

    if(avgLineGroup1.angle < 0 && avgLineGroup2.angle < 0) {
        //Two left lanes detected (from bottom left to top right)
        std::cout << "Two left lanes" << std::endl;
        bool onStreet = checkCarOnStreet(avgLineGroup1, avgLineGroup2);
        if(onStreet){
            steering_dir = calculateSteeringDir(avgLineGroup1, avgLineGroup2);
            return CarPosition::ON_STREET;
        }
        else{
            //distance_to_street = calculateDistanceToStreet(avgLineGroup1, avgLineGroup2);
            //angle_to_street = calculateAngleToStreet(avgLineGroup1, avgLineGroup2);
            return CarPosition::OFF_STREET_TO_LEFT;
        }
        
    }
    else if (avgLineGroup1.angle > 0 && avgLineGroup2.angle > 0)
    {
        //Two right lanes detected (from bottom right to top left)
        std::cout << "Two right lanes" << std::endl;
        std::cout << "Two left lanes" << std::endl;
        bool onStreet = checkCarOnStreet(avgLineGroup1, avgLineGroup2);
        if(onStreet){
            steering_dir = calculateSteeringDir(avgLineGroup1, avgLineGroup2);
            return CarPosition::ON_STREET;
        }
        else{
            //distance_to_street = calculateDistanceToStreet(avgLineGroup1, avgLineGroup2);
            //angle_to_street = calculateAngleToStreet(avgLineGroup1, avgLineGroup2);
            return CarPosition::OFF_STREET_TO_RIGHT;
        }

    }
    else if (avgLineGroup1.angle < 0 && avgLineGroup2.angle > 0 || avgLineGroup1.angle > 0 && avgLineGroup2.angle < 0)
    {
        //group1 left lane, group2 right lane
        std::cout << "One left one right" << std::endl;
        steering_dir = calculateSteeringDir(avgLineGroup1, avgLineGroup2);
        return CarPosition::ON_STREET;

    }

    // Draw average lines on a copy of the original image
    cv::Mat imageCopy = strassenFinder.houghLinesImage.clone(); // Assuming originalImage is accessible
    drawAverageLine(imageCopy, avgLineGroup1);
    drawAverageLine(imageCopy, avgLineGroup2);

    // Display or save the image with drawn lines (optional)
    cv::imshow("Average Lines", imageCopy);

}








CarPosition LaneHandler::getCarPosition(cv::Mat image) {
    StreetLaneResult result = strassenFinder.find_streetLanes(image);


    // Print groups if any were found
    if (!result.groups.empty()) {
        std::cout << "Number of groups found: " << result.groups.size() << std::endl;
        for (size_t i = 0; i < result.groups.size(); ++i) {
            std::cout << "Group " << i + 1 << " size: " << result.groups[i].size() << std::endl;
            for (const auto& line : result.groups[i]) {
                std::cout << "  Angle: " << line.angle << ", X Intercept: " << line.xIntercept << std::endl;
            }
        }
    } else {
        std::cout << "No groups found." << std::endl;
    }

    CarPosition carPosition;
    switch (result.status) {
        case StreetLaneStatus::NO_LANE:
            std::cout << "No street lanes detected." << std::endl;
            carPosition = CarPosition::NO_STREET;
        case StreetLaneStatus::ONE_LANE:
            std::cout << "One street lane detected." << std::endl;
            //TWO Possibilites:
            // ON_STREET -> set steering_dir
            // OFF_STREET -> set distance and angle of street
            //carPosition = handleOneLane(result.groups[0]);
            break; 
        case StreetLaneStatus::TWO_LANES:
            //TWO Possibilites:
            // ON_STREET -> set steering_dir
            // OFF_STREET -> set distance and angle of street
            std::cout << "Two street lanes detected." << std::endl;
            carPosition = handleTwoLanes(result.groups[0], result.groups[1]);
            break;
    }



    // Optionally, you can retrieve and display processed images if needed
    // cv::Mat blurredImage = strassenFinder.blurredImage;
    cv::Mat houghImage = strassenFinder.houghLinesImage;
    // cv::Mat edgesImage = strassenFinder.edgesImage;
    cv::imshow("hough", houghImage);



    return carPosition;
}



int main () {
    LaneHandler laneHandler;


    cv::Mat image = cv::imread("../images/size24.jpg");
    CarPosition pos = laneHandler.getCarPosition(image);

    if(pos == 1){
        std::cout << "Car Position Result: " << "OnStreet" << std::endl;
    }
    else if (pos == 2){
        std::cout << "Car Position Result: " << "OffStreet Street to the left" << std::endl;
    }
    else if (pos == 3){
        std::cout << "Car Position Result: " << "OffStreet Street to the right" << std::endl;
    }
    else {
        std::cout << "Car Position Result: " << "NoStreet" << std::endl;
    }
    cv::waitKey(0);

}




