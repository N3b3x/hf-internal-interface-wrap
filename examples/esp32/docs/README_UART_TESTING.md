# ESP32-C6 UART Comprehensive Test Suite Documentation

## Overview

The UART Comprehensive Test Suite provides extensive validation of the `EspUart` class for ESP32
platforms using ESP-IDF v5.5+.
This comprehensive test suite demonstrates complete UART functionality including basic
communication,
baud rate configuration, flow control, advanced features, callbacks, statistics, diagnostics,
printf support, error handling, ESP32-C6 specific features, performance testing,
callback verification, user event tasks,
and comprehensive pattern detection with a focus on embedded environments using `noexcept`
functions.

**✅ Status: Successfully tested on ESP32-C6-DevKitM-1 hardware with 19 comprehensive tests**

### Supported ESP32 Variants

The implementation automatically detects and adapts to different ESP32 variants with their specific
UART port allocation constraints:

| ESP32 Variant | Total UART Ports | Port Allocation | Special Notes |

|---------------|------------------|-----------------|---------------|

| ESP32         | 3                | UART0, UART1, UART2 | UART0 is console/debug port |

| ESP32-S2      | 3                | UART0, UART1, UART2 | UART0 is console/debug port |

| ESP32-S3      | 3                | UART0, UART1, UART2 | UART0 is console/debug port |

| ESP32-C3      | 2                | UART0, UART1 | UART0 is console/debug port |

| ESP32-C6      | 3                | UART0, UART1, UART2 | UART0 is console/debug port |

| ESP32-H2      | 2                | UART0, UART1 | UART0 is console/debug port |

**⚠️ Important Note**: UART0 is the default console/debug port on all ESP32 variants.
For testing, use UART1 or UART2 to avoid debug output interference.

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
```text
ESP32-C6-DevKitM-1
├── GPIO5 (UART1 TX) ──► Jumper Wire ──► GPIO4 (UART1 RX)
├── GPIO6 (UART1 RTS) ──► Optional ──► GPIO7 (UART1 CTS)
├── GPIO14 (Test Progress) ──► LED indicator for test progression
└── External Loopback: Transmission/reception verification
```text

**For External Loopback Testing:**
- Connect GPIO5 (TX) to GPIO4 (RX) with a jumper wire
- This creates a loopback for transmission/reception verification
- GPIO14 provides visual feedback of test progression
- RTS/CTS pins can be connected for flow control testing

### Test Progression Indicator
```text
ESP32-C6 GPIO14 ──► Visual LED Indicator
                   │
                   └──► Toggles HIGH/LOW for each completed test
                   └──► Provides feedback for logic analyzer capture
```text

**Progression Indicator Features:**
- ✅ Visual feedback for test completion
- ✅ Logic analyzer triggering reference
- ✅ Automated test sequencing verification
- ✅ Real-time test progress monitoring

### Logic Analyzer Setup
```text
ESP32-C6 GPIO4  ──► Logic Analyzer Channel 0 (UART1 RX)
ESP32-C6 GPIO5  ──► Logic Analyzer Channel 1 (UART1 TX)
ESP32-C6 GPIO14 ──► Logic Analyzer Channel 2 (test progression)
ESP32-C6 GND    ──► Logic Analyzer Ground
```text

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
## Navigate to ESP32 examples directory
cd examples/esp32

## Build UART test
./scripts/build*example.sh uart*test Release

## Flash to device and monitor
./scripts/flash*example.sh uart*test Release flash*monitor
```text

### Direct ESP-IDF Build (Alternative)
```bash
## Set target
export IDF*TARGET=esp32c6

## Build UART test
idf.py build -DEXAMPLE*TYPE=uart*test

## Flash to device
idf.py flash monitor
```text

### CI/CD Integration
The test is automatically included in the CI pipeline and will run in both Release and Debug
configurations:
```yaml
matrix:
  example*type: [..., uart*test, ...]
