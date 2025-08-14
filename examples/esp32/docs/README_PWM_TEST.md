# ESP32-C6 PWM Comprehensive Test Suite Documentation

## Overview

The PWM Comprehensive Test Suite provides extensive validation of the `EspPwm` class for ESP32-C6 platforms using ESP-IDF v5.5+. This test suite demonstrates complete PWM functionality including duty cycle control, frequency management, phase shifting, hardware fade operations, timer management, and advanced features with a focus on embedded environments using `noexcept` functions.

**✅ Status: Successfully tested on ESP32-C6-DevKitM-1 hardware**

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

### ESP32-C6 Specific Features
- **LEDC Peripheral Integration**: ESP32-C6 LED Controller (LEDC) backend
- **Timer Management**: Dynamic timer allocation and resource optimization
- **Clock Source Selection**: APB, XTAL, and RC_FAST clock source options
- **Idle Level Control**: Output state configuration during idle periods
- **Interrupt Integration**: PWM period and fade completion callbacks

### System Integration & Diagnostics
- **Status Reporting**: Real-time channel status and configuration monitoring
- **Statistics Collection**: Operation counters and performance metrics
- **Error Handling**: Comprehensive error condition testing and recovery
- **Stress Testing**: High-frequency updates and resource exhaustion scenarios

## Hardware Requirements

### Supported Platforms
- **Primary Target**: ESP32-C6-DevKitM-1
- **ESP-IDF Version**: v5.5 or later
- **Minimum Flash**: 4MB
- **Minimum RAM**: 256KB

### PWM Output Pins
The test suite uses the following safe GPIO pins on ESP32-C6 DevKit-M-1:

```
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
```

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
# ESP-IDF v5.5+ installation required
. $IDF_PATH/export.sh

# Set target platform
export IDF_TARGET=esp32c6
```

### Quick Start
```bash
# Navigate to examples directory
cd examples/esp32

# Build PWM test using example scripts (Recommended)
./scripts/build_example.sh pwm_test Release

# Flash and monitor using example scripts (Recommended)
./scripts/flash_example.sh pwm_test Release flash_monitor
```

### Alternative Build Methods

#### Using ESP-IDF directly
```bash
# Build with idf.py
idf.py build -DEXAMPLE_TYPE=pwm_test -DBUILD_TYPE=Release

# Flash and monitor with idf.py
idf.py -p /dev/ttyUSB0 flash monitor
```

#### Debug Build for Development
```bash
# Build debug version using example scripts
./scripts/build_example.sh pwm_test Debug --clean

# Flash debug build
./scripts/flash_example.sh pwm_test Debug flash_monitor
```

#### Available Example Script Options
```bash
# List all available examples and build types
./scripts/build_example.sh list
./scripts/flash_example.sh list

# Build with additional options
./scripts/build_example.sh pwm_test Release --clean --no-cache

