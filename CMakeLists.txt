cmake_minimum_required(VERSION 3.10)
project(StorageOptimizer)

set(CMAKE_CXX_STANDARD 17)

add_executable(storage_optimizer
    main.cpp
    scanner.cpp
    duplicates.cpp
    huffman.cpp
    optimizer.cpp
    summary.cpp
    utils.cpp
)

target_include_directories(storage_optimizer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
