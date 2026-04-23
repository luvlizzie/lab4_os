# Lab4_OS - Process Synchronization with Ring Buffer FIFO

## Description
This project implements inter-process communication (IPC) using a **ring buffer (FIFO queue)** stored in a shared binary file. The system consists of:
- **Receiver**: Creates shared buffer, manages senders, reads messages
- **Sender**: Connects to shared buffer, sends messages

Messages are delivered in **FIFO (First-In-First-Out)** order with a maximum length of **20 characters**.

## Requirements
- C++17 compatible compiler
- CMake 3.14+
- Google Test (automatically downloaded)

## Features
- **Ring Buffer FIFO Queue** (circular buffer implementation)
- **Multiple sender processes** (up to 10)
- **Binary file** for shared memory access
- **Cross-platform** (macOS/Linux with pthread, Windows with WinAPI)
- **Buffer full/empty handling** (blocks when full)
- **Ready signaling** from senders to receiver
- **Queue status display** (pending messages, positions)
- **Comprehensive unit tests** (15 tests)

## Building and Running Tests

```bash
# Clone the repository
git clone git@github.com:luvlizzie/lab4_os.git
cd lab4_os

# Make the test script executable
chmod +x run_tests.sh

# Build and run all tests
./run_tests.sh

# Expected output:
# [==========] 15 tests from 3 test suites ran.
# [  PASSED  ] 15 tests.
```

## Manual Testing (Multi-Terminal Setup)

### Step 1: Start the Receiver (Terminal 1)

```bash
cd build
./receiver
```

**Input example:**
```
Enter shared file name: messages.bin
Enter buffer size (number of slots): 5
Enter number of sender processes: 2
```

**What you'll see:**
```
Shared buffer created successfully
Launching sender processes...
Waiting for all senders to signal readiness...
Ready to receive messages!

--- Receiver Menu ---
1. Read message
2. Show queue status
3. Exit
Enter choice:
```

### Step 2: Start Senders (Terminals 2 and 3)

**Terminal 2 - Sender 1:**
```bash
cd build
./sender messages.bin 1
```

**Terminal 3 - Sender 2:**
```bash
cd build
./sender messages.bin 2
```

**What each sender shows:**
```
=== Sender 1 started ===
Shared file: messages.bin
Sender 1 ready to work

--- Sender 1 Menu ---
1. Send message
2. Exit
Enter choice:
```

### Step 3: Send Messages from Senders

**In Terminal 2 (Sender 1):**
```
Enter choice: 1
Enter message (max 20 chars): Hello from sender 1!
Message sent successfully!

Enter choice: 1
Enter message (max 20 chars): Second message
Message sent successfully!
```

**In Terminal 3 (Sender 2):**
```
Enter choice: 1
Enter message (max 20 chars): Hi from sender 2!
Message sent successfully!
```

### Step 4: Read Messages in Receiver (Terminal 1)

```
Enter choice: 1

✓ Message #1 received:
   From sender: 1
   Message # from sender: 1
   Content: Hello from sender 1!

Enter choice: 1

✓ Message #2 received:
   From sender: 1
   Message # from sender: 2
   Content: Second message

Enter choice: 1

✓ Message #3 received:
   From sender: 2
   Message # from sender: 1
   Content: Hi from sender 2!
```

### Step 5: Check Queue Status

```
Enter choice: 2

Queue status:
  Total slots: 5
  Pending messages: 0
  Read position: 3
  Write position: 3
  Active senders: 2
```

### Step 6: Exit

- In Receiver: press `3`
- In each Sender: press `2`

## Project Structure

```
lab4_os/
├── include/
│   └── common.h              # Constants, structures, function declarations
├── src/
│   ├── receiver.cpp          # Main receiver process
│   ├── sender.cpp            # Sender process
│   ├── utils.cpp             # Shared buffer operations (ring buffer)
│   └── sync.cpp              # Cross-platform synchronization primitives
├── tests/
│   ├── test_runner.cpp       # Google Test runner (DO NOT DELETE)
│   ├── test_receiver.cpp     # Receiver unit tests (4 tests)
│   ├── test_sender.cpp       # Sender unit tests (4 tests)
│   └── test_ring_buffer.cpp  # Ring buffer tests (7 tests)
├── CMakeLists.txt            # CMake build configuration
├── run_tests.sh              # Build and test script
└── README.md                 # This file
```

