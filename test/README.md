# HardFOC IID Unit Tests

This directory contains comprehensive unit tests for the HardFOC IID ESP32 project using the Unity testing framework.

## Quick Start

```bash
# Prerequisites: ESP-IDF v5.5+, Python 3.8+
pip install pytest pytest-embedded pytest-embedded-serial-esp pytest-embedded-idf

# Build and run tests
cd test
idf.py set-target esp32c6
idf.py build
python test_runner.py
```

## Test Structure

- **`main/`**: Test source files
  - `test_esp_gpio_*.cpp`: EspGpio unit tests
  - `test_esp_adc_*.cpp`: EspAdc unit tests (planned)
  - `test_integration_*.cpp`: Integration tests
- **`components/`**: Test framework components
  - `unity/`: Unity testing framework wrapper
  - `esp_idf_mocks/`: Hardware mocking system
- **`test_runner.py`**: pytest-embedded test runner
- **`sdkconfig.defaults`**: Test-optimized ESP-IDF configuration

## Supported Targets

- ESP32-C6 (Primary)
- ESP32
- ESP32-S3

## Features

- ✅ **Unity Framework**: Lightweight embedded testing
- ✅ **Hardware Mocking**: Test without physical dependencies
- ✅ **CI Integration**: Automated testing in GitHub Actions
- ✅ **Coverage Reports**: Code coverage analysis with gcovr
- ✅ **Multi-Target**: Support for multiple ESP32 variants
- ✅ **Error Injection**: Comprehensive error path testing

## Test Categories

### EspGpio Tests
- **Basic**: Initialization, pin configuration, I/O operations
- **Interrupts**: Interrupt configuration, handling, callbacks
- **Advanced**: Glitch filtering, sleep configuration, power management
- **Integration**: Combined GPIO/ADC operations

### EspAdc Tests (Planned)
- **Basic**: Initialization, channel configuration, error handling
- **One-shot**: Single conversions, multi-channel operations
- **Continuous**: DMA-based continuous sampling
- **Calibration**: Hardware calibration and voltage accuracy
- **Filters**: Digital IIR filters and noise reduction
- **Monitors**: Threshold monitoring with callbacks

## CI/CD Integration

Tests automatically run on:
- Push to `main`/`develop` branches
- Pull requests
- Manual workflow dispatch

Artifacts generated:
- JUnit XML test results
- Code coverage reports (HTML/LCOV)
- Build artifacts and logs

## Coverage Targets

- Line Coverage: > 90%
- Function Coverage: > 95%
- Branch Coverage: > 85%

## Documentation

For detailed information, see:
- [`docs/TESTING.md`](../docs/TESTING.md): Comprehensive testing guide
- [CI Workflow](../.github/workflows/unit-tests.yml): Automated testing pipeline

## Development

To add new tests:

1. Create test file in `main/` directory
2. Follow naming pattern: `test_component_feature.cpp`
3. Use Unity assertions and mock system
4. Update `main/CMakeLists.txt` to include new test file
5. Run tests locally before pushing

## Mock System

The project includes a comprehensive mocking system for ESP-IDF APIs:

```cpp
// Configure mock behavior
mock_gpio_set_pin_state(5, 1);
mock_adc_set_raw_value(0, 2, 2048);

// Verify interactions
TEST_ASSERT_TRUE(mock_was_called("gpio_config"));
TEST_ASSERT_EQUAL_UINT32(3, mock_get_call_count("gpio_set_level"));

// Error injection
mock_inject_error("gpio_config", ESP_ERR_INVALID_ARG, 0);
```

## Running Specific Tests

```bash
# Run with verbose output
python test_runner.py --target esp32c6 --verbose

# Run on specific hardware port
python test_runner.py --target esp32c6 --port /dev/ttyUSB0

# Generate coverage report
idf.py -DCOVERAGE_ENABLED=ON build
python test_runner.py
gcovr --html-details build/coverage/coverage.html build/
```

Happy Testing! 🧪