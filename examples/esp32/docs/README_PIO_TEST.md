---
layout: default
title: "🧪 PIO Test"
description: "PIO Test Suite - Programmable I/O and custom protocol testing"
nav_order: 4
parent: "🧪 Test Documentation"
permalink: /examples/esp32/docs/pio_test/
---

# ESP32-C6 PIO Comprehensive Test Suite Documentation

## Overview

The PIO Comprehensive Test Suite provides extensive validation of the `EspPio` class for ESP32
platforms using the RMT (Remote Control) peripheral with ESP-IDF v5.5+.
This comprehensive test suite demonstrates complete PIO functionality including WS2812 LED protocol
timing,
automated loopback testing, channel-specific callbacks, ESP32 variant detection,
and advanced RMT features with a focus on embedded environments using `noexcept` functions.

**✅ Status: Successfully tested on ESP32-C6-DevKitM-1 hardware with over 25 comprehensive tests**

### Supported ESP32 Variants

The implementation automatically detects and adapts to different ESP32 variants with their specific
RMT channel allocation constraints:

| ESP32 Variant | Total Channels | TX Channels | RX Channels | Channel Allocation |

|---------------|----------------|-------------|-------------|-------------------|

| ESP32         | 8              | 8 (0-7)     | 8 (0-7)     | Any channel can be TX or RX |

| ESP32-S2      | 4              | 4 (0-3)     | 4 (0-3)     | Any channel can be TX or RX |

| ESP32-S3      | 8              | 4 (0-3)     | 4 (4-7)     | **Hardcoded allocation** |

| ESP32-C3      | 4              | 2 (0-1)     | 2 (2-3)     | **Hardcoded allocation** |

| ESP32-C6      | 4              | 2 (0-1)     | 2 (2-3)     | **Hardcoded allocation** |

| ESP32-H2      | 4              | 2 (0-1)     | 2 (2-3)     | **Hardcoded allocation** |

## Features Tested

### Core Functionality
- **Constructor/Destructor Behavior**: Object lifecycle management and resource allocation
- **Lifecycle Management**: Initialize/Deinitialize operations with state validation
- **Channel Configuration**: Multi-channel setup and management
- **Symbol Transmission/Reception**: High-speed data transfer with timing validation
- **Error Handling**: Comprehensive error condition testing and recovery

### ESP32 Variant-Specific Features
- **Automatic Variant Detection**: Runtime detection of ESP32 chip variant
- **Channel Allocation Helpers**: Variant-specific TX/RX channel management
- **Channel Direction Validation**: Hardware-enforced TX/RX channel restrictions
- **Resolution_ns Interface**: User-friendly nanosecond resolution with internal Hz conversion

### Advanced RMT Features
- **Channel-Specific Callbacks**: Individual channel callback management with proper user data handling
- **Resolution Control**: Nanosecond-precision timing configuration with internal conversion
- **Carrier Modulation**: 38kHz carrier generation for IR protocols
- **Loopback Mode Testing**: Internal signal routing for validation
- **Encoder Configuration**: Hardware encoder setup and optimization
- **DMA Support**: Direct Memory Access for high-performance transfers
- **Memory Block Management**: Configurable memory allocation per channel

### WS2812 LED Protocol Testing
- **WS2812 Timing Validation**: Complete timing specification verification
- **Single LED Testing**: Built-in RGB LED on GPIO8
- **Multiple LED Chain Testing**: RGB LED chain support
- **Comprehensive Color Cycle**: 30+ color patterns including:
  - Primary colors (R/G/B) at maximum brightness
  - Secondary colors (Yellow/Magenta/Cyan) and white variations
  - Brightness sweep tests (0-255) for each color channel
  - Bit pattern validation (alternating, edge cases, specific patterns)
  - Rainbow color wheel transitions with HSV to RGB conversion
  - Rapid color change sequences for protocol stress testing
- **Pattern Validation**: Specific bit patterns for timing analysis
- **Rainbow Transitions**: HSV to RGB color wheel with smooth transitions
- **Brightness Sweep**: Individual color channel intensity testing

### Automated Testing & Diagnostics
- **Automated Loopback Testing**: TX/RX verification with GPIO8→GPIO18 connection
- **Logic Analyzer Patterns**: Recognizable test patterns for signal analysis
- **Frequency Sweep**: Multi-frequency square wave generation and validation
- **Test Progression Indicator**: GPIO14 visual feedback for test progress
- **Statistics Tracking**: Comprehensive performance metrics and operation counting
- **System Validation**: End-to-end system functionality verification

