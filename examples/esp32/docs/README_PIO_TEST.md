# ESP32-C6 PIO Comprehensive Test Suite Documentation

## Overview

The PIO Comprehensive Test Suite provides extensive validation of the `EspPio` class for ESP32 platforms using the RMT (Remote Control) peripheral with ESP-IDF v5.5+. This test suite demonstrates complete PIO functionality including WS2812 LED protocol timing, automated loopback testing, channel-specific callbacks, and advanced RMT features with a focus on embedded environments using `noexcept` functions.

**✅ Status: Successfully tested on ESP32-C6-DevKitM-1 hardware**

### Supported ESP32 Variants

The implementation automatically adapts to different ESP32 variants with their specific RMT channel allocation constraints:

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

### Advanced RMT Features
- **Channel-Specific Callbacks**: Individual channel callback management with proper user data handling
- **Resolution Control**: Nanosecond-precision timing configuration with internal conversion
- **Carrier Modulation**: 38kHz carrier generation for IR protocols
- **Loopback Mode Testing**: Internal signal routing for validation
- **Encoder Configuration**: Hardware encoder setup and optimization
- **DMA Support**: Direct Memory Access for high-performance transfers
- **Memory Block Management**: Configurable memory allocation per channel

### Protocol Testing
- **WS2812 LED Protocol**: Complete timing validation using built-in RGB LED on GPIO8
- **Automated Loopback Testing**: TX/RX verification with GPIO8→GPIO18 connection
- **Logic Analyzer Patterns**: Recognizable test patterns for signal analysis
- **Frequency Sweep**: Multi-frequency square wave generation and validation
- **Timing Precision**: Nanosecond-level timing accuracy verification

### Diagnostics & Performance
- **Statistics Tracking**: Comprehensive performance metrics and operation counting
- **System Validation**: End-to-end system functionality verification
- **Callback Functionality**: Interrupt-driven callback testing with channel isolation
- **Stress Testing**: High-load scenarios and rapid operation cycles
- **Channel Isolation**: Independent channel operation verification

## Hardware Setup

### ESP32-C6-DevKitM-1 Pin Configuration
- **GPIO8**: Built-in RGB LED + transmission output (WS2812 protocol)
- **GPIO18**: Reception input for automated loopback testing

### Automated Testing Setup
```
ESP32-C6-DevKitM-1
├── GPIO8 (Built-in RGB LED) ──► Jumper Wire ──► GPIO18 (RX)
├── Built-in RGB LED: WS2812 protocol testing
└── Automated Loopback: Transmission/reception verification
```

**For Automated Testing:**
- Connect GPIO8 to GPIO18 with a jumper wire
- This creates a loopback for transmission/reception verification
- No external components required for basic testing

### WS2812 LED Testing Setup
The ESP32-C6-DevKitM-1 includes a built-in RGB LED on GPIO8, perfect for WS2812 testing:

```
ESP32-C6-DevKitM-1 Built-in RGB LED (GPIO8)
├── WS2812 Protocol Testing
├── Color Pattern Verification
└── Timing Validation
```

**Built-in LED Features:**
- ✅ No external wiring required
- ✅ WS2812 protocol compatible
- ✅ RGB color testing
- ✅ Timing validation

### External WS2812 LED Chain (Optional)
```
ESP32-C6 GPIO8 ──► WS2812 LED Chain ──► Additional LEDs
                   │
                   └──► 5V Power Supply
                   └──► Ground
```

**Requirements for External LEDs:**
- WS2812/WS2812B/NeoPixel LEDs
- 5V power supply for LEDs
- 470Ω resistor in series with data line (recommended)
- Common ground between ESP32 and LED power supply

### Logic Analyzer Setup
```
ESP32-C6 GPIO8 ──► Logic Analyzer Channel 0
ESP32-C6 GPIO18 ──► Logic Analyzer Channel 1 (loopback verification)
ESP32-C6 GND   ──► Logic Analyzer Ground
```

**Logic Analyzer Settings:**
- Sample rate: 20MHz or higher
- Voltage threshold: 1.65V (3.3V logic)
- Trigger: Rising edge on test signal

## Running the Tests

### Prerequisites
- ESP-IDF v5.5 or later
- ESP32-C6-DevKitM-1 development board
- Jumper wire for loopback testing (GPIO8 → GPIO18)

### Using Build Scripts (Recommended)
```bash
# Navigate to ESP32 examples directory
cd examples/esp32

# Source ESP-IDF environment
source /path/to/esp-idf/export.sh

# Set target and build
export IDF_TARGET=esp32c6
./build_example.sh pio_test Release

# Flash to device
idf.py -B build_pio_test_Release flash monitor
```

### Direct ESP-IDF Build (Alternative)
```bash
# Set target
export IDF_TARGET=esp32c6

# Build PIO test
idf.py build -DEXAMPLE_TYPE=pio_test

# Flash to device
idf.py flash monitor
```

