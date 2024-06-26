#ifndef PC_SOCKET_H
#define PC_SOCKET_H

#include <iostream>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <opencv2/opencv.hpp>
#include <sstream>
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

    bool setupConnection(const std::string& ip_address);
    bool sendMessage(const char* message);
    bool receiveMessage(char* buffer, int bufferSize);
    void closeConnection();
    bool sendTestMessage();
    void receiveRaspiData();

private:
    int clientSocket;
    struct sockaddr_in serverAddr;

    bool isJson(const std::string& str);
    SensorData parseJsonData(const std::string& jsonData);
    std::vector<uchar> base64_decode(const std::string &base64str);

    void handleSensorData(std::istringstream& iss);
    void handleImageData(std::istringstream& iss);
};

#endif // PC_SOCKET_H
