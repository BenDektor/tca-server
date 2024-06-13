#include "lane_handler.h"
#include <iostream>


int LaneHandler::get_steering_dir(){
    if(temp_carPosition != CarPosition::ON_STREET){
        std::cout << "Warning: Steering Dir should only be called when CARPOSITION::ON_STREET" << std::endl;
    }
    return steering_dir;
}
int LaneHandler::get_distance_to_street(){
    if(temp_carPosition != CarPosition::OFF_STREET_TO_RIGHT && temp_carPosition != CarPosition::OFF_STREET_TO_LEFT){
        std::cout << "Warning: Steering Dir should only be called when CARPOSITION::OFF_STREET_TO_LEFT or CARPOSITION::OFF_STREET_TO_RIGHT" << std::endl;
    }
    return distance_to_street;
}
int LaneHandler::get_angle_to_street(){
    if(temp_carPosition != CarPosition::OFF_STREET_TO_RIGHT && temp_carPosition != CarPosition::OFF_STREET_TO_LEFT){
        std::cout << "Warning: Steering Dir should only be called when CARPOSITION::OFF_STREET_TO_LEFT or CARPOSITION::OFF_STREET_TO_RIGHT" << std::endl;
    }
    return angle_to_street;
}


LineProperties LaneHandler::calculateAverageLinePropertiesOfAverages(const std::vector<LineProperties>& lines) {
    if (lines.size() != 2) {
        // Handle cases where there are not exactly two lines to average
        std::cout << "ERROR in LaneHandler" << std::endl;
        return lines.empty() ? LineProperties() : lines[0];
    }

    // Extract the points
    cv::Point line1Start = lines[0].startPoint;
    cv::Point line1End = lines[0].endPoint;
    cv::Point line2Start = lines[1].startPoint;
    cv::Point line2End = lines[1].endPoint;

    // Determine the correct start and end points based on y-values
    std::vector<cv::Point> points = {line1Start, line1End, line2Start, line2End};
    std::sort(points.begin(), points.end(), [](const cv::Point& a, const cv::Point& b) {
        return a.y > b.y;
    });

    // After sorting, the first two points will have the highest y-values (start points)
    // and the last two points will have the lowest y-values (end points)
    cv::Point midStartPoint = (points[0] + points[1]) * 0.5;
    cv::Point midEndPoint = (points[2] + points[3]) * 0.5;

    // Calculate the angle based on the midpoints
    float deltaY = midEndPoint.y - midStartPoint.y;
    float deltaX = midEndPoint.x - midStartPoint.x;
    float avgAngle = std::atan2(deltaY, deltaX); // Angle in radians

    // Calculate x-intercept using the point with the higher y-value and angle
    cv::Mat image = strassenFinder.houghLinesImage;
    float avgXIntercept = midStartPoint.x - (midStartPoint.y - image.rows) / std::tan(avgAngle);

    // Adjust midStartPoint to be at the image height
    midStartPoint.y = image.rows;
    midStartPoint.x = avgXIntercept;

    // Create and return the average LineProperties
    LineProperties avgLineProperties;
    avgLineProperties.angle = avgAngle; // Use radians for internal calculations
    avgLineProperties.xIntercept = avgXIntercept;
    avgLineProperties.startPoint = midStartPoint;
    avgLineProperties.endPoint = midEndPoint;

    return avgLineProperties;
}




LineProperties LaneHandler::calculateAverageLineProperties(const std::vector<LineProperties>& lines) {
    if (lines.size() < 2) {
        // Handle cases where there are not enough lines to average
        return lines.empty() ? LineProperties() : lines[0];
    }

    // Initialize accumulators for start and end points
    cv::Point avgStartPoint(0, 0);
    cv::Point avgEndPoint(0, 0);

    // Calculate sums for start and end points
    for (const auto& line : lines) {
        avgStartPoint += line.startPoint;
        avgEndPoint += line.endPoint;
    }

    // Calculate the actual midpoints
    avgStartPoint.x /= lines.size();
    avgStartPoint.y /= lines.size();
    avgEndPoint.x /= lines.size();
    avgEndPoint.y /= lines.size();

    // Calculate the angle based on the midpoints
    float deltaY = avgEndPoint.y - avgStartPoint.y;
    float deltaX = avgEndPoint.x - avgStartPoint.x;
    float avgAngle = std::atan2(deltaY, deltaX); // Angle in radians

    // Determine which point has the higher and lower y-value
    cv::Mat image = strassenFinder.houghLinesImage;
    cv::Point lowerYPoint, higherYPoint;

    if (avgStartPoint.y < avgEndPoint.y) {
        lowerYPoint = avgStartPoint;
        higherYPoint = avgEndPoint;
    } else {
        lowerYPoint = avgEndPoint;
        higherYPoint = avgStartPoint;
    }

    // Adjust points to fixed y-values
    lowerYPoint.y = image.rows / 5;
    lowerYPoint.x = higherYPoint.x - (higherYPoint.y - lowerYPoint.y) / std::tan(avgAngle);

    higherYPoint.y = image.rows;
    higherYPoint.x = lowerYPoint.x + (image.rows - lowerYPoint.y) / std::tan(avgAngle);

    // Calculate x-intercept using the adjusted points and angle
    float avgXIntercept = higherYPoint.x;

    // Create and return the average LineProperties
    LineProperties avgLineProperties;
    avgLineProperties.angle = avgAngle; // Use radians for internal calculations
    avgLineProperties.xIntercept = avgXIntercept;
    avgLineProperties.startPoint = lowerYPoint;
    avgLineProperties.endPoint = higherYPoint;

    return avgLineProperties;
}


