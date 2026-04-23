//
//  test_receiver.cpp
//  
//
//  Created by Кудинова Елизавета on 23.04.2026.
//  Группа 12

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <string>
#include "../include/common.h"

using ::testing::Eq;

class ReceiverTest : public ::testing::Test {
protected:
    void SetUp() override {
        system("mkdir -p test_data");
    }
    
    void TearDown() override {
        system("rm -rf test_data");
    }
};

TEST_F(ReceiverTest, CreatesSharedBuffer) {
    std::string filename = "test_data/receiver_test.bin";
    int bufferSize = 5;
    
    bool result = createSharedBuffer(filename, bufferSize);
    EXPECT_TRUE(result);
    
    // Verify file exists
    std::ifstream file(filename);
    EXPECT_TRUE(file.good());
}

TEST_F(ReceiverTest, OpensExistingBuffer) {
    std::string filename = "test_data/receiver_test.bin";
    int bufferSize = 5;
    
    createSharedBuffer(filename, bufferSize);
    
    SharedRingBuffer* buffer = nullptr;
    int fd;
    bool result = openSharedBuffer(filename, buffer, fd);
    
    EXPECT_TRUE(result);
    EXPECT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->bufferSize, bufferSize);
    
    closeSharedBuffer(buffer, fd);
}

TEST_F(ReceiverTest, InitialBufferIsEmpty) {
    std::string filename = "test_data/receiver_test.bin";
    int bufferSize = 5;
    
    createSharedBuffer(filename, bufferSize);
    
    SharedRingBuffer* buffer = nullptr;
    int fd;
    openSharedBuffer(filename, buffer, fd);
    
    EXPECT_EQ(getPendingMessages(buffer), 0);
    
    closeSharedBuffer(buffer, fd);
}

TEST_F(ReceiverTest, ReadsMessageAfterWrite) {
    std::string filename = "test_data/receiver_test.bin";
    int bufferSize = 5;
    
    createSharedBuffer(filename, bufferSize);
    
    SharedRingBuffer* buffer = nullptr;
    int fd;
    openSharedBuffer(filename, buffer, fd);
    
    writeToBuffer(buffer, 1, 1, "Test message");
    EXPECT_EQ(getPendingMessages(buffer), 1);
    
    int senderId, messageId;
    std::string message;
    bool result = readFromBuffer(buffer, senderId, messageId, message);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(senderId, 1);
    EXPECT_EQ(message, "Test message");
    EXPECT_EQ(getPendingMessages(buffer), 0);
    
    closeSharedBuffer(buffer, fd);
}
