# ESP32-C6 UART Comprehensive Test Suite Documentation

## Overview

The UART Comprehensive Test Suite provides extensive validation of the `EspUart` class for ESP32 platforms using ESP-IDF v5.5+. This comprehensive test suite demonstrates complete UART functionality including basic communication, baud rate configuration, flow control, advanced features, callbacks, statistics, diagnostics, printf support, error handling, ESP32-C6 specific features, performance testing, callback verification, user event tasks, and comprehensive pattern detection with a focus on embedded environments using `noexcept` functions.

**✅ Status: Successfully tested on ESP32-C6-DevKitM-1 hardware with 19 comprehensive tests**

### Supported ESP32 Variants

The implementation automatically detects and adapts to different ESP32 variants with their specific UART port allocation constraints:

| ESP32 Variant | Total UART Ports | Port Allocation | Special Notes |
|---------------|------------------|-----------------|---------------|
| ESP32         | 3                | UART0, UART1, UART2 | UART0 is console/debug port |
| ESP32-S2      | 3                | UART0, UART1, UART2 | UART0 is console/debug port |
| ESP32-S3      | 3                | UART0, UART1, UART2 | UART0 is console/debug port |
| ESP32-C3      | 2                | UART0, UART1 | UART0 is console/debug port |
| ESP32-C6      | 3                | UART0, UART1, UART2 | UART0 is console/debug port |
| ESP32-H2      | 2                | UART0, UART1 | UART0 is console/debug port |

**⚠️ Important Note**: UART0 is the default console/debug port on all ESP32 variants. For testing, use UART1 or UART2 to avoid debug output interference.

## Features Tested

### Core Functionality
- **Constructor/Destructor Behavior**: Object lifecycle management and resource allocation
- **Lifecycle Management**: Initialize/Deinitialize operations with state validation
- **Basic Communication**: TX/RX operations with external loopback testing
- **Baud Rate Configuration**: Multi-baud rate testing and validation
- **Flow Control**: Hardware RTS/CTS and software XON/XOFF flow control
- **Error Handling**: Comprehensive error condition testing and recovery

### Advanced UART Features
- **Pattern Detection**: ESP-IDF v5.5 pattern detection for AT commands and custom patterns
- **Buffer Operations**: ReadUntil, ReadLine, and buffer management
- **Communication Modes**: UART, RS485, and IrDA mode configuration
- **Async Operations**: Interrupt-driven operation with event queues
- **Callbacks**: Event-driven callback system with user data support
- **Statistics and Diagnostics**: Comprehensive performance metrics and error reporting

### ESP32-C6 Specific Features
- **Multi-Port Support**: UART0, UART1, UART2 with independent configuration
- **Hardware Flow Control**: RTS/CTS pin configuration and control
- **Break Signal Support**: Break condition generation and detection
- **Signal Inversion**: TX/RX signal inversion capabilities
- **Wakeup Configuration**: UART wakeup from light sleep mode
- **Printf Support**: Formatted output with variable argument support

### Pattern Detection Testing
- **Line Pattern Detection**: Newline (`\n`) character detection for line-oriented protocols
- **AT Command Detection**: Triple plus (`+++`) pattern detection for AT escape sequences
- **Event-Driven Processing**: Comprehensive event queue monitoring and processing
- **Timing Optimization**: Relaxed timing parameters for reliable pattern detection
- **Position Tracking**: Pattern position detection and data extraction

### Testing Infrastructure
- **External Loopback Testing**: TX→RX jumper wire testing for reliable validation
- **Test Progression Indicator**: GPIO14 visual feedback for test progress
- **Comprehensive Event Monitoring**: UART event queue analysis and validation
- **User Event Tasks**: FreeRTOS task creation for event handling
- **Statistics Tracking**: Performance metrics and operation counting

## Hardware Setup

### ESP32-C6-DevKitM-1 Pin Configuration
- **GPIO4 (RX)**: UART1 reception input
- **GPIO5 (TX)**: UART1 transmission output  
- **GPIO6 (RTS)**: UART1 RTS flow control
- **GPIO7 (CTS)**: UART1 CTS flow control
- **GPIO14**: Test progression indicator (visual feedback)

