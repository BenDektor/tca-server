#include "pc_socket.h"



Socket::Socket(const std::string& ip_address) {
    if (!setupConnection(ip_address)) {
        std::cerr << "Error: Unable to establish connection\n";
        return;
    }

    // Receive the test message from the server
    char testMessage[1024] = {0};
    if (!receiveMessage(testMessage, sizeof(testMessage))) {
        std::cerr << "Error: Unable to receive test message\n";
        return;
    }
    std::cout << "Test message from server: " << testMessage << std::endl;


    if (!sendTestMessage()) {
        std::cerr << "Error: Unable to send test message\n";
        return;
    }
}


Socket::~Socket() {
    closeConnection();
}


bool Socket::setupConnection(const std::string& ip_address) {
    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return false;
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip_address.c_str());
    serverAddr.sin_port = htons(8080); // Use the same port as the Raspberry Pi side

    // Connect to server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        return false;
    }

    return true;
}


bool Socket::sendMessage(const char* message) {
    // Send response message
    std::cout << "sendign this message to phil: " << message << std::endl;

    if (send(clientSocket, message, strlen(message), 0) < 0) {
        return false;
    }
    return true;
}

bool Socket::receiveMessage(char* buffer, int bufferSize) {
    int bytesReceived = recv(clientSocket, buffer, bufferSize, 0);
    if (bytesReceived < 0) {
        return false;
    }
    return true;
}


void Socket::closeConnection() {
    close(clientSocket);
}


bool Socket::sendTestMessage() {
    const char* responseMessage = "Hello from PC!";
    return sendMessage(responseMessage);
}


cv::Mat Socket::receiveFrame() {
    // Receive frame size
    int frameSize = 0;
    if (recv(clientSocket, &frameSize, sizeof(frameSize), 0) < 0) {
        std::cerr << "Error: Failed to receive frame size\n";
        closeConnection();
        exit(1);
    }
    
    // Allocate buffer for frame data based on received size
    std::vector<char> framebuffer(frameSize);

    // Receive the actual frame data
    int framebytesReceived = recv(clientSocket, framebuffer.data(), frameSize, MSG_WAITALL);
    if (framebytesReceived < 0) {
        std::cerr << "Error: Unable to receive frame data\n";
        closeConnection();
        exit(1);
    }

    // Decode the frame using OpenCV
    cv::Mat frame = cv::imdecode(cv::Mat(framebuffer), cv::IMREAD_COLOR);

    // Check if frame decoding was successful
    if (frame.empty()) {
        std::cerr << "Error: Failed to decode frame\n";
        closeConnection();
        exit(1);
    }

    return frame;
}



/*int main() {
    // Initialize the socket with the server's IP address
    Socket socket("192.168.193.223");

    // Continuously receive and display frames
    while (true) {
        // Receive a frame from the server
        cv::Mat frame = socket.receiveFrame();

        // Check if frame decoding was successful
        if (frame.empty()) {
            std::cerr << "Error: Failed to receive frame\n";
            break;
        }

        // Display the frame
        cv::imshow("Video Stream", frame);

        // Check for exit key press
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    return 0;
}*/













/*int main() {
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

    // Create a window to display the video stream
    cv::namedWindow("Video Stream", cv::WINDOW_NORMAL);

    // Continuously receive and display frames
    while (true) {
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
        }

        // Decode the frame using OpenCV
        cv::Mat frame = cv::imdecode(cv::Mat(framebuffer), cv::IMREAD_COLOR);

        // Check if frame decoding was successful
        if (frame.empty()) {
            std::cerr << "Error: Failed to decode frame\n";
            close(clientSocket);
            return 1;
        }

        // Display the frame
        cv::imshow("Video Stream", frame);

        // Check for exit key press
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    // Close socket
    close(clientSocket);

    return 0;
}*/