## Ring Buffer Implementation Details

The ring buffer uses the **classic algorithm** where:
- **Buffer size = N** can store **maximum N-1 messages**
- One slot is always kept empty to distinguish "full" from "empty"

**State detection:**
- **Empty**: readPos == writePos
- **Full**: (writePos + 1) % bufferSize == readPos

**Operations:**
- `writeToBuffer()`: Adds message, returns false if full
- `readFromBuffer()`: Removes and returns oldest message, returns false if empty
- `getPendingMessages()`: Returns number of messages waiting

## Cross-Platform Support

| Feature | macOS/Linux | Windows |
|---------|-------------|---------|
| Shared memory | mmap() | CreateFileMapping() |
| Thread sync | pthread_mutex + cond | Mutex + Events |
| Process creation | fork() + execl() | CreateProcess() |
| Sleep | usleep() | Sleep() |

## Test Results

### All 15 Tests Passing

| Test Suite | Number of Tests | Status |
|------------|-----------------|--------|
| ReceiverTest | 4 | PASSED |
| SenderTest | 4 | PASSED |
| RingBufferTest | 7 | PASSED |
| **Total** | **15** | **ALL PASSED** |

**Sample test output:**
```
[==========] 15 tests from 3 test suites ran.
[  PASSED  ] 15 tests.
```

## Example Session with Output

### Full test run with 2 senders:

**Receiver:**
```
--- Receiver Menu ---
1. Read message
2. Show queue status
3. Exit
Enter choice: 1

✓ Message #1 received:
   From sender: 1
   Message # from sender: 1
   Content: Hello from sender 1

--- Receiver Menu ---
Enter choice: 1

✓ Message #2 received:
   From sender: 1
   Message # from sender: 2
   Content: Second message

--- Receiver Menu ---
Enter choice: 1

✓ Message #3 received:
   From sender: 2
   Message # from sender: 1
   Content: Hi from sender 2

--- Receiver Menu ---
Enter choice: 2

Queue status:
  Total slots: 5
  Pending messages: 0
  Read position: 3
  Write position: 3
  Active senders: 2
```

## Error Handling

| Error | Handling |
|-------|----------|
| Buffer full | `writeToBuffer()` returns false |
| Buffer empty | `readFromBuffer()` returns false |
| Cannot open file | Error message and exit |
| Message too long (>20 chars) | Error message, reject message |
| Invalid integer input | Validation loop, retry |

## Requirements Met

| Requirement | Implementation |
|-------------|----------------|
| Binary file for messages | Shared memory mapped file |
| Ring buffer FIFO | Circular buffer algorithm |
| Max message length 20 chars | `MAX_MESSAGE_LEN = 20` |
| Multiple senders | Up to 10 concurrent senders |
| Receiver creates buffer | `createSharedBuffer()` |
| Signal readiness | `buffer->readySenders++` |
| Read on command | Menu option 1 |
| Send on command | Menu option 1 |
| Buffer full handling | Returns false, sender retries |
| Cross-platform | POSIX + Windows APIs |

## Troubleshooting

### Q: Sender can't open file?
**A:** Make sure Receiver created the file first. Start Receiver, then Senders.

### Q: "Buffer is full" message?
**A:** Receiver needs to read messages. Use option 1 in Receiver menu.

### Q: Receiver shows "No messages"?
**A:** Senders haven't sent messages yet. Send messages from Sender terminals first.

### Q: Build fails?
**A:** Clean and rebuild:
```bash
rm -rf build
./run_tests.sh
```

### Q: Tests fail on macOS?
**A:** Make sure you have Xcode Command Line Tools:
```bash
xcode-select --install
```

## Author
Elizaveta Kudinova

## License
Educational project for Operating Systems course, Belarusian State University, Faculty of Applied Mathematics and Computer Science.

## Related Projects
- [Lab1_OS](https://github.com/luvlizzie/lab1_os) - Process creation
- [Lab2_OS](https://github.com/luvlizzie/lab2_os) - Thread creation
- [Lab3_OS](https://github.com/luvlizzie/lab3_os) - Thread synchronization
