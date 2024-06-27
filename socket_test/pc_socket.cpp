#include "pc_socket.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

Socket::Socket(const std::string& serverIp, int port1, int port2) {
    // Create UDP sockets
    if ((clientSocket1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0 || (clientSocket2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Error: Unable to create UDP socket\n";
        return;
    }

    // Configure server addresses
    memset(&serverAddr1, 0, sizeof(serverAddr1));
    serverAddr1.sin_family = AF_INET;
    serverAddr1.sin_port = htons(port1);

    memset(&serverAddr2, 0, sizeof(serverAddr2));
    serverAddr2.sin_family = AF_INET;
    serverAddr2.sin_port = htons(port2);

    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddr1.sin_addr) <= 0 ||
        inet_pton(AF_INET, serverIp.c_str(), &serverAddr2.sin_addr) <= 0) {
        std::cerr << "Error: Invalid server IP address\n";
        return;
    }

    // Start threads
    thread1 = std::thread(&Socket::sendReceivePort1, this);
    thread2 = std::thread(&Socket::sendPort2, this);
}

Socket::~Socket() {
    thread1.join();
    thread2.join();
    close(clientSocket1);
    close(clientSocket2);
}

void Socket::sendReceivePort1() {
    while (true) {
        if (!sendMessage("Test Message 1 Bene", 0)) {
            std::cerr << "Error: Unable to send message on port 1\n";
        }

        std::string message;
        if (!receiveMessage(message, 0)) {
            std::cerr << "Error: Unable to receive message on port 1\n";
        } else {
            std::cout << "Received message 1: " << message << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust the delay as needed
    }
}

void Socket::sendPort2() {
    while (true) {
        if (!sendMessage("Test Message 2 Bene", 1)) {
            std::cerr << "Error: Unable to send message on port 2\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust the delay as needed
    }
}

bool Socket::sendMessage(const std::string& message, int index) {
    struct sockaddr_in serverAddr;
    int clientSocket;

    // Determine which socket to use based on index
    if (index == 0) {
        serverAddr = serverAddr1;
        clientSocket = clientSocket1;
    } else if (index == 1) {
        serverAddr = serverAddr2;
        clientSocket = clientSocket2;
    } else {
        std::cerr << "Error: Invalid index for sending message\n";
        return false;
    }

    // Send message to server
    int bytesSent = sendto(clientSocket, message.c_str(), message.length(), 0,
                           (const struct sockaddr *)&serverAddr, sizeof(serverAddr));

    if (bytesSent < 0) {
        std::cerr << "Error: Unable to send message on UDP socket\n";
        return false;
    }

    return true;
}

bool Socket::receiveMessage(std::string& message, int index) {
    char buffer[1024]; // Adjust buffer size as necessary
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen = sizeof(serverAddr);
    int clientSocket;

    // Determine which socket to use based on index
    if (index == 0) {
        clientSocket = clientSocket1;
    } else if (index == 1) {
        clientSocket = clientSocket2;
    } else {
        std::cerr << "Error: Invalid index for receiving message\n";
        return false;
    }

    memset(buffer, 0, sizeof(buffer));

    // Receive message from server
    int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0,
                                 (struct sockaddr *)&serverAddr, &serverAddrLen);

    if (bytesReceived < 0) {
        std::cerr << "Error: Unable to receive message on UDP socket\n";
        return false;
    }

    // Convert received data to string
    message = std::string(buffer);
    return true;
}



const std::string SERVER_IP = "172.16.8.137"; // Replace with the actual server IP
const int PORT1 = 3000;
const int PORT2 = 3001;


int main() {
    Socket socket(SERVER_IP, PORT1, PORT2); // Initialize socket with server IP and two ports


    return 0;
}