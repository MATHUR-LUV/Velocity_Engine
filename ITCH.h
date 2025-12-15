#pragma once
#include <cstdint>
#include <algorithm> // For std::reverse logic

#pragma pack(push, 1) // Ensure no gaps in memory

struct ITCHMessage {
    char type;              // 'A' = Add Order
    uint16_t stockLocate;   
    uint16_t tracking;
    uint64_t timestamp;
    uint64_t orderId;
    char side;              // 'B' or 'S'
    uint32_t shares;
    char stock[8];
    uint32_t price;

    // Helper to swap bytes (Big Endian -> Little Endian)
    // We use a built-in GCC compiler function for speed: __builtin_bswap
    void fixEndianness() {
        // Only swap the multi-byte integers
        stockLocate = __builtin_bswap16(stockLocate);
        tracking    = __builtin_bswap16(tracking);
        timestamp   = __builtin_bswap64(timestamp);
        orderId     = __builtin_bswap64(orderId);
        shares      = __builtin_bswap32(shares);
        price       = __builtin_bswap32(price);
    }
};

#pragma pack(pop)