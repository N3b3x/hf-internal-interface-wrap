# DigitalOutputGuard (DOG) Comprehensive Test Suite

## Overview

The DigitalOutputGuard Comprehensive Test Suite (`dog*test`) provides extensive testing of the
`DigitalOutputGuard` class,
which implements RAII (Resource Acquisition Is Initialization) pattern for GPIO output management.
This test suite validates all aspects of the DigitalOutputGuard functionality,
from basic RAII operations to performance characteristics and concurrent access patterns.

## Test Configuration

### App Type
- **Name**: `dog*test`
- **Source File**: `DigitalOutputGuardComprehensiveTest.cpp`
- **Category**: `utility`
- **Build Types**: Debug, Release
- **CI Enabled**: Yes
- **Featured**: Yes

### Test GPIO Pins
The test suite uses only **3 GPIO pins** defined as static constexpr:
- `TEST*GPIO*PIN*1 = 2`
- `TEST*GPIO*PIN*2 = 4` 
- `TEST*GPIO*PIN*3 = 5`

## Test Sections

### 1. Basic Tests (`ENABLE*BASIC*TESTS`)
**Blink Pattern**: 5 blinks at section start/end

Tests fundamental RAII functionality and state management:
- **`creation`**: Basic DigitalOutputGuard creation and validation
- **`raii*cleanup`**: Automatic cleanup verification in scope
- **`manual*state*control`**: Manual SetActive/SetInactive operations

### 2. Constructor Tests (`ENABLE*CONSTRUCTOR*TESTS`)
**Blink Pattern**: 5 blinks at section start/end

Tests constructor variants and error handling:
- **`pointer*constructor`**: Constructor with GPIO pointer
- **`null*pointer*handling`**: Null pointer error handling
- **`ensure*output*mode`**: Automatic output mode configuration
- **`no*ensure*output*mode`**: Input mode GPIO rejection

### 3. State Tests (`ENABLE*STATE*TESTS`)
**Blink Pattern**: 5 blinks at section start/end

Tests state transitions and GPIO control:
- **`state*transitions`**: Multiple active/inactive transitions
- **`get*current*state`**: State query functionality

### 4. Move Semantics Tests (`ENABLE*MOVE*SEMANTICS*TESTS`)
**Blink Pattern**: 5 blinks at section start/end

Tests move operations and resource management:
- **`move*constructor`**: Move constructor functionality
- **`move*assignment`**: Move assignment operator

### 5. Edge Case Tests (`ENABLE*EDGE*CASE*TESTS`)
**Blink Pattern**: 5 blinks at section start/end

Tests edge cases and error conditions:
- **`invalid*operations`**: Operations on invalid guards
- **`multiple*guards*same*gpio`**: Multiple guards managing same GPIO

### 6. Concurrent Tests (`ENABLE*CONCURRENT*TESTS`)
**Blink Pattern**: 5 blinks at section start/end

Tests concurrent access patterns:
- **`concurrent*access`**: Multi-threaded access with 3 concurrent tasks
  - 3 FreeRTOS tasks × 100 operations = 300 total operations
  - Validates thread safety and race condition prevention

### 7. Performance Tests (`ENABLE*PERFORMANCE*TESTS`)
**Blink Pattern**: 5 blinks at section start/end

Tests performance and stress scenarios:
- **`performance`**: Timing measurements for critical operations
- **`stress`**: High-load testing with multiple GPIO pins

## Performance Metrics

### Expected Performance Thresholds
- **Guard Creation/Destruction**: < 100 μs per cycle
- **State Transitions**: < 50 μs per operation  
- **Stress Test**: < 200 μs per iteration

### Typical Performance (ESP32-C6)
- **Guard Creation/Destruction**: ~2-5 μs per cycle
- **State Transitions**: ~1-3 μs per operation
- **Stress Test**: ~5-15 μs per iteration

## Progress Indicators

### GPIO14 Test Progress Indicator
- **Individual Test Progress**: GPIO14 toggles HIGH/LOW after each test completion
- **Section Indicators**: 5 blinks at section start and end
- **Visual Feedback**: Easy monitoring of test progression

### Test Output Format
```text
I (timestamp) DIGITAL*OUTPUT*GUARD*Test: [SUCCESS] PASSED (task): test*name (X.XX ms)
I (timestamp) TestFramework: Test progression indicator: HIGH/LOW
```text

## Building and Running

### Build the Test
```bash
## From examples/esp32 directory
./scripts/build*app.sh dog*test Release
```text

### Flash and Monitor
```bash
## Flash the test
./scripts/flash*app.sh flash dog*test Release

## Monitor test output
./scripts/flash*app.sh monitor
```text

### Flash and Monitor (Combined)
```bash
./scripts/flash*app.sh flash*monitor dog*test Release
```text

## Test Output Example

