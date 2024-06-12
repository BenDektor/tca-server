#include "laneDetection.h"


// Function for finding median
double StrassenErkennung::median(std::vector<double> vec) {

	int vecSize = vec.size();

	if (vecSize == 0) {
		throw std::domain_error("median of empty vector");
	}

	sort(vec.begin(), vec.end());

	int middle;
	double median;

    // if even number of elements in vec, take average of two middle values
	if (vecSize % 2 == 0) {
		middle = vecSize/2;
		median = (vec[middle-1] + vec[middle]) / 2;
	}

	// odd number of values in the vector
	else {
		middle = vecSize/2; 
		median = vec[middle];
	}

	return median;
}


cv::Mat StrassenErkennung::get_mask()
{
    return mask;
}
cv::Mat StrassenErkennung::get_maskedImage()
{
    return maskedImage;
}
cv::Mat StrassenErkennung::get_edgesImage()
{
    return edgesImage;
}
cv::Mat StrassenErkennung::get_houghLinesImage()
{
    return houghLinesImage;
}
cv::Mat StrassenErkennung::get_laneLineImage()
{
    return laneLineImage;
}
cv::Mat StrassenErkennung::get_filteredHoughLinesImage()
{
    return filterdHoughLinesImage;
}



bool StrassenErkennung::houghLines(cv::Mat maskedImage, cv::Mat originalImage, std::vector<cv::Vec4i>& lines, bool drawing_enabled)
{
    bool success;

    lines.clear();

    float rho = 2;
    float pi = 3.14159265358979323846;
    float theta = pi/180;
    float threshold = 45;
    int minLineLength = 60;
    int maxLineGap = 100;


    cv::HoughLinesP(maskedImage, lines, rho, theta, threshold, minLineLength, maxLineGap);

    std::cout << "HoughLines Total: " << lines.size() << std::endl;

    // Check if we got more than one line
    if (!lines.empty() && drawing_enabled) {

        // Initialize lines image
        //cv::Mat allLinesIm(maskedImage.size().height, maskedImage.size().width, CV_8UC3, cv::Scalar(0,0,0)); // CV_8UC3 to make it a 3 channel)
        houghLinesImage = originalImage.clone();

        for (size_t i = 0; i != lines.size(); ++i) {

            // Draw line onto image
            cv::line(houghLinesImage, cv::Point((lines)[i][0], (lines)[i][1]),
                        cv::Point((lines)[i][2], (lines)[i][3]), cv::Scalar(0,0,255), 2, 8 );
        }
        return success = true;
    }
    return success = false;   
}

