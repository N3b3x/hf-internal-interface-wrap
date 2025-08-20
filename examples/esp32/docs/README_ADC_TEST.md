# ESP32-C6 ADC Comprehensive Testing Suite

## Overview

This document describes the comprehensive ADC testing suite for the ESP32-C6 development kit. The test suite thoroughly exercises the EspAdc implementation, providing validation of all major ADC functionalities including hardware validation, initialization, channel configuration, calibration, one-shot and continuous modes, monitor threshold testing with interactive potentiometer control, error handling, statistics, and performance characteristics.

The test suite has been designed to provide complete verification of the ESP32-C6 ADC system with proper hardware-in-the-loop testing, including automatic hardware connection validation and interactive monitor threshold testing.

## Hardware Requirements

### Target Hardware
- **ESP32-C6 DevKit-M-1** (primary target)
- ESP32-C6 DevKitC-1 (also supported)  
- USB-C cable for programming and power
- Computer with ESP-IDF v5.5+ installed

### Required Components
- **10kΩ potentiometer** (for monitor testing on GPIO0)
- **Resistors**: 10kΩ + 1kΩ for voltage divider, 10kΩ for ground reference
- **Jumper wires** for connections
- **Breadboard** (recommended for clean connections)

### Test Hardware Setup

The ADC test suite exercises 3 ADC channels with precise voltage references for comprehensive testing:

#### Channel Configuration  
- **GPIO3 (ADC1_CH3)**: High reference (~3.0V via voltage divider)
- **GPIO0 (ADC1_CH0)**: Variable voltage (potentiometer center tap, 0-3.3V)
- **GPIO1 (ADC1_CH1)**: Low reference (~0V via pull-down resistor)

#### Detailed Circuit Configuration

```
ESP32-C6 DevKit-M-1 Test Setup:

3.3V Power Rail:
   |
   ├── [10kΩ] ──┬── [1kΩ] ── GND     (Voltage Divider for GPIO3)
   │           │
   │           └── GPIO3 (ADC1_CH3)  (~3.0V reference)
   │
   ├── [10kΩ Potentiometer]
   │      ├── 3.3V (top terminal)
   │      ├── GPIO0 (center wiper, ADC1_CH0)  (Variable 0-3.3V)
   │      └── GND (bottom terminal)
   │
   └── GPIO1 (ADC1_CH1) ── [10kΩ] ── GND  (~0V reference)

Voltage Divider Calculation for GPIO3:
- R1 = 10kΩ (to 3.3V)
- R2 = 1kΩ (to GND)  
- VOUT = 3.3V × (1kΩ/(10kΩ+1kΩ)) = 3.3V × (1/11) ≈ 3.0V

Expected Voltages:
- GPIO3: ~3000mV (2800-3300mV range acceptable)
- GPIO0: 0-3300mV (variable via potentiometer)
- GPIO1: ~0mV (0-300mV range acceptable)
- R1 = 10kΩ (to 3.3V)
- R2 = 1kΩ (to GND)  
- VOUT = 3.3V × (1kΩ/(10kΩ+1kΩ)) = 3.3V × (1/11) ≈ 3.0V

Expected Voltages:
- GPIO3: ~3000mV (2800-3300mV range acceptable)
- GPIO0: 0-3300mV (variable via potentiometer)
- GPIO1: ~0mV (0-300mV range acceptable)
```

#### Alternative Simple Setup
If precision components aren't available:
- **GPIO3**: Connect to 3.3V via 10kΩ resistor (will read close to 3.3V)
- **GPIO0**: Connect potentiometer between 3.3V and GND with center tap to GPIO0
- **GPIO1**: Connect to GND via 10kΩ resistor

## Test Suite Description

The comprehensive ADC test suite includes **12 major test categories** that run automatically in sequence:

### 1. Hardware Validation Test ✨ **NEW**
**Function**: `test_hardware_validation()`
**Purpose**: Validates physical hardware connections before running other tests
- **GPIO3 Validation**: Expects ~3000mV (2800-3300mV acceptable range)
- **GPIO1 Validation**: Expects ~0mV (0-300mV acceptable range)  
- **GPIO0 Validation**: Variable voltage check (potentiometer functionality)
- **Connection Verification**: Clear pass/fail feedback with specific troubleshooting guidance
- **Early Failure Detection**: Prevents wasted time on other tests if hardware setup is incorrect

