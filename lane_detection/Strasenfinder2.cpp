#include "Strasenfinder2.h"

cv::Mat StrasenFinder2::preProcessImage(cv::Mat inputImage) {

    cv::Mat grayImage = imagePreProcessor.convert_to_gray(inputImage);
    blurredImage = imagePreProcessor.gaussianBlur(grayImage);
    edgesImage = imagePreProcessor.canny_edge_detection(grayImage);
    mask = imagePreProcessor.create_bigger_mask(inputImage.size());
    maskedImage = imagePreProcessor.apply_mask_to_image(edgesImage, mask);

    return maskedImage;
}



bool StrasenFinder2::houghLinesAlgo(cv::Mat maskedImage, cv::Mat inputImage, std::vector<cv::Vec4i>& lines)
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


float StrasenFinder2::calculateAverageXIntercept(const std::vector<LineProperties>& group) {
        float sum = 0.0f;
        for (const auto& line : group) {
            sum += line.xIntercept;
        }
        return sum / group.size();
}


// group lines, where the xintercept of a line in a group is max 15% percent of the average xintercept in that group
std::vector<std::vector<LineProperties>> StrasenFinder2::groupLines(const std::vector<LineProperties>& lines) {
    std::vector<std::vector<LineProperties>> groups;

    for (const auto& line : lines) {
        bool added = false;
        for (auto& group : groups) {
            float avg_x_intercept = calculateAverageXIntercept(group);
            if (std::abs(line.xIntercept - avg_x_intercept) <= 0.15 * avg_x_intercept) {
                group.push_back(line);
                added = true;
                break;
            }
        }
        if (!added) {
            groups.push_back({line});
        }
    }

    // Print out the generated groups
    /*for (size_t i = 0; i < groups.size(); ++i) {
        std::cout << "Group " << i + 1 << ":" << std::endl;
        for (const auto& line : groups[i]) {
            std::cout << "Angle: " << line.angle << ", X Intercept: " << line.xIntercept << std::endl;
        }
    }*/
    return groups;
}



StreetLaneResult StrasenFinder2::find_streetLanes(cv::Mat inputImage) {
    cv::Mat preProcessedImg = preProcessImage(inputImage);

    std::vector<cv::Vec4i> houghLines;
    if (!houghLinesAlgo(preProcessedImg, inputImage, houghLines)) 
    {
        return {StreetLaneStatus::NO_LANE, {}};
    } 
    else 
    {
        std::cout << "HoughLines returned success" << std::endl;

        std::vector<LineProperties> lineProperties = calculateLineProperties(inputImage, houghLines);
        if (lineProperties.size() < 1) {
            return {StreetLaneStatus::NO_LANE, {}};
        }

        // Group lines based on x-intercepts
        std::vector<std::vector<LineProperties>> groups = groupLines(lineProperties);

        if (groups.size() < 1) 
        {
            return {StreetLaneStatus::NO_LANE, {}};
        } 
        else if (groups.size() == 1) 
        {
            return {StreetLaneStatus::ONE_LANE, groups};
        } 
        else 
        {
            // Find the two largest groups
            std::vector<LineProperties> largestGroup1;
            std::vector<LineProperties> largestGroup2;
            float avgIntercept1 = 0.0f, avgIntercept2 = 0.0f;
            float minAvgDistToCenter = std::numeric_limits<float>::max();
            float imageWidthCenter = inputImage.cols / 2.0f;

            for (const auto& group : groups) {
                float avgIntercept = calculateAverageXIntercept(group);
                float avgDistToCenter = std::abs(avgIntercept - imageWidthCenter);

                if (group.size() > largestGroup1.size()) {
                    largestGroup2 = largestGroup1;
                    avgIntercept2 = avgIntercept1;

                    largestGroup1 = group;
                    avgIntercept1 = avgIntercept;
                    minAvgDistToCenter = avgDistToCenter;
                } else if (group.size() > largestGroup2.size()) {
                    largestGroup2 = group;
                    avgIntercept2 = avgIntercept;
                } else if (group.size() == largestGroup1.size() && avgDistToCenter < minAvgDistToCenter) {
                    largestGroup1 = group;
                    avgIntercept1 = avgIntercept;
                    minAvgDistToCenter = avgDistToCenter;
                } else if (group.size() == largestGroup2.size() && avgDistToCenter < minAvgDistToCenter) {
                    largestGroup2 = group;
                    avgIntercept2 = avgIntercept;
                }
            }

            // Check if only one group was found, return it
            if (largestGroup2.empty()) {
                return {StreetLaneStatus::ONE_LANE, {largestGroup1}};
            } else {
                // Check distance between average x-intercepts of the two largest groups
                float distanceBetweenGroups = std::abs(avgIntercept1 - avgIntercept2);
                float imageWidthThird = inputImage.cols / 3.0f;

                if (distanceBetweenGroups >= imageWidthThird) {
                    return {StreetLaneStatus::TWO_LANES, {largestGroup1, largestGroup2}};
                } else {
                    return {StreetLaneStatus::ONE_LANE, {largestGroup1}};
                }
            }
        }
    }
}







int main(){
    
    cv::Mat image = cv::imread("../images/size8.jpg");

    StrasenFinder2 strassenFinder;
    

    StreetLaneResult result = strassenFinder.find_streetLanes(image);


    // Print street lane status
    switch (result.status) {
        case StreetLaneStatus::NO_LANE:
            std::cout << "No street lanes detected." << std::endl;
            break;
        case StreetLaneStatus::ONE_LANE:
            std::cout << "One street lane detected." << std::endl;
            break;
        case StreetLaneStatus::TWO_LANES:
            std::cout << "Two street lanes detected." << std::endl;
            break;
    }

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


    cv::Mat blurredImage = strassenFinder.blurredImage;
    cv::Mat houghImage = strassenFinder.houghLinesImage;
    cv::Mat edgesImage = strassenFinder.edgesImage;


    cv::imshow("hough", houghImage);
    //cv::imshow("edgesImage", edgesImage);
    //cv::imshow("blurr", blurredImage);

    cv::waitKey(0); // wait for a key press    
    return 0;
}