bool StrassenErkennung::averageLaneLines(std::vector<cv::Vec4i> lines, cv::Mat originalImage, LaneLines& lanelines, bool drawing_enabled)
{
        //---------------Separate Lines Into Positive/Negative Slope--------------------
        // Separate line segments by their slope to decide left line vs. the right line

        // Define arrays for positive/negative lines
        std::vector< std::vector<double> > slopePositiveLines; // format will be [x1 y1 x2 y2 slope]
        std::vector< std::vector<double> > slopeNegativeLines;
        std::vector<float> yValues;

        bool addedPos = false;
        bool addedNeg = false;

        // array counter for appending new lines
        int negativeCounter = 0;
        int positiveCounter = 0;

        // Loop through all lines
        for (size_t i = 0; i != lines.size(); ++i) {

            // Get points for current line
            float x1 = lines[i][0];
            float y1 = lines[i][1];
            float x2 = lines[i][2];
            float y2 = lines[i][3];

            // get line length
            float lineLength =  pow(pow(x2-x1,2) + pow(y2-y1,2), .5);

            //std::cout << "Point 1: (" << x1 << ", " << y1 << "), Point 2: (" << x2 << ", " << y2 << ")  ";

            // Check if line is long enough
            if (lineLength > 30) {
                // Ensure x2 != x1 to avoid division by zero
                if (x2 != x1) {
                    // Calculate slope
                    float slope = - (y2 - y1) / (x2 - x1);  //calculation is with a minus since Koordinationsystem wird gespiegelt

                    // Find angle of line wrt x axis
                    float tanTheta = (fabs(y2 - y1) / fabs(x2 - x1)); // tan(theta) value
                    float angle = atan(tanTheta) * 180 / CV_PI;         // Convert to degrees

                    // Only consider lines with reasonable angles
                    if (fabs(angle) < 89 && fabs(angle) > 33) {
                        if (slope > 0) {
                            //std::cout << " Positive line: ";     //Left lines from SW -> NO
                            // Add line with positive slope
                            slopePositiveLines.push_back({x1, y1, x2, y2, slope});
                            addedPos = true;
                            positiveCounter++;
                        } else if (slope < 0) {
                            //std::cout << " Negative line: ";     //Right lines from SO ->NW
                            // Add line with negative slope
                            slopeNegativeLines.push_back({x1, y1, x2, y2, slope});
                            addedNeg = true;
                            negativeCounter++;
                        }
                        //std::cout << "Valid Angle: "<<angle<<std::endl;
                    }
                    else{ /*std::cout << "Not right angle: "<< fabs(angle)<<std::endl;*/}
                }else{/*std::cout<<"Null divison"<<std::endl;*/}
            }else{/*std::cout<<"Too short"<<std::endl;*/}
        }

        if (!addedPos && !addedNeg) {
            std::cout << "Not enough valid lines found" << std::endl;
            return false;
        }

        std::cout << "Number Pos. Lines: " << positiveCounter << " Number Neg. Lines: " << negativeCounter << std::endl;

        filterdHoughLinesImage = cv::Mat::zeros(originalImage.size(), CV_8UC3);

        if(drawing_enabled){
            // Loop through lines with positive slope and draw them on the filterdHoughLinesImage
            for (const auto& line : slopePositiveLines) {
                cv::line(filterdHoughLinesImage, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), cv::Scalar(0, 0, 255), 2);
            }

            // Loop through lines with negative slope and draw them on the filterdHoughLinesImage
            for (const auto& line : slopeNegativeLines) {
                cv::line(filterdHoughLinesImage, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), cv::Scalar(0, 0, 255), 2);
            }
        }

        //-----------------GET POSITIVE/NEGATIVE SLOPE AVERAGES-----------------------
        // Average the position of lines and extrapolate to the top and bottom of the lane.

        // Calculate positive(left) slope mean
        float posSlopeMean;
        if(!slopePositiveLines.empty())
        {
            std::vector<float> positiveSlopes;
            for (const auto &line : slopePositiveLines) {
                positiveSlopes.push_back(line[4]);
            }

            // Get median of positiveSlopes
            sort(positiveSlopes.begin(), positiveSlopes.end()); // sort vec
            float posSlopeMedian = (positiveSlopes.size() % 2 == 0) ? (positiveSlopes[positiveSlopes.size() / 2 - 1] + positiveSlopes[positiveSlopes.size() / 2]) / 2 : positiveSlopes[positiveSlopes.size() / 2];


            // Get average of 'good' slopes(that are near the median), slopes that are drastically different than the others are thrown out 
            float sum = 0.0;
            int goodPosSlopes = 0;
            for (const auto &slope : positiveSlopes) {
                if (std::abs(slope - posSlopeMedian) < std::abs(posSlopeMedian * 0.2)) {
                    goodPosSlopes++;
                    sum += slope;
                }
            }
            //std::cout << "Sum: " << sum<< "positveGoodSlopes count:  "<< goodPosSlopes<<std::endl;
            // Calculate Mean of 'good' slopes
            if (sum != 0.0) {
                posSlopeMean = sum / goodPosSlopes;
            } else {
                std::cout << "Warning: Taking Median since no 'good' positive slopes were found" << std::endl;
                posSlopeMean = posSlopeMedian;
            }
            //std::cout << "Result SLope: "<<posSlopeMean<<std::endl;
        }

        
        
        ////////////////////////////////////////////////////////////////////////

        // Calculate negative(right) slope mean
        float negSlopeMean;
        if (!slopeNegativeLines.empty()) 
        {
            std::vector<float> negativeSlopes;
            for (const auto &line : slopeNegativeLines) {
                negativeSlopes.push_back(line[4]);
            }
            std::sort(negativeSlopes.begin(), negativeSlopes.end());
            float negSlopeMedian = (negativeSlopes.size() % 2 == 0) ? (negativeSlopes[negativeSlopes.size() / 2 - 1] + negativeSlopes[negativeSlopes.size() / 2]) / 2 : negativeSlopes[negativeSlopes.size() / 2];

            float sum = 0.0;
            int goodNegSlopes = 0;
            for (const auto &slope : negativeSlopes) {
                if (std::abs(slope - negSlopeMedian) < std::abs(negSlopeMedian * 0.2)) {
                    goodNegSlopes++;
                    sum += slope;
                }
            }
            //std::cout << "Sum: " << sum<< "negativeGoodSlopes count:  "<< goodNegSlopes<<std::endl;

            if (sum != 0.0) {
                negSlopeMean = sum / goodNegSlopes;
            } else {
                //std::cout << "Warning: Taking Median since no 'good' negative slopes were found" << std::endl;
                negSlopeMean = negSlopeMedian;
            }
        }
        


        //----------------GET AVERAGE X COORD WHEN Y COORD OF LINE = 0--------------------
        // Positive Lines (left)
        double xInterceptPosMean;
        if(!slopePositiveLines.empty()){
            std::vector<double> xInterceptPos; // define vector for x intercepts of positive slope lines

            // Loop through positive slope lines, find and store x intercept values
            for (const auto &line : slopePositiveLines) {
                double x1 = line[0]; // x value
                double y1 = originalImage.rows - line[1]; // y value...y axis is flipped
                double slope = line[4];
                double yIntercept = y1 - slope * x1; // y-intercept of line
                double xIntercept = -yIntercept / slope; // find x-intercept based off y = mx + b
                if (!std::isnan(xIntercept)) { // check for NaN
                    xInterceptPos.push_back(xIntercept); // add value
                }
            }

            // Get median of x intercepts for positive slope lines
            double xIntPosMed = median(xInterceptPos);

            // Define vector storing 'good' x intercept values
            std::vector<double> xIntPosGood;
            double xIntSum = 0.0; // for finding average

            // Loop through lines again and compare values against median
            for (const auto &line : slopePositiveLines) {
                double x1 = line[0]; // x value
                double y1 = originalImage.rows - line[1]; // y value...y axis is flipped
                double slope = line[4];
                double yIntercept = y1 - slope * x1; // y-intercept of line
                double xIntercept = -yIntercept / slope; // find x-intercept based off y = mx + b

                // check for NaN and check if it's close enough to the median
                if (!std::isnan(xIntercept) && std::abs(xIntercept - xIntPosMed) < 0.35 * xIntPosMed) {
                    xIntPosGood.push_back(xIntercept); // add to 'good' vector
                    xIntSum += xIntercept;
                }
            }
            //std::cout << "xIntSum: " << xIntSum << "xIntPosGood.size(): " << xIntPosGood.size()<< std::endl;

            if (!xIntPosGood.empty()) {
                // Get mean x intercept value for positive slope lines
                xInterceptPosMean = xIntSum / xIntPosGood.size();
            } else {
                std::cout << "Warning: Taking Median since no good enough Pos xIntercept was found" << std::endl;
                xInterceptPosMean = xIntPosMed;
            }
        }
        

        /////////////////////////////////////////////////////////////////
        // Negative Lines
        double xInterceptNegMean;
        if (!slopeNegativeLines.empty()){
            std::vector<double> xInterceptNeg; // define vector for x intercepts of negative slope lines

            // Loop through negative slope lines, find and store x intercept values
            for (const auto &line : slopeNegativeLines) {
                double x1 = line[0]; // x value
                double y1 = originalImage.rows - line[1]; // y value...y axis is flipped
                double slope = line[4];
                double yIntercept = y1 - slope * x1; // y-intercept of line
                double xIntercept = -yIntercept / slope; // find x intercept based off y = mx + b
                if (!std::isnan(xIntercept)) { // check for NaN
                    xInterceptNeg.push_back(xIntercept); // add value
                }
            }

            // Get median of x intercepts for negative slope lines
            double xIntNegMed = median(xInterceptNeg);

            // Define vector storing 'good' x intercept values, same concept as the slope calculations before
            std::vector<double> xIntNegGood;
            double xIntSumNeg; // for finding avg

            // Loop through lines again and compare values against median
            for (const auto &line : slopeNegativeLines) {
                double x1 = line[0]; // x value
                double y1 = originalImage.rows - line[1]; // y value...y axis is flipped
                double slope = line[4];
                double yIntercept = y1 - slope * x1; // y-intercept of line
                double xIntercept = -yIntercept / slope; // find x-intercept based off y = mx + b

                // check for NaN and check if it's close enough to the median
                if (!std::isnan(xIntercept) && std::abs(xIntercept - xIntNegMed) < 0.35 * xIntNegMed) {
                    xIntNegGood.push_back(xIntercept); // add to 'good' vector
                    xIntSumNeg += xIntercept;
                }
            }

            if (!xIntNegGood.empty()) {
                // Get mean x intercept value for negative slope lines
                xInterceptNegMean = xIntSumNeg / xIntNegGood.size();
            } else {
                std::cout << "Warning: Taking Median since no good enough Neg xIntercept was found" << std::endl;
                xInterceptNegMean = xIntNegMed;
            }
        }


        if (addedPos == false)
        {
            std::cout << "Warning: Didnt found left lane ... Now the last left lane is used" << std::endl;
            // Check if any member variable of lastLaneLine has been set
            if (lastLaneLine.posSlopeMean != -1.0f || lastLaneLine.xInterceptPosMean != -1.0) 
            {
                posSlopeMean = lastLaneLine.posSlopeMean;
                xInterceptPosMean = lastLaneLine.xInterceptPosMean;
            } 
            else 
            {
                std::cout << "Warning: Last Laneline has not been set yet --> Using default left lane" << std::endl;
                // Nearly vertical line for default val
                posSlopeMean = 1000;
                xInterceptPosMean = (originalImage.size().width / 4) * 1;
            }
            addedPos = true;
        }
        else if (addedNeg == false)
        {
            std::cout << "Warning: Didnt found right lane ... Now the last right lane is used" << std::endl;
            if(lastLaneLine.negSlopeMean != -1.0f || lastLaneLine.xInterceptNegMean != -1.0)
            {
                negSlopeMean = lastLaneLine.negSlopeMean;
                xInterceptNegMean = lastLaneLine.xInterceptNegMean;
            }
            else 
            {
                std::cout << "Warning: Last Laneline has not been set yet --> Using default right lane" << std::endl;
                // Nearly vertical line for default val
                negSlopeMean = -1000;
                xInterceptNegMean = (originalImage.size().width / 4) * 3;
            }
            addedNeg = true;

        }
        lanelines.posSlopeMean = posSlopeMean;
        lanelines.xInterceptPosMean = xInterceptPosMean;
        lanelines.negSlopeMean = negSlopeMean;
        lanelines.xInterceptNegMean = xInterceptNegMean;

        lastLaneLine = lanelines;
        return true;
}


