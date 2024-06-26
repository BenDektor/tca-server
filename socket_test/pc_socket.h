#ifndef PC_SOCKET_H
#define PC_SOCKET_H

#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>
#include "json_lib/json.hpp"


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

    bool sendMessage(const char* message);
    void processIncomingMessages();
    SensorData receiveJsonData(const std::string& jsonData);
    cv::Mat receiveFrame(const std::vector<char>& imgData);

private:
    bool setupConnection(const std::string& ip_address);
    void closeConnection();
    bool sendTestMessage();
    std::vector<std::string> extractMessages(const std::string& data, const std::string& startDelimiter, const std::string& endDelimiter);

    int clientSocket;
    struct sockaddr_in serverAddr;
};

#endif // PC_SOCKET_H