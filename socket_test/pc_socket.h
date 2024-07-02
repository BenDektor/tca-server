#ifndef PC_SOCKET_H
#define PC_SOCKET_H

#include <string>
#include <netinet/in.h>
#include <thread>
#include <opencv2/opencv.hpp>
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
    Socket(const std::string& serverIp, int portSensorDaten, int portFahrzeugbefehle, int portKameraBilder);
    ~Socket();
    bool sendMessage(const std::string& message, int index);
    bool receiveMessage(std::string& message, int index);

    std::thread sensorDataThread, fahrzeugbefehleThread, kameraBilderThread;

    cv::Mat imageData;
    SensorData sensorData;

private:
    int clientSocketSensorData;
    int clientSocketFahrzeugbefehle;
    int clientSocketKameraBilder;
    struct sockaddr_in serverAddrSensorData;
    struct sockaddr_in serverAddrFahrzeugbefehle;
    struct sockaddr_in serverAddrKameraBilder;

    bool isJson(const std::string& str);
    SensorData parseJsonData(const std::string& jsonData);

    void SensorDataPort();
    void FahrzeugbefehlePort();
    void KameraBilderPort();

    cv::Mat receiveFrame();
    SensorData receiveJsonData();
};


#endif // PC_SOCKET_H