#include "laneFindingAlgorithmus.h"

#include <cmath>
#include <algorithm>
#include <numeric>


// Function for finding median
double StrassenFinder::median(std::vector<double> vec) {

	// get size of vector
	int vecSize = vec.size();

	// if vector is empty throw error
	if (vecSize == 0) {
		throw std::domain_error("median of empty vector");
	}

	// sort vector
	sort(vec.begin(), vec.end());

	// define middle and median
	int middle;
	double median;

		// if even number of elements in vec, take average of two middle values
	if (vecSize % 2 == 0) {
		// a value representing the middle of the array. If array is of size 4 this is 2
		// if it's 8 then middle is 4
		middle = vecSize/2;

		// take average of middle values, so if vector is [1, 2, 3, 4] we want average of 2 and 3
		// since we index at 0 middle will be the higher one vec[2] in the above vector is 3, and vec[1] is 2
		median = (vec[middle-1] + vec[middle]) / 2;
	}

	// odd number of values in the vector
	else {
		middle = vecSize/2; // take the middle again

		// if vector is 1 2 3 4 5, middle will be 5/2 = 2, and vec[2] = 3, the middle value
		median = vec[middle];
	}

	return median;
}


cv::Mat StrassenFinder::get_mask()
{
    return mask;
}
cv::Mat StrassenFinder::get_maskedImage()
{
    return maskedImage;
}
cv::Mat StrassenFinder::get_edgesImage()
{
    return edgesImage;
}
cv::Mat StrassenFinder::get_houghLinesImage()
{
    return houghLinesImage;
}
cv::Mat StrassenFinder::get_laneLineImage()
{
    return laneLineImage;
}
cv::Mat StrassenFinder::get_filteredHoughLinesImage()
{
    return filterdHoughLinesImage;
}

bool operator==(const InterceptAndSlope& lhs, const InterceptAndSlope& rhs) {
    return lhs.xIntercept == rhs.xIntercept && lhs.slope == rhs.slope;
}


// Implement the methods to return the angle and distance
double StrassenFinder::get_distance_to_street() {
    return distance_to_street;
}

double StrassenFinder::get_steering_angle() {
    return steering_angle;
}


double calculateMean(const std::vector<double>& values) {
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

double calculateMean(const std::vector<InterceptAndSlope>& values) {
    double sum = 0.0;
    for (const auto& value : values) {
        sum += value.xIntercept;
    }
    return sum / values.size();
}

double calculateMeanSlope(const std::vector<InterceptAndSlope>& values) {
    double sum = 0.0;
    for (const auto& value : values) {
        sum += value.slope;
    }
    return sum / values.size();
}


std::pair<std::vector<InterceptAndSlope>, std::vector<InterceptAndSlope>> kMeansClustering(const std::vector<InterceptAndSlope>& interceptsAndSlopes, int maxIterations = 100) {
    // Initialize centroids
    double centroid1 = interceptsAndSlopes[0].xIntercept;
    double centroid2 = interceptsAndSlopes[interceptsAndSlopes.size() / 2].xIntercept;

    std::vector<InterceptAndSlope> cluster1, cluster2;
    std::vector<InterceptAndSlope> previousCluster1, previousCluster2;

    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        cluster1.clear();
        cluster2.clear();

        // Assign each x-intercept to the nearest centroid
        for (const auto& interceptAndSlope : interceptsAndSlopes) {
            if (std::abs(interceptAndSlope.xIntercept - centroid1) < std::abs(interceptAndSlope.xIntercept - centroid2)) {
                cluster1.push_back(interceptAndSlope);
            } else {
                cluster2.push_back(interceptAndSlope);
            }
        }

        // Check for convergence
        if (cluster1 == previousCluster1 && cluster2 == previousCluster2) {
            break;
        }

        previousCluster1 = cluster1;
        previousCluster2 = cluster2;

        // Update centroids
        if (!cluster1.empty()) {
            centroid1 = calculateMean(cluster1);
        }
        if (!cluster2.empty()) {
            centroid2 = calculateMean(cluster2);
        }
    }

    return {cluster1, cluster2};
}