### 2. ADC Initialization Test
**Function**: `test_adc_initialization()`
- Tests basic ADC unit initialization
- Verifies ESP32-C6 specific configuration (1 ADC unit, 7 channels)
- Validates channel availability checking
- Ensures proper error handling for invalid channels

### 3. Channel Configuration Test
**Function**: `test_adc_channel_configuration()`
- Tests ADC channel configuration with different settings
- Validates 12dB attenuation configuration for full 3.3V range
- Tests channel enable/disable functionality
- Verifies 12-bit width configuration

### 4. Basic Conversion Test
**Function**: `test_adc_basic_conversion()`
- Tests one-shot ADC readings (raw and voltage)
- Validates 12-bit ADC output range (0-4095)
- Tests both EspAdc and BaseAdc interface methods
- Verifies voltage readings are within expected ranges

### 5. Calibration Test  
**Function**: `test_adc_calibration()`
- Tests ADC calibration initialization for ESP32-C6
- Validates calibration availability for 12dB attenuation
- Tests raw-to-voltage conversion using hardware calibration
- Handles ESP32-C6 specific calibration characteristics

### 6. Multiple Channels Test ✨ **ENHANCED**  
**Function**: `test_adc_multiple_channels()`
- Tests simultaneous reading from all 3 configured channels
- **Hardware-Specific Validation**: Validates expected voltage ranges per channel
  - GPIO3: Expects 2800-3300mV (high reference validation)
  - GPIO1: Expects 0-300mV (low reference validation)
  - GPIO0: Accepts 0-3300mV (potentiometer range validation)
- Ensures proper channel isolation and accuracy
- Tests bulk reading operations with real hardware verification

### 7. Averaging Test
**Function**: `test_adc_averaging()`
- Tests ADC averaging functionality with different sample counts (1, 4, 8, 16 samples)
- Validates noise reduction through averaging
- Tests timing between samples
- Verifies averaged values are within expected bounds

### 8. Continuous Mode Test
**Function**: `test_adc_continuous_mode()`
- Tests ADC continuous (DMA) mode operation with 1kHz sampling
- Validates callback-based data collection (64 samples per frame)
- Tests ISR-safe callback implementation
- Measures continuous sampling performance over 2 seconds
- Verifies proper start/stop functionality

### 9. Monitor Threshold Test ✨ **COMPREHENSIVE INTERACTIVE TESTING**
**Function**: `test_adc_monitor_thresholds()`
**Hardware**: Uses potentiometer on GPIO0 for real-world threshold testing
- **Intelligent Threshold Setting**: Automatically calculates high/low thresholds based on current potentiometer position
- **Interactive User Guidance**: 
  - Displays current voltage reading
  - Shows calculated threshold values
  - Provides clear instructions for potentiometer adjustment
- **Real-Time Monitoring**: 15-second test duration with 2-second status updates
- **ISR-Safe Callback Testing**: Validates monitor callback system with actual hardware events
- **Comprehensive Validation**:
  - Event counting (high/low threshold crossings)
  - Timestamp recording for callback verification
  - Threshold separation validation (minimum 800mV)
  - Automatic threshold bounds checking

### 10. Error Handling Test
**Function**: `test_adc_error_handling()`
- Tests proper error handling for invalid operations
- Validates rejection of invalid channel numbers (channel 99)
- Tests null pointer handling in multi-channel reads
- Ensures disabled channel access is properly rejected
- Validates configuration-before-enable requirements

### 11. Statistics and Diagnostics Test
**Function**: `test_adc_statistics()`
- Tests ADC statistics collection and reporting
- Validates conversion timing measurements (min/max/average)
- Tests diagnostic information gathering (health status, enabled channels)
- Tests statistics reset functionality  
- Verifies performance metrics tracking over multiple conversions

### 12. Performance Test
**Function**: `test_adc_performance()`
- Measures ADC conversion speed over 1000 conversions
- Tests high-frequency reading capabilities
- Validates timing characteristics (expects <1ms per conversion)
- Provides performance benchmarking data (conversions per second)
- Measures total execution time and calculates averages

## Building and Running Tests

### Prerequisites
1. ESP-IDF v5.5+ installed and configured
2. ESP32-C6 development board connected  
3. Hardware test setup completed as described above (especially potentiometer on GPIO0)
4. Proper voltage references connected to GPIO1 and GPIO3

### Build Commands
```bash
cd examples/esp32
./scripts/build_example.sh adc_test

# Build with specific configuration
./scripts/build_example.sh adc_test Debug
```