```text

## Test Categories

### 1. Constructor/Destructor Tests
- `test*uart*construction`: Validates proper object initialization and multiple instance support
- `test*uart*initialization`: Tests manual initialization/deinitialization with state validation

### 2. Basic Communication Tests
- `test*uart*basic*communication`: Basic TX/RX operations with external loopback
- `test*uart*baud*rate*configuration`: Multi-baud rate testing (9600 to 230400)
- `test*uart*flow*control`: Hardware RTS/CTS and software XON/XOFF flow control

### 3. Advanced Features Tests
- `test*uart*pattern*detection`: Comprehensive pattern detection testing including:
  - Line pattern detection (`\n`) with 3/3 patterns detected
  - AT command pattern detection (`+++`) with 2/2 patterns detected
  - Event-driven processing with proper timing optimization
  - Pattern position tracking and data extraction
- `test*uart*buffer*operations`: ReadUntil, ReadLine, and buffer management
- `test*uart*advanced*features`: Break signals, loopback mode, signal inversion, wakeup
- `test*uart*communication*modes`: UART, RS485, and IrDA mode configuration

### 4. Async Operations Tests
- `test*uart*async*operations`: Interrupt-driven operation with event queues
- `test*uart*callbacks`: Event queue access and interrupt configuration
- `test*uart*callback*verification`: Event-driven callback system validation

### 5. Statistics and Diagnostics Tests
- `test*uart*statistics*diagnostics`: Comprehensive performance metrics and error reporting
- `test*uart*printf*support`: Formatted output with variable argument support
- `test*uart*error*handling`: Error condition testing and graceful failure handling

### 6. ESP32-C6 Specific Tests
- `test*uart*esp32c6*features`: ESP32-C6 specific UART capabilities
- `test*uart*performance`: Performance testing and timing validation
- `test*uart*user*event*task`: FreeRTOS task creation for event handling

### 7. Event-Driven Pattern Detection Tests
- `test*uart*event*driven*pattern*detection`: Comprehensive event queue monitoring
- `test*uart*cleanup`: Resource cleanup and memory management

## Pattern Detection Specifications

### ESP-IDF v5.5 Pattern Detection
The test suite uses the modern ESP-IDF v5.5 pattern detection API:

```cpp
esp*err*t uart*enable*pattern*det*baud*intr(
    uart*port*t uart*num, 
    char pattern*chr,           // Character to detect
    uint8*t chr*num,            // Number of consecutive characters
    int chr*tout,               // Timeout between characters (baud cycles)
    int post*idle,              // Idle time after last character
    int pre*idle                 // Idle time before first character
);
```text

### Pattern Detection Parameters

| Pattern Type | Character | Count | chr*tout | post*idle | pre*idle | Purpose |

|--------------|-----------|-------|----------|-----------|----------|---------|

| Line Pattern | `\n` | 1 | 9 | 0 | 0 | Line-oriented protocols |

| AT Escape | `+` | 3 | 5 | 0 | 0 | AT command escape sequences |

### Timing Optimization
**Relaxed Timing Parameters** for reliable pattern detection:
- **`chr*tout`**: Reduced from 9 to 5 baud cycles (more permissive)
- **`post*idle`**: Set to 0 (no idle requirement after pattern)
- **`pre*idle`**: Set to 0 (no idle requirement before pattern)

**Why Reduced Timing is Better:**
- **More Permissive**: Allows for slight timing variations in data transmission
- **Better Reliability**: Reduces false negatives from strict timing requirements
- **Real-World Compatibility**: Matches actual communication timing patterns

## Expected Test Results

### Successful Test Output
```text
[UART*Test] ╔════════════════════════════════════════════════════════════════════════════════╗
[UART*Test] ║                   ESP32-C6 UART COMPREHENSIVE TEST SUITE                       ║
[UART*Test] ║                         HardFOC Internal Interface                             ║
[UART*Test] ╚════════════════════════════════════════════════════════════════════════════════╝
[UART*Test] ║ Target: ESP32-C6 DevKit-M-1                                                    ║
[UART*Test] ║ ESP-IDF: v5.5+                                                                 ║
[UART*Test] ║ Features: UART, Baud Rate Configuration, Flow Control, Pattern Detection,      ║
[UART*Test] ║ Buffer Operations, Advanced Features, Communication Modes, Async Operations,   ║
[UART*Test] ║ Callbacks, Statistics and Diagnostics, printf Support, Error Handling,         ║
[UART*Test] ║ ESP32-C6 Features, Performance, Callback Verification, User Event Task,        ║
[UART*Test] ║ Event-Driven Pattern Detection, Cleanup                                        ║
[UART*Test] ║ Architecture: noexcept (no exception handling)                                 ║
[UART*Test] ╚════════════════════════════════════════════════════════════════════════════════╝

[UART*Test] === CONSTRUCTOR/DESTRUCTOR TESTS ===
[UART*Test] [SUCCESS] PASSED: test*uart*construction (31.04 ms)
[UART*Test] [SUCCESS] PASSED: test*uart*initialization (65.99 ms)

[UART*Test] === BASIC COMMUNICATION TESTS ===
[UART*Test] [SUCCESS] PASSED: test*uart*basic*communication (74.25 ms)
[UART*Test] [SUCCESS] PASSED: test*uart*baud*rate*configuration (126.07 ms)