std::vector<InterceptAndSlope> calculateXIntercepts(const std::vector<std::pair<cv::Vec4i, float>>& filteredLines, const cv::Mat& originalImage) {
    std::vector<InterceptAndSlope> interceptsAndSlopes;
    for (const auto& lineWithSlope : filteredLines) {
        const auto& line = lineWithSlope.first;
        double slope = lineWithSlope.second;

        double x1 = line[0];
        double y1 = originalImage.rows - line[1]; // y axis is flipped
        double yIntercept = y1 - slope * x1; // y-intercept of line
        double xIntercept = -yIntercept / slope; // find x-intercept based on y = mx + b

        if (!std::isnan(xIntercept)) {
            interceptsAndSlopes.push_back({xIntercept, slope});
        }
    }
    return interceptsAndSlopes;
}




void drawAverageLine(cv::Mat& image, double xIntercept, double slope, cv::Scalar color) {
    int y1 = image.rows;
    int y2 = image.rows / 4;

    // Calculate the corresponding x coordinates for the y coordinates
    int x1 = static_cast<int>(xIntercept);
    int x2 = static_cast<int>(xIntercept + (y1 - y2) / slope);

    std::cout << "Line: " << "(" << x1 << " " << y1 << ") (" << x2 << " " <<y2 << ")"<<std::endl;

    cv::line(image, cv::Point(x1, y1), cv::Point(x2, y2), color, 2);
}


// Function to add the angle label to the image
void addAngleLabel(cv::Mat& image, double angle) {
    // Convert the angle to a string
    std::string angleText = "Angle: " + std::to_string(angle) + " degrees";
    
    // Determine the position for the label based on the angle
    int posX;
    if (angle >= 0) {
        posX = image.cols / 4;
    } else {
        posX = (image.cols / 4) * 3;
    }
    int posY = (image.rows / 4) * 3;

    // Define font properties
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 0.7;
    int thickness = 2;
    cv::Scalar color = cv::Scalar(255, 255, 0);

    // Add the text to the image
    cv::putText(image, angleText, cv::Point(posX, posY), fontFace, fontScale, color, thickness);
}





bool StrassenFinder::houghLines(cv::Mat maskedImage, cv::Mat originalImage, std::vector<cv::Vec4i>& lines)
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



int StrassenFinder::calculate_steering_dir(LaneLines laneLines, cv::Mat originalImage)
{
        // Positive Slope Line
        float slope = laneLines.posSlopeMean;
        double x1 = laneLines.xInterceptPosMean;
        int y1 = 0;
        double y2 = originalImage.size().height - (originalImage.size().height - originalImage.size().height*.35);
        double x2 = (y2-y1) / slope + x1;

        x2 = int(x2 + .5);
        y2 = int(y2 + .5);

        // Negative Slope Line
        slope = laneLines.negSlopeMean;
        double x1N = laneLines.xInterceptNegMean;
        int y1N = 0;
        double x2N = (y2-y1N) / slope + x1N;

        x2N = int(x2N + .5);

        int curr_steering_dir = (x2 + x2N) / 2;
        int needed_steering_dir = curr_steering_dir - (originalImage.size().width / 2);
        return needed_steering_dir;
}


