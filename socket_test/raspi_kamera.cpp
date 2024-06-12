
#include <iostream>
#include <opencv2/opencv.hpp>


int main(int argc, char **argv) {

    // Open the camera
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {

        std::cerr << "Error opening camera" << std::endl;
        return -1;
    }

    // Set camera parameters (optional)
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);


    // Capture loop
    cv::Mat frame;

    while (1) {

        // Capture frame
        cap >> frame;


        // Check for end of video
        if (frame.empty()) {
            std::cerr << "Error: Blank frame grabbed" << std::endl;
            break;
        }

        // Display frame
        cv::imshow("Raspberry Pi Camera", frame);

        // Break the loop on key press
        if (cv::waitKey(30) >= 0) break;
    }

    // Release the camera
    cap.release();
    cv::destroyAllWindows();

    return 0;
}


