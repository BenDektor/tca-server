#include "pc_socket.h"

Socket::Socket(const std::string& ip_address) {
    if (!setupConnection(ip_address)) {
        std::cerr << "Error: Unable to establish connection\n";
        return;
    }

    // Receive the test message from the server
    char testMessage[1024] = {0};
    if (recv(clientSocket, testMessage, sizeof(testMessage), 0) <= 0) {
        std::cerr << "Error: Unable to receive test message\n";
        return;
    }
    std::cout << "Test message from server: " << testMessage << std::endl;

    if (!sendTestMessage()) {
        std::cerr << "Error: Unable to send test message\n";
        return;
    }
}

Socket::~Socket() {
    closeConnection();
}

bool Socket::setupConnection(const std::string& ip_address) {
    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return false;
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip_address.c_str());
    serverAddr.sin_port = htons(3000); // Use the same port as the Raspberry Pi side

    // Connect to server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        return false;
    }

    return true;
}

bool Socket::sendMessage(const char* message) {
    std::cout << "Socket: sending this message to raspi: " << message << std::endl;

    if (send(clientSocket, message, strlen(message), 0) < 0) {
        return false;
    }
    return true;
}

void Socket::closeConnection() {
    close(clientSocket);
}

bool Socket::sendTestMessage() {
    const char* responseMessage = "Hello from PC!";
    return sendMessage(responseMessage);
}

std::vector<std::string> Socket::extractMessages(const std::string& data, const std::string& startDelimiter, const std::string& endDelimiter) {
    std::vector<std::string> messages;
    size_t startPos = 0;
    while ((startPos = data.find(startDelimiter, startPos)) != std::string::npos) {
        startPos += startDelimiter.length();
        size_t endPos = data.find(endDelimiter, startPos);
        if (endPos != std::string::npos) {
            messages.push_back(data.substr(startPos, endPos - startPos));
            startPos = endPos + endDelimiter.length();
        }
    }
    return messages;
}

SensorData Socket::receiveJsonData(const std::string& jsonData) {
    SensorData sensorData;
    sensorData.Compass = -1; // Default value to indicate failure

    try {
        auto jsonObject = nlohmann::json::parse(jsonData);
        sensorData.Compass = jsonObject.value("Compass", 0);
        sensorData.DistanceLeft = jsonObject.value("Distance Left", 0.0);
        sensorData.DistanceRear = jsonObject.value("Distance Rear", 0.0);
        sensorData.DistanceRight = jsonObject.value("Distance Right", 0.0);
        sensorData.LightSensor = jsonObject.value("Light Sensor", 0);
        sensorData.TotalSpeed = jsonObject.value("Total Speed", 0.0);
        sensorData.LidarData = jsonObject.value("Lidar Data", std::vector<int>{});
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Error parsing JSON data: " << e.what() << std::endl;
    }

    return sensorData;
}

cv::Mat Socket::receiveFrame(const std::vector<char>& imgData) {
    cv::Mat frame = cv::imdecode(cv::Mat(imgData), cv::IMREAD_COLOR);

    if (frame.empty()) {
        std::cerr << "Error: Failed to decode frame\n";
    }

    return frame;
}

