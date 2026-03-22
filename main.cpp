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