```text
I (254) DIGITAL*OUTPUT*GUARD*Test:
╔══════════════════════════════════════════════════════════════════════════════╗
I (278) DIGITAL*OUTPUT*GUARD*Test: ║ ESP32-C6 DIGITAL OUTPUT GUARD COMPREHENSIVE TEST SUITE v1.0 ║
I (288) DIGITAL*OUTPUT*GUARD*Test: ║ RAII GPIO Management and State Control ║
I (299) DIGITAL*OUTPUT*GUARD*Test:
╚══════════════════════════════════════════════════════════════════════════════╝

I (14920) DIGITAL*OUTPUT*GUARD*Test: Guard creation/destruction: 1000 iterations in 2.27 ms (avg:
2.27 us per cycle)
I (14929) DIGITAL*OUTPUT*GUARD*Test: State transitions: 1000 iterations in 1.13 ms (avg: 1.13 us per
operation)
I (15206) DIGITAL*OUTPUT*GUARD*Test: Stress test: 2000 iterations in 16.12 ms (avg: 8.06 us per
iteration)

I (15895) DIGITAL*OUTPUT*GUARD*Test: Total: 16, Passed: 16, Failed: 0, Success: 100.00%, Time:
3560.80 ms
I (15904) DIGITAL*OUTPUT*GUARD*Test: [SUCCESS] ALL DIGITAL*OUTPUT*GUARD TESTS PASSED!
```cpp

## Test Configuration

### Enabling/Disabling Test Sections
Edit the configuration constants at the top of `DigitalOutputGuardComprehensiveTest.cpp`:

```cpp
// Core DigitalOutputGuard functionality tests
static constexpr bool ENABLE*BASIC*TESTS = true;        // Basic RAII and state management
static constexpr bool ENABLE*CONSTRUCTOR*TESTS = true;  // Constructor variants and error handling
static constexpr bool ENABLE*STATE*TESTS = true;        // State transitions and GPIO control
static constexpr bool ENABLE*MOVE*SEMANTICS*TESTS = true; // Move operations and resource management
static constexpr bool ENABLE*EDGE*CASE*TESTS = true;    // Edge cases and error conditions
static constexpr bool ENABLE*CONCURRENT*TESTS = true;   // Concurrent access testing
static constexpr bool ENABLE*PERFORMANCE*TESTS = true;  // Performance and stress testing
```text

## Performance Interpretation

### Excellent Performance Indicators
- **Guard creation/destruction < 5 μs**: Minimal RAII overhead
- **State transitions < 3 μs**: Direct GPIO control efficiency
- **Stress test < 15 μs**: Good scalability under load
- **100% concurrent test success**: Robust thread safety

### Performance Degradation Warnings
- **Guard creation/destruction > 50 μs**: Potential memory allocation issues
- **State transitions > 20 μs**: GPIO driver inefficiency
- **Stress test > 100 μs**: Resource contention or memory fragmentation
- **Concurrent test failures**: Thread safety violations

## Test Coverage

### RAII Pattern Validation
- ✅ Automatic GPIO activation on construction
- ✅ Automatic GPIO deactivation on destruction
- ✅ Exception safety and cleanup guarantees
- ✅ Scope-based resource management

### GPIO State Management
- ✅ Output mode enforcement
- ✅ Active/inactive state transitions
- ✅ State query functionality
- ✅ Error handling and validation

### Constructor Variants
- ✅ Reference-based constructor
- ✅ Pointer-based constructor
- ✅ Null pointer handling
- ✅ Output mode configuration options

### Move Semantics
- ✅ Move constructor functionality
- ✅ Move assignment operator
- ✅ Resource transfer validation
- ✅ Moved-from state handling

### Edge Cases
- ✅ Invalid guard operations
- ✅ Multiple guards on same GPIO
- ✅ Error condition handling
- ✅ Boundary condition testing

### Concurrent Access
- ✅ Multi-threaded safety
- ✅ Race condition prevention
- ✅ Thread-safe operations
- ✅ Concurrent load testing

### Performance Testing
- ✅ Timing measurements
- ✅ Stress testing
- ✅ Scalability validation
- ✅ Performance regression detection

## Integration with Test Framework

The DOG test integrates with the HardFOC test framework:

- **Test Framework**: Uses `TestFramework.h` for consistent test execution
- **Progress Indicators**: GPIO14-based visual feedback
- **Task Management**: FreeRTOS task-based test execution
- **Error Reporting**: Comprehensive error logging and reporting
- **Performance Metrics**: Detailed timing and performance analysis

## Troubleshooting

### Common Issues

1. **GPIO Initialization Failures**
   - Check GPIO pin availability
   - Verify GPIO configuration
   - Ensure proper ESP-IDF setup

2. **Performance Degradation**
   - Check for memory fragmentation
   - Verify GPIO driver efficiency
   - Monitor system load

3. **Concurrent Test Failures**
   - Check thread safety implementation
   - Verify FreeRTOS configuration
   - Monitor task priorities

### Debug Information

Enable debug logging by modifying the log level in the test configuration or using the ESP-IDF
monitor with increased verbosity.

## Related Documentation

- [DigitalOutputGuard API Documentation](../../../docs/utils/DigitalOutputGuard.md)
- [BaseGpio API Documentation](../../../docs/api/BaseGpio.md)
- [EspGpio API Documentation](../../../docs/esp_api/EspGpio.md)
- [Test Framework Documentation](../main/TestFramework.h)
- [Hardware Types Documentation](../../../docs/api/HardwareTypes.md)
