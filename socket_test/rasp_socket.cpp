#include <iostream>

#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>



int main() {

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error: Unable to create socket\n";
        return 1;
    }


    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080); // Choose any available port


    // Bind socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Unable to bind socket\n";
        return 1;
    }


    // Listen for incoming connections
    if (listen(serverSocket, 1) < 0) {
        std::cerr << "Error: Unable to listen on socket\n";
        return 1;
    }


    // Accept incoming connection
    if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
        std::cerr << "Error: Unable to accept connection\n";
        return 1;
    }


    // Send message
    const char* message = "Hello from Raspberry Pi!";

    if (send(clientSocket, message, strlen(message), 0) < 0) {
        std::cerr << "Error: Unable to send message\n";
    }


    // Close sockets
    close(clientSocket);
    close(serverSocket);

    return 0;

}
