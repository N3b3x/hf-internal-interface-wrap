---
layout: default
title: "🧪 PWM Test"
description: "PWM Test Suite - Pulse width modulation and frequency control testing"
nav_order: 3
parent: "🧪 Test Documentation"
permalink: /examples/esp32/docs/pwm_test/
---

# ESP32 Family PWM Comprehensive Test Suite Documentation

## Overview

The PWM Comprehensive Test Suite provides extensive validation of the `EspPwm` class across the
entire ESP32 family using ESP-IDF v5.5+.
This test suite demonstrates complete PWM functionality including duty cycle control,
frequency management, resolution control, hardware fade operations, timer management,
LEDC peripheral validation,
and advanced features with a focus on embedded environments using `noexcept` functions.

**✅ Status: Successfully tested across ESP32 variants**
**🎯 Focus: LEDC peripheral capabilities and constraints**
**🔧 Updated: Comprehensive LEDC documentation and clock source validation**

## Features Tested

### Core PWM Functionality
- **PWM Signal Generation**: Basic square wave generation with configurable duty cycles
- **Frequency Control**: Dynamic frequency adjustment from 100Hz to 20kHz
- **Duty Cycle Management**: Precise duty cycle control from 0% to 100%
- **Multiple Channel Support**: Simultaneous operation of up to 8 PWM channels

### Advanced PWM Features
- **Phase Shift Control**: Channel synchronization with phase relationships
- **Hardware Fade Operations**: Smooth transitions between duty cycle values
- **Complementary Outputs**: Deadtime-controlled complementary PWM pairs
- **Synchronized Operations**: Coordinated start/stop/update of multiple channels

### LEDC Peripheral Validation (ESP32 Family)
- **Variant-Specific Testing**: Automatic adaptation to ESP32 variant capabilities
- **Clock Source Constraints**: Validation of clock source limitations per variant
- **Timer Resource Management**: Dynamic allocation, sharing, and eviction policies
- **Resolution/Frequency Validation**: Hardware constraint verification
- **Hardware Fade Integration**: Native LEDC fade functionality testing
- **Channel Protection**: Critical channel protection and safe eviction
- **Performance Optimization**: Timer sharing and resource efficiency

### ESP32 Variant-Specific Features
- **Multi-Variant Support**: ESP32, ESP32-S2/S3, ESP32-C3/C6, ESP32-H2
- **LEDC Backend Integration**: Full LEDC peripheral feature utilization
- **Clock Source Selection**: APB (80MHz), XTAL (40MHz), RC_FAST (~17.5MHz)
- **Timer Allocation**: Smart allocation with conflict resolution
- **Idle Level Control**: Output state configuration during idle periods
- **Interrupt Integration**: PWM period and fade completion callbacks

### System Integration & Diagnostics
- **Status Reporting**: Real-time channel status and configuration monitoring
- **Statistics Collection**: Operation counters and performance metrics
- **Error Handling**: Comprehensive error condition testing and recovery
- **Stress Testing**: High-frequency updates and resource exhaustion scenarios

## Hardware Requirements

### Supported Platforms
- **ESP32 Classic**: ESP32-DevKitC, ESP32-WROVER-KIT (16 channels, 8 timers)
- **ESP32-S2**: ESP32-S2-Saola, ESP32-S2-DevKitM (8 channels, 4 timers)  
- **ESP32-S3**: ESP32-S3-DevKitC, ESP32-S3-DevKitM (8 channels, 4 timers)
- **ESP32-C3**: ESP32-C3-DevKitM, ESP32-C3-DevKitC (6 channels, 4 timers)
- **ESP32-C6**: ESP32-C6-DevKitM, ESP32-C6-DevKitC (6 channels, 4 timers) 
- **ESP32-H2**: ESP32-H2-DevKitM (4 channels, 2 timers)
- **ESP-IDF Version**: v5.5 or later
- **Minimum Flash**: 4MB (2MB for basic testing)
- **Minimum RAM**: 256KB

