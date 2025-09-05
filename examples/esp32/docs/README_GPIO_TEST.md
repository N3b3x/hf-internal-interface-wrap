# ESP32-C6 GPIO Comprehensive Test Suite

This directory contains a comprehensive testing suite specifically designed for the EspGpio class on
ESP32-C6 DevKit-M-1 hardware using ESP-IDF v5.5+.

## Overview

The GPIO test suite provides thorough validation of all GPIO functionalities including:

### Core Features
- ✅ Basic GPIO initialization and configuration
- ✅ Input/output operations and state management
- ✅ Pull resistor configuration (floating, pull-up, pull-down)
- ✅ Pin validation and error handling

### Advanced Features (ESP32-C6 Specific)
- ✅ Interrupt functionality with multiple trigger types
- ✅ Drive capability testing (5mA to 40mA)
- ✅ RTC GPIO support for low-power operations
- ✅ Glitch filter configuration
- ✅ Sleep and wake-up functionality
- ✅ Hold functionality for state retention

### Performance & Robustness
- ✅ Stress testing with rapid state changes
- ✅ Concurrent GPIO operations
- ✅ Loopback testing (requires physical wire connection)
- ✅ Power consumption analysis
- ✅ Comprehensive diagnostics and statistics

## Hardware Setup

### ESP32-C6 DevKit-M-1 Pin Layout

The test suite uses the following safe pins on ESP32-C6 DevKit-M-1:

```text
Safe Test Pins:
┌─────────────────────────────────────┐
│ Function            │ GPIO Pin      │
├─────────────────────┼───────────────┤
│ LED Output          │ GPIO 8        │
│ Digital Out 1       │ GPIO 10       │
│ Digital Out 2       │ GPIO 11       │
│ Digital Input 1     │ GPIO 0 (BOOT) │
│ Digital Input 2     │ GPIO 1        │
│ Interrupt Pin       │ GPIO 2        │
│ Pull Test Pin       │ GPIO 3        │
│ Drive Test Pin      │ GPIO 16       │
│ RTC GPIO Pin        │ GPIO 7        │
│ Analog Pin          │ GPIO 6        │
│ Loopback Out        │ GPIO 20       │
│ Loopback In         │ GPIO 21       │
│ Stress Test Pin     │ GPIO 23       │
└─────────────────────┴───────────────┘

Pins to Avoid:
┌─────────────────────────────────────┐
│ GPIO 9              │ Boot strap    │
│ GPIO 15             │ Boot strap    │
│ GPIO 12, 13         │ USB-JTAG      │
│ GPIO 24-30          │ SPI Flash     │
└─────────────────────┴───────────────┘
```text

### Optional Physical Connections

For complete loopback testing, connect:
- **GPIO 20** (Loopback Out) → **GPIO 21** (Loopback In)

## Building and Running

### Build Commands

```bash
## Build GPIO test suite (Release)
idf.py build -DEXAMPLE*TYPE=gpio*test -DBUILD*TYPE=Release

## Build GPIO test suite (Debug)
idf.py build -DEXAMPLE*TYPE=gpio*test -DBUILD*TYPE=Debug

## Flash and monitor
idf.py flash monitor -DEXAMPLE*TYPE=gpio*test
```text

### CI Pipeline

The GPIO test is integrated into the CI pipeline:

```yaml
## In .github/workflows/esp32-component-ci.yml
example*type: [comprehensive, ascii*art, nimble*test, gpio*test]
```text

Build artifacts are available as:
- `fw-gpio*test-release-v5.5-Release`
- `fw-gpio*test-release-v5.5-Debug`

## Test Categories

### 1. Basic Functionality Tests
- GPIO initialization and configuration modes
- Basic input/output operations
- State management and verification

### 2. Configuration Tests
- Pull resistor functionality
- Direction switching
- Output mode configuration

### 3. Advanced Feature Tests
- Interrupt configuration and handling
- Drive capability settings
- Hardware verification functions

### 4. ESP32-C6 Specific Tests
- RTC GPIO operations
- Glitch filter functionality
- Sleep/wake operations
- Hold functionality

### 5. Robustness Tests
- Error handling with invalid pins
- Stress testing with rapid operations
- Concurrent GPIO operations
- Pin validation

### 6. Performance Tests
- Timing analysis
- Power consumption characteristics
- Operation statistics

## Expected Output

```text
╔══════════════════════════════════════════════════════════════════════════════╗
║                    ESP32-C6 GPIO COMPREHENSIVE TEST SUITE                   ║
║                         HardFOC Internal Interface                          ║
╠══════════════════════════════════════════════════════════════════════════════╣
║ Target: ESP32-C6 DevKit-M-1                                                 ║
║ ESP-IDF: v5.5+                                                              ║
║ Features: GPIO, Interrupts, RTC, Sleep, Advanced Features                  ║
╚══════════════════════════════════════════════════════════════════════════════╝

Starting comprehensive GPIO testing...

✅ PASSED: test*basic*gpio*functionality (15.23 ms)
✅ PASSED: test*gpio*initialization*and*configuration (22.45 ms)
✅ PASSED: test*gpio*input*output*operations (18.67 ms)
...

╔══════════════════════════════════════════════════════════════════════════════╗
║                           GPIO TEST SUMMARY                                 ║
╠══════════════════════════════════════════════════════════════════════════════╣
║ Total Tests:       18                                                       ║
║ Passed Tests:      18                                                       ║
║ Failed Tests:       0                                                       ║
║ Success Rate:   100.00%                                                     ║
║ Total Exec Time:  324.56 ms                                                 ║
╚══════════════════════════════════════════════════════════════════════════════╝

🎉 ALL GPIO TESTS PASSED! 🎉
```text

## Development Notes

### Adding New Tests

To add a new test function:

1. **Declare the function:**
   ```cpp
   bool test*my*new*functionality();
   ```

1. **Implement the test:**
   ```cpp
   bool test*my*new*functionality() {
     ESP*LOGI(TAG, "=== Testing My New Functionality ===");
     try {
       // Test implementation
       return true;
     } catch (const std::exception& e) {
       ESP*LOGE(TAG, "Exception: %s", e.what());
       return false;
     }
   }
   ```

1. **Add to test execution:**
   ```cpp
   RUN*TEST(test*my*new*functionality);
   ```

### Test Patterns

The test suite follows these patterns:

- **Consistent logging** with clear test boundaries
- **Exception handling** for robust error reporting
- **State verification** after each operation
- **Resource cleanup** in destructors
- **Timing measurement** for performance analysis

### Debugging

For debugging individual tests:

1. **Enable debug build:**
   ```bash
   idf.py build -DEXAMPLE*TYPE=gpio*test -DBUILD*TYPE=Debug
   ```

1. **Add debug prints:**
   ```cpp
   ESP*LOGD(TAG, "Debug info: %d", value);
   ```

1. **Use specific pin for debugging:**
   ```cpp
   static constexpr hf*pin*num*t DEBUG_PIN = 22;
   ```

## Integration with Main Project

This GPIO test suite serves as:

1. **Validation tool** for EspGpio implementation
2. **Regression testing** for GPIO functionality
3. **Performance benchmarking** for optimization
4. **Hardware verification** for ESP32-C6 compatibility
5. **Documentation** of GPIO capabilities and usage patterns

## Contributing

When modifying the GPIO test suite:

1. **Maintain pin safety** - only use designated safe pins
2. **Add comprehensive logging** for test traceability
3. **Include error handling** for robustness
4. **Update documentation** for new test categories
5. **Verify CI integration** for automated testing

## License

Copyright HardFOC - Internal Interface Testing Suite