# Flash operations
./scripts/flash_example.sh pwm_test Release flash      # Flash only
./scripts/flash_example.sh pwm_test Release monitor   # Monitor only
./scripts/flash_example.sh pwm_test Release flash_monitor  # Both (default)
```

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
- **Purpose**: Validates precise duty cycle control
- **Tests**:
  - Duty cycles: 0%, 25%, 50%, 75%, 100%
  - Raw duty cycle values: 0, 256, 512, 768, 1023 (10-bit)
  - Invalid duty cycle rejection (-10%, 110%)
- **Expected Results**: Accurate duty cycle generation
- **Logic Analyzer (GPIO 2)**:
  ```
  0% Duty:   ________________________________________
  25% Duty:  ██____██____██____██____██____██____██____
  50% Duty:  ████____████____████____████____████____
  75% Duty:  ██████__██████__██████__██████__██████__
  100% Duty: ████████████████████████████████████████
  
  Frequency: ~1kHz, Period: 1ms
  Pulse Width: 0ms, 0.25ms, 0.5ms, 0.75ms, 1ms respectively
  ```

#### `test_frequency_control()`
- **Purpose**: Tests dynamic frequency adjustment
- **Tests**:
  - Frequencies: 100Hz, 500Hz, 1kHz, 5kHz, 10kHz, 20kHz
  - Frequency accuracy validation (±5% tolerance)
  - Invalid frequency rejection (0Hz, >max frequency)
- **Expected Results**: Accurate frequency generation within tolerance
- **Logic Analyzer (GPIO 2, 50% duty cycle)**:
  ```
  100Hz:  ████____████____████____████____  (10ms period)
  500Hz:  ██__██__██__██__██__██__██__██__  (2ms period)
  1kHz:   █_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_  (1ms period)
  5kHz:   █_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_  (200μs period)
  10kHz:  █_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_  (100μs period)
  20kHz:  █_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_  (50μs period)
  ```

#### `test_phase_shift_control()`
- **Purpose**: Tests phase relationship between channels
- **Tests**:
  - Phase shifts: 0°, 90°, 180°, 270°
  - Multi-channel phase coordination
  - Invalid phase shift rejection (>360°)
- **Expected Results**: 
  - May be skipped on ESP32-C6 (LEDC limitation)
  - If supported, accurate phase relationships
- **Logic Analyzer (3 Channels)**:
  ```
  Channel 0 (0°):   ████____████____████____████____
  Channel 1 (90°):  ___████____████____████____████__
  Channel 2 (180°): ____████____████____████____████
  
  Note: ESP32-C6 LEDC may not support phase shift
  Expected: [SKIPPED] message in test output
  ```

### 6. Advanced Features Tests

#### `test_synchronized_operations()`
- **Purpose**: Validates coordinated multi-channel operations
- **Tests**:
  - StartAll() command on channels 0-3
  - UpdateAll() synchronization
  - StopAll() simultaneous shutdown
- **Expected Results**: All channels respond simultaneously
- **Logic Analyzer (4 Channels)**:
  ```
  Before StartAll():
  GPIO 2,6,4,5: ________________________________________
  
  After StartAll():
  GPIO 2:   ████____████____████____████____  (30% duty)
  GPIO 6:   █████___█████___█████___█████___  (40% duty)  
  GPIO 4:   ██████__██████__██████__██████__  (50% duty)
  GPIO 5:   ███████_███████_███████_███████_  (60% duty)
  
  After StopAll():
  GPIO 2,6,4,5: ________________________________________
  ```

#### `test_complementary_outputs()`
- **Purpose**: Tests complementary PWM pair generation
- **Tests**:
  - Primary channel (GPIO 2) and complementary (GPIO 6)
  - Deadtime insertion (1μs)
  - Various duty cycles with complementary behavior
- **Expected Results**: Complementary outputs with deadtime
- **Logic Analyzer (2 Channels)**:
  ```
  Primary (GPIO 2):     ████____████____████____████____
  Complementary (GPIO 6): ___████____████____████____████
                           ^deadtime gaps
  
  Deadtime: 1μs gaps between transitions
  Duty cycles: Primary + Complementary ≈ 100% (minus deadtime)
  ```

### 7. ESP32-Specific Features Tests

#### `test_hardware_fade()`
- **Purpose**: Validates ESP32-C6 hardware fade functionality
- **Tests**:
  - Fade from 10% to 80% over 1000ms
  - Fade from 80% to 20% over 800ms
  - Fade from 20% to 90% over 1200ms
  - Fade to 0% over 500ms
  - Stop fade operation mid-transition
- **Expected Results**: Smooth hardware-controlled transitions
- **Logic Analyzer (GPIO 2)**:
  ```
  Fade 10% → 80% (1000ms):
  T=0ms:    █__██__██__██__██__██__██__██__  (10% duty)
  T=250ms:  ███_███_███_███_███_███_███_███  (30% duty)
  T=500ms:  █████___█████___█████___█████___  (50% duty)
  T=750ms:  ███████_███████_███████_███████  (70% duty)
  T=1000ms: ████████████████████████████████  (80% duty)
  
  Smooth gradual transition visible as duty cycle increases
  ```

#### `test_idle_level_control()`
- **Purpose**: Tests output state during idle periods
- **Tests**:
  - Idle level LOW (0)
  - Idle level HIGH (1)
  - Invalid idle level rejection (2)
- **Expected Results**: Correct idle state configuration
- **Logic Analyzer (GPIO 2)**:
  ```
  Idle Level LOW:  Channel disabled → GPIO 2 stays LOW
  Idle Level HIGH: Channel disabled → GPIO 2 stays HIGH
  ```

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
- **Purpose**: Validates PWM status monitoring capabilities
- **Tests**:
  - Channel status retrieval (enabled, configured, duty, frequency)
  - PWM capabilities reporting
  - Error state tracking
- **Expected Results**: Accurate status information
- **Logic Analyzer (GPIO 2)**:
  ```
  Test Pattern (60% duty, 1kHz):
  ██████__██████__██████__██████__██████__
  
  Status Report Should Show:
  - enabled: true
  - configured: true  
  - current_duty_cycle: 0.60
  - current_frequency: 1000
  ```

#### `test_statistics_and_diagnostics()`
- **Purpose**: Tests operational statistics collection
- **Tests**:
  - Duty cycle update counting
  - Frequency change tracking
  - Channel enable/disable counters
  - Hardware state diagnostics
- **Expected Results**: Accurate operation counters
- **Logic Analyzer (GPIO 2)**:
  ```
  5 duty cycle changes: 20%, 35%, 50%, 65%, 80%
  5 frequency changes: 1000Hz, 1500Hz, 2000Hz, 2500Hz, 3000Hz
  
  Expected Statistics:
  - duty_updates_count: 5
  - frequency_changes_count: 5
  - channel_enables_count: 1
  - channel_disables_count: 1
  ```

### 9. Callback Tests

#### `test_callbacks()`
- **Purpose**: Validates PWM interrupt-driven callbacks
- **Tests**:
  - Period completion callbacks
  - Fault detection callbacks
  - Callback parameter passing
- **Expected Results**: Callbacks trigger appropriately
- **Logic Analyzer (GPIO 2)**:
  ```
  Low duty cycle (1%) for frequent period callbacks:
  █_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_
  
  Each period completion may trigger callback
  (Actual behavior depends on ESP32-C6 interrupt configuration)
  ```

### 10. Edge Cases and Stress Tests

#### `test_edge_cases()`
- **Purpose**: Tests boundary conditions and limits
- **Tests**:
  - Minimum duty cycle (0.0%)
  - Maximum duty cycle (100.0%)
  - Minimum frequency (HF_PWM_MIN_FREQUENCY)
  - High frequency (20kHz)
  - Invalid channel operations
- **Expected Results**: Proper boundary handling
- **Logic Analyzer (GPIO 2)**:
  ```
  0% Duty:   ________________________________________
  100% Duty: ████████████████████████████████████████
  20kHz:     █_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_█_ (50μs period)
  ```

#### `test_stress_scenarios()`
- **Purpose**: Tests system under maximum load
- **Tests**:
  - All 8 channels simultaneously active
  - 20 rapid duty cycle updates per channel
  - 10 rapid frequency changes per channel
  - Synchronized operations under load
- **Expected Results**: System maintains stability
- **Logic Analyzer (8 Channels)**:
  ```
  All channels active with different duty cycles:
  GPIO 2:  ██__██__██__██__██__██__██__██__  (20% + iterations)
  GPIO 6:  ███_███_███_███_███_███_███_███_  (30% + iterations)
  GPIO 4:  ████████████████████████████████  (40% + iterations)
  GPIO 5:  █████___█████___█████___█████___  (50% + iterations)
  GPIO 7:  ██████__██████__██████__██████__  (60% + iterations)
  GPIO 8:  ███████_███████_███████_███████_  (70% + iterations)
  GPIO 9:  ████████████████████████████████  (80% + iterations)
  GPIO 10: █████████_█████████_█████████_█  (90% + iterations)
  
  Rapid changes: Duty cycles and frequencies update every 10-50ms
  Note: Sequence avoids GPIO 3, replacing it with GPIO 6
  ```

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

### Common Waveform Patterns

#### Normal PWM Signal (50% duty, 1kHz)
```
 ____████____████____████____████____
 |<-500μs->|<-500μs->|<--1ms period->|
```

#### Hardware Fade (25% → 75% over 1s)
```
Start: ██__██__██__██__██__██__██__██__  (25%)
  →    ███_███_███_███_███_███_███_███_  (37.5%)
  →    ████████████████████████████████  (50%)
  →    █████___█████___█████___█████___  (62.5%)
End:   ██████__██████__██████__██████__  (75%)
```

#### Complementary Outputs with Deadtime
```
Primary:      ████____████____████____████____
              ↑  ↓    ↑  ↓    ↑  ↓    ↑  ↓
Deadtime:     --XX----XX----XX----XX----XX--
              ↓  ↑    ↓  ↑    ↓  ↑    ↓  ↑  
Complement:   ____████____████____████____████
```

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
```
[SUCCESS] PWM comprehensive testing completed.
Total: 18, Passed: 18, Failed: 0, Success: 100.00%, Time: 15234.56 ms
```

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

The PWM Comprehensive Test Suite provides thorough validation of ESP32-C6 PWM capabilities through the EspPwm class. The combination of automated testing and logic analyzer verification ensures reliable PWM functionality for embedded applications. The test suite covers all essential PWM features while highlighting ESP32-C6 specific capabilities and limitations.

For issues or improvements, refer to the main project documentation or contact the development team.