### Alternative Build Methods
```bash
# Using flash script with auto-build capability
./scripts/flash_example.sh adc_test Release flash_monitor

# Or just build without flashing
./scripts/build_example.sh adc_test Release
```

## Expected Test Output

### Successful Test Run
```
I (1234) ADC_Test: ╔══════════════════════════════════════════════════════════════════════════════╗
I (1235) ADC_Test: ║                    ESP32-C6 ADC COMPREHENSIVE TEST SUITE                    ║
I (1236) ADC_Test: ║                         HardFOC Internal Interface                          ║
I (1237) ADC_Test: ║                                                                              ║
I (1238) ADC_Test: ║  Hardware Setup Required (ESP32-C6 DevKit-M-1):                             ║
I (1239) ADC_Test: ║  - GPIO3 (ADC1_CH3): Connect to 3.3V via voltage divider (high reference)  ║
I (1240) ADC_Test: ║  - GPIO0 (ADC1_CH0): Connect to potentiometer center tap (variable 0-3.3V) ║
I (1241) ADC_Test: ║  - GPIO1 (ADC1_CH1): Connect to ground via 10kΩ resistor (low reference)   ║
I (1242) ADC_Test: ║                                                                              ║
I (1243) ADC_Test: ║  Monitor Test: Adjust potentiometer on GPIO0 during monitor test            ║
I (1244) ADC_Test: ╚══════════════════════════════════════════════════════════════════════════════╝

I (3244) ADC_Test: [TEST 1/12] Running test_hardware_validation...
I (3245) ADC_Test: Validating hardware setup...
I (3246) ADC_Test: Expected connections:
I (3247) ADC_Test:   - GPIO3: 3.3V via voltage divider (should read ~3.0V)
I (3248) ADC_Test:   - GPIO0: Potentiometer center tap (variable 0-3.3V)
I (3249) ADC_Test:   - GPIO1: Ground via 10kΩ resistor (should read ~0V)
I (3254) ADC_Test: GPIO3 (HIGH): 3012 mV
I (3255) ADC_Test: GPIO3: Hardware connection verified
I (3259) ADC_Test: GPIO1 (LOW): 45 mV  
I (3260) ADC_Test: GPIO1: Hardware connection verified
I (3264) ADC_Test: GPIO0 (POT): 1567 mV
I (3265) ADC_Test: GPIO0: Potentiometer reading valid
I (3266) ADC_Test: [SUCCESS] Hardware validation passed - all connections verified

I (3267) ADC_Test: [TEST 2/12] Running test_adc_initialization...
...

I (3845) ADC_Test: [TEST 9/12] Running test_adc_monitor_thresholds...
I (3846) ADC_Test: Testing ADC monitor thresholds...
I (3847) ADC_Test: Current potentiometer voltage: 1567 mV
I (3848) ADC_Test: Setting thresholds:
I (3849) ADC_Test:   - High: 2167 mV (2686 counts)
I (3850) ADC_Test:   - Low:  967 mV (1199 counts)
I (3851) ADC_Test: Monitor system active! Please adjust potentiometer now:
I (3852) ADC_Test:    Current reading: 1567 mV
I (3853) ADC_Test:    Turn potentiometer HIGH (above 2167 mV) to trigger high threshold
I (3854) ADC_Test:    Turn potentiometer LOW (below 967 mV) to trigger low threshold
I (3855) ADC_Test:    Test duration: 15 seconds with real-time feedback

I (5855) ADC_Test: 2/15 sec | Voltage: 1234 mV | High events: 0 | Low events: 1
I (7855) ADC_Test: 4/15 sec | Voltage: 2456 mV | High events: 1 | Low events: 1  
I (9855) ADC_Test: 6/15 sec | Voltage: 1789 mV | High events: 1 | Low events: 1
...
I (18855) ADC_Test: Monitor test completed:
I (18856) ADC_Test:   - High threshold events: 2
I (18857) ADC_Test:   - Low threshold events:  1  
I (18858) ADC_Test:   - Total events:          3
I (18859) ADC_Test: [SUCCESS] ADC monitor threshold test completed

...

I (21234) ADC_Test: ╔══════════════════════════════════════════════════════════════════════════════╗
I (21235) ADC_Test: ║                      ALL ADC TESTS PASSED!                                  ║
I (21236) ADC_Test: ║                                                                              ║  
I (21237) ADC_Test: ║  ESP32-C6 ADC system is working correctly with comprehensive testing        ║
I (21238) ADC_Test: ║  covering hardware validation, initialization, calibration, single/multi-   ║
I (21239) ADC_Test: ║  channel reading, continuous mode, monitor thresholds with bounds,         ║
I (21240) ADC_Test: ║  error handling, statistics, and performance testing.                      ║
I (21241) ADC_Test: ║                                                                              ║
I (21242) ADC_Test: ║  Hardware connections verified:                                             ║
I (21243) ADC_Test: ║  GPIO3 (HIGH)   GPIO0 (POT)   GPIO1 (LOW)   Monitor System                ║
I (21244) ADC_Test: ╚══════════════════════════════════════════════════════════════════════════════╝
```

