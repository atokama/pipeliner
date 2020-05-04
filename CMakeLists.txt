cmake_minimum_required(VERSION 3.16)
project(Pipeliner)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads)

add_library(pipeliner
        include/pipeliner/basic_block.h
        include/pipeliner/generator_block.h
        include/pipeliner/filter_block.h
        source/basic_block.cpp
        source/generator_block.cpp
        source/filter_block.cpp
        )

target_include_directories(pipeliner PUBLIC include)

target_include_directories(pipeliner SYSTEM PUBLIC dependencies)

target_link_libraries(pipeliner ${CMAKE_THREAD_LIBS_INIT})

add_executable(pipeliner_tester
        test/catch.hpp
        test/tester_main.cpp
        test/basic_block_test.cpp
        test/generator_block_test.cpp
        test/filter_block_test.cpp
        test/generator_and_filter_test.cpp
        )


# Tests

file(COPY test/data DESTINATION .)
target_link_libraries(pipeliner_tester pipeliner)

# Example

add_executable(pipeliner_example example/main.cpp)
target_link_libraries(pipeliner_example pipeliner)