### Performance & Stress Testing
- **Callback Functionality**: Interrupt-driven callback testing with channel isolation
- **Stress Testing**: High-load scenarios and rapid operation cycles
- **Channel Isolation**: Independent channel operation verification
- **Timing Precision**: Nanosecond-level timing accuracy verification

## Hardware Setup

### ESP32-C6-DevKitM-1 Pin Configuration
- **GPIO8**: Built-in RGB LED + transmission output (WS2812 protocol)
- **GPIO14**: Test progression indicator (visual feedback)
- **GPIO18**: Reception input for automated loopback testing

### Automated Testing Setup
```text
ESP32-C6-DevKitM-1
├── GPIO8 (Built-in RGB LED) ──► Jumper Wire ──► GPIO18 (RX)
├── GPIO14 (Test Progress) ──► LED indicator for test progression
├── Built-in RGB LED: WS2812 protocol testing
└── Automated Loopback: Transmission/reception verification
```text

**For Automated Testing:**
- Connect GPIO8 to GPIO18 with a jumper wire
- This creates a loopback for transmission/reception verification
- GPIO14 provides visual feedback of test progression
- No external components required for basic testing

### WS2812 LED Testing Setup
The ESP32-C6-DevKitM-1 includes a built-in RGB LED on GPIO8, perfect for comprehensive WS2812
testing:

```text
ESP32-C6-DevKitM-1 Built-in RGB LED (GPIO8)
├── WS2812 Protocol Testing
├── 30+ Color Pattern Verification
├── Timing Validation
├── Brightness Sweep Testing
├── Bit Pattern Analysis
└── Rainbow Transition Effects
```text

**Built-in LED Features:**
- ✅ No external wiring required
- ✅ WS2812 protocol compatible
- ✅ Comprehensive color testing (30+ patterns)
- ✅ Timing validation with nanosecond precision
- ✅ Visual verification of test patterns

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

### External WS2812 LED Chain (Optional)
```text
ESP32-C6 GPIO8 ──► WS2812 LED Chain ──► Additional LEDs
                   │
                   └──► 5V Power Supply
                   └──► Ground
```text

**Requirements for External LEDs:**
- WS2812/WS2812B/NeoPixel LEDs
- 5V power supply for LEDs
- 470Ω resistor in series with data line (recommended)
- Common ground between ESP32 and LED power supply

### Logic Analyzer Setup
```text
ESP32-C6 GPIO8 ──► Logic Analyzer Channel 0 (WS2812 data)
ESP32-C6 GPIO14 ──► Logic Analyzer Channel 1 (test progression)
ESP32-C6 GPIO18 ──► Logic Analyzer Channel 2 (loopback verification)
ESP32-C6 GND   ──► Logic Analyzer Ground
```text

**Logic Analyzer Settings:**
- Sample rate: 20MHz or higher
- Voltage threshold: 1.65V (3.3V logic)
- Trigger: Rising edge on GPIO14 (test progression)
- Capture: Multi-channel for comprehensive analysis

## Running the Tests

### Prerequisites
- ESP-IDF v5.5 or later
- ESP32-C6-DevKitM-1 development board
- Jumper wire for loopback testing (GPIO8 → GPIO18)
- Optional: Logic analyzer for timing verification

### Using Build Scripts (Recommended)
```bash
## Navigate to ESP32 examples directory
cd examples/esp32

## Source ESP-IDF environment
source /path/to/esp-idf/export.sh

## Set target and build
export IDF_TARGET=esp32c6
./build_example.sh pio_test Release

## Flash to device
idf.py -B build_pio_test_Release flash monitor
```text

### Direct ESP-IDF Build (Alternative)
```bash
## Set target
export IDF_TARGET=esp32c6

## Build PIO test
idf.py build -DEXAMPLE_TYPE=pio_test

## Flash to device
idf.py flash monitor
```text

### CI/CD Integration
The test is automatically included in the CI pipeline and will run in both Release and Debug
configurations:
```yaml
matrix:
  example_type: [..., pio_test, ...]
```text

## Test Categories

### 1. ESP32 Variant Information Tests
- `test_esp32_variant_detection`: Automatic ESP32 variant detection and reporting
- `test_channel_allocation_helpers`: TX/RX channel helper function validation
- `test_channel_direction_validation`: Hardware-enforced channel direction validation
- `test_resolution_ns_usage`: Resolution_ns interface with clock calculation testing

