cmake_minimum_required(VERSION 3.14)
project(ProcessLab4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executables
add_executable(receiver src/receiver.cpp src/utils.cpp src/sync.cpp)
add_executable(sender src/sender.cpp src/utils.cpp src/sync.cpp)

target_include_directories(receiver PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(sender PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Threads for mutex/cond
find_package(Threads REQUIRED)
target_link_libraries(receiver PRIVATE Threads::Threads)
target_link_libraries(sender PRIVATE Threads::Threads)

# Windows support
if (MINGW)
    set_target_properties(receiver PROPERTIES SUFFIX "")
    set_target_properties(sender PROPERTIES SUFFIX "")
endif()

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Test runner
add_executable(test_runner
    tests/test_runner.cpp
    tests/test_receiver.cpp
    tests/test_sender.cpp
    tests/test_ring_buffer.cpp
    src/utils.cpp
    src/sync.cpp
)

target_link_libraries(test_runner
    gtest_main
    gmock
    pthread
)

target_include_directories(test_runner PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

enable_testing()
add_test(NAME all_tests COMMAND test_runner)
