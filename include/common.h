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

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

// Constants
static constexpr int MAX_MESSAGE_LEN = 20;
static constexpr int MIN_SENDERS = 1;
static constexpr int MAX_SENDERS = 10;
static constexpr int SHARED_FILE_VERSION = 1;

// Message header structure for ring buffer
struct MessageHeader {
    int senderId;           // Sender process ID
    int messageId;          // Sequential message number from this sender
    int isValid;            // 1 = valid message, 0 = empty slot
};

// Message slot in ring buffer
struct MessageSlot {
    MessageHeader header;
    char data[MAX_MESSAGE_LEN];
};

// Shared file structure (ring buffer)
struct SharedFile {
    int version;            // Version for validation
    int bufferSize;         // Number of slots
    int readPos;            // Next position to read (Receiver)
    int writePos;           // Next position to write (Senders)
    int activeSenders;      // Number of active senders
    MessageSlot slots[];
};

// Process types
enum class ProcessType {
    RECEIVER,
    SENDER
};

// Helper functions
void sleepMs(int milliseconds);
void consolePrint(const std::string& message);

#endif

