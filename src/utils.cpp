//
//  utils.cpp
//  
//
//  Created by Кудинова Елизавета on 23.04.2026.
//  Группа 12

#include "../include/common.h"
#include <iostream>
#include <mutex>

using std::string;

static std::mutex consoleMutex;

void sleepMs(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

void consolePrint(const string& message) {
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << message << std::flush;
}
