#
//  run_tests.sh
//  
//
//  Created by Кудинова Елизавета on 23.04.2026.
//  Группа 12

#!/bin/bash

echo "=== Process Lab4 - Test Runner ==="
echo "Version 1.0"
echo "=================================="

# Clean previous build
rm -rf build
mkdir build
cd build

# Configure and build
cmake ..
make

# Run tests
echo -e "\n=== Running Tests ===\n"
./test_runner

echo -e "\n=== Test Summary ==="
echo "Date: $(date)"

cd ..