### PWM Output Pins
The test suite uses the following safe GPIO pins on ESP32-C6 DevKit-M-1:

```text
PWM Test Pins Configuration (Based on Actual Test Code):
┌─────────────────────────────────────────────────┐
│ Function              │ GPIO Pin  │ Channel ID  │
├───────────────────────┼───────────┼─────────────┤
│ Primary PWM Channel   │ GPIO 2    │ Channel 0   │
│ Secondary PWM Channel │ GPIO 6    │ Channel 1   │
│ Third PWM Channel     │ GPIO 4    │ Channel 2   │
│ Fourth PWM Channel    │ GPIO 5    │ Channel 3   │
│ Additional Channels   │ GPIO 7-9* │ Channel 4-7 │
│ Stress Test Channels  │ GPIO 2,6,4,5,7,8,9,10 │ All 8   │
└───────────────────────┴───────────┴─────────────┘

*Note: GPIO 3 is deliberately avoided in the test code and 
replaced with GPIO 6 when the sequence would use it.

Actual Pin Mapping from Test Code:
- Most tests use: GPIO 2 (primary test pin)
- Multi-channel tests use: GPIO 2, 6, 4, 5 (avoids GPIO 3)
- Complementary tests use: GPIO 2 (primary) + GPIO 6 (complementary)
- Timer management test uses: GPIO 2, 6, 4, 5
- Stress tests use: GPIO 2, 6, 4, 5, 7, 8, 9, 10 (up to 8 channels)
└───────────────────────┴───────────┴─────────────┘

Pins to Avoid (ESP32-C6 Specific):
┌─────────────────────────────────────┐
│ GPIO 3              │ Flash voltage │
│ GPIO 9              │ Boot strap    │
│ GPIO 15             │ Boot strap    │
│ GPIO 12, 13         │ USB-JTAG      │
│ GPIO 18, 19         │ USB Serial    │
│ GPIO 24-30          │ SPI Flash     │
└─────────────────────┴───────────────┘
```text

### Logic Analyzer Setup
For comprehensive testing and verification, connect logic analyzer probes to:
- **Primary Channel**: GPIO 2 (most test activity)
- **Multi-Channel**: GPIO 2, 6, 4, 5 (for synchronized operations)
- **All Channels**: GPIO 2, 6, 4, 5, 7, 8, 9, 10 (for stress testing)
- **Sample Rate**: Minimum 1MHz (recommended 10MHz for high-frequency tests)
- **Trigger**: Rising edge on GPIO 2 (primary channel)
- **Time Base**: 100μs/div for high freq, 1ms/div for duty cycle, 1s/div for fade

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

## Build PWM test using example scripts (Recommended)
./scripts/build_example.sh pwm_test Release

## Flash and monitor using example scripts (Recommended)
./scripts/flash_example.sh pwm_test Release flash_monitor
```text

### Alternative Build Methods

#### Using ESP-IDF directly
```bash
## Build with idf.py
idf.py build -DEXAMPLE_TYPE=pwm_test -DBUILD_TYPE=Release

## Flash and monitor with idf.py
idf.py -p /dev/ttyUSB0 flash monitor
```text

#### Debug Build for Development
```bash
## Build debug version using example scripts
./scripts/build_example.sh pwm_test Debug --clean

## Flash debug build
./scripts/flash_example.sh pwm_test Debug flash_monitor
```text

#### Available Example Script Options
```bash
## List all available examples and build types
./scripts/build_example.sh list
./scripts/flash_example.sh list

## Build with additional options
./scripts/build_example.sh pwm_test Release --clean --no-cache

