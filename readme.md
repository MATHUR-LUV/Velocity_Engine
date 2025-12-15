# Velocity Engine: Sub-Microsecond Limit Order Book

## Overview
A high-frequency trading (HFT) matching engine written in C++20. It implements a Price-Time Priority matching algorithm using contiguous memory structures to minimize cache misses.

## Key Features
- **Latency:** ~280 nanoseconds median matching time (Benchmarked via Google Benchmark).
- **Architecture:** Zero-copy architecture using custom memory arenas.
- **Protocol:** Implements NASDAQ ITCH 5.0 binary protocol parsing.
- **Optimization:** Replaces standard STL Maps with pre-allocated Vectors to reduce memory fragmentation.

## Build & Run
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
./OrderBook.exe



