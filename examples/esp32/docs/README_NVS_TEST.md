---
layout: default
title: "🧪 NVS Test"
description: "NVS Test Suite - Non-volatile storage and data persistence testing"
nav_order: 10
parent: "🧪 Test Documentation"
permalink: /examples/esp32/docs/nvs_test/
---

# ESP32-C6 NVS Comprehensive Test Suite

## Overview

The NVS Comprehensive Test Suite provides extensive validation of the `EspNvs` class for ESP32-C6
platforms using ESP-IDF v5.5+.
This test suite demonstrates complete Non-Volatile Storage functionality, data persistence,
error handling,
and performance optimization with a focus on embedded environments using `noexcept` functions.

**✅ Status: Successfully tested on ESP32-C6-DevKitM-1 hardware**

The comprehensive test suite provides full coverage of the `EspNvs` class, testing all methods,
error conditions, edge cases, and boundary conditions.
The tests are designed to run without exceptions and without RTTI,
making them suitable for embedded environments.

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

## Hardware Requirements

### Supported Platforms
- **Primary Target**: ESP32-C6-DevKitM-1
- **ESP-IDF Version**: v5.5 or later
- **Minimum Flash**: 4MB
- **Minimum RAM**: 256KB

### Connections
- **USB**: For flashing and serial monitoring (built-in USB-JTAG)
- **No External Hardware Required**: All tests use internal NVS partition

## Building and Running

### Prerequisites
```bash
## ESP-IDF v5.5+ installation required
. $IDF_PATH/export.sh

## Set target platform
export IDF_TARGET=esp32c6
```text

### Quick Start
```bash
## Navigate to examples directory
cd examples/esp32

## Build NVS test
idf.py build -DEXAMPLE_TYPE=nvs_test -DBUILD_TYPE=Release

## Flash and monitor
idf.py -p /dev/ttyUSB0 flash monitor
```text

### Alternative Build Methods

#### Using Build Scripts (Recommended)
```bash
## Source ESP-IDF environment
source /path/to/esp-idf/export.sh

## Build with optimization
./build_example.sh nvs_test Release

## Flash to device
idf.py -B build_nvs_test_Release flash monitor
```text

#### Debug Build for Development
```bash
## Build with debug symbols and verbose output
idf.py build -DEXAMPLE_TYPE=nvs_test -DBUILD_TYPE=Debug

## Run with detailed logging
idf.py -p /dev/ttyUSB0 flash monitor
```text

## Running the Test

Once flashed, the test will automatically start and run through all test cases.
The output will show:

- Progress for each test case
- Pass/fail status for each test
- Detailed error messages for any failures
- Final summary with total tests, passed, failed, and execution time

### Expected Output

```text
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
```text

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
