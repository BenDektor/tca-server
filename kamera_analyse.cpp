#include "kamera_analyse.h"

int mapValue(double input, double input_start, double input_end, double output_start, double output_end) {
        return static_cast<int>(output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start));
}


int main() {
    // Initialize the socket with the server's IP address
    Socket socket("172.16.8.137");

    StrassenErkennung strassenErkennung;
    PID pid_controller(2, 0.1, 0.2);

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
        int steering_dir;
        success = strassenErkennung.get_steering_dir(frame, steering_dir, true);

        if(success){
            std::cout << "Steering Direction: " << steering_dir << std::endl;

            //cv::Mat edges = strassenErkennung.get_edgesImage();
            //cv::Mat mask = strassenErkennung.get_mask();
            //cv::Mat maskedImage = strassenErkennung.get_maskedImage();
            //cv::Mat houghImage = strassenErkennung.get_houghLinesImage();
            cv::Mat laneLineImage = strassenErkennung.get_laneLineImage();
            //cv::Mat filteredHoughLinesImage = strassenErkennung.get_filteredHoughLinesImage();

            //cv::imshow("mask", mask);
            //cv::imshow("edges image", edges);
            //cv::imshow("filteredHoughLineImage", filteredHoughLinesImage);
            //cv::imshow("hough", houghImage);
            cv::imshow("laneLineImage", laneLineImage);   


            if(steering_dir > 320) steering_dir = 320;
            if(steering_dir < -320) steering_dir = -320;
            //Convert steering dir to lenk command
            //double pid_output = pid_controller.calculate(0, steering_dir);
            //int lenk_command = mapValue(pid_output, -640.0, 640.0, -6.0, 6.0);

            int lenk_command = mapValue(steering_dir, -320.0, 320.0, -6.0, 6.0);
            // Convert lenkcommand to string
            std::stringstream ss;
            //ss << lenk_command;
            ss << lenk_command;
            std::string message = ss.str();
            std::cout << "this is my messge: " << message.c_str() << std::endl;

            // Send message to client
            socket.sendMessage(message.c_str());
        }
        else
        {
            cv::imshow("laneLineImage", frame);
            socket.sendMessage("Error");
        }
        cv::waitKey(1);
    }

    return 0;
}