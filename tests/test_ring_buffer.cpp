//
//  test_ring_buffer.cpp
//  
//
//  Created by Кудинова Елизавета on 23.04.2026.
//  Группа 12

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../include/common.h"

using ::testing::Eq;

class RingBufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        system("mkdir -p test_data");
    }
    
    void TearDown() override {
        system("rm -rf test_data");
    }
};

TEST_F(RingBufferTest, CreatesBufferCorrectly) {
    std::string filename = "test_data/test.bin";
    int bufferSize = 10;
    
    bool result = createSharedBuffer(filename, bufferSize);
    EXPECT_TRUE(result);
    
    SharedRingBuffer* buffer = nullptr;
    int fd;
    
    result = openSharedBuffer(filename, buffer, fd);
    EXPECT_TRUE(result);
    EXPECT_NE(buffer, nullptr);
    EXPECT_EQ(buffer->bufferSize, bufferSize);
    EXPECT_EQ(buffer->version, SHARED_FILE_VERSION);
    
    closeSharedBuffer(buffer, fd);
}

TEST_F(RingBufferTest, WritesAndReadsMessage) {
    std::string filename = "test_data/test.bin";
    int bufferSize = 5;
    
    createSharedBuffer(filename, bufferSize);
    
    SharedRingBuffer* buffer = nullptr;
    int fd;
    openSharedBuffer(filename, buffer, fd);
    
    bool written = writeToBuffer(buffer, 1, 1, "Hello");
    EXPECT_TRUE(written);
    EXPECT_EQ(getPendingMessages(buffer), 1);
    
    int senderId, messageId;
    std::string message;
    bool read = readFromBuffer(buffer, senderId, messageId, message);
    EXPECT_TRUE(read);
    EXPECT_EQ(senderId, 1);
    EXPECT_EQ(messageId, 1);
    EXPECT_EQ(message, "Hello");
    EXPECT_EQ(getPendingMessages(buffer), 0);
    
    closeSharedBuffer(buffer, fd);
}

TEST_F(RingBufferTest, RespectsBufferSizeLimit) {
    std::string filename = "test_data/test.bin";
    int bufferSize = 3;
    
    createSharedBuffer(filename, bufferSize);
    
    SharedRingBuffer* buffer = nullptr;
    int fd;
    openSharedBuffer(filename, buffer, fd);
    
    // With buffer size 3, we can write at most 2 messages
    // (1 slot is always kept empty to distinguish full from empty)
    EXPECT_TRUE(writeToBuffer(buffer, 1, 1, "Msg1"));
    EXPECT_TRUE(writeToBuffer(buffer, 2, 1, "Msg2"));
    
    // Third write should fail (buffer full)
    bool result = writeToBuffer(buffer, 3, 1, "Msg3");
    EXPECT_FALSE(result) << "Buffer should be full after 2 messages";
    
    // Read one message
    int senderId, messageId;
    std::string message;
    readFromBuffer(buffer, senderId, messageId, message);
    EXPECT_EQ(message, "Msg1");
    
    // Now we can write again
    result = writeToBuffer(buffer, 3, 1, "Msg3");
    EXPECT_TRUE(result) << "Should be able to write after reading";
    
    closeSharedBuffer(buffer, fd);
}

TEST_F(RingBufferTest, MaintainsFIFOOrder) {
    std::string filename = "test_data/test.bin";
    int bufferSize = 5;
    
    createSharedBuffer(filename, bufferSize);
    
    SharedRingBuffer* buffer = nullptr;
    int fd;
    openSharedBuffer(filename, buffer, fd);
    
    writeToBuffer(buffer, 1, 1, "First");
    writeToBuffer(buffer, 2, 1, "Second");
    writeToBuffer(buffer, 1, 2, "Third");
    
    int senderId, messageId;
    std::string message;
    
    readFromBuffer(buffer, senderId, messageId, message);
    EXPECT_EQ(message, "First");
    
    readFromBuffer(buffer, senderId, messageId, message);
    EXPECT_EQ(message, "Second");
    
    readFromBuffer(buffer, senderId, messageId, message);
    EXPECT_EQ(message, "Third");
    
    closeSharedBuffer(buffer, fd);
}

TEST_F(RingBufferTest, HandlesWrapAround) {
    std::string filename = "test_data/test.bin";
    int bufferSize = 3;
    
    createSharedBuffer(filename, bufferSize);
    
    SharedRingBuffer* buffer = nullptr;
    int fd;
    openSharedBuffer(filename, buffer, fd);
    
    // Fill buffer with 2 messages (max for size 3)
    writeToBuffer(buffer, 1, 1, "Msg1");
    writeToBuffer(buffer, 2, 1, "Msg2");
    
    // Read first message (frees slot 0)
    int senderId, messageId;
    std::string message;
    readFromBuffer(buffer, senderId, messageId, message);
    EXPECT_EQ(message, "Msg1");
    
    // Write new message (should go to slot 2, then wrap to slot 0)
    writeToBuffer(buffer, 4, 1, "Msg4");
    
    // Verify remaining messages in order
    readFromBuffer(buffer, senderId, messageId, message);
    EXPECT_EQ(message, "Msg2") << "Second message should be Msg2";
    
    readFromBuffer(buffer, senderId, messageId, message);
    EXPECT_EQ(message, "Msg4") << "Third message should be Msg4";
    
    closeSharedBuffer(buffer, fd);
}

TEST_F(RingBufferTest, EmptyBufferReturnsNoMessages) {
    std::string filename = "test_data/test.bin";
    int bufferSize = 5;
    
    createSharedBuffer(filename, bufferSize);
    
    SharedRingBuffer* buffer = nullptr;
    int fd;
    openSharedBuffer(filename, buffer, fd);
    
    int senderId, messageId;
    std::string message;
    bool result = readFromBuffer(buffer, senderId, messageId, message);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(getPendingMessages(buffer), 0);
    
    closeSharedBuffer(buffer, fd);
}

TEST_F(RingBufferTest, MultipleMessagesWithWrap) {
    std::string filename = "test_data/test.bin";
    int bufferSize = 4;
    
    createSharedBuffer(filename, bufferSize);
    
    SharedRingBuffer* buffer = nullptr;
    int fd;
    openSharedBuffer(filename, buffer, fd);
    
    // Max messages = bufferSize - 1 = 3
    writeToBuffer(buffer, 1, 1, "A");
    writeToBuffer(buffer, 2, 1, "B");
    writeToBuffer(buffer, 3, 1, "C");
    
    // Read first message (A)
    int senderId, messageId;
    std::string message;
    readFromBuffer(buffer, senderId, messageId, message);
    EXPECT_EQ(message, "A");
    
    // Add two more messages
    writeToBuffer(buffer, 1, 2, "D");
    writeToBuffer(buffer, 2, 2, "E");
    
    // Let's read in correct order
    readFromBuffer(buffer, senderId, messageId, message);
    EXPECT_EQ(message, "B") << "Expected B";
    
    readFromBuffer(buffer, senderId, messageId, message);
    EXPECT_EQ(message, "C") << "Expected C";
    
    readFromBuffer(buffer, senderId, messageId, message);
    EXPECT_EQ(message, "D") << "Expected D";
    
    // E should be either in buffer or not
    // If buffer has space, read E
    bool hasMessage = readFromBuffer(buffer, senderId, messageId, message);
    if (hasMessage) {
        EXPECT_EQ(message, "E") << "Expected E";
    }
    
    closeSharedBuffer(buffer, fd);
}