### 2. Constructor/Destructor Tests
- `test_constructor_default`: Validates proper object initialization
- `test_destructor_cleanup`: Ensures clean resource deallocation

### 3. Lifecycle Tests
- `test_initialization_states`: Tests manual initialization/deinitialization
- `test_lazy_initialization`: Validates automatic initialization

### 4. Channel Configuration Tests
- `test_channel_configuration`: Basic channel setup validation with variant awareness
- `test_multiple_channel_configuration`: Multi-channel operation with TX/RX allocation

### 5. Transmission Tests
- `test_basic_symbol_transmission`: Basic symbol transmission with improved error handling
- `test_transmission_edge_cases`: Error handling and boundary conditions

### 6. WS2812 LED Protocol Tests
- `test_ws2812_single_led`: Single LED color transmission using built-in RGB LED
- `test_ws2812_multiple_leds`: RGB LED chain testing
- `test_ws2812_color_cycle`: Comprehensive 30+ color pattern testing including:
  - Primary colors at maximum brightness
  - Secondary colors and white variations
  - Gradient patterns and brightness levels
  - Specific bit patterns for timing verification
  - Color wheel simulation
  - Rapid color change stress testing
- `test_ws2812_brightness_sweep`: Individual color channel intensity sweep (0-255)
- `test_ws2812_pattern_validation`: Specific bit patterns for protocol accuracy
- `test_ws2812_rainbow_transition`: HSV to RGB rainbow transitions with smooth color wheel

### 7. Logic Analyzer Test Scenarios
- `test_logic_analyzer_patterns`: Recognizable test patterns for signal analysis
- `test_frequency_sweep`: Multi-frequency square wave generation

### 8. Advanced RMT Feature Tests
- `test_rmt_encoder_configuration`: Hardware encoder setup
- `test_rmt_carrier_modulation`: 38kHz carrier generation
- `test_rmt_advanced_configuration`: DMA and advanced features

### 9. Loopback and Reception Tests
- `test_loopback_functionality`: Transmission/reception verification with automated setup

### 10. Callback Tests
- `test_callback_functionality`: Channel-specific interrupt-driven callbacks with user data

### 11. Statistics and Diagnostics Tests
- `test_statistics_and_diagnostics`: Comprehensive performance metrics and error reporting

### 12. Stress and Performance Tests
- `test_stress_transmission`: High-load testing with rapid operation cycles

### 13. System Validation Tests
- `test_pio_system_validation`: End-to-end comprehensive system functionality

## WS2812 Protocol Specifications

### Timing Requirements

| Symbol | High Time | Low Time | Tolerance |

|--------|-----------|----------|-----------|

| '0' bit| 350ns    | 900ns    | ±150ns    |

| '1' bit| 700ns    | 600ns    | ±150ns    |

| Reset  | -        | >50µs    | -         |

### Color Format
- **Data Order**: GRB (Green, Red, Blue)
- **Resolution**: 8 bits per color channel
- **Total**: 24 bits per LED

### Comprehensive Test Colors
#### Primary Colors (Maximum Brightness)
- **Red**: RGB(255, 0, 0) → GRB(0, 255, 0)
- **Green**: RGB(0, 255, 0) → GRB(255, 0, 0)  
- **Blue**: RGB(0, 0, 255) → GRB(0, 0, 255)

#### Secondary Colors
- **Yellow**: RGB(255, 255, 0) → GRB(255, 255, 0)
- **Magenta**: RGB(255, 0, 255) → GRB(0, 255, 255)
- **Cyan**: RGB(0, 255, 255) → GRB(255, 0, 255)

#### White Variations
- **White Max**: RGB(255, 255, 255) → GRB(255, 255, 255)
- **White Mid**: RGB(128, 128, 128) → GRB(128, 128, 128)
- **White Low**: RGB(64, 64, 64) → GRB(64, 64, 64)

#### Bit Pattern Test Values
- **Pattern 01**: 0x55 (01010101) for alternating bit testing
- **Pattern 10**: 0xAA (10101010) for alternating bit testing
- **Pattern F0**: 0xF0 (11110000) for nibble testing
- **Pattern 0F**: 0x0F (00001111) for nibble testing

#### Rainbow Transition
- **360° Hue Sweep**: HSV to RGB conversion with smooth transitions
- **Brightness Control**: Scalable intensity for visual comfort
- **Timing Analysis**: Smooth color wheel for protocol stress testing

