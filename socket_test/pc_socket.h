#ifndef PC_SOCKET_H
#define PC_SOCKET_H

#include <string>
#include <netinet/in.h>

class Socket {
public:
    Socket(const std::string& serverIp, int port1, int port2, int port3);
    ~Socket();
    bool sendMessage(const std::string& message, int index);
    bool receiveMessage(std::string& message, int index);

private:
    bool setupSocket(int port, struct sockaddr_in& serverAddr, int& clientSocket);
    int clientSockets[3];
    struct sockaddr_in serverAddrs[3];
};


#endif // PC_SOCKET_H