# ESP32-C6 NVS Comprehensive Test Suite

This document explains how to build and run the comprehensive NVS (Non-Volatile Storage) test suite for the ESP32-C6 platform.

## Overview

The NVS comprehensive test suite provides full coverage of the `EspNvs` class, testing all methods, error conditions, edge cases, and boundary conditions. The tests are designed to run without exceptions and without RTTI, making them suitable for embedded environments.

## Test Coverage

The test suite covers:

1. **Initialization and Deinitialization**
   - Normal initialization/deinitialization
   - Double initialization/deinitialization
   - Lazy initialization patterns
   - Error handling

2. **U32 Operations**
   - Basic set/get operations
   - Boundary values (min/max)
   - Overwriting existing values
   - Invalid parameters (null pointers, empty keys)
   - Key length limits

3. **String Operations**
   - Basic string storage/retrieval
   - Empty strings
   - Long strings
   - Buffer size validation
   - Special characters

4. **Blob Operations**
   - Binary data storage/retrieval
   - Large blobs
   - Buffer size validation
   - Null bytes in data

5. **Key Management**
   - Key existence checking
   - Key size retrieval
   - Key erasure
   - Invalid key handling

6. **Commit Operations**
   - Normal commits
   - Multiple commits
   - Commits on uninitialized NVS

7. **Statistics and Diagnostics**
   - Operation counting
   - Error tracking
   - Performance monitoring
   - Health diagnostics

8. **Metadata**
   - Description retrieval
   - Namespace information
   - Maximum limits

9. **Edge Cases**
   - Special characters in keys/values
   - Rapid successive operations
   - Type overwriting
   - Binary data with null bytes

10. **Stress Testing**
    - Multiple namespaces
    - Large number of keys
    - Rapid init/deinit cycles
    - Mixed operation types

## Building the Test

### Prerequisites
- ESP-IDF v5.5 or later
- ESP32-C6 DevKit
- USB cable for flashing

### Build Steps

1. **Set up ESP-IDF environment:**
   ```bash
   . $IDF_PATH/export.sh
   ```

2. **Navigate to the examples directory:**
   ```bash
   cd examples/esp32
   ```

3. **Build the NVS test:**
   ```bash
   idf.py build -DEXAMPLE_TYPE=nvs_test -DBUILD_TYPE=Release
   ```

   For debug build with more verbose output:
   ```bash
   idf.py build -DEXAMPLE_TYPE=nvs_test -DBUILD_TYPE=Debug
   ```

4. **Flash to device:**
   ```bash
   idf.py -p /dev/ttyUSB0 flash monitor
   ```
   
   Replace `/dev/ttyUSB0` with your actual serial port.

## Running the Test

Once flashed, the test will automatically start and run through all test cases. The output will show:

- Progress for each test case
- Pass/fail status for each test
- Detailed error messages for any failures
- Final summary with total tests, passed, failed, and execution time

### Expected Output

```
╔══════════════════════════════════════════════════════════════════════════════╗
║                    ESP32-C6 NVS COMPREHENSIVE TEST SUITE                     ║
╚══════════════════════════════════════════════════════════════════════════════╝

╔══════════════════════════════════════════════════════════════════════════════╗
║ Running: test_nvs_initialization                                            ║
╚══════════════════════════════════════════════════════════════════════════════╝
[SUCCESS] PASSED: test_nvs_initialization (XX.XX ms)

... (more tests) ...

=== NVS TEST SUMMARY ===
Total: 10, Passed: 10, Failed: 0, Success: 100.00%, Time: XXX.XX ms
[SUCCESS] ALL NVS TESTS PASSED!
```

## Troubleshooting

### Common Issues

1. **NVS partition not found:**
   - Ensure your partition table includes an NVS partition
   - Check `sdkconfig` for proper NVS configuration

2. **Initialization failures:**
   - The NVS partition might be corrupted
   - Try erasing flash: `idf.py -p /dev/ttyUSB0 erase-flash`

3. **Test failures:**
   - Check the detailed error messages in the output
   - Ensure sufficient free space in NVS partition
   - Verify ESP-IDF version is 5.5 or later

## Configuration

The test uses the following compiler flags to ensure embedded-friendly code:
- `-fno-exceptions`: No C++ exceptions
- `-fno-rtti`: No Run-Time Type Information
- `-O2`: Optimization level 2 for release builds
- `-Wall -Wextra`: Enable most warnings

## Customization

To add more tests or modify existing ones, edit:
`examples/esp32/main/NvsComprehensiveTest.cpp`

The test framework in `TestFramework.h` provides:
- Automatic timing measurement
- Result tracking
- Standardized output format

## Notes

- The test suite is designed to be self-contained and requires no external dependencies
- All tests use the `noexcept` specifier to ensure no exceptions are thrown
- The code is compatible with C++17 standard
- Thread safety is enabled by default (HF_THREAD_SAFE=1)