int StrassenErkennung::calculate_steering_dir(LaneLines laneLines, cv::Mat originalImage)
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
        return needed_steering_dir; //pos -> means car is facing to the right , neg -> car is facing to the left
}


void StrassenErkennung::plot_laneLines(LaneLines laneLines, cv::Mat originalImage)
{
    //-----------------------PLOT LANE LINES------------------------
        // Need end points of line to draw in. Have x1,y1 (xIntercept,im.shape[1]) where
        // im.shape[1] is the bottom of the image. take y2 as some num (min/max y in the good lines?)
        // then find corresponding x

        // Create image, lane lines on real image
        laneLineImage = originalImage.clone();
        //cv::Mat laneFill = originalImage.clone();

        // Positive Slope Line
        float slope = laneLines.posSlopeMean;
        double x1 = laneLines.xInterceptPosMean;
        int y1 = 0;
        double y2 = originalImage.size().height - (originalImage.size().height - originalImage.size().height*.35);
        double x2 = (y2-y1) / slope + x1;

        // Add positive slope line to image
        x1 = int(x1 + .5);
        x2 = int(x2 + .5);
        y1 = int(y1 + .5);
        y2 = int(y2 + .5);
        //std::cout << "positiveSlopeLine: " << "bottomPoint: ("<<x1 << ", "<< originalImage.size().height-y1 << ") topPoint: ("<< x2 << ", "<<originalImage.size().height - y2 <<")"<<std::endl;
        cv::line(laneLineImage, cv::Point(x1, originalImage.size().height-y1), cv::Point(x2, originalImage.size().height - y2),
                                                                                            cv::Scalar(0,255,0), 3, 8 );


        // Negative Slope Line
        slope = laneLines.negSlopeMean;
        double x1N = laneLines.xInterceptNegMean;
        int y1N = 0;
        double x2N = (y2-y1N) / slope + x1N;

        // Add negative slope line to image
        x1N = int(x1N + .5);
        x2N = int(x2N + .5);
        y1N = int(y1N + .5);
        //std::cout << "negativeSlopeLine: " << "bottomPoint: ("<<x1N << ", "<< originalImage.size().height-y1N << ") topPoint: ("<< x2N << ", "<<originalImage.size().height - y2 <<")"<<std::endl;
        cv::line(laneLineImage, cv::Point(x1N, originalImage.size().height-y1N), cv::Point(x2N, originalImage.size().height - y2),
                                                                                            cv::Scalar(0,255,0), 3, 8 );

        //draw middle of image line
        cv::line(laneLineImage, cv::Point(originalImage.size().width / 2, originalImage.size().height - y2 - 10), cv::Point(originalImage.size().width / 2, originalImage.size().height - y2 + 10), cv::Scalar(0,0,255), 2, 8);

        //draw current steering direction
        int curr_steering_dir = (x2 + x2N) / 2;
        cv::line(laneLineImage, cv::Point(curr_steering_dir, originalImage.size().height - y2 - 10), cv::Point(curr_steering_dir, originalImage.size().height - y2 + 10), cv::Scalar(255,0,0), 2, 8);

        //draw helper horizontal sterring line
        cv::line(laneLineImage, cv::Point(curr_steering_dir, originalImage.size().height - y2), cv::Point(originalImage.size().width / 2, originalImage.size().height - y2), cv::Scalar(255,0,0), 1, 8);
}


