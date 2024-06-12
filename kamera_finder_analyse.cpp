#include "kamera_finder_analyse.h"

int main() {
    // Initialize the socket with the server's IP address
    Socket socket("172.16.8.137");

    StrassenFinder strassenFinder;
    bool is_driving = false;

    // Continuously receive and display frames
    while (true) {
        // Receive a frame from the server
        cv::Mat frame = socket.receiveFrame();

        // Check if frame decoding was successful
        if (frame.empty()) {
            std::cerr << "Error: Failed to receive frame\n";
            break;
        }

        bool success;

        // Find lanes using StrassenFinder
        success = strassenFinder.find_lanes(frame);

        if (success) {
            // Example: Display images at a throttled rate
            static int display_counter = 0;
            if (++display_counter % 10 == 0) { // Display every 10th frame
                cv::Mat mask = strassenFinder.get_mask();
                cv::Mat houghImage = strassenFinder.get_houghLinesImage();
                cv::Mat laneLineImage = strassenFinder.get_laneLineImage();
                cv::Mat filteredHoughLinesImage = strassenFinder.get_filteredHoughLinesImage();

                cv::imshow("mask", mask);
                cv::imshow("filteredHoughLineImage", filteredHoughLinesImage);
                cv::imshow("laneLineImage", laneLineImage);
            }

            // Get distance to street and steering angle
            double distance_to_street = strassenFinder.get_distance_to_street();
            double steering_angle = strassenFinder.get_steering_angle();

            // Define thresholds for distance and angle
            const double distance_threshold = 470.0; // pixels
            const double angle_threshold_1 = 50.0; // degrees
            const double angle_threshold_2 = 70.0; // degrees

            // Determine the steering command based on distance and angle
            int steering_command = 0;
            if (std::abs(distance_to_street) > distance_threshold) {
                if (std::abs(steering_angle) > angle_threshold_2) {
                    steering_command = (steering_angle > 0) ? 6 : -6; // Max steering command in the direction
                } else if (std::abs(steering_angle) > angle_threshold_1) {
                    steering_command = (steering_angle > 0) ? 3 : -3; // Moderate steering command
                } else {
                    steering_command = (steering_angle > 0) ? 1 : -1; // Minimal steering command
                }
            } else {
                if (std::abs(steering_angle) > angle_threshold_2) {
                    steering_command = (steering_angle > 0) ? 6 : -6; // Max steering command in the direction
                } else if (std::abs(steering_angle) > angle_threshold_1) {
                    steering_command = (steering_angle > 0) ? 3 : -3; // Moderate steering command
                } else {
                    steering_command = 0; // Go straight
                }
            }

            // Start driving if not already driving
            if (!is_driving) {
                socket.sendMessage("start");
                is_driving = true;
            }

            // Send steering command to the car
            std::stringstream ss;
            ss << "steer:" << steering_command;
            std::string message = ss.str();
            std::cout << "Sending command: " << message << std::endl;
            socket.sendMessage(message.c_str());

        } else {
            socket.sendMessage("Error");

            // Stop the car if no lanes are detected
            if (is_driving) {
                socket.sendMessage("stop");
                is_driving = false;
            }
        }

        cv::waitKey(1);
    }

    return 0;
}
