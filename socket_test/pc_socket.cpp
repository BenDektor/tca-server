#include "pc_socket.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <sys/time.h>

Socket::Socket(const std::string& serverIp, int portSensorDaten, int portFahrzeugbefehle, int portKameraBilder) {
    // Create UDP sockets
    if ((clientSocketSensorData = socket(AF_INET, SOCK_DGRAM, 0)) < 0 || (clientSocketFahrzeugbefehle = socket(AF_INET, SOCK_DGRAM, 0)) < 0 || (clientSocketKameraBilder = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Error: Unable to create UDP socket\n";
        return;
    }

    // Configure server addresses
    memset(&serverAddrSensorData, 0, sizeof(serverAddrSensorData));
    serverAddrSensorData.sin_family = AF_INET;
    serverAddrSensorData.sin_port = htons(portSensorDaten);

    memset(&serverAddrFahrzeugbefehle, 0, sizeof(serverAddrFahrzeugbefehle));
    serverAddrFahrzeugbefehle.sin_family = AF_INET;
    serverAddrFahrzeugbefehle.sin_port = htons(portFahrzeugbefehle);

    memset(&serverAddrKameraBilder, 0, sizeof(serverAddrKameraBilder));
    serverAddrKameraBilder.sin_family = AF_INET;
    serverAddrKameraBilder.sin_port = htons(portKameraBilder);


    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddrSensorData.sin_addr) <= 0 ||
        inet_pton(AF_INET, serverIp.c_str(), &serverAddrFahrzeugbefehle.sin_addr) <= 0 ||
        inet_pton(AF_INET, serverIp.c_str(), &serverAddrKameraBilder.sin_addr) <= 0) {
        std::cerr << "Error: Invalid server IP address\n";
        return;
    }

    // Start threads
    sensorDataThread = std::thread(&Socket::SensorDataPort, this);
    fahrzeugbefehleThread = std::thread(&Socket::FahrzeugbefehlePort, this);
    kameraBilderThread = std::thread(&Socket::KameraBilderPort, this);
}

Socket::~Socket() {
    sensorDataThread.join();
    fahrzeugbefehleThread.join();
    kameraBilderThread.join();
    close(clientSocketKameraBilder);
    close(clientSocketFahrzeugbefehle);
    close(clientSocketSensorData);
}

void Socket::SensorDataPort() {
    while (true) {
        if (!sendMessage("Request Sensor Data Message", 0)) {
            std::cerr << "Error: Unable to send message on port 1\n";
        }

        /*std::string message;
        if (!receiveMessage(message, 0)) {
            std::cerr << "Error: Unable to receive message on port 1\n";
        } else {
            std::cout << "Received sensordata 1: " << message << std::endl;
        }*/

        SensorData sensorData_raw = receiveJsonData();
        if (sensorData_raw.Compass == -1) { // Assuming -1 indicates a failed JSON parse or empty data
            std::cout << "Empty sensor data" << std::endl;
        } else {
            sensorData = sensorData_raw;
             std::cout << "Compass: " << sensorData.Compass << ", Distance Left: " << sensorData.DistanceLeft
                      << ", Distance Rear: " << sensorData.DistanceRear << ", Distance Right: " << sensorData.DistanceRight
                      << ", Light Sensor: " << sensorData.LightSensor << ", Total Speed: " << sensorData.TotalSpeed << "\n";
            std::cout << "Lidar Values (Size: " << sensorData.LidarData.size() << "): ";
            for (int value : sensorData.LidarData) {
                std::cout << value << " ";
            }
            std::cout << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust the delay as needed
    }
}

void Socket::KameraBilderPort() {
    while (true) {
        if (!sendMessage("Request Image Data Message", 2)) {
            std::cerr << "Error: Unable to send message on port 1\n";
        }

        /*std::string message;
        if (!receiveMessage(message, 2)) {
            std::cerr << "Error: Unable to receive message on port 1\n";
        } else {
            std::cout << "Received image data 1: " << message << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust the delay as needed*/
        cv::Mat frame = receiveFrame(); // Receive a frame
        if (frame.empty()) {
            std::cerr << "Error: Failed to receive frame on port 2\n";
            continue; // Skip further processing if frame reception failed
        }
        else{
            std::cout << "Succesffullly received image data" << std::endl;
        }
        imageData = frame;
    }
}

void Socket::FahrzeugbefehlePort() {
    while (true) {
        // sendMessage("0/1,-6-6", 1);
        if (!sendMessage("Sending Fahrzeugbefehle ...", 1)) {
            std::cerr << "Error: Unable to send message on port 2\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust the delay as needed
    }
}

bool Socket::sendMessage(const std::string& message, int index) {
    struct sockaddr_in serverAddr;
    int clientSocket;

    // Determine which socket to use based on index
    if (index == 0) {
        serverAddr = serverAddrSensorData;
        clientSocket = clientSocketSensorData;
    } else if (index == 1) {
        serverAddr = serverAddrFahrzeugbefehle;
        clientSocket = clientSocketFahrzeugbefehle;
    } else if (index == 2) {
        serverAddr = serverAddrKameraBilder;
        clientSocket = clientSocketKameraBilder;
    }
     else {
        std::cerr << "Error: Invalid index for sending message\n";
        return false;
    }

    // Send message to server
    int bytesSent = sendto(clientSocket, message.c_str(), message.length(), 0,
                           (const struct sockaddr *)&serverAddr, sizeof(serverAddr));

    if (bytesSent < 0) {
        std::cerr << "Error: Unable to send message on UDP socket\n";
        return false;
    }

    return true;
}

bool Socket::receiveMessage(std::string& message, int index) {
    char buffer[1024]; // Adjust buffer size as necessary
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen = sizeof(serverAddr);
    int clientSocket;

    // Determine which socket to use based on index
    if (index == 0) {
        clientSocket = clientSocketSensorData;
    } else if (index == 1) {
        clientSocket = clientSocketFahrzeugbefehle;
    } else if (index == 2) {
        clientSocket = clientSocketKameraBilder;
    } else {
        std::cerr << "Error: Invalid index for receiving message\n";
        return false;
    }

    memset(buffer, 0, sizeof(buffer));

    // Receive message from server
    int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0,
                                 (struct sockaddr *)&serverAddr, &serverAddrLen);

    if (bytesReceived < 0) {
        std::cerr << "Error: Unable to receive message on UDP socket\n";
        return false;
    }

    // Convert received data to string
    message = std::string(buffer);
    return true;
}

cv::Mat Socket::receiveFrame() {
    // Set timeout for the socket
    struct timeval timeout;
    timeout.tv_sec = 2;  // Timeout of 5 seconds
    timeout.tv_usec = 0; // 0 microseconds

    // Use the appropriate socket for receiving frames (clientSocketKameraBilder)
    int clientSocket = clientSocketKameraBilder;

    // Set the receive timeout option on the socket
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cerr << "Error: Failed to set socket timeout\n";
        return cv::Mat(); // Return empty matrix on failure
    }

    // Receive frame size
    int frameSize = 0;
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen = sizeof(serverAddr);

    if (recvfrom(clientSocket, &frameSize, sizeof(frameSize), 0,
                 (struct sockaddr *)&serverAddr, &serverAddrLen) < 0) {
        std::cerr << "Error: Failed to receive frame size or timeout occurred\n";
        return cv::Mat(); // Return empty matrix on failure
    }

    // Allocate buffer for frame data based on received size
    std::vector<uchar> framebuffer(frameSize);

    // Receive the actual frame data
    int framebytesReceived = recvfrom(clientSocket, framebuffer.data(), framebuffer.size(), 0,
                                      (struct sockaddr *)&serverAddr, &serverAddrLen);

    if (framebytesReceived < 0) {
        std::cerr << "Error: Unable to receive frame data or timeout occurred\n";
        return cv::Mat(); // Return empty matrix on failure
    }

    // Decode the frame using OpenCV
    cv::Mat frame = cv::imdecode(cv::Mat(framebuffer), cv::IMREAD_COLOR);

    // Check if frame decoding was successful
    if (frame.empty()) {
        std::cerr << "Error: Failed to decode frame\n";
        return cv::Mat(); // Return empty matrix on failure
    }
    std::cout << "returning frame" << std::endl;

    return frame;
}


