//
//  receiver.cpp
//  
//
//  Created by Кудинова Елизавета on 23.04.2026.
//  Группа 12

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include "../include/common.h"

using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::string;
using std::vector;

#ifdef _WIN32
#include <process.h>
#else
#include <sys/wait.h>
#include <signal.h>
#endif

// Global sync primitives for receiver
SyncPrimitives receiverSync;

void runSender(const string& filename, int senderId) {
#ifdef _WIN32
    string cmd = "start sender.exe " + filename + " " + std::to_string(senderId);
    system(cmd.c_str());
#else
    pid_t pid = fork();
    if (pid == 0) {
        execl("./sender.exe", "sender.exe", filename.c_str(), std::to_string(senderId).c_str(), nullptr);
        exit(0);
    }
#endif
}

int main() {
    try {
        cout << "\n=== Process Lab4 - Receiver (Ring Buffer FIFO) ===" << endl;
        cout << string(60, '=') << endl;
        
        // Initialize synchronization
        syncInit(receiverSync);
        
        // Get filename and buffer size
        string filename = getValidatedString("Enter shared file name: ");
        int bufferSize = getValidatedInt("Enter buffer size (number of slots): ",
                                         MIN_BUFFER_SIZE, MAX_BUFFER_SIZE);
        
        // Create shared buffer
        if (!createSharedBuffer(filename, bufferSize)) {
            cerr << "Error: Cannot create shared buffer" << endl;
            return 1;
        }
        
        consolePrint("Shared buffer created successfully\n");
        
        // Get number of senders
        int senderCount = getValidatedInt("Enter number of sender processes: ",
                                          MIN_SENDERS, MAX_SENDERS);
        
        // Launch sender processes
        consolePrint("\nLaunching sender processes...\n");
        for (int i = 0; i < senderCount; ++i) {
            runSender(filename, i + 1);
            sleepMs(100);
        }
        
        // Open shared buffer
        SharedRingBuffer* buffer = nullptr;
        int fd;
        
        if (!openSharedBuffer(filename, buffer, fd)) {
            cerr << "Error: Cannot open shared buffer" << endl;
            return 1;
        }
        
        buffer->activeSenders = senderCount;
        
        // Wait for all senders to signal readiness
        consolePrint("\nWaiting for all senders to signal readiness...\n");
        
        // Simple polling for sender readiness (in real implementation would use events)
        int readyTimeout = 10; // seconds
        while (buffer->readySenders < senderCount && readyTimeout > 0) {
            sleepMs(1000);
            readyTimeout--;
        }
        
        if (buffer->readySenders < senderCount) {
            consolePrint("Warning: Not all senders signaled readiness\n");
        } else {
            consolePrint("All senders are ready!\n");
        }
        
        consolePrint("\nReady to receive messages!\n");
        
        // Main loop
        bool running = true;
        int messagesRead = 0;
        
        while (running) {
            cout << "\n--- Receiver Menu ---" << endl;
            cout << "1. Read message" << endl;
            cout << "2. Show queue status" << endl;
            cout << "3. Exit" << endl;
            cout << "Enter choice: ";
            
            int choice;
            cin >> choice;
            cin.ignore(256, '\n');
            
            switch (choice) {
                case 1: {
                    int senderId, messageId;
                    string message;
                    
                    if (readFromBuffer(buffer, senderId, messageId, message)) {
                        messagesRead++;
                        cout << "\n✓ Message #" << messagesRead << " received:" << endl;
                        cout << "   From sender: " << senderId << endl;
                        cout << "   Message # from sender: " << messageId << endl;
                        cout << "   Content: " << message << endl;
                    } else {
                        consolePrint("No messages in buffer. Queue is empty.\n");
                    }
                    break;
                }
                case 2: {
                    int pending = getPendingMessages(buffer);
                    cout << "\nQueue status:" << endl;
                    cout << "  Total slots: " << buffer->bufferSize << endl;
                    cout << "  Pending messages: " << pending << endl;
                    cout << "  Read position: " << buffer->readPos << endl;
                    cout << "  Write position: " << buffer->writePos << endl;
                    cout << "  Active senders: " << buffer->activeSenders << endl;
                    break;
                }
                case 3:
                    running = false;
                    break;
                default:
                    cerr << "Invalid choice" << endl;
            }
        }
        
        // Signal all senders to terminate
        consolePrint("\nSignaling senders to terminate...\n");
        sleepMs(500);
        
        // Cleanup
        closeSharedBuffer(buffer, fd);
        syncDestroy(receiverSync);
        
        cout << "\n=== Receiver finished (" << messagesRead << " messages read) ===" << endl;
        
    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
