#include "pc_socket.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

Socket::Socket(const std::string& serverIp, int portSensorDaten, int portFahrzeugbefehle, int portKameraBilder) {
    // Create UDP sockets
    if ((clientSocketSensorData = socket(AF_INET, SOCK_DGRAM, 0)) < 0 || (clientSocketFahrzeugbefehle = socket(AF_INET, SOCK_DGRAM, 0)) < 0 || (clientSocketKameraBilder = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Error: Unable to create UDP socket\n";
        return;
    }

    // Configure server addresses
    memset(&serverAddrSensorData, 0, sizeof(serverAddrSensorData));
    serverAddrSensorData.sin_family = AF_INET;
    serverAddrSensorData.sin_port = htons(portSensorDaten);

    memset(&serverAddrFahrzeugbefehle, 0, sizeof(serverAddrFahrzeugbefehle));
    serverAddrFahrzeugbefehle.sin_family = AF_INET;
    serverAddrFahrzeugbefehle.sin_port = htons(portFahrzeugbefehle);

    memset(&serverAddrKameraBilder, 0, sizeof(serverAddrKameraBilder));
    serverAddrKameraBilder.sin_family = AF_INET;
    serverAddrKameraBilder.sin_port = htons(portKameraBilder);


    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddrSensorData.sin_addr) <= 0 ||
        inet_pton(AF_INET, serverIp.c_str(), &serverAddrFahrzeugbefehle.sin_addr) <= 0 ||
        inet_pton(AF_INET, serverIp.c_str(), &serverAddrKameraBilder.sin_addr) <= 0) {
        std::cerr << "Error: Invalid server IP address\n";
        return;
    }

    // Start threads
    sensorDataThread = std::thread(&Socket::SensorDataPort, this);
    fahrzeugbefehleThread = std::thread(&Socket::FahrzeugbefehlePort, this);
    kameraBilderThread = std::thread(&Socket::KameraBilderPort, this);
}

Socket::~Socket() {
    sensorDataThread.join();
    fahrzeugbefehleThread.join();
    kameraBilderThread.join();
    close(clientSocketKameraBilder);
    close(clientSocketFahrzeugbefehle);
    close(clientSocketSensorData);
}

void Socket::SensorDataPort() {
    while (true) {
        if (!sendMessage("Request Sensor Data Message", 0)) {
            std::cerr << "Error: Unable to send message on port 1\n";
        }

        std::string message;
        if (!receiveMessage(message, 0)) {
            std::cerr << "Error: Unable to receive message on port 1\n";
        } else {
            std::cout << "Received sensordata 1: " << message << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust the delay as needed
    }
}

void Socket::KameraBilderPort() {
    while (true) {
        if (!sendMessage("Request Image Data Message", 2)) {
            std::cerr << "Error: Unable to send message on port 1\n";
        }

        std::string message;
        if (!receiveMessage(message, 2)) {
            std::cerr << "Error: Unable to receive message on port 1\n";
        } else {
            std::cout << "Received image data 1: " << message << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust the delay as needed
    }
}

void Socket::FahrzeugbefehlePort() {
    while (true) {
        if (!sendMessage("Sending Fahrzeugbefehle ...", 1)) {
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
        serverAddr = serverAddrSensorData;
        clientSocket = clientSocketSensorData;
    } else if (index == 1) {
        serverAddr = serverAddrFahrzeugbefehle;
        clientSocket = clientSocketFahrzeugbefehle;
    } else if (index == 2) {
        serverAddr = serverAddrKameraBilder;
        clientSocket = clientSocketKameraBilder;
    }
     else {
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
        clientSocket = clientSocketSensorData;
    } else if (index == 1) {
        clientSocket = clientSocketFahrzeugbefehle;
    } else if (index == 2) {
        clientSocket = clientSocketKameraBilder;
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
const int PORT3 = 3002;


int main() {
    Socket socket(SERVER_IP, PORT1, PORT2, PORT3); // Initialize socket with server IP and two ports


    return 0;
}