#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <vector>
#include <iomanip>
#include <thread>
#include <mutex>
#include <sstream>

using namespace std;
mutex mtx;
atomic<bool> running(true);  // Used to control the lifetime of threads
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

    // Define SOCKET variables outside the threads
    SOCKET HClient = INVALID_SOCKET;
    SOCKET OClient = INVALID_SOCKET;

    // Accept two client connections
    thread connectH([&]() {
        HClient = accept(serverSocket, nullptr, nullptr);
        if (HClient == INVALID_SOCKET) {
            cerr << "Accept failed." << endl;
            closesocket(serverSocket);
            WSACleanup();
            running = false;  // Stop all threads on accept failure
        }
        });

    thread connectO([&]() {
        OClient = accept(serverSocket, nullptr, nullptr);
        if (OClient == INVALID_SOCKET) {
            cerr << "Accept failed." << endl;
            closesocket(serverSocket);
            WSACleanup();
            running = false;  // Stop all threads on accept failure
        }
        });
     
    connectH.join();
    connectO.join();

    std::cout << "Connections established." << std::endl;
    std::vector<string> Hq;
    std::vector<string> Oq;
    //bool isReady;
    
    mutex mtx;
    std::thread hydrogenThread([&]() {
        while (running) {
            string rh = receiveHydrogen(HClient);
            if (rh.empty()) break;  // Exit loop if receive fails

            lock_guard<mutex> lock(mtx);
            Hq.emplace_back(rh);
            cout << "Received from Hydrogen: " << rh << endl;
        }
        });

    thread oxygenThread([&]() {
        while (running) {
            string ro = receiveOxygen(OClient);
            if (ro.empty()) break;  // Exit loop if receive fails

            lock_guard<mutex> lock(mtx);
            Oq.emplace_back(ro);
            cout << "Received from Oxygen: " << ro << endl;
        }
        });

    // thread that continually checks the size of Hq and Oq, if Hq >= 2 and Oq, it then sends a confirmation to the two clients using the sendConfirmation function
    thread checkThread([&]() {
        while (running) {
            lock_guard<mutex> lock(mtx);
            if (Hq.size() >= 2 && !Oq.empty()) {
                string hm1 = Hq[0];
                string hm2 = Hq[1];
                string om = Oq[0];

                sendConfirmation(HClient, hm1 + ", bonded");
                sendConfirmation(HClient, hm2 + ", bonded");
                sendConfirmation(OClient, om + ", bonded");

                Hq.erase(Hq.begin(), Hq.begin() + 2);
                Oq.erase(Oq.begin());
            }
        }
        });
    
    //join threads
    hydrogenThread.join();
    oxygenThread.join();
    checkThread.join();

    // Close sockets
    closesocket(HClient);
    closesocket(OClient);
    closesocket(serverSocket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}