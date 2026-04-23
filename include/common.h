//
//  common.h
//  
//
//  Created by Кудинова Елизавета on 23.04.2026.
//  Группа 12

#ifndef COMMON_H
#define COMMON_H

#include <cstddef>
#include <string>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

// Constants
static constexpr int MAX_MESSAGE_LEN = 20;
static constexpr int MIN_SENDERS = 1;
static constexpr int MAX_SENDERS = 10;
static constexpr int MIN_BUFFER_SIZE = 1;
static constexpr int MAX_BUFFER_SIZE = 100;
static constexpr int SHARED_FILE_VERSION = 1;

// Message header
struct MessageHeader {
    int senderId;
    int messageId;
    int isValid;        // 1 = valid, 0 = empty
    char reserved[4];   // Padding for alignment
};

// Message slot
struct MessageSlot {
    MessageHeader header;
    char data[MAX_MESSAGE_LEN];
};

// Shared ring buffer structure
struct SharedRingBuffer {
    int version;
    int bufferSize;
    std::atomic<int> readPos;
    std::atomic<int> writePos;
    std::atomic<int> activeSenders;
    int readySenders;           // Senders that signaled ready
    MessageSlot slots[];
};

// Process synchronization primitives
#ifdef _WIN32
struct SyncPrimitives {
    HANDLE readyEvent;
    HANDLE dataAvailableEvent;
    HANDLE spaceAvailableEvent;
};
#else
struct SyncPrimitives {
    pthread_mutex_t mutex;
    pthread_cond_t readyCond;
    pthread_cond_t dataAvailableCond;
    pthread_cond_t spaceAvailableCond;
    int readyFlag;
    int dataAvailableFlag;
    int spaceAvailableFlag;
};
#endif

// Helper functions
void sleepMs(int milliseconds);
void consolePrint(const std::string& message);
int getValidatedInt(const std::string& prompt, int min, int max);
std::string getValidatedString(const std::string& prompt);

// Shared memory functions
bool createSharedBuffer(const std::string& filename, int bufferSize);
bool openSharedBuffer(const std::string& filename, SharedRingBuffer*& buffer, int& fd);
void closeSharedBuffer(SharedRingBuffer* buffer, int fd);

// Ring buffer operations
bool writeToBuffer(SharedRingBuffer* buffer, int senderId, int messageId, const std::string& message);
bool readFromBuffer(SharedRingBuffer* buffer, int& senderId, int& messageId, std::string& message);
int getPendingMessages(SharedRingBuffer* buffer);

// Synchronization
void syncInit(SyncPrimitives& sync);
void syncDestroy(SyncPrimitives& sync);
void syncSignalReady(SyncPrimitives& sync);
void syncWaitForReady(SyncPrimitives& sync);
void syncSignalDataAvailable(SyncPrimitives& sync);
void syncWaitForDataAvailable(SyncPrimitives& sync);
void syncSignalSpaceAvailable(SyncPrimitives& sync);
void syncWaitForSpaceAvailable(SyncPrimitives& sync);

#endif