### External Loopback Testing Setup
```
ESP32-C6-DevKitM-1
├── GPIO5 (UART1 TX) ──► Jumper Wire ──► GPIO4 (UART1 RX)
├── GPIO6 (UART1 RTS) ──► Optional ──► GPIO7 (UART1 CTS)
├── GPIO14 (Test Progress) ──► LED indicator for test progression
└── External Loopback: Transmission/reception verification
```

**For External Loopback Testing:**
- Connect GPIO5 (TX) to GPIO4 (RX) with a jumper wire
- This creates a loopback for transmission/reception verification
- GPIO14 provides visual feedback of test progression
- RTS/CTS pins can be connected for flow control testing

### Test Progression Indicator
```
ESP32-C6 GPIO14 ──► Visual LED Indicator
                   │
                   └──► Toggles HIGH/LOW for each completed test
                   └──► Provides feedback for logic analyzer capture
```

**Progression Indicator Features:**
- ✅ Visual feedback for test completion
- ✅ Logic analyzer triggering reference
- ✅ Automated test sequencing verification
- ✅ Real-time test progress monitoring

### Logic Analyzer Setup
```
ESP32-C6 GPIO4  ──► Logic Analyzer Channel 0 (UART1 RX)
ESP32-C6 GPIO5  ──► Logic Analyzer Channel 1 (UART1 TX)
ESP32-C6 GPIO14 ──► Logic Analyzer Channel 2 (test progression)
ESP32-C6 GND    ──► Logic Analyzer Ground
```

**Logic Analyzer Settings:**
- Sample rate: 1MHz or higher
- Voltage threshold: 1.65V (3.3V logic)
- Trigger: Rising edge on GPIO14 (test progression)
- Capture: Multi-channel for comprehensive analysis

## Running the Tests

### Prerequisites
- ESP-IDF v5.5 or later
- ESP32-C6-DevKitM-1 development board
- Jumper wire for loopback testing (GPIO5 → GPIO4)
- Optional: Logic analyzer for signal verification

### Using Build Scripts (Recommended)
```bash
# Navigate to ESP32 examples directory
cd examples/esp32

# Build UART test
./scripts/build_example.sh uart_test Release

# Flash to device and monitor
./scripts/flash_example.sh uart_test Release flash_monitor
```

### Direct ESP-IDF Build (Alternative)
```bash
# Set target
export IDF_TARGET=esp32c6

# Build UART test
idf.py build -DEXAMPLE_TYPE=uart_test

# Flash to device
idf.py flash monitor
```

### CI/CD Integration
The test is automatically included in the CI pipeline and will run in both Release and Debug configurations:
```yaml
matrix:
  example_type: [..., uart_test, ...]
```

## Test Categories

### 1. Constructor/Destructor Tests
- `test_uart_construction`: Validates proper object initialization and multiple instance support
- `test_uart_initialization`: Tests manual initialization/deinitialization with state validation

### 2. Basic Communication Tests
- `test_uart_basic_communication`: Basic TX/RX operations with external loopback
- `test_uart_baud_rate_configuration`: Multi-baud rate testing (9600 to 230400)
- `test_uart_flow_control`: Hardware RTS/CTS and software XON/XOFF flow control

### 3. Advanced Features Tests
- `test_uart_pattern_detection`: Comprehensive pattern detection testing including:
  - Line pattern detection (`\n`) with 3/3 patterns detected
  - AT command pattern detection (`+++`) with 2/2 patterns detected
  - Event-driven processing with proper timing optimization
  - Pattern position tracking and data extraction
- `test_uart_buffer_operations`: ReadUntil, ReadLine, and buffer management
- `test_uart_advanced_features`: Break signals, loopback mode, signal inversion, wakeup
- `test_uart_communication_modes`: UART, RS485, and IrDA mode configuration

### 4. Async Operations Tests
- `test_uart_async_operations`: Interrupt-driven operation with event queues
- `test_uart_callbacks`: Event queue access and interrupt configuration
- `test_uart_callback_verification`: Event-driven callback system validation

### 5. Statistics and Diagnostics Tests
- `test_uart_statistics_diagnostics`: Comprehensive performance metrics and error reporting
- `test_uart_printf_support`: Formatted output with variable argument support
- `test_uart_error_handling`: Error condition testing and graceful failure handling

