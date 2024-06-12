#include <opencv2/opencv.hpp>

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <fstream>

class Socket{
    private:
        int clientSocket;
        struct sockaddr_in serverAddr;

    public:
        Socket(const std::string& ip_address);
        
        ~Socket();

        bool setupConnection(const std::string& ip_address);
        bool sendMessage(const char* message);
        bool receiveMessage(char* buffer, int bufferSize);
        void closeConnection();
        bool sendTestMessage();
        cv::Mat receiveFrame();
};