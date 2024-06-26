#include "socket_raspi.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

Socket::Socket(const std::string& serverIp, int port1, int port2) : serverIp(serverIp) {
    clientSockets[0] = 0;
    clientSockets[1] = 0;

    memset(&serverAddrs[0], 0, sizeof(serverAddrs[0]));
    memset(&serverAddrs[1], 0, sizeof(serverAddrs[1]));

    serverAddrs[0].sin_family = AF_INET;
    serverAddrs[0].sin_port = htons(port1);

    serverAddrs[1].sin_family = AF_INET;
    serverAddrs[1].sin_port = htons(port2);

    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddrs[0].sin_addr) <= 0) {
        std::cerr << "Error: Invalid server IP address\n";
    }

    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddrs[1].sin_addr) <= 0) {
        std::cerr << "Error: Invalid server IP address\n";
    }

    // Connect to both ports
    if (!connectToServer(0) || !connectToServer(1)) {
        std::cerr << "Error: Unable to connect to server\n";
    }

    // Send test messages
    if (!sendMessage("Test Message 1", 0) || !sendMessage("Test Message 2", 1)) {
        std::cerr << "Error: Unable to send test messages\n";
    }

    std::string message1 = receiveMessage(0);
    std::string message2 = receiveMessage(1);

    if (message1.empty() || message2.empty()) {
        std::cerr << "Error: Unable to receive messages\n";
        closeConnection();
        throw std::runtime_error("Failed to receive messages");
    }

    std::cout << "Received message 1: " << message1 << std::endl;
    std::cout << "Received message 2: " << message2 << std::endl;
}

Socket::~Socket() {
    closeConnection();
}

bool Socket::connectToServer(int index) {
    // Create socket
    if ((clientSockets[index] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error: Unable to create socket\n";
        return false;
    }

    // Connect to server
    if (connect(clientSockets[index], (struct sockaddr *)&serverAddrs[index], sizeof(serverAddrs[index])) < 0) {
        std::cerr << "Error: Unable to connect to server on port " << ntohs(serverAddrs[index].sin_port) << "\n";
        return false;
    }

    return true;
}

bool Socket::sendMessage(const char* message, int index) {
    // Send message
    if (send(clientSockets[index], message, strlen(message), 0) < 0) {
        std::cerr << "Error: Unable to send message\n";
        return false;
    }
    return true;
}

std::string Socket::receiveMessage(int index) {
    char buffer[21]; // Buffer to store up to 20 characters + 1 for null terminator
    memset(buffer, 0, sizeof(buffer)); // Clear the buffer
    int bytesReceived = recv(clientSockets[index], buffer, sizeof(buffer) - 1, 0); // Leave space for null terminator

    if (bytesReceived < 0) {
        std::cerr << "Error: Unable to receive message, error code " << errno << ": " << strerror(errno) << std::endl;
        return "";
    } else if (bytesReceived == 0) {
        return ""; // If no data is received, return an empty string
    }

    buffer[bytesReceived] = '\0'; // Ensure the string is null-terminated
    return std::string(buffer);
}

void Socket::closeConnection() {
    // Close client sockets if valid
    for (int i = 0; i < 2; i++) {
        if (clientSockets[i] >= 0) {
            if (close(clientSockets[i]) < 0) {
                std::cerr << "Error: Failed to close client socket\n";
            }
            clientSockets[i] = -1;  // Set to -1 to indicate the socket is no longer valid
        }
    }
}



// Define global constants
const std::string SERVER_IP = "172.16.8.137"; // Replace with the actual server IP
const int PORT1 = 3000;
const int PORT2 = 3001;

int main() {
    Socket socket(SERVER_IP, PORT1, PORT2); // Initialize socket with server IP and two ports

    return 0;
}