### 6. ESP32-C6 Specific Tests
- `test_uart_esp32c6_features`: ESP32-C6 specific UART capabilities
- `test_uart_performance`: Performance testing and timing validation
- `test_uart_user_event_task`: FreeRTOS task creation for event handling

### 7. Event-Driven Pattern Detection Tests
- `test_uart_event_driven_pattern_detection`: Comprehensive event queue monitoring
- `test_uart_cleanup`: Resource cleanup and memory management

## Pattern Detection Specifications

### ESP-IDF v5.5 Pattern Detection
The test suite uses the modern ESP-IDF v5.5 pattern detection API:

```cpp
esp_err_t uart_enable_pattern_det_baud_intr(
    uart_port_t uart_num, 
    char pattern_chr,           // Character to detect
    uint8_t chr_num,            // Number of consecutive characters
    int chr_tout,               // Timeout between characters (baud cycles)
    int post_idle,              // Idle time after last character
    int pre_idle                 // Idle time before first character
);
```

### Pattern Detection Parameters
| Pattern Type | Character | Count | chr_tout | post_idle | pre_idle | Purpose |
|--------------|-----------|-------|----------|-----------|----------|---------|
| Line Pattern | `\n` | 1 | 9 | 0 | 0 | Line-oriented protocols |
| AT Escape | `+` | 3 | 5 | 0 | 0 | AT command escape sequences |

### Timing Optimization
**Relaxed Timing Parameters** for reliable pattern detection:
- **`chr_tout`**: Reduced from 9 to 5 baud cycles (more permissive)
- **`post_idle`**: Set to 0 (no idle requirement after pattern)
- **`pre_idle`**: Set to 0 (no idle requirement before pattern)

**Why Reduced Timing is Better:**
- **More Permissive**: Allows for slight timing variations in data transmission
- **Better Reliability**: Reduces false negatives from strict timing requirements
- **Real-World Compatibility**: Matches actual communication timing patterns

## Expected Test Results

### Successful Test Output
```
[UART_Test] ╔════════════════════════════════════════════════════════════════════════════════╗
[UART_Test] ║                   ESP32-C6 UART COMPREHENSIVE TEST SUITE                       ║
[UART_Test] ║                         HardFOC Internal Interface                             ║
[UART_Test] ╚════════════════════════════════════════════════════════════════════════════════╝
[UART_Test] ║ Target: ESP32-C6 DevKit-M-1                                                    ║
[UART_Test] ║ ESP-IDF: v5.5+                                                                 ║
[UART_Test] ║ Features: UART, Baud Rate Configuration, Flow Control, Pattern Detection,      ║
[UART_Test] ║ Buffer Operations, Advanced Features, Communication Modes, Async Operations,   ║
[UART_Test] ║ Callbacks, Statistics and Diagnostics, printf Support, Error Handling,         ║
[UART_Test] ║ ESP32-C6 Features, Performance, Callback Verification, User Event Task,        ║
[UART_Test] ║ Event-Driven Pattern Detection, Cleanup                                        ║
[UART_Test] ║ Architecture: noexcept (no exception handling)                                 ║
[UART_Test] ╚════════════════════════════════════════════════════════════════════════════════╝

[UART_Test] === CONSTRUCTOR/DESTRUCTOR TESTS ===
[UART_Test] [SUCCESS] PASSED: test_uart_construction (31.04 ms)
[UART_Test] [SUCCESS] PASSED: test_uart_initialization (65.99 ms)

[UART_Test] === BASIC COMMUNICATION TESTS ===
[UART_Test] [SUCCESS] PASSED: test_uart_basic_communication (74.25 ms)
[UART_Test] [SUCCESS] PASSED: test_uart_baud_rate_configuration (126.07 ms)

[UART_Test] === ADVANCED FEATURES TESTS ===
[UART_Test] [SUCCESS] PASSED (task): test_uart_pattern_detection (444.76 ms)
[UART_Test] [SUCCESS] PASSED: test_uart_buffer_operations (269.45 ms)
[UART_Test] [SUCCESS] PASSED: test_uart_advanced_features (673.89 ms)
[UART_Test] [SUCCESS] PASSED: test_uart_communication_modes (96.81 ms)
[UART_Test] [SUCCESS] PASSED: test_uart_async_operations (181.19 ms)
[UART_Test] [SUCCESS] PASSED: test_uart_callbacks (94.81 ms)
[UART_Test] [SUCCESS] PASSED: test_uart_statistics_diagnostics (101.05 ms)
[UART_Test] [SUCCESS] PASSED: test_uart_printf_support (75.37 ms)
[UART_Test] [SUCCESS] PASSED: test_uart_error_handling (85.34 ms)

[UART_Test] === ESP32-C6 SPECIFIC TESTS ===
[UART_Test] [SUCCESS] PASSED: test_uart_esp32c6_features (135.87 ms)
[UART_Test] [SUCCESS] PASSED: test_uart_performance (82.07 ms)
[UART_Test] [SUCCESS] PASSED: test_uart_callback_verification (1404.49 ms)

[UART_Test] === USER EVENT TASK TEST ===
[UART_Test] [SUCCESS] PASSED: test_uart_user_event_task (1483.02 ms)

[UART_Test] === COMPREHENSIVE EVENT-DRIVEN PATTERN DETECTION TEST ===
[UART_Test] [SUCCESS] PASSED: test_uart_event_driven_pattern_detection (419.06 ms)

[UART_Test] === CLEANUP TEST ===
[UART_Test] [SUCCESS] PASSED: test_uart_cleanup (72.50 ms)

[UART_Test] === UART TEST SUMMARY ===
[UART_Test] Total: 19, Passed: 19, Failed: 0, Success: 100.00%, Time: 5917.04 ms
[UART_Test] [SUCCESS] ALL UART TESTS PASSED!
```

