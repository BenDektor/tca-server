#ifndef PC_SOCKET_H
#define PC_SOCKET_H

#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <vector>
#include "json_lib/json.hpp"

using namespace boost::asio;
using ip::tcp;
using namespace cv;

struct SensorData {
    int Compass;
    double DistanceLeft;
    double DistanceRear;
    double DistanceRight;
    std::vector<int> LidarData;
    int LightSensor;
    double TotalSpeed;
};

class Socket {
public:
    Socket(const std::string& ip_address);
    ~Socket();

    bool sendTestMessage();
    cv::Mat receiveFrame();
    SensorData receiveJsonData();

private:
    boost::asio::io_context io_context_;
    tcp::socket socket_;

    bool setupConnection(const std::string& ip_address);
    bool sendMessage(const char* message);
    bool receiveMessage(char* buffer, int bufferSize);
    void closeConnection();

    bool isJson(const std::string& str);
    SensorData parseJsonData(const std::string& jsonData);
};

#endif // PC_SOCKET_H