void printLineProperties(const LineProperties& lineProps) {
    std::cout << "Angle: " << lineProps.angle << ", ";
    std::cout << "X Intercept: " << lineProps.xIntercept << ", ";
    std::cout << "Start Point: (" << lineProps.startPoint.x << ", " << lineProps.startPoint.y << "), ";
    std::cout << "End Point: (" << lineProps.endPoint.x << ", " << lineProps.endPoint.y << ")" << std::endl;
}

void LaneHandler::drawLine(const LineProperties& line, const std::string& color, int strength) {
    cv::Point pt1(line.startPoint);
    cv::Point pt2(line.endPoint);

    cv::Scalar lineColor;

    // Determine color based on input string
    if (color == "blue") {
        lineColor = cv::Scalar(255, 0, 0); // Blue color
    } else if (color == "red") {
        lineColor = cv::Scalar(0, 0, 255); // Red color
    } else if (color == "green") {
        lineColor = cv::Scalar(0, 255, 0); // Green color
    } else if (color == "yellow") {
        lineColor = cv::Scalar(0, 255, 255); // Yellow color
    } else {
        // Default to green if color string is unrecognized
        lineColor = cv::Scalar(0, 255, 0); // Green color
    }

    // Draw the line
    cv::line(drawing_image, pt1, pt2, lineColor, strength); // Draw line with specified color and thickness
}


void LaneHandler::drawOffsetLines(int xValue) {
    cv::Point pt1(xValue, drawing_image.rows / 5);
    cv::Point pt2(drawing_image.cols / 2, drawing_image.rows / 5);
    drawLine(LineProperties{0.0f, 0.0f, pt1, pt2}, "green", 2);

    cv::Point pt3(pt1.x, pt1.y + 20);
    cv::Point pt4(pt1.x, pt1.y - 20);
    drawLine(LineProperties{0.0f, 0.0f, pt3, pt4}, "green", 3);
}

void LaneHandler::drawVerticalHelperLine(){
    // Draw vertical line using drawLine function
    cv::Point pt1(drawing_image.cols / 2, drawing_image.rows / 5 + 15);
    cv::Point pt2(drawing_image.cols / 2, drawing_image.rows / 5 - 15);
    drawLine(LineProperties{0.0f, 0.0f, pt1, pt2}, "red");
}