### Pattern Detection Test Results
**Test 1: Line Pattern Detection (`\n`)**
```
[UART_Test] === Test 1: Line-oriented pattern detection ('\n') ===
[UART_Test] Line pattern detection enabled
[UART_Test] Test data sent: 'Line1\nLine2\nLine3\n' (length: 18)
[UART_Test] UART_PATTERN_DET event received!
[UART_Test] Pattern 1 detected at position: 5
[UART_Test] UART_PATTERN_DET event received!
[UART_Test] Pattern 2 detected at position: 11
[UART_Test] UART_PATTERN_DET event received!
[UART_Test] Pattern 3 detected at position: 17
[UART_Test] Line pattern detection: 3/3 patterns detected. PASSED
```

**Test 2: AT Pattern Detection (`+++`)**
```
[UART_Test] === Test 2: AT escape sequence pattern detection ('+++') ===
[UART_Test] AT escape sequence pattern detection enabled
[UART_Test] AT test data sent (length: 42)
[UART_Test] UART_PATTERN_DET event received for +++!
[UART_Test] AT Pattern 1 detected at position: 9
[UART_Test] UART_PATTERN_DET event received for +++!
[UART_Test] AT Pattern 2 detected at position: 30
[UART_Test] AT pattern detection: 2/2 patterns detected. PASSED
```

### Event-Driven Pattern Detection Results
```
[UART_Test] Event-driven pattern detection results:
[UART_Test]   Total events received: 3
[UART_Test]   Data events: 0
[UART_Test]   Pattern events: 3
[UART_Test]   Other events: 0
[UART_Test]   Pattern detected: YES
[UART_Test]   Pattern position: 8
[UART_Test]   Expected patterns: 3
[UART_Test] [SUCCESS] Event-driven pattern detection test completed successfully
```

## Troubleshooting

### Common Issues

#### Test Failures
- **Pattern Detection Issues**: Verify timing parameters and interrupt configuration
- **GPIO Conflicts**: Check pin availability and external loopback connection
- **Initialization Failures**: Ensure ESP-IDF v5.5+ and proper hardware
- **UART Port Conflicts**: Use UART1 or UART2, avoid UART0 (console port)

#### Pattern Detection Issues
- **No Pattern Events**: Check interrupt configuration and timing parameters
- **Timing Issues**: Verify `chr_tout`, `post_idle`, and `pre_idle` values
- **Buffer Overflow**: Add delays between data transmission and event processing
- **Event Queue Issues**: Ensure event queue is properly configured and sized

#### External Loopback Issues
- **No Reception**: Verify jumper wire connection GPIO5 → GPIO4
- **Data Mismatch**: Check for loose connections or interference
- **Timing Issues**: Ensure proper UART configuration and baud rate
- **Flow Control**: Verify RTS/CTS configuration if using hardware flow control

