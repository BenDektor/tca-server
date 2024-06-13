#include "lane_handler.h"
#include <iostream>

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

void LaneHandler::drawLine(const LineProperties& line, const std::string& color) {
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
    } else {
        // Default to green if color string is unrecognized
        lineColor = cv::Scalar(0, 255, 0); // Green color
    }

    // Draw the line
    cv::line(drawing_image, pt1, pt2, lineColor, 2); // Draw line with specified color and thickness 2
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
    LineProperties combinedAvgLine = calculateAverageLinePropertiesOfAverages({line1, line2});

    std::cout << "Line 1:" << std::endl;
    printLineProperties(line1);
    
    std::cout << "Line 2:" << std::endl;
    printLineProperties(line2);
    
    std::cout << "Combined Average Line:" << std::endl;
    printLineProperties(combinedAvgLine);


    drawLine(combinedAvgLine, "blue");

    // Draw offset line (steering line)
    cv::Point pt1(combinedAvgLine.endPoint.x, drawing_image.rows / 5);
    cv::Point pt2(drawing_image.cols / 2, drawing_image.rows / 5);
    drawLine(LineProperties{0.0f, 0.0f, pt1, pt2}, "red");


    
    int offset_to_middle = (strassenFinder.houghLinesImage.cols / 2) - combinedAvgLine.endPoint.x;  //if positive vale the average lane is to the left -> so steering to left is needed; else negative, average lane is to the right
    
    
    // Draw offset label
    std::string offset_text = "Offset: " + std::to_string(offset_to_middle);
    cv::putText(drawing_image, offset_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
    
    
    return offset_to_middle;
};


CarPosition LaneHandler::handleTwoLanes(const std::vector<LineProperties>& lineGroup1, const std::vector<LineProperties>& lineGroup2){
    LineProperties avgLineGroup1 = calculateAverageLineProperties(lineGroup1);
    LineProperties avgLineGroup2 = calculateAverageLineProperties(lineGroup2);

    
    // Draw average lines on a copy of the original image
    drawLine(avgLineGroup1, "green");
    drawLine(avgLineGroup2, "green");

    // Draw vertical line using drawLine function
    cv::Point pt1(drawing_image.cols / 2, drawing_image.rows);
    cv::Point pt2(drawing_image.cols / 2, drawing_image.rows / 5);
    drawLine(LineProperties{0.0f, 0.0f, pt1, pt2}, "red");



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
            std::cout << "Steering direction: " << steering_dir << std::endl;
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
        bool onStreet = checkCarOnStreet(avgLineGroup1, avgLineGroup2);
        if(onStreet){
            steering_dir = calculateSteeringDir(avgLineGroup1, avgLineGroup2);
            std::cout << "Steering direction: " << steering_dir << std::endl;
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
        std::cout << "Steering direction: " << steering_dir << std::endl;
        return CarPosition::ON_STREET;

    }

    // Display or save the image with drawn lines (optional)
}




CarPosition LaneHandler::getCarPosition(cv::Mat image) {
    StreetLaneResult result = strassenFinder.find_streetLanes(image);

    drawing_image = strassenFinder.houghLinesImage.clone(); // Assuming originalImage is accessible


    // Print groups if any were found
    if (!result.groups.empty()) {
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
    }

    CarPosition carPosition;
    switch (result.status) {
        case StreetLaneStatus::NO_LANE:
            std::cout << "No street lanes detected." << std::endl;
            carPosition = CarPosition::NO_STREET;
        case StreetLaneStatus::ONE_LANE:
            std::cout << "One street lane detected. not handled yet" << std::endl;
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

    cv::imshow("Average Lines", drawing_image);

    return carPosition;
}



int main () {
    LaneHandler laneHandler;


    cv::Mat image = cv::imread("../images/size5.jpg");
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




