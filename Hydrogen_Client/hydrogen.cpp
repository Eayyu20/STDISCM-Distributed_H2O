#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <chrono>
#include <string>

using namespace std;

#pragma comment(lib, "ws2_32.lib")
constexpr int BUFFER_SIZE = 1024;

void logRequest(int id, const char* action) {
    auto now = chrono::system_clock::now();
    time_t timestamp = chrono::system_clock::to_time_t(now);
    cout << "H" << id << ", " << action << ", " << ctime(&timestamp) << endl;
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
    serverAddr.sin_addr.s_addr = inet_addr("172.20.10.3");  // Server IP address
    serverAddr.sin_port = htons(12345);

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Connection failed." << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server." << endl;

    int N;
    cout << "Enter the number of hydrogen bond requests (N): ";
    cin >> N;

    for (int i = 1; i <= N; ++i) {
        // Sending bond request to the server
        int requestMessage = htonl(i);  // Convert to network byte order

        if (send(clientSocket, (char*)&requestMessage, sizeof(requestMessage), 0) == SOCKET_ERROR) {
            cerr << "Failed to send request: " << WSAGetLastError() << endl;
            break;  // Exit loop on send failure
        }

        logRequest(i, "request");
    }

    // Receive bond confirmations
    for (int i = 1; i <= N; ++i) {
        char buffer[BUFFER_SIZE] = { 0 };
        if (recv(clientSocket, buffer, BUFFER_SIZE, 0) == SOCKET_ERROR) {
            cerr << "Failed to receive confirmation: " << WSAGetLastError() << endl;
            break;  // Exit loop on receive failure
        }
        logRequest(i, "bonded");
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