#### Test Progression Indicator Issues
- **No GPIO14 Activity**: Check GPIO14 initialization and connectivity
- **Irregular Toggle Pattern**: Verify test sequence completion
- **Logic Analyzer Sync**: Use GPIO14 as trigger reference

#### UART Configuration Issues
- **Port Selection**: Use `TEST_UART_PORT_1` for testing (avoid UART0)
- **Pin Configuration**: Verify TX/RX/RTS/CTS pin assignments
- **Baud Rate**: Ensure consistent baud rate across all operations
- **Buffer Sizes**: Use appropriate buffer sizes for ESP32-C6 (256+ bytes)

### Debug Mode
Enable detailed logging by building in Debug mode:
```bash
# Using build scripts (recommended)
./scripts/build_example.sh uart_test Debug

# Or direct ESP-IDF build
idf.py build -DEXAMPLE_TYPE=uart_test -DBUILD_TYPE=Debug
```

### Pattern Detection Debugging
```cpp
// Enable debug logging for pattern detection
ESP_LOGI(TAG, "Pattern detection enabled: '%c' x%d (chr_tout=%d, post_idle=%d, pre_idle=%d)",
         pattern_chr, chr_num, chr_tout, post_idle, pre_idle);

// Monitor event queue activity
ESP_LOGI(TAG, "Event received: type=%d, size=%zu", event.type, event.size);

// Check pattern position
int pattern_pos = uart->PopPatternPosition();
ESP_LOGI(TAG, "Pattern detected at position: %d", pattern_pos);
```

## Performance Metrics

### Build Information
- **Build Status**: ✅ SUCCESS
- **Target**: ESP32-C6
- **Binary Size**: Varies based on configuration
- **ESP-IDF Version**: v5.5+
- **Test Count**: 19 comprehensive tests

### Typical Results (ESP32-C6 @ 160MHz)
- **Initialization Time**: ~1ms
- **UART Configuration**: ~0.5ms
- **Pattern Detection Setup**: ~1ms
- **Constructor/Destructor Tests**: ~97ms total
- **Basic Communication Tests**: ~200ms total
- **Advanced Features Tests**: ~1.5s total
- **Async Operations Tests**: ~181ms
- **Statistics and Diagnostics**: ~101ms
- **ESP32-C6 Specific Tests**: ~1.6s total
- **User Event Task Test**: ~1.5s (comprehensive task testing)
- **Event-Driven Pattern Detection**: ~419ms (comprehensive monitoring)
- **Complete Test Suite**: ~5.9s (5917ms)

### Memory Usage
- **RAM**: ~4-6KB for enhanced test framework and event processing
- **Flash**: ~25-30KB for comprehensive test code
- **Event Queue**: 32 events (configurable, used extensively)
- **Pattern Queue**: 16-32 pattern positions (configurable)
- **Task Stacks**: 8KB for pattern detection test, 4KB for user event task
- **Buffer Management**: 256+ byte RX/TX buffers for reliable operation

## Integration with Development Workflow

### Continuous Integration
The test automatically runs in CI for:
- Pull request validation
- Main branch commits
- Release candidate testing
- ESP32 variant compatibility verification

### Hardware-in-the-Loop Testing
For production validation:
1. Use external loopback (GPIO5 → GPIO4) for reliable testing
2. Monitor GPIO14 for test progression verification
3. Use logic analyzer for signal verification
4. Run automated test suite (19 tests)
5. Validate pattern detection against specifications
6. Verify UART port independence and configuration

## Advanced Configuration

### Custom UART Ports
Modify the test for different hardware:
```cpp
// UART1 configuration for testing (avoid UART0 - console port)
static constexpr hf_u8_t TEST_UART_PORT_1 = 1;
static constexpr hf_u8_t TEST_TX_PIN = 5;   // UART1 TX
static constexpr hf_u8_t TEST_RX_PIN = 4;   // UART1 RX
static constexpr hf_u8_t TEST_RTS_PIN = 6;  // UART1 RTS
static constexpr hf_u8_t TEST_CTS_PIN = 7;  // UART1 CTS
```

