# Distributed H2O Simulation

This project demonstrates a distributed computing approach to bonding Hydrogen and Oxygen to create H2O. It consists of three main components: the Server, Oxygen, and Hydrogen application. The server application must run before the Hydrogen and Oxygen applications.

## Requirements

- Windows OS (due to the use of Winsock2 for networking).
- C++ compiler (e.g., GCC, MSVC)
- Network connectivity between the Server, Oxygen, and Hydrogen applications.

## Setup

1. **Winsock2 Library**: Ensure that the Winsock2 library is accessible to your compiler. This project uses `winsock2.h`, which is typically available on Windows systems.

2. **Configuration**: Configure the IP address and port in both the Oxygen and Hydrogen applications to match the master's listening address and port. Ensure that both all applications are connected to the same network.

3. **Compilation**: Compile both the master and slave components using your C++ compiler. For example, with g++:

```bash
g++ server.cpp -o server -lws2_32
g++ oxygen.cpp -o oxygen -lws2_32
g++ hydrogen.cpp -o hydrogen -lws2_32
``` 

## Running the Application

1. **Start the Server**: Run the main executable. Once either Hydrogen or Oxygen application connects, the server can receive bond requests and send bond confirmations.

2. **Start the Hydrogen and Oxygen Applications**: Run both executable on seperate machines, ensuring they are configured to connect to the master's IP address and port.

3. **Input Parameters**: Once connected, the Hydrogen and Oxygen application will prompt the following:
   - **Hydrogen Application**: The number of Hydrogen as represented by the variable "N".
   - **Oxygen Application**: The number of Oxygen as represented by the variable "M".
  
4. **Expected Output**: The server and applications will interact with each other to create H20 bonds. Log messages will show in all applications to show the status of an atom's request and bond.

## Authors

* **Go, Eldrich**
* **Pangan, John**
* **Pinawin, Timothy**
* **Yu, Ethan**