[UART*Test] === ADVANCED FEATURES TESTS ===
[UART*Test] [SUCCESS] PASSED (task): test*uart*pattern*detection (444.76 ms)
[UART*Test] [SUCCESS] PASSED: test*uart*buffer*operations (269.45 ms)
[UART*Test] [SUCCESS] PASSED: test*uart*advanced*features (673.89 ms)
[UART*Test] [SUCCESS] PASSED: test*uart*communication*modes (96.81 ms)
[UART*Test] [SUCCESS] PASSED: test*uart*async*operations (181.19 ms)
[UART*Test] [SUCCESS] PASSED: test*uart*callbacks (94.81 ms)
[UART*Test] [SUCCESS] PASSED: test*uart*statistics*diagnostics (101.05 ms)
[UART*Test] [SUCCESS] PASSED: test*uart*printf*support (75.37 ms)
[UART*Test] [SUCCESS] PASSED: test*uart*error*handling (85.34 ms)

[UART*Test] === ESP32-C6 SPECIFIC TESTS ===
[UART*Test] [SUCCESS] PASSED: test*uart*esp32c6*features (135.87 ms)
[UART*Test] [SUCCESS] PASSED: test*uart*performance (82.07 ms)
[UART*Test] [SUCCESS] PASSED: test*uart*callback*verification (1404.49 ms)

[UART*Test] === USER EVENT TASK TEST ===
[UART*Test] [SUCCESS] PASSED: test*uart*user*event*task (1483.02 ms)

[UART*Test] === COMPREHENSIVE EVENT-DRIVEN PATTERN DETECTION TEST ===
[UART*Test] [SUCCESS] PASSED: test*uart*event*driven*pattern*detection (419.06 ms)

[UART*Test] === CLEANUP TEST ===
[UART*Test] [SUCCESS] PASSED: test*uart*cleanup (72.50 ms)

[UART*Test] === UART TEST SUMMARY ===
[UART*Test] Total: 19, Passed: 19, Failed: 0, Success: 100.00%, Time: 5917.04 ms
[UART*Test] [SUCCESS] ALL UART TESTS PASSED!
```text

### Pattern Detection Test Results
**Test 1: Line Pattern Detection (`\n`)**
```text
[UART*Test] === Test 1: Line-oriented pattern detection ('\n') ===
[UART*Test] Line pattern detection enabled
[UART*Test] Test data sent: 'Line1\nLine2\nLine3\n' (length: 18)
[UART*Test] UART*PATTERN*DET event received!
[UART*Test] Pattern 1 detected at position: 5
[UART*Test] UART*PATTERN*DET event received!
[UART*Test] Pattern 2 detected at position: 11
[UART*Test] UART*PATTERN*DET event received!
[UART*Test] Pattern 3 detected at position: 17
[UART*Test] Line pattern detection: 3/3 patterns detected. PASSED
```text

**Test 2: AT Pattern Detection (`+++`)**
```text
[UART*Test] === Test 2: AT escape sequence pattern detection ('+++') ===
[UART*Test] AT escape sequence pattern detection enabled
[UART*Test] AT test data sent (length: 42)
[UART*Test] UART*PATTERN*DET event received for +++!
[UART*Test] AT Pattern 1 detected at position: 9
[UART*Test] UART*PATTERN*DET event received for +++!
[UART*Test] AT Pattern 2 detected at position: 30
[UART*Test] AT pattern detection: 2/2 patterns detected. PASSED
```text

### Event-Driven Pattern Detection Results
```text
[UART*Test] Event-driven pattern detection results:
[UART*Test]   Total events received: 3
[UART*Test]   Data events: 0
[UART*Test]   Pattern events: 3
[UART*Test]   Other events: 0
[UART*Test]   Pattern detected: YES
[UART*Test]   Pattern position: 8
[UART*Test]   Expected patterns: 3
[UART*Test] [SUCCESS] Event-driven pattern detection test completed successfully
```text

## Troubleshooting

### Common Issues

#### Test Failures
- **Pattern Detection Issues**: Verify timing parameters and interrupt configuration
- **GPIO Conflicts**: Check pin availability and external loopback connection
- **Initialization Failures**: Ensure ESP-IDF v5.5+ and proper hardware
- **UART Port Conflicts**: Use UART1 or UART2, avoid UART0 (console port)

#### Pattern Detection Issues
- **No Pattern Events**: Check interrupt configuration and timing parameters
- **Timing Issues**: Verify `chr*tout`, `post*idle`, and `pre*idle` values
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
- **Port Selection**: Use `TEST*UART*PORT*1` for testing (avoid UART0)
- **Pin Configuration**: Verify TX/RX/RTS/CTS pin assignments
- **Baud Rate**: Ensure consistent baud rate across all operations
- **Buffer Sizes**: Use appropriate buffer sizes for ESP32-C6 (256+ bytes)

### Debug Mode
Enable detailed logging by building in Debug mode:
```bash
## Using build scripts (recommended)
./scripts/build*example.sh uart*test Debug

