cmake_minimum_required(VERSION 3.16)
project(Pipeliner)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads)

set(PIPELINER_INCLUDE_FILES
        include/pipeliner/debug.h
        include/pipeliner/basic_block.h
        include/pipeliner/generator_block.h
        include/pipeliner/filter_block.h
        include/pipeliner/labelling_block.h
        include/pipeliner/computation_block.h)

set(PIPELINER_SOURCE_FILES
        source/basic_block.cpp
        source/generator_block.cpp
        source/filter_block.cpp
        source/labelling_block.cpp)

source_group("Include Files" FILES ${PIPELINER_INCLUDE_FILES})
source_group("Source Files" FILES ${PIPELINER_SOURCE_FILES})

# Create two versions of Pipeliner library - with and without debug.
# Note, that it's not about Release/Debug, but about debug logging (see Debug.h usage)
function(AddPipelinerLib Name DebugEnabled)
    add_library(${Name} ${PIPELINER_INCLUDE_FILES} ${PIPELINER_SOURCE_FILES})
    target_include_directories(${Name} PUBLIC include)
    target_include_directories(${Name} SYSTEM PUBLIC dependencies)
    target_link_libraries(${Name} ${CMAKE_THREAD_LIBS_INIT})
    if (${DebugEnabled})
        target_compile_definitions(${Name} PUBLIC PILI_DEBUG_ENABLED)
    endif ()
	if(MSVC)
		# Set __cplusplus macro correctly in Visual Studio
		# https://peter.bloomfield.online/report-__cplusplus-correctly-with-cmake-and-visual-studio/
		target_compile_options(${Name} PUBLIC "/Zc:__cplusplus")
	endif()
endfunction()

AddPipelinerLib(pipeliner OFF)
AddPipelinerLib(pipeliner_dbg ON)

# Tests
set(PIPELINER_TESTER_FILES
        test/catch.hpp
        test/tester_main.cpp
        test/basic_block_test.cpp
        test/generator_block_test.cpp
        test/filter_block_test.cpp
        test/generator_and_filter_test.cpp
        test/labelling_block_test.cpp
        test/computation_block_test.cpp)

source_group("Tester Files" FILES ${PIPELINER_TESTER_FILES})
add_executable(pipeliner_tester ${PIPELINER_TESTER_FILES})
target_link_libraries(pipeliner_tester pipeliner_dbg)
file(COPY test/data DESTINATION .)

# Benchmark
set(HAYAI_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dependencies/hayai")
set(PIPELINER_BENCHMARK_FILES
        benchmark/hayai_main.hpp
        benchmark/hayai_posix_main.cpp
        benchmark/benchmark_test.cpp)
source_group("Benchmark Files" ${PIPELINER_BENCHMARK_FILES})
add_executable(pipeliner_benchmark ${PIPELINER_BENCHMARK_FILES})
target_link_libraries(pipeliner_benchmark pipeliner)
target_include_directories(pipeliner_benchmark SYSTEM PUBLIC ${HAYAI_INCLUDE_DIR})
