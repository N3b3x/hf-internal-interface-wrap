# ESP32-C6 ADC Comprehensive Testing Suite

## Overview

The ADC comprehensive testing suite provides thorough validation of the `EspAdc` implementation on
ESP32-C6 hardware.
It includes 12 test categories covering hardware validation, initialization, channel configuration,
calibration, one-shot and continuous modes, threshold monitoring, error handling, statistics,
and performance characteristics.

## Hardware Requirements

### Target Hardware
- **ESP32-C6 DevKit-M-1** (primary target)
- ESP32-C6 DevKitC-1 (also supported, just wifi antenna difference)
- USB-C cable for programming and power
- Computer with ESP-IDF v5.5+ installed

### Required Components
- **10kΩ potentiometer** (for threshold monitoring on GPIO0)
- **Resistors**: Two 10kΩ resistors for voltage references
- **Jumper wires** for connections
- **Breadboard** (recommended for clean connections)

### Hardware Setup

The test suite exercises 3 ADC channels with specific voltage references:

#### Channel Configuration
- **GPIO3 (ADC1_CH3)**: Voltage divider reference (~1.65V)
- **GPIO0 (ADC1_CH0)**: Potentiometer center tap (0-3.3V variable)
- **GPIO1 (ADC1_CH1)**: Ground reference (~0V)

#### Circuit Connections

```text
ESP32-C6 Test Circuit:

3.3V Rail:
   |
   ├── [10kΩ] ──┬── [10kΩ] ── GND    (Voltage Divider for GPIO3)
   │            │
   │            └── GPIO3 (ADC1_CH3)  (~1.65V reference)
   │
   ├── [Potentiometer 10kΩ]
   │      ├── 3.3V (top terminal)
   │      ├── GPIO0 (center wiper, ADC1_CH0)  (Variable 0-3.3V)
   │      └── GND (bottom terminal)
   │
   └── GPIO1 (ADC1_CH1) ── [10kΩ] ── GND  (~0V reference)

Expected Voltages:
- GPIO3: ~1650mV (1500-1800mV acceptable)
- GPIO0: 0-3300mV (variable via potentiometer)
- GPIO1: ~0mV (0-300mV acceptable)
```text

## Test Suite Structure

The test suite includes 12 comprehensive test categories:

### 1. Hardware Validation Test
**Function**: `test_hardware_validation()`
- Validates physical hardware connections before other tests
- GPIO3: Expects ~1650mV (voltage divider validation)
- GPIO1: Expects ~0mV (ground connection validation)
- GPIO0: Variable voltage check (potentiometer functionality)
- Provides specific troubleshooting guidance for connection issues

### 2. ADC Initialization Test
**Function**: `test_adc_initialization()`
- Tests basic ADC unit initialization
- Verifies ESP32-C6 configuration (1 ADC unit, 7 channels)
- Validates channel availability checking
- Tests error handling for invalid channels

### 3. Channel Configuration Test
**Function**: `test_adc_channel_configuration()`
- Tests channel configuration with 12dB attenuation
- Validates channel enable/disable functionality
- Tests 12-bit width configuration
- Verifies configuration persistence

### 4. Basic Conversion Test
**Function**: `test_adc_basic_conversion()`
- Tests one-shot ADC readings (raw and voltage)
- Validates 12-bit ADC output range (0-4095)
- Tests both `EspAdc` and `BaseAdc` interface methods
- Verifies voltage readings within expected ranges

### 5. Calibration Test
**Function**: `test_adc_calibration()`
- Tests ADC calibration initialization
- Validates calibration availability for different attenuations
- Tests raw-to-voltage conversion using hardware calibration
- Handles ESP32-C6 specific calibration characteristics

### 6. Multiple Channels Test
**Function**: `test_adc_multiple_channels()`
- Tests simultaneous reading from all configured channels
- Hardware-specific validation per channel:
  - GPIO3: Expects 1500-1800mV (voltage divider validation)
  - GPIO1: Expects 0-300mV (ground reference validation)
  - GPIO0: Accepts 0-3300mV (potentiometer range validation)
