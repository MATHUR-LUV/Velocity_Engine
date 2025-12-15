#include <iostream>
#include <vector>
#include <cstring>
#include "OrderBook.h"
#include "ITCH.h"

int main() {
    OrderBook book;

    // 1. SIMULATE A RAW NETWORK PACKET
    // This looks like gibberish, but it is exactly what NASDAQ sends over the wire.
    // It is an "Add Order" message (Type 'A').
    unsigned char rawPacket[] = {
        0x41,                               // Type: 'A' (Add Order)
        0x00, 0x01,                         // Locate: 1
        0x00, 0x02,                         // Tracking: 2
        0x00, 0x00, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, // Timestamp
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, // Order ID: 7
        0x42,                               // Side: 'B' (Buy)
        0x00, 0x00, 0x00, 0x64,             // Shares: 100 (Hex 64)
        0x41, 0x41, 0x50, 0x4C, 0x20, 0x20, 0x20, 0x20, // Stock: "AAPL    "
        0x00, 0x01, 0x86, 0xA0              // Price: 100000 (100.0000)
    };

    std::cout << "--- HFT ITCH Parser Started ---\n";

    // 2. THE ZERO-COPY CAST (The Magic)
    // We point our struct pointer directly at the raw buffer.
    // No malloc, no new, no copying data.
    ITCHMessage* msg = reinterpret_cast<ITCHMessage*>(rawPacket);

    // 3. FIX ENDIANNESS (Network -> Host)
    msg->fixEndianness();

    // 4. PARSE & PRINT
    // Convert 8-char stock array to string
    std::string stockSymbol(msg->stock, 8); 
    
    // ITCH prices have 4 decimal places implied (Price 10000 = $1.00)
    double realPrice = msg->price / 10000.0;

    std::cout << "Parsed Message:" << std::endl;
    std::cout << "Type:  " << msg->type << std::endl;
    std::cout << "Stock: " << stockSymbol << std::endl;
    std::cout << "Side:  " << msg->side << std::endl;
    std::cout << "Shares:" << msg->shares << std::endl;
    std::cout << "Price: $" << realPrice << std::endl;

    // 5. FEED INTO ENGINE
    OrderType side = (msg->side == 'B') ? OrderType::Buy : OrderType::Sell;
    book.addOrder((int)msg->orderId, side, realPrice, (int)msg->shares);

    return 0;
}