bool StrassenErkennung::get_steering_dir(cv::Mat inputImage, int& steering_direction, bool drawing_enabled)
{
    cv::Mat grayImage = imagePreProcessor.convert_to_gray(inputImage);
    //cv::imshow("Gray Image", grayImage);

    //cv::Mat blurredImage = imagePreProcessor.gaussianBlur(grayImage);
    //cv::imshow("Blurred Image", blurredImage);


    edgesImage = imagePreProcessor.canny_edge_detection(grayImage);

    mask = imagePreProcessor.create_mask(inputImage.size());
    //cv::imshow("Blurred Image", blurredImage);

    maskedImage = imagePreProcessor.apply_mask_to_image(edgesImage, mask);
    //cv::imshow("maskedImage", maskedImage);

    std::vector<cv::Vec4i> houghlines;
    if(houghLines(maskedImage, inputImage, houghlines, drawing_enabled))
    {
        std::cout << "HoughLines returned success" <<std::endl;
    }
    else
    {
        return false;
    }

    LaneLines laneLines;
    if(averageLaneLines(houghlines, inputImage, laneLines, drawing_enabled))
    {
        steering_direction = calculate_steering_dir(laneLines, inputImage);
        if(drawing_enabled)
        {
            plot_laneLines(laneLines, inputImage);
        }
    }
    else
    {
        return false;
    }
    
    return true;
}


/*int main(){
    
    cv::Mat image = cv::imread("../real_image3.jpg");

    StrassenErkennung strassenErkennung;
    

    bool success;
    int steering_dir;
    success = strassenErkennung.get_steering_dir(image, steering_dir, true);

    if(success){
        std::cout << "Steering Direction: " << steering_dir << std::endl;

        cv::Mat edges = strassenErkennung.get_edgesImage();
        cv::Mat mask = strassenErkennung.get_mask();
        cv::Mat maskedImage = strassenErkennung.get_maskedImage();
        cv::Mat houghImage = strassenErkennung.get_houghLinesImage();
        cv::Mat laneLineImage = strassenErkennung.get_laneLineImage();
        cv::Mat filteredHoughLinesImage = strassenErkennung.get_filteredHoughLinesImage();

        //cv::imshow("mask", mask);
        cv::imshow("edges image", edges);
        //cv::imshow("filteredHoughLineImage", filteredHoughLinesImage);
        //cv::imshow("hough", houghImage);
        //cv::imshow("laneLineImage", laneLineImage);
        cv::imshow("maskedImage", maskedImage);
        cv::imshow("mask", mask);

        cv::waitKey(0); // wait for a key press    

    }
    return 0;
}*/
