#include "pc_socket.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <atomic>

Socket::Socket(const std::string& serverIp, int port1, int port2, int port3) {
    if (!setupSocket(port1, serverAddrs[0], clientSockets[0]) ||
        !setupSocket(port2, serverAddrs[1], clientSockets[1]) ||
        !setupSocket(port3, serverAddrs[2], clientSockets[2])) {
        std::cerr << "Error: Unable to set up UDP sockets\n";
    }

    // Configure server addresses
    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddrs[0].sin_addr) <= 0 ||
        inet_pton(AF_INET, serverIp.c_str(), &serverAddrs[1].sin_addr) <= 0 ||
        inet_pton(AF_INET, serverIp.c_str(), &serverAddrs[2].sin_addr) <= 0) {
        std::cerr << "Error: Invalid server IP address\n";
    }
}

Socket::~Socket() {
    for (int i = 0; i < 3; ++i) {
        close(clientSockets[i]);
    }
}

bool Socket::setupSocket(int port, struct sockaddr_in& serverAddr, int& clientSocket) {
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Error: Unable to create UDP socket\n";
        return false;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    return true;
}

bool Socket::sendMessage(const std::string& message, int index) {
    if (index < 0 || index >= 3) {
        std::cerr << "Error: Invalid index for sending message\n";
        return false;
    }

    int bytesSent = sendto(clientSockets[index], message.c_str(), message.length(), 0,
                           (const struct sockaddr *)&serverAddrs[index], sizeof(serverAddrs[index]));

    if (bytesSent < 0) {
        std::cerr << "Error: Unable to send message on UDP socket\n";
        return false;
    }

    return true;
}

bool Socket::receiveMessage(std::string& message, int index) {
    if (index < 0 || index >= 3) {
        std::cerr << "Error: Invalid index for receiving message\n";
        return false;
    }

    char buffer[1024];
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen = sizeof(serverAddr);
    memset(buffer, 0, sizeof(buffer));

    int bytesReceived = recvfrom(clientSockets[index], buffer, sizeof(buffer), 0,
                                 (struct sockaddr *)&serverAddr, &serverAddrLen);

    if (bytesReceived < 0) {
        std::cerr << "Error: Unable to receive message on UDP socket\n";
        return false;
    }

    message = std::string(buffer);
    return true;
}


const std::string SERVER_IP = "172.16.8.137"; // Replace with the actual server IP
const int PORT1 = 3000;
const int PORT2 = 3001;
const int PORT3 = 3002;

std::atomic<bool> running(true);

void receiveMessages(Socket& socket, int index) {
    while (running.load()) {
        std::string message;
        if (socket.receiveMessage(message, index)) {
            std::cout << "Received message on port " << (index == 0 ? PORT1 : PORT2) << ": " << message << std::endl;
        } else {
            std::cerr << "Error: Unable to receive message on port " << (index == 0 ? PORT1 : PORT2) << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void sendMessages(Socket& socket) {
    int messageCount = 0;
    while (running.load()) {
        std::string message = "Test Message " + std::to_string(++messageCount);
        if (!socket.sendMessage(message, 2)) {
            std::cerr << "Error: Unable to send message to port " << PORT3 << std::endl;
        } else {
            std::cout << "Sent message to port " << PORT3 << ": " << message << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    Socket socket(SERVER_IP, PORT1, PORT2, PORT3);

    std::thread receiver1(receiveMessages, std::ref(socket), 0);
    std::thread receiver2(receiveMessages, std::ref(socket), 1);
    std::thread sender(sendMessages, std::ref(socket));

    std::cout << "Press Enter to stop...\n";
    std::cin.get();
    running.store(false);

    receiver1.join();
    receiver2.join();
    sender.join();

    return 0;
}