void Socket::processIncomingMessages() {
    char buffer[8192];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        std::cerr << "Connection closed by server or error occurred\n";
        return;
    }

    std::string receivedData(buffer, bytesRead);

    // Extract JSON messages
    auto jsonMessages = extractMessages(receivedData, "<json_start>", "<json_end>");

    // Process JSON messages
    for (const auto& jsonData : jsonMessages) {
        if (!jsonData.empty()) {
            SensorData sensorData = receiveJsonData(jsonData);
            if (sensorData.Compass != -1) {
                std::cout << "Compass: " << sensorData.Compass << ", Distance Left: " << sensorData.DistanceLeft
                          << ", Distance Rear: " << sensorData.DistanceRear << ", Distance Right: " << sensorData.DistanceRight
                          << ", Light Sensor: " << sensorData.LightSensor << ", Total Speed: " << sensorData.TotalSpeed << "\n";
                std::cout << "Lidar Values (Size: " << sensorData.LidarData.size() << "): ";
                for (int value : sensorData.LidarData) {
                    std::cout << value << " ";
                }
                std::cout << std::endl;
            }
        }
    }

    // Extract image data messages
    size_t startPos = 0;
    while ((startPos = receivedData.find("<img_start>", startPos)) != std::string::npos) {
        startPos += std::string("<img_start>").length();
        size_t endPos = receivedData.find("<img_end>", startPos);
        if (endPos != std::string::npos) {
            // Read the size prefix
            int frameSize;
            std::memcpy(&frameSize, receivedData.data() + startPos, sizeof(int));
            startPos += sizeof(int);

            if (endPos > startPos && (endPos - startPos) == frameSize) {
                std::vector<char> imgData(receivedData.begin() + startPos, receivedData.begin() + endPos);
                cv::Mat frame = receiveFrame(imgData);
                if (!frame.empty()) {
                    cv::imshow("Received Frame", frame);
                    cv::waitKey(1);
                }
            }
            startPos = endPos + std::string("<img_end>").length();
        }
    }
}

int main() {
    std::string serverIp = "172.16.8.137";
    Socket client(serverIp);

    while (true) {
        client.processIncomingMessages();
    }

    return 0;
}





/*int main() {
    // Initialize the socket with the server's IP address
    Socket socket("192.168.193.223");

    // Continuously receive and display frames
    while (true) {
        // Receive a frame from the server
        cv::Mat frame = socket.receiveFrame();

        // Check if frame decoding was successful
        if (frame.empty()) {
            std::cerr << "Error: Failed to receive frame\n";
            break;
        }

        // Display the frame
        cv::imshow("Video Stream", frame);

        // Check for exit key press
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    return 0;
}*/













/*int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error: Unable to create socket\n";
        return 1;
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("192.168.193.223"); // IP of Raspberry Pi
    serverAddr.sin_port = htons(8080); // Use the same port as the Raspberry Pi side

    // Connect to server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Unable to connect to server\n";
        close(clientSocket);
        return 1;
    }

    // Receive message
    char buffer[1024] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived < 0) {
        std::cerr << "Error: Unable to receive message\n";
        close(clientSocket);
        return 1;
    } else {
        std::cout << "Message from Raspberry Pi: " << buffer << std::endl;
    }

    // Send response message
    const char* responseMessage = "Hello from PC!";
    if (send(clientSocket, responseMessage, strlen(responseMessage), 0) < 0) {
        std::cerr << "Error: Unable to send message\n";
        close(clientSocket);
        return 1;
    }

    // Create a window to display the video stream
    cv::namedWindow("Video Stream", cv::WINDOW_NORMAL);

    // Continuously receive and display frames
    while (true) {
        // Receive frame size
        int frameSize = 0;
        if (recv(clientSocket, &frameSize, sizeof(frameSize), 0) < 0) {
            std::cerr << "Error: Failed to receive frame size\n";
            close(clientSocket);
            return 1;
        }

        // Allocate buffer for frame data based on received size
        std::vector<char> framebuffer(frameSize);

        // Receive the actual frame data
        int framebytesReceived = recv(clientSocket, framebuffer.data(), frameSize, MSG_WAITALL);
        if (framebytesReceived < 0) {
            std::cerr << "Error: Unable to receive frame data\n";
            close(clientSocket);
            return 1;
        }

        // Decode the frame using OpenCV
        cv::Mat frame = cv::imdecode(cv::Mat(framebuffer), cv::IMREAD_COLOR);

        // Check if frame decoding was successful
        if (frame.empty()) {
            std::cerr << "Error: Failed to decode frame\n";
            close(clientSocket);
            return 1;
        }

        // Display the frame
        cv::imshow("Video Stream", frame);

        // Check for exit key press
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    // Close socket
    close(clientSocket);

    return 0;
}*/