//
//  test_runner.cpp
//  
//
//  Created by Кудинова Елизавета on 23.04.2026.
//  Группа 12

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
