#include "kamera_analyse.h"

int mapValue(double input, double input_start, double input_end, double output_start, double output_end) {
    double limited_input;
    if(input > input_end) {
        limited_input = input_end;
    } else if(input < input_start) {
        limited_input = input_start;
    } else {
        limited_input = input;
    }
    
    double mapped_value = output_start + ((output_end - output_start) / (input_end - input_start)) * (limited_input - input_start);
    return static_cast<int>(mapped_value);
}



int main() {
    // Initialize the socket with the server's IP address
    Socket socket("172.16.8.137");

    LaneHandler laneHandler;
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

        CarPosition carPosition = laneHandler.getCarPosition(frame);

        std::stringstream stream_command;

        if(carPosition == CarPosition::ON_STREET){
            std::cout << "Car Position Result: " << "OnStreet" << std::endl;
            std::cout << "Steering Direction: " << laneHandler.get_steering_dir() << std::endl;
            int steering_dir = laneHandler.get_steering_dir();
            int lenk_command = mapValue(steering_dir, -320.0, 320.0, -6.0, 6.0);
            std::cout << "Mapped Command: " << lenk_command << std::endl;
            stream_command << lenk_command;
        }
        else if (carPosition == CarPosition::OFF_STREET_TO_LEFT){
            std::cout << "Car Position Result: " << "OffStreet Street to the left" << std::endl;
            std::cout << "Distance to Street: " << laneHandler.get_distance_to_street() << std::endl;
            std::cout << "Angle to Street: " << laneHandler.get_angle_to_street() << std::endl;

            int distance_to_street = laneHandler.get_distance_to_street();
            int angle_to_street = laneHandler.get_angle_to_street();

            if(distance_to_street < 410){ //nah an der strase
                if(angle_to_street > 70) {// angle can be from 0-90
                    stream_command << -4; // nach rechts lenken, um sich der Spur anzupassen
                }
                else{
                    stream_command << -1;
                }

            }
            else { // weiter weg von der strase
                stream_command << 2; //leicht nach links lenken, um schneller zur Strase zu kommen
            }

        }   
        else if (carPosition == CarPosition::OFF_STREET_TO_RIGHT){
            std::cout << "Car Position Result: " << "OffStreet Street to the right" << std::endl;
            std::cout << "Distance to Street: " << laneHandler.get_distance_to_street() << std::endl;
            std::cout << "Angle to Street: " << laneHandler.get_angle_to_street() << std::endl;

            int distance_to_street = laneHandler.get_distance_to_street();
            int angle_to_street = laneHandler.get_angle_to_street();

            if(distance_to_street < 410){ //nah an der strase
                if(angle_to_street > 70) {// angle can be from 0-90
                    stream_command << 4; // nach links lenken, um sich der Spur anzupassen
                }
                else{
                    stream_command << 1;
                }

            }
            else { // weiter weg von der strase
                stream_command << -2; //leicht nach rechts lenken, um schneller zur Strase zu kommen
            }
        }
        else if (carPosition == CarPosition::NO_STREET){
            std::cout << "Car Position Result: " << "NoStreet" << std::endl;
            stream_command << "Error";
        }
        else {
            std::cout << "Unknwon Car Position" << std::endl;
            stream_command << "Error";
        }

        
        std::string message = stream_command.str();
        std::cout << "this is my message to raspi: " << message.c_str() << std::endl;
        // Send message to client
        socket.sendMessage(message.c_str());

        cv::waitKey(1);
    }

    return 0;
}