LaneDetectionResult StrassenFinder::averageLaneLines(std::vector<cv::Vec4i> lines, cv::Mat originalImage) {
    float totalAngle = 0;
    int left_counter = 0;
    int right_counter = 0;
    int counter = 0;
    std::vector<std::pair<cv::Vec4i, float>> linesWithSlopes;

    // Loop through all lines
    for (size_t i = 0; i < lines.size(); ++i) {
        // Get points for current line
        float x1 = lines[i][0];
        float y1 = lines[i][1];
        float x2 = lines[i][2];
        float y2 = lines[i][3];

        if (x2 - x1 != 0) {
            // Calculate the slope of the current line
            float slope = -(y2 - y1) / (x2 - x1);
            float tanTheta = (fabs(y2 - y1) / fabs(x2 - x1)); // tan(theta) value
            float angle = atan(tanTheta) * 180 / CV_PI;

            // Add current slope to total slope
            totalAngle += angle;
            if (slope > 0) {
                left_counter++;
            } else {
                right_counter++;
            }
            counter++;
            linesWithSlopes.push_back({lines[i], slope});
        }
    }

    if (counter == 0) {
        // No lines with valid slopes found
        return LaneDetectionResult::NO_LINE;
    }

    // Calculate the average angle
    float averageAngle = totalAngle / counter;
    std::cout << "Total average Angle: " << averageAngle << std::endl;

    // Filter out lines with the opposite direction than the average
    std::vector<std::pair<cv::Vec4i, float>> filteredLines;
    for (const auto& lineWithSlope : linesWithSlopes) {
        const auto& line = lineWithSlope.first;
        float slope = lineWithSlope.second;

        // Get points for current line
        float x1 = line[0];
        float y1 = line[1];
        float x2 = line[2];
        float y2 = line[3];

        if (x2 - x1 != 0) {
            // Calculate the angle of the current line
            float tanTheta = (fabs(y2 - y1) / fabs(x2 - x1)); // tan(theta) value
            float angle = atan(tanTheta) * 180 / CV_PI;

            if (left_counter >= right_counter) {
                if (slope > 0 && angle <= 12 + averageAngle && angle >= averageAngle - 12) {
                    filteredLines.push_back(lineWithSlope);
                }
            } else {
                if (slope < 0 && angle <= 12 + averageAngle && angle >= averageAngle - 12) {
                    filteredLines.push_back(lineWithSlope);
                }
            }
        }
    }

    // Calculate x-intercepts for the filtered lines
    std::vector<InterceptAndSlope> interceptsAndSlopes = calculateXIntercepts(filteredLines, originalImage);

    // Perform K-means clustering on x-intercepts
    auto clusters = kMeansClustering(interceptsAndSlopes, 100); // 100 as max iterations

    std::vector<InterceptAndSlope> cluster1 = clusters.first;
    std::vector<InterceptAndSlope> cluster2 = clusters.second;

    double meanXInterceptCluster1 = calculateMean(cluster1);
    double meanXInterceptCluster2 = calculateMean(cluster2);

    double meanSlopeCluster1 = calculateMeanSlope(cluster1);
    double meanSlopeCluster2 = calculateMeanSlope(cluster2);

    std::cout << "Cluster 1: ";
    for (const auto& x : cluster1) {
        std::cout << x.xIntercept << " ";
    }
    std::cout << "\nMean of Cluster 1: " << meanXInterceptCluster1 << std::endl;
    std::cout << "Mean slope of Cluster 1: " << meanSlopeCluster1 << std::endl;

    std::cout << "Cluster 2: ";
    for (const auto& x : cluster2) {
        std::cout << x.xIntercept << " ";
    }
    std::cout << "\nMean of Cluster 2: " << meanXInterceptCluster2 << std::endl;
    std::cout << "Mean slope of Cluster 2: " << meanSlopeCluster2 << std::endl;

    int xInterceptMeanDifference =  std::abs(meanXInterceptCluster1 - meanXInterceptCluster2);
    std::cout << "Difference between means: " << xInterceptMeanDifference << std::endl;


    // Draw filtered lines onto the image
    for (size_t i = 0; i != filteredLines.size(); ++i) {
        // Draw line onto image
        const auto& line = filteredLines[i].first;
        cv::line(originalImage, cv::Point(line[0], line[1]),
                 cv::Point(line[2], line[3]), cv::Scalar(255, 0, 0), 2, 8);
    }


     // Create a copy of the original image to draw the average lines
    cv::Mat imageWithAverageLines = originalImage.clone();

    // Draw the average lines for each cluster
    drawAverageLine(imageWithAverageLines, meanXInterceptCluster1, meanSlopeCluster1, cv::Scalar(0, 255, 0)); // Green for cluster 1
    drawAverageLine(imageWithAverageLines, meanXInterceptCluster2, meanSlopeCluster2, cv::Scalar(0, 0, 255)); // Red for cluster 2

     // Calculate the midpoint for the average line
    double meanMidXIntercept = (meanXInterceptCluster1 + meanXInterceptCluster2) / 2;
    double meanMidSlope = (meanSlopeCluster1 + meanSlopeCluster2) / 2;
    // Calculate the angle of the mid mean line with respect to the horizontal axis
    double meanMidAngle = atan(meanMidSlope) * 180 / CV_PI;

    // Adjust the angle to be with respect to the vertical axis
    double meanMidAngleFromVertical;
    if (meanMidAngle < 0) {
        meanMidAngleFromVertical = -(90 + meanMidAngle);
    } else {
        meanMidAngleFromVertical = 90 - meanMidAngle;
    }
    std::cout << "Angle of the mid mean line from the vertical: " << meanMidAngleFromVertical << " pixels" << std::endl;


    // Draw the average line between the two clusters
    drawAverageLine(imageWithAverageLines, meanMidXIntercept, meanMidSlope, cv::Scalar(255, 255, 0)); // Yellow for the middle line
    addAngleLabel(imageWithAverageLines, meanMidAngleFromVertical);

    // Calculate the intersection point with the middle width of the image
    int midX = imageWithAverageLines.cols / 2;
    double meanMidYIntercept = -meanMidSlope * meanMidXIntercept;
    int midY = static_cast<int>(meanMidSlope * midX + meanMidYIntercept);


    // Draw a line from the bottom middle to the intersection point
    int bottomMiddleX = imageWithAverageLines.cols / 2;
    int bottomMiddleY = imageWithAverageLines.rows;
    cv::line(imageWithAverageLines, cv::Point(bottomMiddleX, bottomMiddleY), cv::Point(midX, imageWithAverageLines.rows - midY), cv::Scalar(0, 255, 255), 2); // Cyan for the connecting line
    std::string distanceText = "Street Distance: " + std::to_string(midY) + " pixels";
    cv::putText(imageWithAverageLines, distanceText, cv::Point(bottomMiddleX+10, bottomMiddleY-30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);

    distance_to_street = midY;

    steering_angle = meanMidAngleFromVertical; //if pos -> steering to the right ... neg -> left

    // Save or display the image with average lines
    cv::imshow("image_with_average_lines", imageWithAverageLines); // Or use cv::imshow to display

    if(xInterceptMeanDifference < 300){
        std::cout << "Warning: Probably found only one LaneLine" << std::endl;
        return LaneDetectionResult::ONE_LINE;
    }
    return LaneDetectionResult::TWO_LINES;
}



bool StrassenFinder::find_lanes(cv::Mat inputImage)
{
    cv::Mat grayImage = imagePreProcessor.convert_to_gray(inputImage);
    //cv::imshow("Gray Image", grayImage);


    edgesImage = imagePreProcessor.canny_edge_detection(grayImage);

    mask = imagePreProcessor.create_bigger_mask(inputImage.size());
    //cv::imshow("Blurred Image", blurredImage);

    maskedImage = imagePreProcessor.apply_mask_to_image(edgesImage, mask);

    std::vector<cv::Vec4i> houghlines;
    if(houghLines(maskedImage, inputImage, houghlines))
    {
        std::cout << "HoughLines returned success" <<std::endl;
    }
    else
    {
        return false;
    }

    if(averageLaneLines(houghlines, inputImage))
    {
        std::cout << "Car distance to street: " << distance_to_street << std::endl;
        std::cout << "Angle of the mid mean line from the vertical: " << steering_angle << " degrees" << std::endl;
        return true;
    }
    else
    {
        return false;
    }
    
    return true;
}


int main(){
    
    cv::Mat image = cv::imread("../finder_image2.jpeg");

    StrassenFinder strassenFinder;
    

    bool success;
    int steering_dir;
    success = strassenFinder.find_lanes(image);

    if(success){
        std::cout << "Steering Direction: " << steering_dir << std::endl;

        cv::Mat edges = strassenFinder.get_edgesImage();
        cv::Mat mask = strassenFinder.get_mask();
        cv::Mat houghImage = strassenFinder.get_houghLinesImage();
        cv::Mat filteredHoughLinesImage = strassenFinder.get_filteredHoughLinesImage();
        cv::Mat laneLineImage = strassenFinder.get_laneLineImage();

        //cv::imshow("edges image", edges);
        //cv::imshow("filteredHoughLineImage", filteredHoughLinesImage);
        //cv::imshow("hough", houghImage);
        //cv::imshow("laneLineImage", laneLineImage);
        //cv::imshow("mask", mask);

        cv::waitKey(0); // wait for a key press    

    }
    return 0;
}
