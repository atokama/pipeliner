cmake_minimum_required(VERSION 3.16)
project(Pipeliner)

set(CMAKE_CXX_STANDARD 14)

find_package(Threads)

add_executable(pipeliner main.cpp)

add_executable(pipeliner_tester
        test/catch.hpp
        test/tester_main.cpp
        test/basic_block_test.cpp
        )

target_link_libraries(pipeliner_tester ${CMAKE_THREAD_LIBS_INIT})
