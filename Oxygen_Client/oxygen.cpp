#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <chrono>
#include <string>
#include <thread>

using namespace std;

#pragma comment(lib, "ws2_32.lib")
constexpr int BUFFER_SIZE = 1024;

const int SENTINEL_VALUE = -99;
atomic<bool> finished(false);

void logRequest(int id, const char* action) {
    auto now = chrono::system_clock::now();
    time_t timestamp = chrono::system_clock::to_time_t(now);
    cout << "O" << id << ", " << action << ", " << ctime(&timestamp) << endl;
}

void listeningThread(SOCKET clientSocket) {
    while (true) {
        int bondedNumber;
        int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&bondedNumber), sizeof(bondedNumber), 0);
        if (bytesReceived <= 0) {
            cerr << "Failed to receive bonded number: " << WSAGetLastError() << endl;
            finished = true;
            break; // Exiting the thread due to error
        }

        // Convert from network byte order to host byte order
        bondedNumber = ntohl(bondedNumber);
        if (bondedNumber == SENTINEL_VALUE) {
            cout << "Received sentinel value. Ending session." << endl;
            finished = true;
            break;
        }

        // Process the received bonded number 
        logRequest(bondedNumber, "bonded");
    }
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed." << endl;
        return 1;
    }

    // Create socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed." << endl;
        WSACleanup();
        return 1;
    }

    // Connect to the server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("10.147.17.27");  // Server IP address
    serverAddr.sin_port = htons(12345);

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Connection failed." << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server." << endl;

    int M;

    // Create and start the listening thread
    std::thread listeningThread([clientSocket]() {
        listeningThread(clientSocket);
        });

    // Detach the thread to run independently
    listeningThread.detach();

    cout << "Enter the number of oxygen bond requests (M): ";
    cin >> M;

    for (int i = 1; i <= M; ++i) {
        // Sending bond request to the server
        int requestMessage = htonl(i);  // Convert to network byte order

        if (send(clientSocket, (char*)&requestMessage, sizeof(requestMessage), 0) == SOCKET_ERROR) {
            cerr << "Failed to send request: " << WSAGetLastError() << endl;
            break;  // Exit loop on send failure
        }

        logRequest(i, "request");
    }

    while (!finished) {

    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
