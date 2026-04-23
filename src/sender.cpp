//
//  sender.cpp
//  
//
//  Created by Кудинова Елизавета on 23.04.2026.
//  Группа 12

#include <iostream>
#include <string>
#include <cstdlib>
#include "../include/common.h"

using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::string;

// Global sync primitives
SyncPrimitives senderSync;

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            cerr << "Usage: sender.exe <shared_file> <sender_id>" << endl;
            return 1;
        }
        
        string filename = argv[1];
        int senderId = std::stoi(argv[2]);
        int messageCounter = 0;
        
        cout << "\n=== Sender " << senderId << " started ===" << endl;
        cout << "Shared file: " << filename << endl;
        
        // Initialize synchronization
        syncInit(senderSync);
        
        // Open shared buffer
        SharedRingBuffer* buffer = nullptr;
        int fd;
        
        int retryCount = 5;
        while (retryCount > 0) {
            if (openSharedBuffer(filename, buffer, fd)) {
                break;
            }
            consolePrint("Waiting for shared buffer to be created...\n");
            sleepMs(1000);
            retryCount--;
        }
        
        if (!buffer) {
            cerr << "Error: Cannot open shared buffer" << endl;
            return 1;
        }
        
        // Increment ready senders count
        buffer->readySenders++;
        consolePrint("Sender " + std::to_string(senderId) + " signaled readiness\n");
        
        // Main loop
        bool running = true;
        
        while (running) {
            cout << "\n--- Sender " << senderId << " Menu ---" << endl;
            cout << "1. Send message" << endl;
            cout << "2. Exit" << endl;
            cout << "Enter choice: ";
            
            int choice;
            cin >> choice;
            cin.ignore(256, '\n');
            
            switch (choice) {
                case 1: {
                    string message;
                    cout << "Enter message (max " << MAX_MESSAGE_LEN << " chars): ";
                    std::getline(cin, message);
                    
                    if (message.length() > MAX_MESSAGE_LEN) {
                        cerr << "Message too long. Max " << MAX_MESSAGE_LEN << " characters." << endl;
                        break;
                    }
                    
                    if (message.empty()) {
                        cerr << "Message cannot be empty." << endl;
                        break;
                    }
                    
                    messageCounter++;
                    
                    // Try to write with retry on buffer full
                    bool written = false;
                    int retries = 3;
                    while (!written && retries > 0) {
                        if (writeToBuffer(buffer, senderId, messageCounter, message)) {
                            written = true;
                            cout << "Message sent successfully!" << endl;
                            consolePrint("Sender " + std::to_string(senderId) +
                                        " sent message #" + std::to_string(messageCounter) + "\n");
                        } else {
                            consolePrint("Buffer full. Waiting for receiver to read...\n");
                            sleepMs(1000);
                            retries--;
                        }
                    }
                    
                    if (!written) {
                        cerr << "Failed to send message after retries. Buffer may be full." << endl;
                        messageCounter--;
                    }
                    break;
                }
                case 2:
                    running = false;
                    break;
                default:
                    cerr << "Invalid choice" << endl;
            }
        }
        
        // Decrement active senders count
        buffer->activeSenders--;
        
        // Cleanup
        closeSharedBuffer(buffer, fd);
        syncDestroy(senderSync);
        
        cout << "\n=== Sender " << senderId << " finished ("
             << messageCounter << " messages sent) ===" << endl;
        
    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
