#ifndef PC_SOCKET_H
#define PC_SOCKET_H

#include <string>
#include <netinet/in.h>
#include <thread>

class Socket {
public:
    Socket(const std::string& serverIp, int port1, int port2);
    ~Socket();
    bool sendMessage(const std::string& message, int index);
    bool receiveMessage(std::string& message, int index);

    std::thread sensorDataThread, fahrzeugbefehleThread;


private:
    int clientSocketSensorData;
    int clientSocketFahrzeugbefehle;
    struct sockaddr_in serverAddrSensorData;
    struct sockaddr_in serverAddrFahrzeugbefehle;


    void SensorDataPort();
    void FahrzeugbefehlePort();
};


#endif // PC_SOCKET_H