#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "OrderBook.h"
#include "ITCH.h"
#include "LockFreeQueue.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 1234
#define BUFFER_SIZE 1024
#define QUEUE_SIZE 100000 // Buffer for 100k pending messages

// Global flag to stop threads cleanly
std::atomic<bool> running(true);

// THE PRODUCER THREAD (Network Listener)
// Reads UDP packets and pushes them to the Queue
void networkThread(LockFreeQueue<ITCHMessage>& queue) {
    // 1. Setup Winsock
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Network Thread: Bind failed.\n";
        return;
    }

    std::cout << "[Network] Listening on Port " << PORT << "...\n";

    char buffer[BUFFER_SIZE];
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    while (running) {
        // This is a BLOCKING call (it sleeps until data arrives)
        int bytesReceived = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, 
                                     (sockaddr*)&clientAddr, &clientAddrLen);

        if (bytesReceived > 0 && bytesReceived >= sizeof(ITCHMessage)) {
            // Cast raw bytes to struct
            ITCHMessage* msg = reinterpret_cast<ITCHMessage*>(buffer);
            
            // PUSH to Queue (Non-blocking)
            // If queue is full, we sadly drop the packet (Standard HFT practice)
            if (!queue.push(*msg)) {
                // std::cerr << "Queue Full! Dropping packet.\n"; 
            }
        }
    }
    closesocket(sockfd);
}

// THE CONSUMER THREAD (Matching Engine)
// Pops messages from Queue and processes them
void engineThread(LockFreeQueue<ITCHMessage>& queue) {
    std::cout << "[Engine] Ready to match.\n";
    
    OrderBook book;
    ITCHMessage msg;

    while (running) {
        // Try to pop a message
        if (queue.pop(msg)) {
            // WE HAVE DATA! Process it.
            
            // 1. Fix Endianness (CPU heavy work done here, not in network thread)
            msg.fixEndianness();

            // 2. Add to OrderBook
            double realPrice = msg.price / 10000.0;
            OrderType side = (msg.side == 'B') ? OrderType::Buy : OrderType::Sell;
            
            // Uncomment to debug (will slow it down!)
            // std::cout << "Processing Order: " << msg.orderId << "\n";
            
            book.addOrder((int)msg.orderId, side, realPrice, (int)msg.shares);
        } 
        else {
            // Queue is empty. Busy-wait or yield?
            // In HFT, we usually "Busy Spin" (burn CPU) to react instantly.
            // For laptop battery life, let's yield.
            std::this_thread::yield(); 
        }
    }
}

int main() {
    // Initialize Winsock once
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Create the Lock-Free Queue
    LockFreeQueue<ITCHMessage> queue(QUEUE_SIZE);

    // Launch Threads
    std::thread producer(networkThread, std::ref(queue));
    std::thread consumer(engineThread, std::ref(queue));

    // Keep main thread alive
    producer.join();
    consumer.join();

    WSACleanup();
    return 0;
}

// #include <iostream>
// #include <vector>
// #include <cstring>
// #include <winsock2.h> // Windows Socket Header
// #include <ws2tcpip.h>
// #include "OrderBook.h"
// #include "ITCH.h"

// #pragma comment(lib, "ws2_32.lib") // Tell compiler to use Winsock

// #define PORT 1234
// #define BUFFER_SIZE 1024

// int main() {
//     // 1. Initialize Windows Sockets (WSA)
//     WSADATA wsaData;
//     if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
//         std::cerr << "WSAStartup failed." << std::endl;
//         return 1;
//     }

//     // 2. Create UDP Socket
//     SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
//     if (sockfd == INVALID_SOCKET) {
//         std::cerr << "Socket creation failed." << std::endl;
//         WSACleanup();
//         return 1;
//     }

//     // 3. Bind the socket to Port 1234
//     sockaddr_in serverAddr;
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all network cards
//     serverAddr.sin_port = htons(PORT);       // Host to Network Short (Endianness)

//     if (bind(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
//         std::cerr << "Bind failed. Is port 1234 already in use?" << std::endl;
//         closesocket(sockfd);
//         WSACleanup();
//         return 1;
//     }

//     std::cout << "--- Velocity Engine Listening on UDP Port " << PORT << " ---\n";

//     OrderBook book;
//     char buffer[BUFFER_SIZE];
//     sockaddr_in clientAddr;
//     int clientAddrLen = sizeof(clientAddr);

//     // 4. The Event Loop
//     while (true) {
//         // Block until a packet arrives
//         int bytesReceived = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, 
//                                      (sockaddr*)&clientAddr, &clientAddrLen);

//         if (bytesReceived == SOCKET_ERROR) {
//             std::cerr << "Receive failed." << std::endl;
//             continue;
//         }

//         // Check if packet size matches our expected format
//         // (A real engine would handle partial packets, but we assume full packets for now)
//         if (bytesReceived < sizeof(ITCHMessage)) {
//             // std::cerr << "Packet too small ignored." << std::endl;
//             continue;
//         }

//         // 5. Zero-Copy Parse
//         ITCHMessage* msg = reinterpret_cast<ITCHMessage*>(buffer);
        
//         // 6. Endianness Fix
//         msg->fixEndianness();

//         // 7. Execute
//         OrderType side = (msg->side == 'B') ? OrderType::Buy : OrderType::Sell;
//         double realPrice = msg->price / 10000.0;
        
//         // Print less info to keep it fast, or uncomment to debug
//         // std::cout << "Received Order: " << msg->stock << " $" << realPrice << std::endl;

//         book.addOrder((int)msg->orderId, side, realPrice, (int)msg->shares);
//     }

//     // Cleanup (Unreachable in this loop, but good practice)
//     closesocket(sockfd);
//     WSACleanup();
//     return 0;
// }