## Logic Analyzer Test Patterns

### Pattern 1: Basic Timing Test
```text
1µs HIGH → 1µs LOW → 2µs HIGH → 2µs LOW → 0.5µs HIGH → 0.5µs LOW → 3µs HIGH → 1.5µs LOW → 0.75µs
HIGH → 4µs LOW
```text

### Pattern 2: Frequency Sweep
- 1kHz: 500µs HIGH, 500µs LOW
- 5kHz: 100µs HIGH, 100µs LOW
- 10kHz: 50µs HIGH, 50µs LOW
- 50kHz: 10µs HIGH, 10µs LOW
- 100kHz: 5µs HIGH, 5µs LOW

### Pattern 3: Test Progression Monitoring
- **GPIO14 Toggle**: HIGH/LOW transition for each completed test
- **Test Sequence**: Visual feedback for 25+ individual tests
- **Timing Reference**: Consistent toggle pattern for analyzer triggering

## Expected Test Results

### Successful Test Output
```text
[PIO_Test] ╔═══════════════════════════════════════════════════════════════════════════════╗
[PIO_Test] ║                    ESP32-C6 PIO COMPREHENSIVE TEST SUITE                     ║
[PIO_Test] ║  Testing EspPio with ESP-IDF v5.5 RMT peripheral                             ║
[PIO_Test] ║  Includes WS2812 LED protocol and automated loopback testing                 ║
[PIO_Test] ║                                                                               ║
[PIO_Test] ║  Test Pins (ESP32-C6 DevKitM-1):                                             ║
[PIO_Test] ║    GPIO 8 - Built-in RGB LED (WS2812) + TX for loopback                     ║
[PIO_Test] ║    GPIO 14 - Test progression indicator                                       ║
[PIO_Test] ║    GPIO 18 - RX for automated loopback verification                          ║
[PIO_Test] ║                                                                               ║
[PIO_Test] ║  For automated testing: Connect GPIO 8 to GPIO 18 with jumper wire          ║
[PIO_Test] ╚═══════════════════════════════════════════════════════════════════════════════╝

[PIO_Test] === ESP32 VARIANT INFORMATION TESTS ===
[PIO_Test] [SUCCESS] PASSED: test_esp32_variant_detection (0.05 ms)
[PIO_Test] [SUCCESS] PASSED: test_channel_allocation_helpers (0.12 ms)
[PIO_Test] [SUCCESS] PASSED: test_channel_direction_validation (0.08 ms)
[PIO_Test] [SUCCESS] PASSED: test_resolution_ns_usage (1.45 ms)

[PIO_Test] === CONSTRUCTOR/DESTRUCTOR TESTS ===
[PIO_Test] [SUCCESS] PASSED: test_constructor_default (0.05 ms)
[PIO_Test] [SUCCESS] PASSED: test_destructor_cleanup (0.32 ms)

[PIO_Test] === WS2812 LED PROTOCOL TESTS ===
[PIO_Test] [SUCCESS] PASSED: test_ws2812_single_led (2.34 ms)
[PIO_Test] [SUCCESS] PASSED: test_ws2812_multiple_leds (5.67 ms)
[PIO_Test] [SUCCESS] PASSED: test_ws2812_color_cycle (45.23 ms)
[PIO_Test] [SUCCESS] PASSED: test_ws2812_brightness_sweep (12.89 ms)
[PIO_Test] [SUCCESS] PASSED: test_ws2812_pattern_validation (8.76 ms)
[PIO_Test] [SUCCESS] PASSED: test_ws2812_rainbow_transition (15.43 ms)

...

[PIO_Test] === PIO TEST SUMMARY ===
[PIO_Test] Total: 25, Passed: 25, Failed: 0, Success: 100.00%, Time: 185.67 ms
[PIO_Test] [SUCCESS] ALL PIO TESTS PASSED!

[PIO_Test] ║  New Features Tested:                                                         ║
[PIO_Test] ║    ✓ Channel-specific callbacks with user data                               ║
[PIO_Test] ║    ✓ Resolution_hz for direct ESP-IDF compatibility                         ║
[PIO_Test] ║    ✓ ESP32 variant-specific channel validation                              ║
[PIO_Test] ║    ✓ Enhanced clock divider calculation                                     ║
[PIO_Test] ║    ✓ Test progression indicator on GPIO14                                   ║
[PIO_Test] ║    ✓ Comprehensive WS2812 color testing (30+ patterns)                     ║
```text