## Flash operations
./scripts/flash_example.sh pwm_test Release flash      # Flash only
./scripts/flash_example.sh pwm_test Release monitor   # Monitor only
./scripts/flash_example.sh pwm_test Release flash_monitor  # Both (default)
```text

## Test Categories

### 1. Constructor/Destructor Tests

#### `test_constructor_default()`
- **Purpose**: Validates proper object construction and initialization
- **Tests**: 
  - Default constructor behavior
  - Constructor with unit configuration
  - Legacy constructor with clock frequency
- **Expected Results**: Clean object creation without initialization
- **Logic Analyzer**: No output expected (no PWM signals generated)

#### `test_destructor_cleanup()`
- **Purpose**: Ensures proper resource cleanup and deinitialization
- **Tests**:
  - Automatic resource cleanup on object destruction
  - Hardware state restoration
  - Memory leak prevention
- **Expected Results**: PWM signals stop when object is destroyed
- **Logic Analyzer**: 
  - Signal on GPIO 2 during test
  - Signal goes LOW and stays LOW after destructor

### 2. Lifecycle Management Tests

#### `test_initialization_states()`
- **Purpose**: Validates PWM hardware initialization state management
- **Tests**:
  - Initial uninitialized state
  - Manual initialization process
  - Double initialization protection
  - Proper deinitialization
- **Expected Results**: State transitions match expected lifecycle
- **Logic Analyzer**: No output expected (initialization only)

#### `test_lazy_initialization()`
- **Purpose**: Tests automatic initialization on first use
- **Tests**:
  - EnsureInitialized() behavior
  - Automatic hardware setup
  - EnsureDeinitialized() cleanup
- **Expected Results**: Hardware initializes only when needed
- **Logic Analyzer**: No output expected (initialization only)

### 3. Configuration Tests

#### `test_mode_configuration()`
- **Purpose**: Validates PWM mode configuration capabilities
- **Tests**:
  - Basic PWM mode setting
  - Fade mode configuration
  - Mode switching behavior
- **Expected Results**: Modes set correctly without errors
- **Logic Analyzer**: No output expected (configuration only)

#### `test_clock_source_configuration()`
- **Purpose**: Tests different PWM clock source options
- **Tests**:
  - Default clock source (APB_CLK: 80MHz)
  - XTAL clock source (40MHz)
  - RC_FAST clock source (~8MHz)
  - APB clock source (80MHz)
- **Expected Results**: All clock sources configure successfully
- **Logic Analyzer**: No output expected (configuration only)

### 4. Channel Management Tests

#### `test_channel_configuration()`
- **Purpose**: Validates multi-channel PWM configuration
- **Tests**:
  - Configuration of channels 0-3 on GPIO 2, 4, 5, 6
  - Different initial duty cycles per channel
  - Invalid channel configuration rejection
- **Expected Results**: All valid channels configure successfully
- **Logic Analyzer**: No output expected (configuration only)

#### `test_channel_enable_disable()`
- **Purpose**: Tests channel enable/disable functionality
- **Tests**:
  - Initial disabled state
  - Channel enable operation
  - Channel disable operation
  - Invalid channel operations
- **Expected Results**: Channels enable/disable as commanded
- **Logic Analyzer**:
  - GPIO 2: HIGH pulses when enabled (50% duty cycle, ~1kHz)
  - GPIO 2: LOW constant when disabled

### 5. PWM Control Tests

#### `test_duty_cycle_control()`
- **Purpose**: Validates precise duty cycle control across the full range
- **Tests**:
  - **Float duty cycles**: 0.0, 0.25, 0.5, 0.75, 1.0 (0% to 100%)
  - **Raw duty cycle values**: 0, 256, 512, 768, 1023 (10-bit resolution)
  - **Invalid input rejection**: Negative values (-0.1) and over-range (1.1)
  - **Accuracy verification**: Actual vs commanded duty cycle within ±1%
- **Expected Results**: 
  - Clean square wave generation on GPIO 2
  - Duty cycle accuracy within tolerance
  - Proper rejection of invalid values
  - Both float and raw value interfaces work correctly

#### `test_frequency_control()`
- **Purpose**: Tests dynamic frequency adjustment and accuracy validation
- **Tests**:
  - **Frequency range**: 100Hz, 500Hz, 1kHz, 5kHz, 10kHz, 20kHz
  - **Accuracy verification**: Measured vs commanded frequency within ±5% tolerance
  - **Invalid frequency rejection**: Zero frequency and values exceeding HF_PWM_MAX_FREQUENCY
  - **Real-time updates**: Frequency changes while PWM is running
- **Expected Results**: 
  - Accurate frequency generation across the full range
  - Proper error handling for invalid frequencies
  - Stable operation during frequency transitions
  - Expected periods: 10ms, 2ms, 1ms, 200μs, 100μs, 50μs respectively

#### `test_phase_shift_control()`
- **Purpose**: Tests phase relationship capabilities between PWM channels
- **Tests**:
  - **Phase values**: 0°, 90°, 180°, 270° between channels 0, 1, 2
  - **Multi-channel coordination**: Simultaneous phase-shifted operation
  - **Invalid input rejection**: Phase values greater than 360°
  - **Hardware limitation detection**: Graceful handling if unsupported
- **Expected Results**: 
  - **ESP32-C6 LEDC limitation**: Test likely skipped with [SKIPPED] message
  - **If supported**: Accurate phase relationships with time offsets
  - **Error handling**: Proper rejection of invalid phase values
  - **Channels used**: GPIO 2, 6, 4 for the three test channels

### 6. Advanced Features Tests

#### `test_synchronized_operations()`
- **Purpose**: Validates coordinated multi-channel operations and timing synchronization
- **Tests**:
  - **StartAll()**: Simultaneous activation of channels 0-3 (GPIO 2,6,4,5)
  - **UpdateAll()**: Synchronized parameter updates across all active channels
  - **StopAll()**: Coordinated shutdown of all PWM outputs
  - **Channel configuration**: Each channel has different duty cycles (30%, 40%, 50%, 60%)
- **Expected Results**: 
  - All channels start/stop within microseconds of each other
  - No visible timing skew between channels during synchronized operations
  - Clean transitions with minimal glitching
  - Proper channel isolation (no cross-talk between channels)

#### `test_complementary_outputs()`
- **Purpose**: Tests complementary PWM pair generation with deadtime control
- **Tests**:
  - **Channel pairing**: Primary channel (GPIO 2) paired with complementary (GPIO 6)
  - **Deadtime insertion**: 1μs deadtime between complementary transitions
  - **Duty cycle testing**: Multiple duty cycles (20%, 50%, 80%) with complementary behavior
  - **SetComplementaryOutput()**: Configuration and validation of complementary relationship
- **Expected Results**: 
  - Primary and complementary outputs are never high simultaneously
  - Deadtime gaps visible during all transitions (rising/falling edges)
  - Combined duty cycle ≈ 100% minus deadtime
  - Proper error handling for invalid deadtime values

### 7. ESP32-Specific Features Tests

#### `test_hardware_fade()`
- **Purpose**: Validates ESP32-C6 LEDC hardware fade functionality and smooth transitions
- **Tests**:
  - **Fade sequences**: 10%→80% (1000ms), 80%→20% (800ms), 20%→90% (1200ms), 90%→0% (500ms)
  - **SetHardwareFade()**: Configure target duty cycle and fade duration
  - **IsFadeActive()**: Monitor fade operation status during transitions
  - **StopHardwareFade()**: Interrupt fade operation mid-transition
  - **Fade completion detection**: Verify fade finishes within expected timeframe
- **Expected Results**: 
  - Smooth, continuous duty cycle transitions (no stepping or glitching)
  - Accurate fade timing within ±10% of commanded duration
  - IsFadeActive() returns true during fade, false when complete
  - StopHardwareFade() immediately halts transition
  - Hardware-controlled operation (no CPU intervention during fade)

#### `test_idle_level_control()`
- **Purpose**: Tests GPIO output state configuration when PWM channel is idle/disabled
- **Tests**:
  - **SetIdleLevel(0)**: Configure output to remain LOW when channel disabled
  - **SetIdleLevel(1)**: Configure output to remain HIGH when channel disabled
  - **Invalid value rejection**: Test SetIdleLevel(2) returns error
  - **State verification**: Check actual GPIO state matches configured idle level
- **Expected Results**: 
  - GPIO 2 maintains configured idle level when channel is disabled
  - Valid idle levels (0, 1) are accepted and applied correctly
  - Invalid idle levels are rejected with appropriate error codes
  - Idle level setting persists across enable/disable cycles

#### `test_timer_management()`
- **Purpose**: Validates ESP32-C6 timer resource allocation
- **Tests**:
  - Automatic timer assignment for channels 0-3
  - Forced timer assignment to specific timer
  - Timer resource optimization
- **Expected Results**: Efficient timer resource usage
- **Logic Analyzer**: No specific output pattern (resource management)

### 8. Status and Diagnostics Tests

#### `test_status_reporting()`
- **Purpose**: Validates PWM status monitoring and diagnostic capabilities
- **Tests**:
  - **GetChannelStatus()**: Retrieve channel state (enabled, configured, duty, frequency)
  - **GetCapabilities()**: Query hardware capabilities and limitations
  - **GetLastError()**: Error state tracking for individual channels
  - **Status accuracy**: Verify reported values match actual configuration
- **Expected Results**: 
  - Channel status correctly reports: enabled=true, configured=true
  - Current duty cycle and frequency match last set values
  - Capabilities structure contains valid hardware limits
  - Error tracking accurately reflects last operation result

#### `test_statistics_and_diagnostics()`
- **Purpose**: Tests operational statistics collection and hardware diagnostics
- **Tests**:
  - **Operation counting**: 5 duty cycle updates, 5 frequency changes, enable/disable cycles
  - **GetStatistics()**: Retrieve duty_updates_count, frequency_changes_count, enable/disable counters
  - **GetDiagnostics()**: Hardware state (initialized, fade_ready, active_channels, active_timers)
  - **Counter accuracy**: Verify statistics match actual performed operations
- **Expected Results**: 
  - Statistics accurately reflect operations: 5 duty updates, 5 frequency changes
  - Enable/disable counters track channel state changes correctly
  - Diagnostics show hardware initialization status and resource usage
  - All counters increment properly during test execution

### 9. Callback Tests

#### `test_callbacks()`
- **Purpose**: Validates PWM interrupt-driven callback functionality
- **Tests**:
  - **SetPeriodCallback()**: Register callback for PWM period completion events
  - **SetFaultCallback()**: Register callback for fault/error detection
  - **Callback triggering**: Use low duty cycle (1%) to generate frequent period events
  - **Parameter passing**: Verify channel ID and user data are passed correctly
- **Expected Results**: 
  - Period callbacks may trigger based on ESP32-C6 LEDC interrupt capabilities
  - Fault callbacks trigger appropriately on error conditions
  - Callback functions receive correct channel ID and parameters
  - Test completion regardless of callback activity (hardware dependent)

### 10. Edge Cases and Stress Tests

#### `test_edge_cases()`
- **Purpose**: Tests boundary conditions, limits, and error handling
- **Tests**:
  - **Duty cycle boundaries**: 0.0% (constant LOW) and 100.0% (constant HIGH)
  - **Frequency boundaries**: HF_PWM_MIN_FREQUENCY and high frequency (20kHz)
  - **Invalid channel operations**: Operations on non-existent channels
  - **Parameter validation**: Verify proper rejection of out-of-range values
- **Expected Results**: 
  - Boundary values are accepted and generate correct outputs
  - Invalid parameters return appropriate error codes
  - System remains stable under boundary conditions
  - No undefined behavior or crashes with invalid inputs

#### `test_stress_scenarios()`
- **Purpose**: Tests system stability under maximum load and rapid operations
- **Tests**:
  - **Maximum channels**: All 8 channels (GPIO 2,6,4,5,7,8,9,10) active simultaneously
  - **Rapid duty updates**: 20 duty cycle changes per channel (every 10ms)
  - **Rapid frequency updates**: 10 frequency changes per channel (every 50ms)
  - **Synchronized operations**: StartAll/UpdateAll/StopAll under load
- **Expected Results**: 
  - System maintains stability throughout rapid update sequences
  - No channel interference or cross-talk between channels
  - Memory usage remains stable (no leaks)
  - All channels maintain independent operation despite high update rates
  - Timer resource allocation handles maximum channel load efficiently

## Logic Analyzer Analysis Guide

### Key Measurements

1. **Duty Cycle Accuracy**:
   - Measure ON time vs total period
   - Should match commanded duty cycle ±1%

2. **Frequency Accuracy**:
   - Measure period between rising edges
   - Should match commanded frequency ±5%

3. **Phase Relationships**:
   - Measure time offset between channel rising edges
   - Calculate phase difference in degrees

4. **Fade Operation**:
   - Capture extended time base (1-2 seconds)
   - Verify smooth duty cycle transitions

5. **Synchronization**:
   - Verify simultaneous start/stop operations
   - Check for timing skew between channels

### Logic Analyzer Measurement Guidelines

#### Key Measurement Techniques
- **Duty Cycle**: Measure pulse width (ON time) vs total period
- **Frequency**: Measure time between consecutive rising edges
- **Phase Relationships**: Measure time offset between channels (if supported)
- **Fade Operations**: Use extended time base to capture transitions
- **Synchronization**: Verify simultaneous channel operations

## Troubleshooting

### Common Issues

1. **No PWM Output**:
   - Check GPIO pin assignment
   - Verify channel configuration and enable status
   - Confirm PWM initialization

2. **Incorrect Duty Cycle**:
   - Check resolution settings (default: 10-bit = 1024 levels)
   - Verify raw duty value calculation
   - Test with different duty cycle values

3. **Frequency Inaccuracy**:
   - Verify clock source configuration
   - Check ESP32-C6 clock limitations
   - Consider timer resolution vs frequency trade-offs

4. **Phase Shift Not Working**:
   - ESP32-C6 LEDC may not support phase shift
   - Check for "SKIPPED" messages in test output
   - Consider using timer offsets for phase control

### Performance Optimization

1. **High Frequency Operation**:
   - Use APB clock source (80MHz) for best resolution
   - Lower resolution for higher frequencies
   - Consider hardware limitations (~40kHz practical max)

2. **Multiple Channel Efficiency**:
   - Group channels with same frequency on same timer
   - Use synchronized operations for coordinated updates
   - Minimize individual channel updates

3. **Memory Usage**:
   - Configure only needed channels
   - Use appropriate resolution settings
   - Monitor stack usage in callback functions

## Expected Test Results

### Success Criteria

All tests should pass with output similar to:
```text
[SUCCESS] PWM comprehensive testing completed.
Total: 18, Passed: 18, Failed: 0, Success: 100.00%, Time: 15234.56 ms
```text

### Typical Test Sequence Timing
- **Constructor/Destructor Tests**: ~500ms
- **Lifecycle Tests**: ~1000ms  
- **Configuration Tests**: ~800ms
- **Channel Management**: ~1200ms
- **PWM Control Tests**: ~3000ms (includes delays for observation)
- **Advanced Features**: ~2000ms
- **ESP32-Specific**: ~4000ms (includes fade operations)
- **Status/Diagnostics**: ~800ms
- **Callbacks**: ~1500ms
- **Edge Cases/Stress**: ~2000ms

**Total Expected Runtime**: ~15-20 seconds

### Hardware Validation

With logic analyzer connected, verify:
1. ✅ Clean square wave generation
2. ✅ Accurate duty cycle control
3. ✅ Precise frequency generation
4. ✅ Smooth hardware fade transitions
5. ✅ Synchronized multi-channel operations
6. ✅ Proper complementary output behavior
7. ✅ Stable operation under stress conditions

## Conclusion

The PWM Comprehensive Test Suite provides thorough validation of ESP32-C6 PWM capabilities through
the EspPwm class.
The combination of automated testing and logic analyzer verification ensures reliable PWM
functionality for embedded applications.
The test suite covers all essential PWM features while highlighting ESP32-C6 specific capabilities
and limitations.

For issues or improvements, refer to the main project documentation or contact the development team.
