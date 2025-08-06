# Unit Testing Guide for HardFOC IID Project

This document provides comprehensive guidance on running, developing, and maintaining unit tests for the HardFOC IID ESP32 project using the Unity testing framework.

## Table of Contents

- [Overview](#overview)
- [Test Architecture](#test-architecture)
- [Quick Start](#quick-start)
- [Running Tests Locally](#running-tests-locally)
- [Continuous Integration](#continuous-integration)
- [Writing Tests](#writing-tests)
- [Test Coverage](#test-coverage)
- [Troubleshooting](#troubleshooting)
- [Advanced Configuration](#advanced-configuration)

## Overview

The HardFOC IID project uses a comprehensive testing framework that includes:

- **Unity Framework**: Lightweight C unit testing framework optimized for embedded systems
- **ESP-IDF Integration**: Native ESP32 testing support with hardware simulation
- **Mock System**: Hardware abstraction layer for testing without physical dependencies
- **CI/CD Pipeline**: Automated testing across multiple ESP32 variants
- **Coverage Analysis**: Code coverage reporting and analysis

### Supported Targets

- ESP32-C6 (Primary target)
- ESP32 (Classic)
- ESP32-S3
- ESP32-S2 (Limited support)
- ESP32-C3 (Limited support)

## Test Architecture

```
test/
├── CMakeLists.txt                    # Main test project configuration
├── sdkconfig.defaults               # ESP-IDF configuration for testing
├── test_runner.py                   # pytest-embedded test runner
├── main/                           # Test application
│   ├── CMakeLists.txt              # Test component configuration
│   ├── test_main.cpp               # Main test runner
│   ├── test_esp_gpio_basic.cpp     # EspGpio basic tests
│   ├── test_esp_gpio_interrupts.cpp # EspGpio interrupt tests
│   ├── test_esp_gpio_advanced.cpp  # EspGpio advanced features
│   ├── test_esp_adc_*.cpp          # EspAdc test suites
│   └── test_integration_*.cpp      # Integration tests
├── components/                     # Test components
│   ├── unity/                      # Unity framework wrapper
│   │   ├── CMakeLists.txt
│   │   └── include/unity_config.h  # Unity configuration
│   └── esp_idf_mocks/             # Hardware mocking system
│       ├── CMakeLists.txt
│       ├── include/               # Mock headers
│       └── src/                   # Mock implementations
└── include/                       # Test-specific headers
```

## Quick Start

### Prerequisites

1. **ESP-IDF v5.5+** installed and configured
2. **Python 3.8+** with pip
3. **Git** for version control
4. **Hardware (optional)**: ESP32 development board for hardware testing

### Installation

```bash
# Install Python dependencies
pip install pytest pytest-embedded pytest-embedded-serial-esp pytest-embedded-idf

# Install coverage tools (optional)
pip install gcovr

# Clone and setup the project
git clone <repository-url>
cd hardfoc-iid
git submodule update --init --recursive
```

### Running Tests

```bash
# Navigate to test directory
cd test

# Configure for your target
idf.py set-target esp32c6

# Build and run all tests
idf.py build
python test_runner.py

# Or use the CI-style approach
python -m pytest test_runner.py --target esp32c6 --build-dir build
```

## Running Tests Locally

### Basic Test Execution

```bash
# Build tests for ESP32-C6
cd test
idf.py set-target esp32c6
idf.py build

# Run tests on hardware
python test_runner.py --target esp32c6 --port /dev/ttyUSB0

# Run tests with verbose output
python test_runner.py --target esp32c6 --verbose

# Run specific test filter (if implemented)
python test_runner.py --target esp32c6 --filter EspGpio
```

### Cross-Platform Testing

```bash
# Test on different ESP32 variants
for target in esp32c6 esp32 esp32s3; do
    echo "Testing on $target"
    idf.py set-target $target
    idf.py build
    python test_runner.py --target $target
done
```

### Debug Mode Testing

```bash
# Build with debug symbols and assertions
idf.py -DCMAKE_BUILD_TYPE=Debug build

# Enable detailed logging
idf.py menuconfig
# Navigate to Component config → Log output → Default log verbosity → Debug

# Run with debug output
python test_runner.py --target esp32c6 --verbose
```

## Continuous Integration

The project uses GitHub Actions for automated testing. Tests run on:

- **Multiple ESP32 targets**: ESP32-C6, ESP32, ESP32-S3
- **Build configurations**: Debug and Release
- **Test categories**: Unit tests, integration tests
- **Coverage analysis**: Code coverage with reports

### CI Workflow Triggers

- Push to `main` or `develop` branches
- Pull requests to `main` or `develop`
- Manual workflow dispatch with options:
  - Test filter selection
  - Coverage enable/disable

### CI Artifacts

After each test run, the following artifacts are available:

- **Test Results**: JUnit XML, JSON reports
- **Coverage Reports**: HTML and LCOV format
- **Build Artifacts**: Firmware binaries, memory maps
- **Logs**: Detailed test execution logs

### Manual CI Trigger

```bash
# Trigger workflow via GitHub CLI
gh workflow run "ESP32 Unit Tests" \
  --field test_filter=EspGpio \
  --field coverage_enabled=true
```

## Writing Tests

### Test File Structure

Each test file should follow this structure:

```cpp
/**
 * @file test_esp_component.cpp
 * @brief Unit tests for EspComponent class
 */

#include "unity.h"
#include "unity_config.h"
#include "mock/mock_state_manager.h"

// Include component under test
#include "EspComponent.h"

// Test setup (called before each test)
void setUp(void) {
    mock_state_reset();
    // Component-specific setup
}

// Test teardown (called after each test)
void tearDown(void) {
    // Cleanup if needed
}

// Test cases
extern "C" {

void test_component_initialization(void) {
    // Arrange
    EspComponent component(/* parameters */);
    
    // Act
    bool result = component.Initialize();
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(component.IsInitialized());
}

void test_component_error_handling(void) {
    // Setup error injection
    mock_inject_error("function_name", ESP_ERR_INVALID_ARG, 0);
    
    EspComponent component(/* parameters */);
    
    // Should fail due to injected error
    TEST_ASSERT_FALSE(component.Initialize());
    
    // Clear error injection
    mock_clear_error_injection();
}

} // extern "C"
```

### Unity Assertions

The project provides custom assertions for ESP32 and HardFOC types:

```cpp
// ESP-IDF assertions
TEST_ASSERT_ESP_OK(esp_function_call());
TEST_ASSERT_NOT_ESP_OK(esp_function_call());
TEST_ASSERT_ESP_ERR(ESP_ERR_INVALID_ARG, esp_function_call());

// GPIO assertions
TEST_ASSERT_GPIO_SUCCESS(gpio.SetState(ACTIVE));
TEST_ASSERT_GPIO_ERROR(GPIO_ERR_INVALID_PIN, gpio.Initialize());

// ADC assertions
TEST_ASSERT_ADC_SUCCESS(adc.ReadChannel(0, &value));
TEST_ASSERT_ADC_ERROR(ADC_ERR_INVALID_CHANNEL, adc.ReadChannel(99, &value));

// Timing assertions
TEST_ASSERT_EXECUTION_TIME_LESS_THAN(100, {
    // Code that should execute in less than 100ms
});
```

### Mock System Usage

```cpp
// Configure mock behavior
mock_gpio_set_pin_state(5, 1);  // Set GPIO 5 to high
mock_adc_set_raw_value(0, 2, 2048);  // Set ADC unit 0, channel 2 to 2048

// Verify mock interactions
TEST_ASSERT_TRUE(mock_was_called("gpio_config"));
TEST_ASSERT_EQUAL_UINT32(3, mock_get_call_count("gpio_set_level"));

// Error injection testing
mock_inject_error("gpio_config", ESP_ERR_INVALID_ARG, 2);  // Fail after 2 calls
// ... test error handling ...
mock_clear_error_injection();
```

## Test Coverage

### Enabling Coverage

Coverage is enabled by default in Debug builds. To explicitly control:

```bash
# Build with coverage
idf.py -DCOVERAGE_ENABLED=ON build

# Build without coverage
idf.py -DCOVERAGE_ENABLED=OFF build
```

### Generating Coverage Reports

```bash
# After running tests, generate coverage report
cd test
gcovr --root . \
  --html-details build/coverage/coverage.html \
  --exclude "test/components/.*" \
  --exclude ".*build.*" \
  build/

# Open coverage report
open build/coverage/coverage.html
```

### Coverage Targets

The project aims for:
- **Line Coverage**: > 90%
- **Function Coverage**: > 95%
- **Branch Coverage**: > 85%

### Coverage Exclusions

Some code is excluded from coverage analysis:
- Mock implementations
- Test utilities
- Third-party components
- Platform-specific error handling

## Troubleshooting

### Common Issues

#### 1. Test Timeout

```
Error: Test monitoring ended: timeout
```

**Solution**: Increase timeout or check for infinite loops
```bash
# In test_runner.py, increase test_timeout
test_timeout = 600  # 10 minutes
```

#### 2. Hardware Not Found

```
Error: No ESP32 device found
```

**Solutions**:
- Check USB connection and drivers
- Specify port manually: `--port /dev/ttyUSB0`
- Use different USB cable/port
- Check device permissions

#### 3. Build Failures

```
Error: Component 'unity' not found
```

**Solution**: Ensure Unity component is properly configured
```bash
# Check component paths in test/CMakeLists.txt
# Verify EXTRA_COMPONENT_DIRS includes Unity path
```

#### 4. Mock Assertion Failures

```
Mock function not called as expected
```

**Solutions**:
- Verify mock setup in `setUp()`
- Check mock call expectations
- Use `mock_get_call_count()` to debug

### Debug Techniques

#### 1. Enable Verbose Logging

```cpp
// In test setup
mock_system_set_logging(true, ESP_LOG_VERBOSE);
```

#### 2. Print Mock State

```cpp
// Add to test for debugging
printf("GPIO pin 5 state: %d\n", mock_gpio_get_pin_state(5));
printf("Function called %d times\n", mock_get_call_count("gpio_config"));
```

#### 3. Use Memory Debugging

```bash
# Enable in sdkconfig
CONFIG_HEAP_POISONING_COMPREHENSIVE=y
CONFIG_HEAP_TASK_TRACKING=y
```

## Advanced Configuration

### Custom Test Configurations

Create target-specific configurations:

```bash
# test/sdkconfig.esp32c6
CONFIG_ESP32C6_SPECIFIC_OPTION=y

# test/sdkconfig.esp32
CONFIG_ESP32_SPECIFIC_OPTION=y
```

### Performance Testing

```cpp
void test_performance_gpio_toggle(void) {
    EspGpio gpio(5, GPIO_OUTPUT);
    gpio.Initialize();
    
    uint32_t start_time = esp_timer_get_time();
    
    for (int i = 0; i < 1000; i++) {
        gpio.SetState(ACTIVE);
        gpio.SetState(INACTIVE);
    }
    
    uint32_t end_time = esp_timer_get_time();
    uint32_t duration_us = end_time - start_time;
    
    // Assert performance requirement
    TEST_ASSERT_LESS_THAN_UINT32(10000, duration_us);  // < 10ms for 1000 toggles
}
```

### Memory Leak Testing

```cpp
void test_memory_leak_detection(void) {
    size_t initial_free = esp_get_free_heap_size();
    
    {
        EspGpio gpio(5, GPIO_OUTPUT);
        gpio.Initialize();
        // GPIO should clean up automatically when destroyed
    }
    
    size_t final_free = esp_get_free_heap_size();
    
    // Allow for small variations due to system overhead
    TEST_ASSERT_INT32_WITHIN(100, initial_free, final_free);
}
```

### Custom Mock Behaviors

```cpp
// Create custom mock response
void custom_gpio_config_mock(const gpio_config_t* config) {
    mock_record_call("gpio_config", (void*)config, sizeof(gpio_config_t));
    
    // Custom validation logic
    if (config->pin_bit_mask == 0) {
        // Simulate hardware error
        mock_inject_error("gpio_config", ESP_ERR_INVALID_ARG, 0);
    }
}
```

## Best Practices

### Test Organization

1. **One test per behavior**: Each test should verify one specific behavior
2. **Descriptive names**: Use clear, descriptive test function names
3. **Arrange-Act-Assert**: Structure tests with clear sections
4. **Independent tests**: Tests should not depend on each other

### Mock Usage

1. **Reset state**: Always reset mock state in `setUp()`
2. **Verify interactions**: Check that expected functions were called
3. **Error injection**: Test error paths using mock error injection
4. **Realistic behavior**: Make mocks behave like real hardware

### Performance

1. **Test execution time**: Keep individual tests under 1 second
2. **Resource cleanup**: Ensure proper cleanup in `tearDown()`
3. **Memory efficiency**: Avoid memory leaks in test code
4. **Parallel safety**: Design tests to be parallelizable

---

## Getting Help

For questions or issues with testing:

1. Check this documentation first
2. Review existing test examples
3. Check CI logs for similar issues
4. Create an issue in the project repository

Happy testing! 🧪