void LaneHandler::drawCarPositionText(CarPosition position) {
    std::string positionText;

    switch (position) {
        case CarPosition::NO_STREET:
            positionText = "Car Position: No Street";
            break;
        case CarPosition::ON_STREET:
            positionText = "Car Position: On Street";
            break;
        case CarPosition::OFF_STREET_TO_LEFT:
            positionText = "Car Position: Off Street (Left)";
            break;
        case CarPosition::OFF_STREET_TO_RIGHT:
            positionText = "Car Position: Off Street (Right)";
            break;
        case CarPosition::UNKNOWN:
            positionText = "Car Position: Unknown";
            break;
        default:
            positionText = "Car Position: Unknown";
            break;
    }

    cv::putText(drawing_image, positionText, cv::Point(drawing_image.cols - 500, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
}




// Function to check if the car is on the street
bool LaneHandler::checkCarOnStreet(const LineProperties& line1, const LineProperties& line2) {
    int imageWidth = strassenFinder.houghLinesImage.cols;
    // Check if line1 xIntercept is within the image width range
    if (line1.xIntercept >= 0 && line1.xIntercept <= imageWidth) {
        return true;
    }
    // Check if line2 is different from a default-constructed LineProperties and within the image width range
    if (line2.xIntercept >= 0 && line2.xIntercept <= imageWidth) {        
        return true;
    }
    return false;
}

bool LaneHandler::checkCarOnStreet(const LineProperties& line) {
    int imageWidth = strassenFinder.houghLinesImage.cols;
    // Check if line xIntercept is within the image width range
    if (line.xIntercept >= 0 && line.xIntercept <= imageWidth) {
        return true;
    }
    return false;
}



int LaneHandler::calculateSteeringDir(const LineProperties& line){
    
    drawVerticalHelperLine();

    int offset_to_middle;

    // Draw offset line (steering line)
    cv::Point pt1;
    if(line.endPoint.y > line.startPoint.y){
        drawOffsetLines(line.startPoint.x);
        offset_to_middle = (strassenFinder.houghLinesImage.cols / 2) - line.startPoint.x;  //if positive value the average lane is to the left -> so steering to left is needed; else negative, average lane is to the right
    }
    else{
        drawOffsetLines(line.endPoint.x);
        offset_to_middle = (strassenFinder.houghLinesImage.cols / 2) - line.endPoint.x;  //if positive value the average lane is to the left -> so steering to left is needed; else negative, average lane is to the right
    }

    // Draw steering label
    std::string steering_text = "SteeringDir: " + std::to_string(offset_to_middle) + " px";
    cv::putText(drawing_image, steering_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);

    return offset_to_middle;
}

std::pair<int, float> LaneHandler::calculateDistanceAndAngleToStreet(const LineProperties &line) {
    printLineProperties(line);

    // Get the middle x-coordinate of the image
    int middleX = strassenFinder.houghLinesImage.cols / 2;

    // Calculate the slope (m) and intercept (b) of the given line
    float slope = std::tan(line.angle);
    float intercept = line.startPoint.y - slope * line.startPoint.x;

    // Calculate the intersection point (x, y) with the vertical middle line
    int intersectionY = slope * middleX + intercept;

    // Draw the middle line from the bottom of the image up to the intersection point
    cv::Point pt1(middleX, strassenFinder.houghLinesImage.rows);
    cv::Point pt2(middleX, intersectionY);
    drawLine(LineProperties{0.0f, 0.0f, pt1, pt2}, "yellow");

    // Calculate the distance from the bottom of the image to the intersection point
    int distanceToStreet = strassenFinder.houghLinesImage.rows - intersectionY;

    // Draw offset label
    std::string distance_text = "Distance: " + std::to_string(distanceToStreet);
    cv::putText(drawing_image, distance_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);

    // Calculate the angle between the given line and the vertical middle line
    float angleToVertical = 90 - std::atan(std::abs(slope)) * 180.0 / CV_PI;

    // Draw angle label
    std::string angle_text = "Angle: " + std::to_string(angleToVertical) + " degrees";
    cv::putText(drawing_image, angle_text, cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);

    return std::make_pair(distanceToStreet, angleToVertical);
}




CarPosition LaneHandler::handleTwoLanes(const std::vector<LineProperties>& lineGroup1, const std::vector<LineProperties>& lineGroup2){
    LineProperties avgLineGroup1 = calculateAverageLineProperties(lineGroup1);
    LineProperties avgLineGroup2 = calculateAverageLineProperties(lineGroup2);
    LineProperties combinedAvgLine = calculateAverageLinePropertiesOfAverages({avgLineGroup1, avgLineGroup2});

    // Draw average lines on a copy of the original image
    drawLine(avgLineGroup1, "green");
    drawLine(avgLineGroup2, "green");
    drawLine(combinedAvgLine, "blue");


    /*std::cout << "Average Line Properties for Group 1:" << std::endl;
    printLineProperties(avgLineGroup1);
    std::cout << "Average Line Properties for Group 2:" << std::endl;
    printLineProperties(avgLineGroup2);*/
    

    if(avgLineGroup1.angle < 0 && avgLineGroup2.angle < 0) {
        //Two left lanes detected (from bottom left to top right)
        std::cout << "Two left lanes" << std::endl;
        bool onStreet = checkCarOnStreet(avgLineGroup1, avgLineGroup2);
        if(onStreet){
            steering_dir = calculateSteeringDir(combinedAvgLine);
            return CarPosition::ON_STREET;
        }
        else{
            std::pair<int, float> result = calculateDistanceAndAngleToStreet(combinedAvgLine);
            distance_to_street = result.first;
            angle_to_street = result.second;
            return CarPosition::OFF_STREET_TO_LEFT;
        }
        
    }
    else if (avgLineGroup1.angle > 0 && avgLineGroup2.angle > 0)
    {
        //Two right lanes detected (from bottom right to top left)
        std::cout << "Two right lanes" << std::endl;
        bool onStreet = checkCarOnStreet(avgLineGroup1, avgLineGroup2);
        if(onStreet){
            steering_dir = calculateSteeringDir(combinedAvgLine);
            return CarPosition::ON_STREET;
        }
        else{
            std::pair<int, float> result = calculateDistanceAndAngleToStreet(combinedAvgLine);
            distance_to_street = result.first;
            angle_to_street = result.second;
            return CarPosition::OFF_STREET_TO_RIGHT;
        }

    }
    else if (avgLineGroup1.angle < 0 && avgLineGroup2.angle > 0 || avgLineGroup1.angle > 0 && avgLineGroup2.angle < 0)
    {
        //group1 left lane, group2 right lane
        std::cout << "One left one right" << std::endl;
        steering_dir = calculateSteeringDir(combinedAvgLine);
        return CarPosition::ON_STREET;

    }

    return CarPosition::UNKNOWN;
}


CarPosition LaneHandler::handleOneLane(const std::vector<LineProperties>& lineGroup){
    LineProperties avgLine = calculateAverageLineProperties(lineGroup);
    
    printLineProperties(avgLine);

    bool onStreet = checkCarOnStreet(avgLine);
    if(onStreet){
        steering_dir = calculateSteeringDir(avgLine);
        std::cout << "One left lane" << std::endl;
        std::cout << "Steering direction: " << steering_dir << std::endl;
        return CarPosition::ON_STREET;
    }
    else{
        std::pair<int, float> result = calculateDistanceAndAngleToStreet(avgLine);
        distance_to_street = result.first;
        angle_to_street = result.second;

        if(avgLine.angle < 0){
            return CarPosition::OFF_STREET_TO_LEFT;
        }
        else if (avgLine.angle > 0){
            return CarPosition::OFF_STREET_TO_RIGHT;
        }   
    }

    return CarPosition::UNKNOWN;
}


CarPosition LaneHandler::getCarPosition(cv::Mat image) {
    StreetLaneResult result = strassenFinder.find_streetLanes(image);

    drawing_image = strassenFinder.houghLinesImage.clone(); // Assuming originalImage is accessible


    // Print groups if any were found
    /*if (!result.groups.empty()) {
        std::cout << "Number of groups found: " << result.groups.size() << std::endl;
        for (size_t i = 0; i < result.groups.size(); ++i) {
            std::cout << "Group " << i + 1 << " size: " << result.groups[i].size() << std::endl;
            for (const auto& line : result.groups[i]) {
                std::cout << "  Angle: " << line.angle << ", X Intercept: " << line.xIntercept;
                std::cout << "  Angle: " << line.startPoint << ", X Intercept: " << line.endPoint << std::endl;
            }
        }
    } else {
        std::cout << "No groups found." << std::endl;
    }*/

    CarPosition carPosition;
    switch (result.status) {
        case StreetLaneStatus::NO_LANE:
            std::cout << "No street lanes detected." << std::endl;
            carPosition = CarPosition::NO_STREET;
        case StreetLaneStatus::ONE_LANE:
            std::cout << "One street lane detected" << std::endl;
            carPosition = handleOneLane(result.groups[0]);
            break; 
        case StreetLaneStatus::TWO_LANES:
            std::cout << "Two street lanes detected." << std::endl;
            carPosition = handleTwoLanes(result.groups[0], result.groups[1]);
            break;
    }

    // Optionally, you can retrieve and display processed images if needed
    // cv::Mat blurredImage = strassenFinder.blurredImage;
    cv::Mat houghImage = strassenFinder.houghLinesImage;
    // cv::Mat edgesImage = strassenFinder.edgesImage;
    cv::imshow("hough", houghImage);

    drawCarPositionText(carPosition);
    cv::imshow("Average Lines", drawing_image);

    temp_carPosition = carPosition;

    return carPosition;
}



int main () {
    LaneHandler laneHandler;


    cv::Mat image = cv::imread("../images/finder_image7.jpeg");
    CarPosition pos = laneHandler.getCarPosition(image);

    if(pos == CarPosition::ON_STREET){
        std::cout << "Car Position Result: " << "OnStreet" << std::endl;
        std::cout << "Steering Direction: " << laneHandler.get_steering_dir() << std::endl;
    }
    else if (pos == CarPosition::OFF_STREET_TO_LEFT){
        std::cout << "Car Position Result: " << "OffStreet Street to the left" << std::endl;
        std::cout << "Distance to Street: " << laneHandler.get_distance_to_street() << std::endl;
        std::cout << "Angle to Street: " << laneHandler.get_angle_to_street() << std::endl;
    }   
    else if (pos == CarPosition::OFF_STREET_TO_RIGHT){
        std::cout << "Car Position Result: " << "OffStreet Street to the right" << std::endl;
        std::cout << "Distance to Street: " << laneHandler.get_distance_to_street() << std::endl;
        std::cout << "Angle to Street: " << laneHandler.get_angle_to_street() << std::endl;
    }
    else if (pos == CarPosition::NO_STREET){
        std::cout << "Car Position Result: " << "NoStreet" << std::endl;
    }
    else {
        std::cout << "Unknwon Car Position" << std::endl;
    }
    cv::waitKey(0);
}




