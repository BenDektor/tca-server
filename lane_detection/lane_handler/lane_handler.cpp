#include "lane_handler.h"
#include <iostream>


// Function to calculate average LineProperties from a vector of LineProperties
LineProperties LaneHandler::calculateAverageLineProperties(const std::vector<LineProperties>& lines) {
    // Initialize accumulators for average calculation
    float avgAngle = 0.0f;
    float avgXIntercept = 0.0f;
    cv::Point avgStartPoint(0, 0);
    cv::Point avgEndPoint(0, 0);

    // Calculate sums
    for (const auto& line : lines) {
        avgAngle += line.angle;
        avgXIntercept += line.xIntercept;
        avgStartPoint += line.startPoint;
        avgEndPoint += line.endPoint;
    }

    // Calculate averages
    avgAngle /= lines.size();
    avgXIntercept /= lines.size();
    avgStartPoint.x /= lines.size();
    avgStartPoint.y /= lines.size();
    avgEndPoint.x /= lines.size();
    avgEndPoint.y /= lines.size();

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
    }
    else if (avgLineGroup1.angle > 0 && avgLineGroup2.angle > 0)
    {
        //Two right lanes detected (from bottom right to top left)
        std::cout << "Two right lanes" << std::endl;

    }
    else if (avgLineGroup1.angle < 0 && avgLineGroup2.angle > 0)
    {
        //group1 left lane, group2 right lane
        std::cout << "One left one right" << std::endl;

    }
    else if (avgLineGroup1.angle > 0 && avgLineGroup2.angle < 0)
    {
        //group1 right lane, group2 left lane
        std::cout << "One right one left" << std::endl;

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
    cv::waitKey(0);


    return carPosition;
}



int main () {
    LaneHandler laneHandler;


    cv::Mat image = cv::imread("../images/finder_image2.jpeg");
    CarPosition pos = laneHandler.getCarPosition(image);
}




