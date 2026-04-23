//
//  utils.cpp
//  
//
//  Created by Кудинова Елизавета on 23.04.2026.
//  Группа 12

#include "../include/common.h"
#include <iostream>
#include <cstring>
#include <mutex>

using std::string;
using std::mutex;
using std::lock_guard;
using std::cout;
using std::cin;
using std::cerr;
using std::endl;

static mutex consoleMutex;

void sleepMs(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

void consolePrint(const string& message) {
    lock_guard<mutex> lock(consoleMutex);
    std::cout << message << std::flush;
}

int getValidatedInt(const string& prompt, int min, int max) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= min && value <= max) {
            cin.ignore(256, '\n');
            return value;
        }
        cerr << "Invalid input. Please enter a number between "
             << min << " and " << max << endl;
        cin.clear();
        cin.ignore(256, '\n');
    }
}

string getValidatedString(const string& prompt) {
    string value;
    while (true) {
        cout << prompt;
        std::getline(cin, value);
        if (!value.empty()) {
            return value;
        }
        cerr << "Input cannot be empty. Please try again." << endl;
    }
}

bool createSharedBuffer(const string& filename, int bufferSize) {
    size_t totalSize = sizeof(SharedRingBuffer) + bufferSize * sizeof(MessageSlot);
    
#ifdef _WIN32
    HANDLE hFile = CreateFileA(
        filename.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hFile == INVALID_HANDLE_VALUE) return false;
    
    HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, totalSize, NULL);
    if (hMapping == NULL) {
        CloseHandle(hFile);
        return false;
    }
    
    SharedRingBuffer* buffer = static_cast<SharedRingBuffer*>(
        MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, totalSize));
    
    if (buffer == NULL) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return false;
    }
    
    buffer->version = SHARED_FILE_VERSION;
    buffer->bufferSize = bufferSize;
    buffer->readPos = 0;
    buffer->writePos = 0;
    buffer->activeSenders = 0;
    buffer->readySenders = 0;
    
    // Initialize all slots as invalid
    for (int i = 0; i < bufferSize; ++i) {
        buffer->slots[i].header.isValid = 0;
    }
    
    UnmapViewOfFile(buffer);
    CloseHandle(hMapping);
    CloseHandle(hFile);
#else
    int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) return false;
    
    if (ftruncate(fd, totalSize) == -1) {
        close(fd);
        return false;
    }
    
    SharedRingBuffer* buffer = static_cast<SharedRingBuffer*>(
        mmap(NULL, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    
    if (buffer == MAP_FAILED) {
        close(fd);
        return false;
    }
    
    buffer->version = SHARED_FILE_VERSION;
    buffer->bufferSize = bufferSize;
    buffer->readPos = 0;
    buffer->writePos = 0;
    buffer->activeSenders = 0;
    buffer->readySenders = 0;
    
    for (int i = 0; i < bufferSize; ++i) {
        buffer->slots[i].header.isValid = 0;
    }
    
    munmap(buffer, totalSize);
    close(fd);
#endif
    
    return true;
}

bool openSharedBuffer(const string& filename, SharedRingBuffer*& buffer, int& fd) {
#ifdef _WIN32
    HANDLE hFile = CreateFileA(
        filename.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hFile == INVALID_HANDLE_VALUE) return false;
    
    HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (hMapping == NULL) {
        CloseHandle(hFile);
        return false;
    }
    
    buffer = static_cast<SharedRingBuffer*>(
        MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0));
    
    if (buffer == NULL) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return false;
    }
    
    fd = reinterpret_cast<int>(hMapping);
#else
    fd = open(filename.c_str(), O_RDWR);
    if (fd == -1) return false;
    
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        close(fd);
        return false;
    }
    
    buffer = static_cast<SharedRingBuffer*>(
        mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    
    if (buffer == MAP_FAILED) {
        close(fd);
        return false;
    }
#endif
    
    return buffer != nullptr;
}

void closeSharedBuffer(SharedRingBuffer* buffer, int fd) {
    if (!buffer) return;
    
#ifdef _WIN32
    UnmapViewOfFile(buffer);
    CloseHandle(reinterpret_cast<HANDLE>(fd));
#else
    munmap(buffer, 0);
    close(fd);
#endif
}

bool writeToBuffer(SharedRingBuffer* buffer, int senderId, int messageId, const string& message) {
    if (!buffer) return false;
    
    int currentWritePos = buffer->writePos;
    int nextWritePos = (currentWritePos + 1) % buffer->bufferSize;
    
    // Check if buffer is full - using atomic loads
    int readPos = buffer->readPos;
    
    consolePrint("writeToBuffer: writePos=" + std::to_string(currentWritePos) +
                 ", readPos=" + std::to_string(readPos) +
                 ", nextWritePos=" + std::to_string(nextWritePos) + "\n");
    
    if (nextWritePos == readPos) {
        consolePrint("Buffer is full!\n");
        return false;
    }
    
    MessageSlot* slot = &buffer->slots[currentWritePos];
    slot->header.senderId = senderId;
    slot->header.messageId = messageId;
    slot->header.isValid = 1;
    
    strncpy(slot->data, message.c_str(), MAX_MESSAGE_LEN - 1);
    slot->data[MAX_MESSAGE_LEN - 1] = '\0';
    
    buffer->writePos = nextWritePos;
    
    consolePrint("Message written at position " + std::to_string(currentWritePos) +
                 " by sender " + std::to_string(senderId) + "\n");
    
    return true;
}

bool readFromBuffer(SharedRingBuffer* buffer, int& senderId, int& messageId, string& message) {
    if (!buffer) return false;
    
    int currentReadPos = buffer->readPos;
    int writePos = buffer->writePos;
    
    consolePrint("readFromBuffer: readPos=" + std::to_string(currentReadPos) +
                 ", writePos=" + std::to_string(writePos) + "\n");
    
    if (currentReadPos == writePos) {
        consolePrint("Buffer is empty\n");
        return false;
    }
    
    MessageSlot* slot = &buffer->slots[currentReadPos];
    
    if (slot->header.isValid == 0) {
        consolePrint("Invalid slot, skipping\n");
        buffer->readPos = (currentReadPos + 1) % buffer->bufferSize;
        return false;
    }
    
    senderId = slot->header.senderId;
    messageId = slot->header.messageId;
    message = string(slot->data);
    
    slot->header.isValid = 0;
    
    int newReadPos = (currentReadPos + 1) % buffer->bufferSize;
    buffer->readPos = newReadPos;
    
    consolePrint("Message read from position " + std::to_string(currentReadPos) +
                 ", new readPos=" + std::to_string(newReadPos) + "\n");
    
    return true;
}

int getPendingMessages(SharedRingBuffer* buffer) {
    if (!buffer) return 0;
    
    int writePos = buffer->writePos;
    int readPos = buffer->readPos;
    
    if (writePos >= readPos) {
        return writePos - readPos;
    } else {
        return buffer->bufferSize - readPos + writePos;
    }
}
