#include <iostream>
#include <winsock2.h>
#include <vector>
#include <iomanip>
#include <thread>
#include <mutex>

using namespace std;
mutex mtx;
#define LIMIT 100000000
#pragma comment(lib, "ws2_32.lib")

void sendConfirmation(SOCKET clientSocket, const string& message) {
    // Convert the string to network byte order
    size_t lengthNetwork = htonl(message.size());

    // Send the size of the string first
    int bytesSent = send(clientSocket, reinterpret_cast<const char*>(&lengthNetwork), sizeof(lengthNetwork), 0);
    if (bytesSent == SOCKET_ERROR) {
        cerr << "Failed to send message length: " << WSAGetLastError() << endl;
        return;
    }

    // Then, send the string
    bytesSent = send(clientSocket, message.c_str(), message.size(), 0);
    if (bytesSent == SOCKET_ERROR) {
        cerr << "Failed to send message: " << WSAGetLastError() << endl;
        return;
    }

    //print that you sent <something> at <timestamp>
}

string receiveHydrogen(SOCKET clientSocket) {
    // Receive the length of the string
    size_t lengthNetwork;
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&lengthNetwork), sizeof(lengthNetwork), 0);
    if (bytesReceived <= 0) {
        cerr << "Failed to receive string length." << endl;
        return ""; // Return an empty string in case of error
    }
    size_t length = ntohl(lengthNetwork);

    // Receive the string
    vector<char> buffer(length);
    bytesReceived = recv(clientSocket, buffer.data(), length, 0);
    if (bytesReceived <= 0) {
        cerr << "Failed to receive string." << endl;
        return ""; // Return an empty string in case of error
    }
    //print that you received <something> at <timestamp>

    // Convert the buffer to a string and return it
    return string(buffer.begin(), buffer.end());
}

string receiveOxygen(SOCKET clientSocket) {
    // Receive the length of the string
    size_t lengthNetwork;
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&lengthNetwork), sizeof(lengthNetwork), 0);
    if (bytesReceived <= 0) {
        cerr << "Failed to receive string length." << endl;
        return ""; // Return an empty string in case of error
    }
    size_t length = ntohl(lengthNetwork);

    // Receive the string
    vector<char> buffer(length);
    bytesReceived = recv(clientSocket, buffer.data(), length, 0);
    if (bytesReceived <= 0) {
        cerr << "Failed to receive string." << endl;
        return ""; // Return an empty string in case of error
    }
    //print that you received <something> at <timestamp>

    // Convert the buffer to a string and return it
    return string(buffer.begin(), buffer.end());
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // Create socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    // Bind the socket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12345);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Waiting for connections..." << std::endl;

    // Accept two client connections
    SOCKET Hclient = accept(serverSocket, nullptr, nullptr);
    if (Hclient == INVALID_SOCKET) {
        std::cerr << "Accept failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    SOCKET OClient = accept(serverSocket, nullptr, nullptr);
    if (OClient == INVALID_SOCKET) {
        std::cerr << "Accept failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connections established." << std::endl;
    std::vector<String> Hq;
    std::vector<String> Oq;
    //bool isReady;
    
    //thread that receives hydrogen and appends to Hq
    std::vector<string> rh = receiveHydrogen(Hclient);
    //dont forget mutex when appending

    //thread that receives oxygen and appends to Oq
    std::vector<string> ro = receiveOxygen(OClient);
    //dont forget mutex when appending

    // thread that continually checks the size of Hq and Oq, if Hq >= 2 and Oq, it then sends a confirmation to the two clients using the sendConfirmation function
    // Close sockets
    closesocket(Hclient);
    closesocket(OClient);
    closesocket(serverSocket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}