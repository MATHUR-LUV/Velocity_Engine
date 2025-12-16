#include "OrderBook.h"
#include <iostream>
#include <algorithm> // For std::min

void OrderBook::addOrder(int id, OrderType type, double price, int quantity) {
    
    if (type == OrderType::Buy) {
        // ---------------- BUY ORDER LOGIC ----------------
        // Goal: Match with Sellers (Asks). 
        // Asks are sorted Low -> High (Cheapest first).

        while (quantity > 0 && !asks.empty()) {
            OrderLevel& bestAsk = asks.front(); // The cheapest sell order

            // If the seller wants more than we are willing to pay, stop matching.
            if (bestAsk.price > price) {
                break; 
            }

            // MATCH FOUND!
            // Get the first order in the queue at this price level
            Order& sellOrder = bestAsk.orders.front();
            
            // Calculate trade size
            int tradeQty = std::min(quantity, sellOrder.quantity);

            std::cout << "[TRADE] Buy Order " << id << " matched " << tradeQty 
                      << " shares @ $" << bestAsk.price << std::endl;

            // Update quantities
            quantity -= tradeQty;
            sellOrder.quantity -= tradeQty;

            // Cleanup: If the specific sell order is empty, remove it
            if (sellOrder.quantity == 0) {
                bestAsk.orders.erase(bestAsk.orders.begin());
                
                // If the entire price level is empty, remove the level
                if (bestAsk.orders.empty()) {
                    asks.erase(asks.begin());
                }
            }
        }

        // If we still have shares to buy, add to the 'Bids' book
        if (quantity > 0) {
            // We need to find the correct spot to insert this order.
            // Bids are sorted High -> Low (Highest payer at top).
            
            auto it = bids.begin();
            while (it != bids.end() && it->price > price) {
                it++;
            }

            // Scenario 1: Price level already exists
            if (it != bids.end() && it->price == price) {
                it->orders.emplace_back(id, type, price, quantity);
            } 
            // Scenario 2: New price level (Insert it)
            else {
                // insert() puts the new element BEFORE the iterator 'it'
                auto newLevelIter = bids.insert(it, OrderLevel(price));
                newLevelIter->orders.emplace_back(id, type, price, quantity);
            }
            std::cout << "[ORDER] Buy Order " << id << " added to book @ " << price << std::endl;
        }

    } else {
        // ---------------- SELL ORDER LOGIC ----------------
        // Goal: Match with Buyers (Bids).
        // Bids are sorted High -> Low (Highest payer first).

        while (quantity > 0 && !bids.empty()) {
            OrderLevel& bestBid = bids.front(); // The highest buy offer

            // If the buyer offers less than we want, stop matching.
            if (bestBid.price < price) {
                break;
            }

            // MATCH FOUND!
            Order& buyOrder = bestBid.orders.front();
            int tradeQty = std::min(quantity, buyOrder.quantity);

            std::cout << "[TRADE] Sell Order " << id << " matched " << tradeQty 
                      << " shares @ $" << bestBid.price << std::endl;

            quantity -= tradeQty;
            buyOrder.quantity -= tradeQty;

            if (buyOrder.quantity == 0) {
                bestBid.orders.erase(bestBid.orders.begin());
                if (bestBid.orders.empty()) {
                    bids.erase(bids.begin());
                }
            }
        }

        // If we still have shares to sell, add to the 'Asks' book
        if (quantity > 0) {
            // Asks are sorted Low -> High (Cheapest seller at top).
            
            auto it = asks.begin();
            while (it != asks.end() && it->price < price) {
                it++;
            }

            if (it != asks.end() && it->price == price) {
                it->orders.emplace_back(id, type, price, quantity);
            } else {
                auto newLevelIter = asks.insert(it, OrderLevel(price));
                newLevelIter->orders.emplace_back(id, type, price, quantity);
            }
            std::cout << "[ORDER] Sell Order " << id << " added to book @ " << price << std::endl;
        }
    }
}