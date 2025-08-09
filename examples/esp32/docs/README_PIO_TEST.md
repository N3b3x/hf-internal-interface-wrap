# PIO Comprehensive Test Suite Documentation

## Overview

The PIO Comprehensive Test Suite is designed to thoroughly validate the `EspPio` class implementation using the ESP32-C6's RMT (Remote Control) peripheral with ESP-IDF v5.5+. This test suite provides comprehensive coverage of PIO functionality including WS2812 LED protocol timing and logic analyzer test scenarios.

## Features Tested

### Core Functionality
- Constructor/Destructor behavior
- Lifecycle management (Initialize/Deinitialize)
- Channel configuration and management
- Symbol transmission and reception
- Error handling and edge cases

### RMT-Specific Features
- Carrier modulation (38kHz for IR protocols)
- Loopback mode testing
- Encoder configuration
- Advanced RMT configuration (DMA, memory blocks, queue depth)
- Idle level configuration

### Protocol Testing
- **WS2812 LED Protocol**: Complete timing validation and RGB color transmission
- **Logic Analyzer Patterns**: Various test patterns for signal verification
- **Frequency Sweep**: Square wave generation at different frequencies

### Diagnostics
- Statistics and performance metrics
- System validation
- Callback functionality
- Stress testing

## Hardware Setup

### GPIO Pin Configuration
- **GPIO2**: Primary transmission output (connect WS2812 LEDs or logic analyzer)
- **GPIO3**: Reception input (optional, for bidirectional testing)
- **GPIO4**: Loopback testing

### WS2812 LED Testing Setup
```
ESP32-C6 GPIO2 ──► WS2812 LED Chain ──► Additional LEDs
                   │
                   └──► 5V Power Supply
                   └──► Ground
```

**Requirements:**
- WS2812/WS2812B/NeoPixel LEDs
- 5V power supply for LEDs
- 470Ω resistor in series with data line (recommended)
- Common ground between ESP32 and LED power supply

### Logic Analyzer Setup
```
ESP32-C6 GPIO2 ──► Logic Analyzer Channel 0
ESP32-C6 GPIO3 ──► Logic Analyzer Channel 1 (optional)
ESP32-C6 GND   ──► Logic Analyzer Ground
```

**Logic Analyzer Settings:**
- Sample rate: 20MHz or higher
- Voltage threshold: 1.65V (3.3V logic)
- Trigger: Rising edge on test signal

## Running the Tests

### Build and Flash
```bash
# Navigate to ESP32 examples directory
cd examples/esp32

# Build PIO test
idf.py build -DEXAMPLE_TYPE=pio_test

# Flash to device
idf.py flash monitor
```

### Using Build Scripts
```bash
# Linux/macOS
./build_example.sh pio_test

# Windows PowerShell
.\build_example.ps1 pio_test
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
- `test_ws2812_single_led`: Single LED color transmission
- `test_ws2812_multiple_leds`: RGB LED chain testing

### 6. Logic Analyzer Test Scenarios
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
[PIO_Test] ║  Includes WS2812 LED protocol and logic analyzer test scenarios              ║
[PIO_Test] ╚═══════════════════════════════════════════════════════════════════════════════╝

[PIO_Test] [SUCCESS] PASSED: test_constructor_default (0.05 ms)
[PIO_Test] [SUCCESS] PASSED: test_ws2812_single_led (2.34 ms)
...
[PIO_Test] === PIO TEST SUMMARY ===
[PIO_Test] Total: 20, Passed: 20, Failed: 0, Success: 100.00%, Time: 125.67 ms
[PIO_Test] [SUCCESS] ALL PIO TESTS PASSED!
```

### WS2812 LED Verification
If WS2812 LEDs are connected, you should observe:
1. **Single LED Test**: LED turns red
2. **Multiple LED Test**: First LED red, second green, third blue
3. Reset sequences between tests

### Logic Analyzer Verification
Capture signals on GPIO2 and verify:
1. Timing accuracy within ±150ns tolerance
2. Correct high/low durations
3. Proper WS2812 bit encoding
4. Frequency sweep patterns

## Troubleshooting

### Common Issues

#### Test Failures
- **Timing Issues**: Verify TEST_RESOLUTION_NS matches RMT capabilities
- **GPIO Conflicts**: Check pin availability and configuration
- **Initialization Failures**: Ensure ESP-IDF v5.5+ and proper hardware

#### WS2812 LED Issues
- **No LED Response**: Check power supply, data line connection
- **Wrong Colors**: Verify GRB data format, timing accuracy
- **Intermittent Operation**: Add series resistor, check ground connections

#### Logic Analyzer Issues
- **No Signal**: Verify probe connections, ground reference
- **Timing Inaccuracy**: Increase sample rate, check triggering
- **Signal Distortion**: Reduce probe capacitance, improve connections

### Debug Mode
Enable detailed logging by building in Debug mode:
```bash
idf.py build -DEXAMPLE_TYPE=pio_test -DBUILD_TYPE=Debug
```

## Performance Metrics

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
1. Connect WS2812 LEDs to test board
2. Use logic analyzer for timing verification
3. Run automated test suite
4. Validate timing against specifications

## Advanced Configuration

### Custom GPIO Pins
Modify the test for different hardware:
```cpp
static constexpr hf_gpio_num_t TEST_GPIO_TX = 5;  // Change to your pin
static constexpr hf_gpio_num_t TEST_GPIO_RX = 6;  // Change to your pin
```

### Timing Resolution
Adjust for different requirements:
```cpp
static constexpr uint32_t TEST_RESOLUTION_NS = 50;  // Higher precision
```

### Test Parameters
Customize test behavior:
```cpp
static constexpr uint32_t WS2812_T0H = 400;  // Adjust for LED variant
static constexpr uint32_t WS2812_T1H = 800;  // Adjust for LED variant
```

## References

- [ESP-IDF RMT Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/rmt.html)
- [WS2812B Datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
- [ESP32-C6 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf)
- [ESP-IDF v5.5 Migration Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/migration-guides/release-5.x/5.0/peripherals.html)