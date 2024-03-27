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
#define LIMIT 100000000
#pragma comment(lib, "ws2_32.lib")

void logRequest(int id, const char* action, char client) {
    auto now = chrono::system_clock::now();
    time_t timestamp = chrono::system_clock::to_time_t(now);
    cout << client << id << ", " << action << ", " << ctime(&timestamp) << endl;
}

void sendConfirmation(SOCKET clientSocket, int id, char client) {
    int bondedNumber = htonl(id);

    if (send(clientSocket, (char*)&bondedNumber, sizeof(bondedNumber), 0) == SOCKET_ERROR) {
        cerr << "Failed to send confirmation: " << clientSocket << WSAGetLastError() << endl;
    }
    logRequest(id, "bonded", client);
}

string receiveHydrogen(SOCKET clientSocket) {
    int requestNumber;
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&requestNumber), sizeof(requestNumber), 0);
    if (bytesReceived <= 0) {
        return "";
    }

    // Convert from network byte order to host byte order
    requestNumber = ntohl(requestNumber);

    logRequest(requestNumber, "request", 'H');

    // Convert the integer to a string for consistent return type
    return "H" + to_string(requestNumber);
}

string receiveOxygen(SOCKET clientSocket) {
    int requestNumber;
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&requestNumber), sizeof(requestNumber), 0);
    if (bytesReceived <= 0) {
        return "";
    }

    // Convert from network byte order to host byte order
    requestNumber = ntohl(requestNumber);

    logRequest(requestNumber, "request", 'O');

    // Convert the integer to a string for consistent return type
    return "O" + to_string(requestNumber);
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
        }
    });

    thread connectO([&]() {
        OClient = accept(serverSocket, nullptr, nullptr);
        if (OClient == INVALID_SOCKET) {
            cerr << "Accept failed." << endl;
            closesocket(serverSocket);
            WSACleanup();
        }
    });
     
    connectH.join();
    connectO.join();

    std::cout << "Connections established." << std::endl;
    std::vector<string> Hq;
    std::vector<string> Oq;

    //Threads for accepting atoms from clients
    mutex mtx;
    
    thread hydrogenThread([&]() {
        while (true) {
            string rh = receiveHydrogen(HClient);
            if (!rh.empty()) {
                lock_guard<mutex> lock(mtx);
                Hq.emplace_back(rh);
            }
        }
    });
    hydrogenThread.detach();

    thread oxygenThread([&]() {
        while (true) {
            string ro = receiveOxygen(OClient);
            if (!ro.empty()) {
                lock_guard<mutex> lock(mtx);
                Oq.emplace_back(ro);
            }
        }
    });
    oxygenThread.detach();

    //Bonding
    while (true) {
        bool molecules_available = false;
        {
            lock_guard<mutex> lock(mtx);
            molecules_available = (Hq.size() >= 2 && !Oq.empty());
        }
        if (molecules_available) {
            string hm1, hm2, om;
            {
                lock_guard<mutex> lock(mtx);
                hm1 = Hq[0];
                hm2 = Hq[1];
                om = Oq[0];

                Hq.erase(Hq.begin(), Hq.begin() + 2);
                Oq.erase(Oq.begin());

                sendConfirmation(HClient, stoi(hm1.substr(1)), 'H');

                sendConfirmation(HClient, stoi(hm2.substr(1)), 'H');

                sendConfirmation(OClient, stoi(om.substr(1)), 'O');
            }
        }
    }

    // Close sockets
    closesocket(HClient);
    closesocket(OClient);
    closesocket(serverSocket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}