SensorData Socket::receiveJsonData() {
    char buffer[4096];
    SensorData data;
    data.Compass = -1; // Default value to indicate failure
    struct sockaddr_in senderAddr;
    socklen_t senderAddrLen = sizeof(senderAddr);

    // Set timeout for the socket
    struct timeval timeout;
    timeout.tv_sec = 5;  // Timeout of 5 seconds
    timeout.tv_usec = 0; // 0 microseconds

    // Use the appropriate socket (clientSocketSensorData)
    int clientSocket = clientSocketSensorData;

    // Set the receive timeout option on the socket
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cerr << "Error: Failed to set socket timeout\n";
        return data; // Return default data on failure
    }

    int bytesRead = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&senderAddr, &senderAddrLen);
    if (bytesRead <= 0) {
        std::cerr << "Connection closed by server, error occurred, or timeout occurred\n";
        return data;
    }

    std::string receivedData(buffer, bytesRead);
    size_t jsonStartIndex = receivedData.find('{');
    if (jsonStartIndex != std::string::npos) {
        receivedData = receivedData.substr(jsonStartIndex);
    }

    if (isJson(receivedData)) {
        data = parseJsonData(receivedData);
    } else {
        std::cout << "Received non-JSON message: " << receivedData << std::endl;
    }

    return data;
}




bool Socket::isJson(const std::string& str) {
    return !str.empty() && (str.front() == '{' || str.front() == '[') && (str.back() == '}' || str.back() == ']');
}

SensorData Socket::parseJsonData(const std::string& jsonData) {
    SensorData data;
    try {
        auto jsonObject = nlohmann::json::parse(jsonData);
        data.Compass = jsonObject.value("Compass", 0);
        data.DistanceLeft = jsonObject.value("Distance Left", 0.0);
        data.DistanceRear = jsonObject.value("Distance Rear", 0.0);
        data.DistanceRight = jsonObject.value("Distance Right", 0.0);
        data.LightSensor = jsonObject.value("Light Sensor", 0);
        data.TotalSpeed = jsonObject.value("Total Speed", 0.0);
        data.LidarData = jsonObject.value("Lidar Data", std::vector<int>{});
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Error parsing JSON data: " << e.what() << std::endl;
    }
    return data;
}




const std::string SERVER_IP = "172.16.17.67"; // Replace with the actual server IP
const int PORT1 = 3000;
const int PORT2 = 3001;
const int PORT3 = 3002;


int main() {
    Socket socket(SERVER_IP, PORT1, PORT2, PORT3); // Initialize socket with server IP and two ports

    while(true){
        cv::Mat frame = socket.imageData.clone();
        if(!frame.empty()){
            cv::imshow("f", frame);

        }
        cv::waitKey(1);
    }


    return 0;
}