## Or direct ESP-IDF build
idf.py build -DEXAMPLE*TYPE=uart*test -DBUILD*TYPE=Debug
```text

### Pattern Detection Debugging
```cpp
// Enable debug logging for pattern detection
ESP*LOGI(TAG, "Pattern detection enabled: '%c' x%d (chr*tout=%d, post*idle=%d, pre*idle=%d)",
         pattern*chr, chr*num, chr*tout, post*idle, pre*idle);

// Monitor event queue activity
ESP*LOGI(TAG, "Event received: type=%d, size=%zu", event.type, event.size);

// Check pattern position
int pattern*pos = uart->PopPatternPosition();
ESP*LOGI(TAG, "Pattern detected at position: %d", pattern*pos);
```text

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
static constexpr hf*u8*t TEST*UART*PORT*1 = 1;
static constexpr hf*u8*t TEST*TX*PIN = 5;   // UART1 TX
static constexpr hf*u8*t TEST*RX*PIN = 4;   // UART1 RX
static constexpr hf*u8*t TEST*RTS*PIN = 6;  // UART1 RTS
static constexpr hf*u8*t TEST*CTS*PIN = 7;  // UART1 CTS
```text

### Pattern Detection Configuration
```cpp
// Line pattern detection with optimized timing
result = uart->EnablePatternDetection('\n', 1, 9, 0, 0);

// AT escape sequence with relaxed timing
result = uart->EnablePatternDetection('+', 3, 5, 0, 0);

// Custom pattern with specific timing requirements
result = uart->EnablePatternDetection('$', 2, 10, 5, 5);
```text

### Event Queue Configuration
```cpp
// Enhanced event queue for comprehensive testing
config.event*queue*size = 32;  // Larger queue for pattern detection

// Interrupt configuration for pattern detection
result = uart->ConfigureInterrupts(
    UART*RXFIFO*FULL*INT*ENA*M | UART*RXFIFO*TOUT*INT*ENA*M, 32, 5);
```text

### Test Parameters
Customize test behavior:
```cpp
// Pattern detection timing optimization
static constexpr int PATTERN*CHR*TIMEOUT = 5;    // Relaxed timing
static constexpr int PATTERN*POST*IDLE = 0;      // No idle requirement
static constexpr int PATTERN*PRE*IDLE = 0;       // No idle requirement

// Test progression configuration
static constexpr uint32*t TEST*PROGRESS*DELAY*MS = 100; // Progress indicator timing
static constexpr uint32*t PATTERN*TEST*TIMEOUT*MS = 3000; // Pattern detection timeout
```text

## ESP32-C6 Specific Features

### UART Peripheral
- **Ports**: 3 UART ports (UART0, UART1, UART2)
- **Clock Source**: PLL*F80M (80 MHz) with automatic fallback
- **FIFO Size**: 128 bytes hardware FIFO
- **Buffer Support**: Configurable ring buffers (256+ bytes recommended)
- **Interrupt Support**: Comprehensive interrupt configuration
- **Pattern Detection**: Hardware pattern detection with ESP-IDF v5.5

### Pattern Detection Capabilities
- **Character Patterns**: Single or multiple consecutive characters
- **Timing Control**: Configurable character timeout and idle periods
- **Position Tracking**: Pattern position detection and queue management
- **Event Generation**: Automatic UART*PATTERN*DET events
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
```text
W (3456) EspUart: uart*write*bytes*with*break failed (ERROR), trying manual break via GPIO
W (3556) EspUart: Break condition sent via manual GPIO control for 100 ms (ESP32-C6 fallback)
```text
- ESP32-C6 has limited break signal support in ESP-IDF v5.5
- Automatic fallback to manual GPIO control for break signals
- This is expected behavior for this MCU variant

**Communication Mode Support:**
```text
W (4276) EspUart: RS485 advanced features not supported in ESP-IDF v5.5
W (4286) EspUart: IrDA not supported in ESP-IDF v5.5
W (4286) UART*Test: IrDA not supported on ESP32-C6 (expected): 4
```text
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
- [ESP-IDF Pattern Detection API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/uart.html#*CPPv429uart*enable*pattern*det*baud*intr)
- [ESP32-C6 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c6*technical*reference*manual_en.pdf)
- [ESP-IDF v5.5 Migration Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/migration-guides/release-5.x/5.0/peripherals.html)
- [ESP32-C6-DevKitM-1 User Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/hw-reference/esp32c6/user-guide-devkitm-1.html)
- [UART Pattern Detection Best Practices](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-guides/uart.html#uart-pattern-detection)
