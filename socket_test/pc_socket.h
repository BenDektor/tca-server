#ifndef PC_SOCKET_H
#define PC_SOCKET_H

#include <string>
#include <netinet/in.h>
#include <thread>
#include <opencv2/opencv.hpp>

class Socket {
public:
    Socket(const std::string& serverIp, int portSensorDaten, int portFahrzeugbefehle, int portKameraBilder);
    ~Socket();
    bool sendMessage(const std::string& message, int index);
    bool receiveMessage(std::string& message, int index);

    std::thread sensorDataThread, fahrzeugbefehleThread, kameraBilderThread;


private:
    int clientSocketSensorData;
    int clientSocketFahrzeugbefehle;
    int clientSocketKameraBilder;
    struct sockaddr_in serverAddrSensorData;
    struct sockaddr_in serverAddrFahrzeugbefehle;
    struct sockaddr_in serverAddrKameraBilder;


    void SensorDataPort();
    void FahrzeugbefehlePort();
    void KameraBilderPort();

    cv::Mat receiveFrame();
};


#endif // PC_SOCKET_H