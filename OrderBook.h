#pragma once
#include <vector>
#include <algorithm> // Needed for sorting
#include "Order.h"

// A "Level" represents all orders at a specific price point
struct OrderLevel {
    double price;
    std::vector<Order> orders;

    OrderLevel(double p) : price(p) {}
};

class OrderBook {
    // 1. Asks (Sells): We want Low Prices first
    // We will keep this vector SORTED by price (Low -> High)
    std::vector<OrderLevel> asks;

    // 2. Bids (Buys): We want High Prices first
    // We will keep this vector SORTED by price (High -> Low)
    std::vector<OrderLevel> bids;

public:
    void addOrder(int id, OrderType type, double price, int quantity);
};