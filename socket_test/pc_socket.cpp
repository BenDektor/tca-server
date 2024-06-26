#include "pc_socket.h"

Socket::Socket(const std::string& ip_address)
    : socket_(io_context_)
{
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
    try {
        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(ip_address, "3000");

        boost::asio::connect(socket_, endpoints);
    } catch (const boost::system::system_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool Socket::sendMessage(const char* message) {
    try {
        boost::asio::write(socket_, boost::asio::buffer(message, std::strlen(message)));
    } catch (const boost::system::system_error& e) {
        std::cerr << "Error sending message: " << e.what() << std::endl;
        return false;
    }

    std::cout << "Socket: sending this message to raspi: " << message << std::endl;
    return true;
}

bool Socket::receiveMessage(char* buffer, int bufferSize) {
    try {
        size_t bytesRead = boost::asio::read(socket_, boost::asio::buffer(buffer, bufferSize));
        if (bytesRead == 0) {
            return false;
        }
    } catch (const boost::system::system_error& e) {
        std::cerr << "Error receiving message: " << e.what() << std::endl;
        return false;
    }

    return true;
}

void Socket::closeConnection() {
    socket_.close();
}

bool Socket::sendTestMessage() {
    const char* responseMessage = "Hello from PC!";
    return sendMessage(responseMessage);
}

cv::Mat Socket::receiveFrame() {
    // Receive frame size
    int frameSize = 0;
    boost::asio::read(socket_, boost::asio::buffer(&frameSize, sizeof(frameSize)));

    // Allocate buffer for frame data based on received size
    std::vector<char> framebuffer(frameSize);
    boost::asio::read(socket_, boost::asio::buffer(framebuffer));

    // Decode the frame using OpenCV
    cv::Mat frame = cv::imdecode(cv::Mat(framebuffer), cv::IMREAD_COLOR);
    if (frame.empty()) {
        std::cerr << "Error: Failed to decode frame\n";
    }

    return frame;
}

SensorData Socket::receiveJsonData() {
    char buffer[4096];
    SensorData sensorData;
    sensorData.Compass = -1; // Default value to indicate failure

    size_t bytesRead = boost::asio::read(socket_, boost::asio::buffer(buffer));
    if (bytesRead == 0) {
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



int main() {
    std::string serverIp = "172.16.8.137";
    Socket client(serverIp);

    // Receiving JSON data first
    while (true) {
        std::cout << "test" << std::endl;
        SensorData sensorData = client.receiveJsonData();
        if (sensorData.Compass == -1) { // Assuming -1 indicates a failed JSON parse or empty data
            std::cout << "Empty sensor data" <<std::endl;
        }
        else{
            std::cout << "Compass: " << sensorData.Compass << ", Distance Left: " << sensorData.DistanceLeft
                    << ", Distance Rear: " << sensorData.DistanceRear << ", Distance Right: " << sensorData.DistanceRight
                    << ", Light Sensor: " << sensorData.LightSensor << ", Total Speed: " << sensorData.TotalSpeed << "\n";
            std::cout << "Lidar Values (Size: " << sensorData.LidarData.size() << "): ";
            for (int value : sensorData.LidarData) {
                std::cout << value << " ";
            }
            std::cout << std::endl;
        }
     

        //std::this_thread::sleep_for(std::chrono::milliseconds(300));

        // Receiving and displaying a frame using OpenCV
        /*cv::Mat frame = client.receiveFrame();
        if (!frame.empty()) {
            cv::imshow("Received Frame", frame);
            cv::waitKey(1); // Display the frame
        }*/
    }

    return 0;
}