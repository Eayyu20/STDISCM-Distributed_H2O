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

bool timerStarted = false;
clock_t starttime, endtime;

// Sentinel value for ending the program
const int SENTINEL_VALUE = -99;

// SOCKET struct to include client type
struct ClientSocket {
    SOCKET socket;
    char type;  // 'H' for hydrogen, 'O' for oxygen
};

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

    if (id != SENTINEL_VALUE) {
        logRequest(id, "bonded", client);
    }
}

string receiveHydrogen(SOCKET clientSocket) {
    int requestNumber;
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&requestNumber), sizeof(requestNumber), 0);
    if (bytesReceived <= 0) {
        return "";
    }

    // Start timer upon first request
    if (!timerStarted) {
        starttime = clock();
        timerStarted = true;
    }

    // Convert from network byte order to host byte order
    requestNumber = ntohl(requestNumber);

    if (requestNumber == SENTINEL_VALUE) {
        return to_string(requestNumber);
    }

    logRequest(requestNumber, "request", 'H');

    // Receive last Hyrdogen sent and convert the integer to a string for consistent return type
    return "H" + to_string(requestNumber);
}

string receiveOxygen(SOCKET clientSocket) {
    int requestNumber;
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&requestNumber), sizeof(requestNumber), 0);
    if (bytesReceived <= 0) {
        return "";
    }

    // Start timer upon first request
    if (!timerStarted) {
        starttime = clock();
        timerStarted = true;
    }

    // Convert from network byte order to host byte order
    requestNumber = ntohl(requestNumber);

    if (requestNumber == SENTINEL_VALUE) {
        return to_string(requestNumber);
    }

    logRequest(requestNumber, "request", 'O');

    // Receive last Oxygen sent and convert the integer to a string for consistent return type
    return "O" + to_string(requestNumber);
}

// Function to accept and identify client type
ClientSocket acceptAndIdentifyClient(SOCKET serverSocket) {
    ClientSocket client;
    client.socket = accept(serverSocket, nullptr, nullptr);
    if (client.socket == INVALID_SOCKET) {
        cerr << "Accept failed." << endl;
        return client;
    }

    // Read the first byte to identify the client type
    char clientType;
    int bytesReceived = recv(client.socket, &clientType, 1, 0);
    if (bytesReceived > 0 && (clientType == 'H' || clientType == 'O')) {
        client.type = clientType;
    }
    else {
        cerr << "Failed to identify client type." << endl;
        closesocket(client.socket);
        client.socket = INVALID_SOCKET;
    }

    return client;
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
    ClientSocket HClient = { INVALID_SOCKET, 0 };
    ClientSocket OClient = { INVALID_SOCKET, 0 };

    // Accept two client connections
    thread connectH([&]() {
        HClient = acceptAndIdentifyClient(serverSocket);
    });

    thread connectO([&]() {
        OClient = acceptAndIdentifyClient(serverSocket);
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
            string rh = receiveHydrogen(HClient.socket);
            if (!rh.empty()) {
                lock_guard<mutex> lock(mtx);
                Hq.emplace_back(rh);
            }
        }
    });
    hydrogenThread.detach();

    thread oxygenThread([&]() {
        while (true) {
            string ro = receiveOxygen(OClient.socket);
            if (!ro.empty()) {
                lock_guard<mutex> lock(mtx);
                Oq.emplace_back(ro);
            }
        }
    });
    oxygenThread.detach();

    //Bonding

    boolean finished = false;

    while (!finished) {
        string hm1 = "-1", hm2 = "-1", om = "-1";
        bool molecules_available = false;
        {
            lock_guard<mutex> lock(mtx);
            molecules_available = (Hq.size() >= 2 && !Oq.empty());
        }
        if (molecules_available) {
            {
                lock_guard<mutex> lock(mtx);
                hm1 = Hq[0];
                hm2 = Hq[1];
                om = Oq[0];

                Hq.erase(Hq.begin(), Hq.begin() + 2);
                Oq.erase(Oq.begin());

                sendConfirmation(HClient.socket, stoi(hm1.substr(1)), 'H');

                sendConfirmation(HClient.socket, stoi(hm2.substr(1)), 'H');

                sendConfirmation(OClient.socket, stoi(om.substr(1)), 'O');
            }
        }

        if (!Hq.empty() && !Oq.empty()) {
            if (Hq[0] == "-99" && Oq[0] == "-99") {
                finished = true;
            }
        }
    }

    // After the loop, send the sentinel value to both clients
    sendConfirmation(HClient.socket, SENTINEL_VALUE, 'H');
    sendConfirmation(OClient.socket, SENTINEL_VALUE, 'O');

    // stop timer
    endtime = clock();

    //Calculate Time Taken
    double time_taken = double(endtime - starttime) / double(CLOCKS_PER_SEC);
    cout << "Time taken by program is : " << fixed
        << time_taken << setprecision(5);
    cout << " sec " << endl;


    // Close sockets
    closesocket(HClient.socket);
    closesocket(OClient.socket);
    closesocket(serverSocket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}