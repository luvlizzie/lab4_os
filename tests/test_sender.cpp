//
//  test_sender.cpp
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

class SenderTest : public ::testing::Test {
protected:
    void SetUp() override {
        system("mkdir -p test_data");
    }
    
    void TearDown() override {
        system("rm -rf test_data");
    }
};

TEST_F(SenderTest, WritesMessageToBuffer) {
    std::string filename = "test_data/sender_test.bin";
    int bufferSize = 5;
    
    createSharedBuffer(filename, bufferSize);
    
    SharedRingBuffer* buffer = nullptr;
    int fd;
    openSharedBuffer(filename, buffer, fd);
    
    bool result = writeToBuffer(buffer, 1, 1, "Hello from sender");
    EXPECT_TRUE(result);
    EXPECT_EQ(getPendingMessages(buffer), 1);
    
    closeSharedBuffer(buffer, fd);
}

TEST_F(SenderTest, RespectsMaxMessageLength) {
    std::string longMessage(25, 'A');
    EXPECT_GT(longMessage.length(), MAX_MESSAGE_LEN);
}

TEST_F(SenderTest, MultipleWritesIncreaseCount) {
    std::string filename = "test_data/sender_test.bin";
    int bufferSize = 5;
    
    createSharedBuffer(filename, bufferSize);
    
    SharedRingBuffer* buffer = nullptr;
    int fd;
    openSharedBuffer(filename, buffer, fd);
    
    writeToBuffer(buffer, 1, 1, "Msg1");
    writeToBuffer(buffer, 1, 2, "Msg2");
    writeToBuffer(buffer, 1, 3, "Msg3");
    
    EXPECT_EQ(getPendingMessages(buffer), 3);
    
    closeSharedBuffer(buffer, fd);
}

TEST_F(SenderTest, CannotWriteWhenBufferFull) {
    std::string filename = "test_data/sender_test.bin";
    int bufferSize = 3;  // Can hold max 2 messages
    
    createSharedBuffer(filename, bufferSize);
    
    SharedRingBuffer* buffer = nullptr;
    int fd;
    openSharedBuffer(filename, buffer, fd);
    
    writeToBuffer(buffer, 1, 1, "Msg1");
    writeToBuffer(buffer, 1, 2, "Msg2");
    
    // Third write should fail
    bool result = writeToBuffer(buffer, 1, 3, "Msg3");
    EXPECT_FALSE(result);
    
    closeSharedBuffer(buffer, fd);
}
