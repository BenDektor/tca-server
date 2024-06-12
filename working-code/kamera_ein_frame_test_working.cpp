#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <fstream>


int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error: Unable to create socket\n";
        return 1;
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("192.168.193.223"); // IP of Raspberry Pi
    serverAddr.sin_port = htons(8080); // Use the same port as the Raspberry Pi side

    // Connect to server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Unable to connect to server\n";
        close(clientSocket);
        return 1;
    }

    // Receive message
    char buffer[1024] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived < 0) {
        std::cerr << "Error: Unable to receive message\n";
        close(clientSocket);
        return 1;
    } else {
        std::cout << "Message from Raspberry Pi: " << buffer << std::endl;
    }

    // Send response message
    const char* responseMessage = "Hello from PC!";
    if (send(clientSocket, responseMessage, strlen(responseMessage), 0) < 0) {
        std::cerr << "Error: Unable to send message\n";
        close(clientSocket);
        return 1;
    }


 // Receive frame size
    int frameSize = 0;
    if (recv(clientSocket, &frameSize, sizeof(frameSize), 0) < 0) {
        std::cerr << "Error: Failed to receive frame size\n";
        close(clientSocket);
        return 1;
    }


    // Allocate buffer for frame data based on received size
    std::vector<char> framebuffer(frameSize);

    // Receive the actual frame data
    int framebytesReceived = recv(clientSocket, framebuffer.data(), frameSize, MSG_WAITALL);
    if (framebytesReceived < 0) {
        std::cerr << "Error: Unable to receive frame data\n";
        close(clientSocket);
        return 1;
    } else {
        std::cout << "Frame received successfully!" << std::endl;

        // Optionally save or display the image
        std::ofstream outFile("received_frame.jpg", std::ios::binary);
        outFile.write(framebuffer.data(), frameSize);
        outFile.close();
    }

    




    // Close socket
    close(clientSocket);

    return 0;
}