### Built-in RGB LED Verification
The ESP32-C6-DevKitM-1's built-in RGB LED demonstrates comprehensive testing:
1. **Single LED Test**: LED turns red
2. **Color Cycle Test**: 30+ color patterns including:
   - Primary colors (R/G/B) at full brightness
   - Secondary colors (Yellow/Magenta/Cyan)
   - White variations at different intensities
   - Specific bit patterns for timing analysis
   - Rainbow color wheel transitions
   - Rapid color change sequences
3. **Brightness Sweep**: Smooth 0-255 intensity transitions for each color
4. **Pattern Validation**: Specific bit patterns for protocol verification
5. Reset sequences between all tests

### Automated Loopback Verification
With GPIO8 → GPIO18 jumper wire:
1. **Transmission Test**: Data sent from GPIO8
2. **Reception Test**: Same data received on GPIO18
3. **Data Integrity**: Automatic verification of transmission/reception
4. **Timing Validation**: Signal timing accuracy verification

### Test Progression Monitoring
With GPIO14 monitoring:
1. **Visual Feedback**: LED toggles for each completed test
2. **Logic Analyzer Triggering**: Consistent reference for capture
3. **Test Sequence Verification**: Real-time progress monitoring
4. **Automated Testing**: Self-contained test progression

### Logic Analyzer Verification
Capture signals on GPIO8, GPIO14, and GPIO18 to verify:
1. **WS2812 Timing**: Accuracy within ±150ns tolerance
2. **Color Pattern Accuracy**: Correct bit encoding for all 30+ color patterns
3. **Test Progression**: GPIO14 toggle pattern
4. **Loopback Verification**: TX/RX signal correlation
5. **Frequency Sweep**: Multi-frequency pattern validation

## Troubleshooting

### Common Issues

#### Test Failures
- **Timing Issues**: Verify resolution_ns values are within hardware constraints (use GetResolutionConstraints())
- **GPIO Conflicts**: Check pin availability and configuration
- **Initialization Failures**: Ensure ESP-IDF v5.5+ and proper hardware
- **Variant Detection**: Verify ESP32 variant is properly detected and supported

#### Built-in RGB LED Issues
- **No LED Response**: Check if LED is enabled in board configuration
- **Wrong Colors**: Verify GRB data format, timing accuracy
- **Dim LED**: Normal behavior for built-in LED at reduced intensity
- **Color Pattern Issues**: Verify test progression indicator shows completion
- **Rapid Color Changes**: Normal during stress testing sequences

#### Test Progression Indicator Issues
- **No GPIO14 Activity**: Check GPIO14 initialization and connectivity
- **Irregular Toggle Pattern**: Verify test sequence completion
- **Logic Analyzer Sync**: Use GPIO14 as trigger reference

#### Loopback Testing Issues
- **No Reception**: Verify jumper wire connection GPIO8 → GPIO18
- **Data Mismatch**: Check for loose connections or interference
- **Timing Issues**: Ensure proper RMT configuration
- **Channel Allocation**: Verify TX/RX channels available for current ESP32 variant

#### WS2812 Testing Issues
- **Color Accuracy**: Use logic analyzer to verify bit timing
- **Pattern Recognition**: Monitor GPIO8 during specific pattern tests
- **Rainbow Transitions**: Smooth color changes indicate proper HSV conversion
- **Brightness Sweep**: Verify gradual intensity changes

#### Logic Analyzer Issues
- **No Signal**: Verify probe connections, ground reference
- **Timing Inaccuracy**: Increase sample rate, check triggering on GPIO14
- **Signal Distortion**: Reduce probe capacitance, improve connections
- **Multi-channel Sync**: Use GPIO14 as common trigger reference

### Debug Mode
Enable detailed logging by building in Debug mode:
```bash
## Using build scripts (recommended)
./build_example.sh pio_test Debug

## Or direct ESP-IDF build
idf.py build -DEXAMPLE_TYPE=pio_test -DBUILD_TYPE=Debug
```text

## Performance Metrics

### Build Information
- **Build Status**: ✅ SUCCESS
- **Target**: ESP32-C6
- **Binary Size**: 0x335d0 bytes (209,872 bytes)
- **Free Space**: 86% of partition available
- **ESP-IDF Version**: v5.5
- **Test Count**: 25+ comprehensive tests

