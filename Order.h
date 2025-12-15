#pragma once
#include <ctime>

// Enum to represent the side of the order (Buy or Sell)
enum class OrderType {
    Buy,
    Sell
};

// The Order Structure
struct Order {
    int id;             // Unique ID to identify the order
    OrderType type;     // Buy or Sell
    double price;       // Price per share
    int quantity;       // Number of shares

    // Simple Constructor to initialize the object easily
    Order(int _id, OrderType _type, double _price, int _quantity)
        : id(_id), type(_type), price(_price), quantity(_quantity) {}
};