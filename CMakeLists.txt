cmake_minimum_required(VERSION 3.14)
project(ProcessLab4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Executables
add_executable(receiver.exe src/receiver.cpp src/utils.cpp)
add_executable(sender.exe src/sender.cpp src/utils.cpp)

target_include_directories(receiver.exe PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(sender.exe PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Windows support
if (MINGW)
    set_target_properties(receiver.exe PROPERTIES SUFFIX "")
    set_target_properties(sender.exe PROPERTIES SUFFIX "")
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
    src/utils.cpp
)

target_link_libraries(test_runner
    gtest_main
    gmock
)

target_include_directories(test_runner PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

enable_testing()
add_test(NAME all_tests COMMAND test_runner)