### Pattern Detection Configuration
```cpp
// Line pattern detection with optimized timing
result = uart->EnablePatternDetection('\n', 1, 9, 0, 0);

// AT escape sequence with relaxed timing
result = uart->EnablePatternDetection('+', 3, 5, 0, 0);

// Custom pattern with specific timing requirements
result = uart->EnablePatternDetection('$', 2, 10, 5, 5);
```

### Event Queue Configuration
```cpp
// Enhanced event queue for comprehensive testing
config.event_queue_size = 32;  // Larger queue for pattern detection

// Interrupt configuration for pattern detection
result = uart->ConfigureInterrupts(
    UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M, 32, 5);
```

### Test Parameters
Customize test behavior:
```cpp
// Pattern detection timing optimization
static constexpr int PATTERN_CHR_TIMEOUT = 5;    // Relaxed timing
static constexpr int PATTERN_POST_IDLE = 0;      // No idle requirement
static constexpr int PATTERN_PRE_IDLE = 0;       // No idle requirement

// Test progression configuration
static constexpr uint32_t TEST_PROGRESS_DELAY_MS = 100; // Progress indicator timing
static constexpr uint32_t PATTERN_TEST_TIMEOUT_MS = 3000; // Pattern detection timeout
```

## ESP32-C6 Specific Features

### UART Peripheral
- **Ports**: 3 UART ports (UART0, UART1, UART2)
- **Clock Source**: PLL_F80M (80 MHz) with automatic fallback
- **FIFO Size**: 128 bytes hardware FIFO
- **Buffer Support**: Configurable ring buffers (256+ bytes recommended)
- **Interrupt Support**: Comprehensive interrupt configuration
- **Pattern Detection**: Hardware pattern detection with ESP-IDF v5.5

### Pattern Detection Capabilities
- **Character Patterns**: Single or multiple consecutive characters
- **Timing Control**: Configurable character timeout and idle periods
- **Position Tracking**: Pattern position detection and queue management
- **Event Generation**: Automatic UART_PATTERN_DET events
- **Queue Management**: Configurable pattern position queue

### Testing Advantages
- **Port Independence**: UART1/UART2 avoid console interference
- **External Loopback**: Simple jumper wire testing
- **Visual Feedback**: GPIO14 progression indicator
- **Comprehensive Testing**: 19 individual test cases
- **Pattern Validation**: Line and AT command pattern detection
- **Event Monitoring**: Complete event queue analysis
- **Performance Metrics**: Timing and statistics validation

### ESP32-C6 Specific Observations
From the actual test output, several ESP32-C6 specific behaviors were observed:

**Break Signal Handling:**
```
W (3456) EspUart: uart_write_bytes_with_break failed (ERROR), trying manual break via GPIO
W (3556) EspUart: Break condition sent via manual GPIO control for 100 ms (ESP32-C6 fallback)
```
- ESP32-C6 has limited break signal support in ESP-IDF v5.5
- Automatic fallback to manual GPIO control for break signals
- This is expected behavior for this MCU variant

**Communication Mode Support:**
```
W (4276) EspUart: RS485 advanced features not supported in ESP-IDF v5.5
W (4286) EspUart: IrDA not supported in ESP-IDF v5.5
W (4286) UART_Test: IrDA not supported on ESP32-C6 (expected): 4
```
- RS485 mode supported but with limited advanced features
- IrDA mode not supported (expected limitation)
- UART mode fully functional

**Pattern Detection Performance:**
- **Line Pattern Test**: 3/3 patterns detected in ~445ms
- **AT Pattern Test**: 2/2 patterns detected in ~445ms  
- **Event-Driven Test**: 3/3 patterns detected in ~419ms
- **User Event Task**: Comprehensive task testing in ~1.5s

## References

- [ESP-IDF UART Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/uart.html)
- [ESP-IDF Pattern Detection API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/uart.html#_CPPv429uart_enable_pattern_det_baud_intr)
- [ESP32-C6 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf)
- [ESP-IDF v5.5 Migration Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/migration-guides/release-5.x/5.0/peripherals.html)
- [ESP32-C6-DevKitM-1 User Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/hw-reference/esp32c6/user-guide-devkitm-1.html)
- [UART Pattern Detection Best Practices](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-guides/uart.html#uart-pattern-detection)
