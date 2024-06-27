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

    std::thread thread1, thread2;


private:
    int clientSocket1;
    int clientSocket2;
    struct sockaddr_in serverAddr1;
    struct sockaddr_in serverAddr2;


    void sendReceivePort1();
    void sendPort2();
};


#endif // PC_SOCKET_H