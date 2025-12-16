cmake_minimum_required(VERSION 3.20)
project(OrderBook)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- GOOGLE BENCHMARK SETUP ---
# 1. Disable self-tests (Fixes the previous error)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

# 2. CRITICAL FIX: Turn off "Warnings as Errors" for Google Benchmark
# This prevents the 'CacheUnknown' switch error you just saw.
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3
)
FetchContent_MakeAvailable(googlebenchmark)

# --- YOUR PROJECT ---
add_executable(OrderBook main.cpp OrderBook.cpp)
# target_link_libraries(OrderBook benchmark::benchmark)
target_link_libraries(OrderBook benchmark::benchmark ws2_32)