### Hardware Setup Issues  
If hardware connections are incorrect, you'll see specific error messages:

```
I (3254) ADC_Test: GPIO3 (HIGH): 156 mV
E (3255) ADC_Test: GPIO3: Expected ~3000mV, got 156 mV - check voltage divider!
I (3259) ADC_Test: GPIO1 (LOW): 3287 mV
E (3260) ADC_Test: GPIO1: Expected ~0mV, got 3287 mV - check ground connection!
E (3266) ADC_Test: [FAILED] Hardware validation failed - check connections before proceeding
```

## Troubleshooting

### Common Hardware Issues

#### GPIO3 Reading Too Low (<2800mV)
- **Problem**: Voltage divider not connected or wrong resistor values
- **Solution**: Verify 10kΩ + 1kΩ voltage divider: 3.3V → 10kΩ → GPIO3 → 1kΩ → GND
- **Expected**: ~3000mV (2800-3300mV acceptable)

#### GPIO1 Reading Too High (>300mV)  
- **Problem**: Not properly connected to ground
- **Solution**: Verify GPIO1 → 10kΩ → GND connection
- **Expected**: ~0mV (0-300mV acceptable)

#### GPIO0 Potentiometer Issues
- **Problem**: No variation when turning potentiometer
- **Solution**: Check 3-terminal connection: 3.3V → top, GND → bottom, GPIO0 → center wiper
- **Expected**: Variable 0-3300mV when turning potentiometer

#### Monitor Test No Events
- **Problem**: Potentiometer not being adjusted during test
- **Solution**: Actively turn potentiometer during the 15-second monitor test period
- **Expected**: High/low threshold events when crossing calculated thresholds

### Test Failure Analysis

#### Performance Issues
- **Slow Conversions**: If >1ms per conversion, check ESP-IDF configuration and hardware
- **Calibration Failures**: ESP32-C6 may not have calibration data - this is normal for some units

#### Monitor Test Troubleshooting  
- **No Threshold Events**: Ensure potentiometer is actively adjusted during test
- **Threshold Separation Warnings**: Potentiometer at extreme position - move to mid-range
- **ISR Callback Issues**: Hardware/software timing issues - check ESP-IDF version compatibility

## Test Configuration Details

### ADC Configuration Used
- **Attenuation**: 12dB (full 3.3V range)
- **Bit Width**: 12-bit (0-4095 counts)
- **Sampling Frequency**: 1kHz (continuous mode)
- **Buffer Size**: 64 samples per frame, 4 frames max

### Expected Performance Characteristics
- **Conversion Speed**: <1ms per one-shot conversion
- **Continuous Mode**: 1000 samples/second sustained
- **Accuracy**: ±50mV typical with calibration  
- **Monitor Response**: <10ms threshold detection latency

## Architecture Notes

This comprehensive test suite validates:
1. **Hardware Abstraction**: Tests both EspAdc and BaseAdc interfaces  
2. **ESP32-C6 Specific Features**: Single ADC unit, 7-channel configuration
3. **ESP-IDF v5.5+ APIs**: Latest ADC calibration and monitor APIs
4. **Real-World Usage**: Interactive hardware testing with potentiometer control
5. **Production Readiness**: Complete error handling and diagnostics validation

---

**Note**: This test suite provides comprehensive validation of the ESP32-C6 ADC system using the HardFOC EspAdc implementation. It ensures proper functionality across all major ADC operations while maintaining the noexcept architecture requirements and providing real-world hardware validation capabilities.
- Follow the noexcept architecture strictly
- Add comprehensive test coverage for new features
- Update documentation for any changes
- Ensure backward compatibility

---

**Note**: This test suite provides comprehensive validation of the ESP32-C6 ADC system using the HardFOC EspAdc implementation. It ensures proper functionality across all major ADC operations while maintaining the noexcept architecture requirements.