#include "pc_socket.h"

std::vector<uchar> Socket::base64_decode(const std::string &base64str) {
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

void Socket::receiveRaspiData() {
    char buffer[1024];  // Adjust buffer size as needed
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead == -1) {
        std::cerr << "Error receiving message. Error code " << errno << ": " << strerror(errno) << std::endl;
        return;
    }
    else{
        std::cout << "received some data: " << bytesRead << std::endl;
    }

    // Ensure null-terminated string
    buffer[bytesRead] = '\0';

    // Parse received message
    std::istringstream iss(buffer);
    std::string headerStr;
    if (std::getline(iss, headerStr, '|')) {
        int header = std::stoi(headerStr);  // Convert header string to int

        std::cout << "The header of the message is:"  << header << std::endl;

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
    // Extract raw data from the stream
    std::string rawData((std::istreambuf_iterator<char>(iss)), std::istreambuf_iterator<char>());

    SensorData sensorData;
    if (isJson(rawData)) {
        // Parse the JSON data to create a SensorData object
        sensorData = parseJsonData(rawData);
    } else {
        std::cerr << "Received non-JSON sensor data: " << rawData << std::endl;
        // Optionally, handle non-JSON data here
    }

    // Process the SensorData object
    std::cout << "Received Sensor Data: Compass=" << sensorData.Compass
              << ", DistanceLeft=" << sensorData.DistanceLeft
              << ", DistanceRear=" << sensorData.DistanceRear
              << ", DistanceRight=" << sensorData.DistanceRight
              << ", LightSensor=" << sensorData.LightSensor
              << ", TotalSpeed=" << sensorData.TotalSpeed << std::endl;

    std::cout << "Lidar Data: ";
    for (int value : sensorData.LidarData) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
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
        client.receiveRaspiData();
    }

    return 0;
}