- Tests bulk reading operations with real hardware verification

### 7. Averaging Test
**Function**: `test_adc_averaging()`
- Tests ADC averaging with different sample counts (1, 4, 8, 16)
- Validates noise reduction through averaging
- Tests timing between samples
- Verifies averaged values within expected bounds

### 8. Continuous Mode Test
**Function**: `test_adc_continuous_mode()`
- Tests continuous (DMA) mode operation with 1kHz sampling
- Validates callback-based data collection
- Tests ISR-safe callback implementation
- Measures continuous sampling performance over 2 seconds
- Verifies proper start/stop functionality

### 9. Monitor Threshold Test
**Function**: `test_adc_monitor_thresholds()`
- Interactive threshold testing using potentiometer on GPIO0
- Automatic threshold calculation based on baseline reading
- Real-time voltage monitoring with ESP-IDF buffer draining
- Interactive user guidance with step-by-step instructions
- Comprehensive validation:
  - Event counting (high/low threshold crossings)
  - ISR callback verification
  - Real-time voltage display updates

### 10. Error Handling Test
**Function**: `test_adc_error_handling()`
- Tests error handling for invalid operations
- Validates rejection of invalid channel numbers
- Tests null pointer handling
- Ensures disabled channel access rejection
- Validates configuration requirements

### 11. Statistics and Diagnostics Test
**Function**: `test_adc_statistics()`
- Tests ADC statistics collection
- Validates conversion timing measurements
- Tests diagnostic information gathering
- Tests statistics reset functionality
- Verifies performance metrics tracking

### 12. Performance Test
**Function**: `test_adc_performance()`
- Measures ADC conversion speed over 1000 conversions
- Tests high-frequency reading capabilities
- Validates timing characteristics (expects <1ms per conversion)
- Provides performance benchmarking data
- Measures execution time and calculates averages

## Building and Running Tests

### Prerequisites
1. ESP-IDF v5.5+ installed and configured
2. ESP32-C6 development board connected
3. Hardware test setup completed (potentiometer on GPIO0 required)
4. Voltage references connected to GPIO1 and GPIO3

### Build Commands

```bash
cd examples/esp32

## Build ADC test
./scripts/build_app.sh adc_test Release

## Flash and monitor
./scripts/flash_app.sh adc_test Release flash
./scripts/flash_app.sh monitor
```text

## Expected Test Results

### Successful Test Run

```text
I (270) ADC_Test: ╔══════════════════════════════════════════════════════════════════════════════╗
I (292) ADC_Test: ║                    ESP32-C6 ADC COMPREHENSIVE TEST SUITE                     ║
I (301) ADC_Test: ║                         HardFOC Internal Interface                           ║
I (310) ADC_Test: ║                                                                              ║
I (319) ADC_Test: ║  Hardware Setup Required (ESP32-C6 DevKit-M-1):                             ║
I (328) ADC_Test: ║  - GPIO3 (ADC1_CH3): Connect to 3.3V via voltage divider (high reference)  ║
I (337) ADC_Test: ║  - GPIO0 (ADC1_CH0): Connect to potentiometer center tap (variable 0-3.3V) ║
I (346) ADC_Test: ║  - GPIO1 (ADC1_CH1): Connect to ground via 10kΩ resistor (low reference)   ║
I (356) ADC_Test: ║                                                                              ║
I (365) ADC_Test: ║  Monitor Test: Adjust potentiometer on GPIO0 during monitor test            ║
I (374) ADC_Test: ╚══════════════════════════════════════════════════════════════════════════════╝

I (2664) ADC_Test: GPIO3 (HIGH): 1627 mV
I (2667) ADC_Test: GPIO3: Hardware connection verified
I (2672) ADC_Test: GPIO1 (LOW): 0 mV
I (2676) ADC_Test: GPIO1: Hardware connection verified
I (2680) ADC_Test: GPIO0 (POT): 1965 mV
I (2684) ADC_Test: GPIO0: Potentiometer reading valid
I (2689) ADC_Test: [SUCCESS] Hardware validation passed - all connections verified

...

I (12238) ADC_Test: 📈  0/10 sec | Voltage: 1328 mV (1.328V) | High events:  0 | Target: >1873 mV
I (14238) ADC_Test: 📈  2/10 sec | Voltage: 2050 mV (2.050V) | High events: 947 | Target: >1873 mV
I (14238) ADC_Test: 🎉 HIGH THRESHOLD TRIGGERED! Event #948 detected

...

I (33568) ADC_Test: Total: 12, Passed: 12, Failed: 0, Success: 100.00%, Time: 28902.67 ms
I (33573) ADC_Test: [SUCCESS] ALL ADC TESTS PASSED!
```text