### Typical Results (ESP32-C6 @ 160MHz)
- **Initialization Time**: <1ms
- **Channel Configuration**: <0.5ms
- **Single Symbol Transmission**: ~10µs
- **WS2812 24-bit Transmission**: ~30µs
- **Color Cycle Test (30+ patterns)**: ~45ms
- **Brightness Sweep Test**: ~13ms
- **Rainbow Transition Test**: ~15ms
- **Stress Test (100 symbols)**: ~1ms
- **Complete Test Suite**: ~185ms

### Memory Usage
- **RAM**: ~3KB for enhanced test framework
- **Flash**: ~20KB for comprehensive test code
- **RMT Memory**: 64 symbols per channel (configurable)

## Integration with Development Workflow

### Continuous Integration
The test automatically runs in CI for:
- Pull request validation
- Main branch commits
- Release candidate testing
- ESP32 variant compatibility verification

### Hardware-in-the-Loop Testing
For production validation:
1. Use built-in RGB LED for comprehensive WS2812 testing
2. Monitor GPIO14 for test progression verification
3. Connect jumper wire for loopback verification
4. Use logic analyzer for timing verification
5. Run automated test suite (25+ tests)
6. Validate timing against WS2812 specifications
7. Verify variant-specific channel allocation

## Advanced Configuration

### Custom GPIO Pins
Modify the test for different hardware:
```cpp
// ESP32-C6 DevKitM-1 specific configuration
#if defined(CONFIG_IDF_TARGET_ESP32C6)
static constexpr hf_gpio_num_t TEST_GPIO_TX = 8;   // Built-in RGB LED
static constexpr hf_gpio_num_t TEST_GPIO_RX = 18;  // Loopback RX
static constexpr hf_gpio_num_t TEST_GPIO_PROGRESS = 14; // Progress indicator
#else
static constexpr hf_gpio_num_t TEST_GPIO_TX = 2;   // Other ESP32 variants
static constexpr hf_gpio_num_t TEST_GPIO_RX = 3;   // Other ESP32 variants
static constexpr hf_gpio_num_t TEST_GPIO_PROGRESS = 2; // Other variants
#endif
```text

### Timing Resolution
Adjust for different requirements:
```cpp
// ESP32-C6 specific resolution configuration
#if defined(CONFIG_IDF_TARGET_ESP32C6)
config.resolution_ns = 125; // 8MHz resolution - optimized for WS2812 timing
#else
config.resolution_ns = TEST_RESOLUTION_STANDARD_NS; // 1µs standard resolution
#endif
```text

### Test Parameters
Customize test behavior:
```cpp
// WS2812 timing specifications
static constexpr uint32_t WS2812_T0H = 350;  // WS2812B high time for '0'
static constexpr uint32_t WS2812_T0L = 900;  // WS2812B low time for '0'
static constexpr uint32_t WS2812_T1H = 700;  // WS2812B high time for '1'
static constexpr uint32_t WS2812_T1L = 600;  // WS2812B low time for '1'

// Test progression configuration
static constexpr uint32_t TEST_PROGRESS_DELAY_MS = 100; // Progress indicator timing
```text

## ESP32-C6 Specific Features

### RMT Peripheral
- **Channels**: 2 TX (0-1) + 2 RX (2-3) channels
- **Clock Source**: PLL_F80M (80 MHz) with automatic fallback
- **Memory**: Configurable memory blocks per channel
- **DMA**: Supported for large transfers
- **Resolution**: 125ns minimum (8MHz) for WS2812 precision

### Built-in RGB LED
- **GPIO**: GPIO8
- **Protocol**: WS2812 compatible
- **Power**: 3.3V logic level
- **Features**: No external components required
- **Testing**: 30+ comprehensive color patterns

### Testing Advantages
- **No External Wiring**: Uses built-in LED for WS2812 testing
- **Visual Feedback**: GPIO14 progression indicator
- **Loopback Verification**: Simple jumper wire connection
- **Variant Awareness**: Automatic ESP32 variant detection
- **Self-Contained**: Minimal external dependencies
- **Reliable**: Consistent hardware configuration
- **Comprehensive**: 25+ individual test cases

## References

- [ESP-IDF RMT Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/rmt.html)
- [WS2812B Datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
- [ESP32-C6 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf)
- [ESP-IDF v5.5 Migration Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/migration-guides/release-5.x/5.0/peripherals.html)
- [ESP32-C6-DevKitM-1 User Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/hw-reference/esp32c6/user-guide-devkitm-1.html)
