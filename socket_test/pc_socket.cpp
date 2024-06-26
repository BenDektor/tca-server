#include "pc_socket.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

Client::Client(const char *ip, int port1, int port2) : ip(ip), port1(port1), port2(port2) {}

void Client::connectToPort(int port) {
    int sockfd;
    struct sockaddr_in server;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(1);
    }

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_port = htons(port);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    // Connection successful
    std::cout << "Connected to port " << port << std::endl;

    // You can now send/receive data over sockfd
    close(sockfd);
}

void Client::connectToServer() {
    connectToPort(port1);
    connectToPort(port2);
}

int main() {
    const char *ip = "172.16.8.137"; // Replace with Raspberry Pi's IP address
    int port1 = 3000; // Example port 1
    int port2 = 3001; // Example port 2

    Client client(ip, port1, port2);
    client.connectToServer();

    return 0;
}