### CI/CD Integration
The test is automatically included in the CI pipeline and will run in both Release and Debug configurations:
```yaml
matrix:
  example_type: [..., pio_test, ...]
```

## Test Categories

### 1. Constructor/Destructor Tests
- `test_constructor_default`: Validates proper object initialization
- `test_destructor_cleanup`: Ensures clean resource deallocation

### 2. Lifecycle Tests
- `test_initialization_states`: Tests manual initialization/deinitialization
- `test_lazy_initialization`: Validates automatic initialization

### 3. Channel Configuration Tests
- `test_channel_configuration`: Basic channel setup validation
- `test_multiple_channel_configuration`: Multi-channel operation

### 4. Transmission Tests
- `test_basic_symbol_transmission`: Basic symbol transmission
- `test_transmission_edge_cases`: Error handling and boundary conditions

### 5. WS2812 LED Protocol Tests
- `test_ws2812_timing_validation`: Timing specification verification
- `test_ws2812_single_led`: Single LED color transmission using built-in RGB LED
- `test_ws2812_multiple_leds`: RGB LED chain testing

### 6. Automated Loopback Tests
- `test_loopback_functionality`: Transmission/reception verification
- `test_logic_analyzer_patterns`: Recognizable test patterns
- `test_frequency_sweep`: Multi-frequency square wave generation

### 7. Advanced RMT Feature Tests
- `test_rmt_encoder_configuration`: Hardware encoder setup
- `test_rmt_carrier_modulation`: 38kHz carrier generation
- `test_rmt_advanced_configuration`: DMA and advanced features

### 8. System Tests
- `test_callback_functionality`: Interrupt-driven callbacks
- `test_statistics_and_diagnostics`: Performance metrics
- `test_stress_transmission`: High-load testing
- `test_pio_system_validation`: Comprehensive validation

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

### Test Colors
- **Red**: RGB(255, 0, 0) → GRB(0, 255, 0)
- **Green**: RGB(0, 255, 0) → GRB(255, 0, 0)  
- **Blue**: RGB(0, 0, 255) → GRB(0, 0, 255)

## Logic Analyzer Test Patterns

### Pattern 1: Basic Timing Test
```
1µs HIGH → 1µs LOW → 2µs HIGH → 2µs LOW → 0.5µs HIGH → 0.5µs LOW → 3µs HIGH → 1.5µs LOW → 0.75µs HIGH → 4µs LOW
```

### Pattern 2: Frequency Sweep
- 1kHz: 500µs HIGH, 500µs LOW
- 5kHz: 100µs HIGH, 100µs LOW
- 10kHz: 50µs HIGH, 50µs LOW
- 50kHz: 10µs HIGH, 10µs LOW
- 100kHz: 5µs HIGH, 5µs LOW

## Expected Test Results

### Successful Test Output
```
[PIO_Test] ╔═══════════════════════════════════════════════════════════════════════════════╗
[PIO_Test] ║                    ESP32-C6 PIO COMPREHENSIVE TEST SUITE                     ║
[PIO_Test] ║  Testing EspPio with ESP-IDF v5.5 RMT peripheral                             ║
[PIO_Test] ║  Includes WS2812 LED protocol and automated loopback testing                 ║
[PIO_Test] ║                                                                               ║
[PIO_Test] ║  Test Pins (ESP32-C6 DevKitM-1):                                             ║
[PIO_Test] ║    GPIO 8 - Built-in RGB LED (WS2812) + TX for loopback                     ║
[PIO_Test] ║    GPIO 18 - RX for automated loopback verification                          ║
[PIO_Test] ║                                                                               ║
[PIO_Test] ║  For automated testing: Connect GPIO 8 to GPIO 18 with jumper wire          ║
[PIO_Test] ╚═══════════════════════════════════════════════════════════════════════════════╝

[PIO_Test] [SUCCESS] PASSED: test_constructor_default (0.05 ms)
[PIO_Test] [SUCCESS] PASSED: test_ws2812_single_led (2.34 ms)
...
[PIO_Test] === PIO TEST SUMMARY ===
[PIO_Test] Total: 20, Passed: 20, Failed: 0, Success: 100.00%, Time: 125.67 ms
[PIO_Test] [SUCCESS] ALL PIO TESTS PASSED!
```

### Built-in RGB LED Verification
The ESP32-C6-DevKitM-1's built-in RGB LED should show:
1. **Single LED Test**: LED turns red
2. **Multiple LED Test**: Color cycling through red, green, blue
3. Reset sequences between tests

### Automated Loopback Verification
With GPIO8 → GPIO18 jumper wire:
1. **Transmission Test**: Data sent from GPIO8
2. **Reception Test**: Same data received on GPIO18
3. **Data Integrity**: Automatic verification of transmission/reception
4. **Timing Validation**: Signal timing accuracy

