#pragma once
#include <vector>
#include <atomic>
#include <optional>

template <typename T>
class LockFreeQueue {
    std::vector<T> buffer;
    size_t capacity;
    
    // Atomic indices to track where to write (head) and read (tail)
    // alignas(64) prevents "False Sharing" (cache line bouncing between CPU cores)
    alignas(64) std::atomic<size_t> head;
    alignas(64) std::atomic<size_t> tail;

public:
    // Constructor: Pre-allocate the vector
    LockFreeQueue(size_t size) : buffer(size), capacity(size), head(0), tail(0) {}

    // Writer Thread calls this (Push)
    bool push(const T& item) {
        size_t currentHead = head.load(std::memory_order_relaxed);
        size_t nextHead = (currentHead + 1) % capacity;

        // If nextHead catches up to tail, the queue is full
        if (nextHead == tail.load(std::memory_order_acquire)) {
            return false; // Queue Full (Drop packet or retry)
        }

        buffer[currentHead] = item;
        
        // "Release" ensures the consumer sees the data written above
        head.store(nextHead, std::memory_order_release);
        return true;
    }

    // Reader Thread calls this (Pop)
    bool pop(T& item) {
        size_t currentTail = tail.load(std::memory_order_relaxed);

        // If tail equals head, the queue is empty
        if (currentTail == head.load(std::memory_order_acquire)) {
            return false; // Empty
        }

        item = buffer[currentTail];

        size_t nextTail = (currentTail + 1) % capacity;
        
        // "Release" ensures the producer sees that we freed up a slot
        tail.store(nextTail, std::memory_order_release);
        return true;
    }
};