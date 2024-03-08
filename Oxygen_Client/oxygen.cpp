#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <chrono>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;

#pragma comment(lib, "ws2_32.lib")
constexpr int BUFFER_SIZE = 1024;
mutex mtx;

void logRequest(int id, const char* action) {
    auto now = std::chrono::system_clock::now();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(now);
    std::cout << "(O" << id << ", " << action << ", " << std::ctime(&timestamp) << ")";
}

// Listener thread function
void listenerThread(SOCKET clientSocket, int expectedMessages) {
    for (int i = 1; i <= expectedMessages; ++i) {
        char buffer[BUFFER_SIZE] = { 0 };
        recv(clientSocket, buffer, BUFFER_SIZE, 0);
        logRequest(i, "bonded");
    }
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // Create socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    // Connect to the server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("192.168.1.37"); // Change to the server's IP address
    serverAddr.sin_port = htons(12345);

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    int M;
    std::cout << "Enter the number of Oxygen bond requests (M): ";
    std::cin >> M;

    for (int i = 1; i <= M; ++i) {
        // Sending bond request to the server
        std::string requestMessage = "O" + std::to_string(i);
        send(clientSocket, requestMessage.c_str(), requestMessage.size(), 0);
        logRequest(i, "request");
    }

    for (int i = 1; i <= M; ++i) {
        // Waiting for the confirmation from the server
        char buffer[BUFFER_SIZE] = { 0 };
        recv(clientSocket, buffer, BUFFER_SIZE, 0);
        logRequest(i, "bonded");
    }

    // Start the listener thread
    //std::thread listener(clientSocket, M);

    //for (int i = 1; i <= M; ++i) {
    //    // Sending bond request to the server
    //    std::string requestMessage = "O" + std::to_string(i) + " request";
    //    send(clientSocket, requestMessage.c_str(), requestMessage.size(), 0);
    //    logRequest(i, "request");
    //}

    //listener.join();

    // Close socket
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
