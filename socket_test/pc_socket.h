#ifndef PC_SOCKET_H
#define PC_SOCKET_H

#include <string>
#include <netinet/in.h>

class Socket {
public:
    Socket(const std::string& serverIp, int port1, int port2);
    ~Socket();
    bool connectToServer(int index);
    bool sendMessage(const char* message, int index);
    std::string receiveMessage(int index);
    void closeConnection();

private:
    int clientSockets[2];
    struct sockaddr_in serverAddrs[2];
    std::string serverIp;
};


#endif // PC_SOCKET_H