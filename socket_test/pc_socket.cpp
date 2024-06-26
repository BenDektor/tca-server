#include "pc_socket.h"

std::vector<uchar> base64_decode(const std::string &base64str) {
    std::string decoded;
    cv::Mat data = cv::Mat(1, base64str.length(), CV_8UC1, const_cast<char*>(base64str.c_str()));
    cv::Mat binaryData = cv::imdecode(data, cv::IMREAD_COLOR);
    if (binaryData.empty()) {
        std::cerr << "Error decoding base64 data" << std::endl;
        return std::vector<uchar>();
    }
    return std::vector<uchar>(binaryData.data, binaryData.data + binaryData.total());
}

Socket::Socket(const std::string& ip_address) {
    if (!setupConnection(ip_address)) {
        std::cerr << "Error: Unable to establish connection\n";
        return;
    }

    // Receive the test message from the server
    char testMessage[1024] = {0};
    if (!receiveMessage(testMessage, sizeof(testMessage))) {
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
    std::cout << "Sending message to server: " << message << std::endl;
    if (send(clientSocket, message, strlen(message), 0) < 0) {
        return false;
    }
    return true;
}

bool Socket::receiveMessage(char* buffer, int bufferSize) {
    int bytesReceived = recv(clientSocket, buffer, bufferSize, 0);
    if (bytesReceived < 0) {
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

cv::Mat Socket::receiveFrame() {
    // Receive frame size
    int frameSize = 0;
    if (recv(clientSocket, &frameSize, sizeof(frameSize), 0) < 0) {
        std::cerr << "Error: Failed to receive frame size\n";
        closeConnection();
        exit(1);
    }

    // Allocate buffer for frame data based on received size
    std::vector<char> framebuffer(frameSize);

    // Receive the actual frame data
    int framebytesReceived = recv(clientSocket, framebuffer.data(), frameSize, MSG_WAITALL);
    if (framebytesReceived < 0) {
        std::cerr << "Error: Unable to receive frame data\n";
        closeConnection();
        exit(1);
    }

    // Decode the frame using OpenCV
    cv::Mat frame = cv::imdecode(cv::Mat(framebuffer), cv::IMREAD_COLOR);

    // Check if frame decoding was successful
    if (frame.empty()) {
        std::cerr << "Error: Failed to decode frame\n";
        closeConnection();
        exit(1);
    }

    return frame;
}

SensorData Socket::receiveJsonData() {
    char buffer[4096];
    SensorData sensorData;
    sensorData.Compass = -1; // Default value to indicate failure

    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        std::cerr << "Connection closed by server or error occurred\n";
        return sensorData;
    }

    std::string receivedData(buffer, bytesRead);
    size_t jsonStartIndex = receivedData.find('{');
    if (jsonStartIndex != std::string::npos) {
        receivedData = receivedData.substr(jsonStartIndex);
    }

    if (isJson(receivedData)) {
        sensorData = parseJsonData(receivedData);
    } else {
        std::cout << "Received non-JSON message: " << receivedData << std::endl;
    }

    return sensorData;
}

bool Socket::isJson(const std::string& str) {
    return !str.empty() && (str.front() == '{' || str.front() == '[') && (str.back() == '}' || str.back() == ']');
}

SensorData Socket::parseJsonData(const std::string& jsonData) {
    SensorData sensorData;
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

void Socket::receiveMessage() {
    char buffer[1024];  // Adjust buffer size as needed
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead == -1) {
        std::cerr << "Error receiving message. Error code " << errno << ": " << strerror(errno) << std::endl;
        return;
    }

    // Ensure null-terminated string
    buffer[bytesRead] = '\0';

    // Parse received message
    std::istringstream iss(buffer);
    std::string headerStr;
    if (std::getline(iss, headerStr, '|')) {
        int header = std::stoi(headerStr);  // Convert header string to int

        // Determine how to handle based on header
        switch (header) {
            case 1:
                handleSensorData(iss);  // Pass remaining data stream to handleSensorData
                break;
            case 2:
                handleImageData(iss);   // Pass remaining data stream to handleImageData
                break;
            default:
                std::cerr << "Unknown header: " << header << std::endl;
                break;
        }
    }
}

void Socket::handleSensorData(std::istringstream& iss) {
    // Extract sensor data from iss
    int compass, distanceLeft, distanceRight;
    if (!(iss >> compass >> distanceLeft >> distanceRight)) {
        std::cerr << "Error parsing sensor data" << std::endl;
        return;
    }

    // Process sensor data
    std::cout << "Received Sensor Data: Compass=" << compass
              << ", DistanceLeft=" << distanceLeft
              << ", DistanceRight=" << distanceRight << std::endl;
}

void Socket::handleImageData(std::istringstream& iss) {
    // Extract image data from iss (assuming base64 encoded)
    std::string base64Image;
    if (!std::getline(iss, base64Image)) {
        std::cerr << "Error parsing image data" << std::endl;
        return;
    }

    // Decode base64 and process image (decode and display or save to file)
    std::vector<uchar> buffer = base64_decode(base64Image);
    cv::Mat image = cv::imdecode(buffer, cv::IMREAD_COLOR);

    if (image.empty()) {
        std::cerr << "Error decoding image" << std::endl;
        return;
    }

    // Display or save image
    cv::imshow("Received Image", image);
    cv::waitKey(0);
}

int main() {
    std::string serverIp = "172.16.8.137";
    Socket client(serverIp);

    while (true) {
        client.receiveMessage();
    }

    return 0;
}