### Logic Analyzer Verification
Capture signals on GPIO8 and verify:
1. Timing accuracy within ±150ns tolerance
2. Correct high/low durations
3. Proper WS2812 bit encoding
4. Frequency sweep patterns

## Troubleshooting

### Common Issues

#### Test Failures
- **Timing Issues**: Verify resolution_ns values are within hardware constraints (use GetResolutionConstraints())
- **GPIO Conflicts**: Check pin availability and configuration
- **Initialization Failures**: Ensure ESP-IDF v5.5+ and proper hardware

#### Built-in RGB LED Issues
- **No LED Response**: Check if LED is enabled in board configuration
- **Wrong Colors**: Verify GRB data format, timing accuracy
- **Dim LED**: Normal behavior for built-in LED

#### Loopback Testing Issues
- **No Reception**: Verify jumper wire connection GPIO8 → GPIO18
- **Data Mismatch**: Check for loose connections or interference
- **Timing Issues**: Ensure proper RMT configuration

#### Logic Analyzer Issues
- **No Signal**: Verify probe connections, ground reference
- **Timing Inaccuracy**: Increase sample rate, check triggering
- **Signal Distortion**: Reduce probe capacitance, improve connections

### Debug Mode
Enable detailed logging by building in Debug mode:
```bash
# Using build scripts (recommended)
./build_example.sh pio_test Debug

# Or direct ESP-IDF build
idf.py build -DEXAMPLE_TYPE=pio_test -DBUILD_TYPE=Debug
```

## Performance Metrics

### Build Information
- **Build Status**: ✅ SUCCESS
- **Target**: ESP32-C6
- **Binary Size**: 0x335d0 bytes (209,872 bytes)
- **Free Space**: 86% of partition available
- **ESP-IDF Version**: v5.5

### Typical Results (ESP32-C6 @ 160MHz)
- **Initialization Time**: <1ms
- **Channel Configuration**: <0.5ms
- **Single Symbol Transmission**: ~10µs
- **WS2812 24-bit Transmission**: ~30µs
- **Stress Test (100 symbols)**: ~1ms

### Memory Usage
- **RAM**: ~2KB for test framework
- **Flash**: ~15KB for test code
- **RMT Memory**: 64 symbols per channel (configurable)

## Integration with Development Workflow

### Continuous Integration
The test automatically runs in CI for:
- Pull request validation
- Main branch commits
- Release candidate testing

### Hardware-in-the-Loop Testing
For production validation:
1. Use built-in RGB LED for WS2812 testing
2. Connect jumper wire for loopback verification
3. Use logic analyzer for timing verification
4. Run automated test suite
5. Validate timing against specifications

## Advanced Configuration

### Custom GPIO Pins
Modify the test for different hardware:
```cpp
// ESP32-C6 DevKitM-1 specific configuration
#if defined(CONFIG_IDF_TARGET_ESP32C6)
static constexpr hf_gpio_num_t TEST_GPIO_TX = 8;   // Built-in RGB LED
static constexpr hf_gpio_num_t TEST_GPIO_RX = 18;  // Loopback RX
#else
static constexpr hf_gpio_num_t TEST_GPIO_TX = 2;   // Other ESP32 variants
static constexpr hf_gpio_num_t TEST_GPIO_RX = 3;   // Other ESP32 variants
#endif
```

### Timing Resolution
Adjust for different requirements:
```cpp
// ESP32-C6 specific resolution configuration
#if defined(CONFIG_IDF_TARGET_ESP32C6)
config.resolution_ns = 1000; // 1µs resolution - automatically optimized for ESP32-C6 RMT
#else
config.resolution_ns = TEST_RESOLUTION_STANDARD_NS; // Use nanosecond equivalent of standard resolution
#endif
```

### Test Parameters
Customize test behavior:
```cpp
static constexpr uint32_t WS2812_T0H = 350;  // WS2812B timing
static constexpr uint32_t WS2812_T1H = 700;  // WS2812B timing
```

## ESP32-C6 Specific Features

### RMT Peripheral
- **Channels**: 2 RMT channels (vs 4 on other ESP32 variants)
- **Clock Source**: PLL_F80M (80 MHz)
- **Memory**: Configurable memory blocks
- **DMA**: Supported for large transfers

### Built-in RGB LED
- **GPIO**: GPIO8
- **Protocol**: WS2812 compatible
- **Power**: 3.3V logic level
- **Features**: No external components required

### Automated Testing Advantages
- **No External Wiring**: Uses built-in LED for WS2812 testing
- **Loopback Verification**: Simple jumper wire connection
- **Self-Contained**: Minimal external dependencies
- **Reliable**: Consistent hardware configuration

## References

- [ESP-IDF RMT Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/rmt.html)
- [WS2812B Datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
- [ESP32-C6 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf)
- [ESP-IDF v5.5 Migration Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/migration-guides/release-5.x/5.0/peripherals.html)
- [ESP32-C6-DevKitM-1 User Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/hw-reference/esp32c6/user-guide-devkitm-1.html)