### Monitor Threshold Test Details

The monitor test provides interactive guidance:

1. **Baseline Setup**: 5-second stabilization period using one-shot mode
2. **Threshold Calculation**: Automatic high/low threshold setting based on baseline
3. **High Threshold Test**: 10-second period to trigger high threshold
4. **Low Threshold Test**: 10-second period to trigger low threshold
5. **Real-Time Display**: Continuous voltage updates using ESP-IDF buffer draining

## Troubleshooting

### Hardware Connection Issues

#### GPIO3 Reading Incorrect
- **Expected**: ~1650mV (1500-1800mV range)
- **Problem**: Voltage divider not connected properly
- **Solution**: Verify 3.3V → 10kΩ → GPIO3 → 10kΩ → GND connection

#### GPIO1 Reading Too High
- **Expected**: ~0mV (0-300mV range)
- **Problem**: Not properly connected to ground
- **Solution**: Verify GPIO1 → 10kΩ → GND connection

#### GPIO0 Potentiometer Issues
- **Expected**: Variable 0-3300mV when turning potentiometer
- **Problem**: No variation or fixed reading
- **Solution**: Check 3-terminal connection: 3.3V → top, GND → bottom, GPIO0 → center

### Test Failure Analysis

#### Monitor Test No Events
- **Problem**: Potentiometer not adjusted during test
- **Solution**: Actively turn potentiometer during 10-second monitoring periods
- **Expected**: Threshold events when crossing calculated thresholds

#### Performance Issues
- **Problem**: Slow conversions (>1ms per conversion)
- **Solution**: Check ESP-IDF configuration and hardware connections
- **Expected**: ~50µs per one-shot conversion

#### Calibration Failures
- **Problem**: Calibration not available
- **Note**: Some ESP32-C6 units may not have calibration data - this is normal
- **Solution**: Linear conversion is used as fallback

## Test Configuration

### ADC Configuration Used
- **Attenuation**: 12dB (full 3.3V range)
- **Resolution**: 12-bit (0-4095 counts)
- **Sampling Frequency**: 1kHz (continuous mode), 2kHz (monitor mode)
- **Buffer Configuration**: 64 samples per frame, 4 frames maximum

### Expected Performance
- **Conversion Speed**: <1ms per one-shot conversion
- **Continuous Mode**: 1000+ samples/second sustained
- **Accuracy**: ±50mV typical with calibration
- **Monitor Response**: Real-time threshold detection

## Architecture Notes

The test suite validates:
1. **Hardware Abstraction**: Tests both `EspAdc` and `BaseAdc` interfaces
2. **ESP32-C6 Features**: Single ADC unit, 7-channel configuration
3. **ESP-IDF v5.5+ APIs**: Latest ADC calibration and monitor APIs
4. **Real-World Usage**: Interactive hardware testing with potentiometer
5. **Production Readiness**: Complete error handling and diagnostics

## Related Documentation

- **[EspAdc API Reference](../../../docs/esp_api/EspAdc.md)** - Complete API documentation
- **[BaseAdc API Reference](../../../docs/api/BaseAdc.md)** - Base class interface
- **[ESP32 API Overview](../../../docs/esp_api/README.md)** - ESP32 implementation overview
- **[HardwareTypes Reference](../../../docs/api/HardwareTypes.md)** - Type definitions