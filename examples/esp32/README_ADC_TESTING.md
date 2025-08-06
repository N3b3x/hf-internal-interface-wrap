# ESP32-C6 ADC Comprehensive Testing Suite

## Overview

This document describes the comprehensive ADC testing suite for the ESP32-C6 development kit. The test suite thoroughly exercises the EspAdc implementation, providing validation of all major ADC functionalities including initialization, channel configuration, calibration, one-shot and continuous modes, error handling, and performance characteristics.

## Hardware Requirements

### Target Hardware
- **ESP32-C6 DevKit-M-1** (primary target)
- ESP32-C6 DevKitC-1 (also supported)
- USB-C cable for programming and power
- Computer with ESP-IDF v5.5+ installed

### Test Hardware Setup

The ADC test suite exercises 3 ADC channels with the following recommended setup:

#### Channel Configuration
- **GPIO0 (ADC1_CH0)**: Test channel 1 - Connect to variable voltage source (0-3.3V)
- **GPIO1 (ADC1_CH1)**: Test channel 2 - Connect to 3.3V via voltage divider (e.g., 1.65V)
- **GPIO2 (ADC1_CH2)**: Test channel 3 - Connect to ground via 10kΩ resistor (~0V)

#### Recommended Test Circuit
```
3.3V ----[10kΩ]----+----[10kΩ]---- GND
                   |
                 GPIO1 (should read ~1.65V)

3.3V ----[Variable]---- GPIO0 (test with different voltages)

GND -----[10kΩ]----- GPIO2 (should read ~0V)
```

#### Alternative Simple Setup
If you don't have external components:
- **GPIO0**: Leave floating or connect to 3.3V
- **GPIO1**: Connect to 3.3V
- **GPIO2**: Connect to GND

## Test Suite Description

The comprehensive ADC test suite includes 10 major test categories:

### 1. ADC Initialization Test
**Function**: `test_adc_initialization()`
- Tests basic ADC unit initialization
- Verifies ESP32-C6 specific configuration (1 ADC unit, 7 channels)
- Validates channel availability checking
- Ensures proper error handling for invalid channels

### 2. Channel Configuration Test
**Function**: `test_adc_channel_configuration()`
- Tests ADC channel configuration with different settings
- Validates attenuation and bit-width configuration
- Tests channel enable/disable functionality
- Verifies configuration persistence

### 3. Basic Conversion Test
**Function**: `test_adc_basic_conversion()`
- Tests one-shot ADC readings (raw and voltage)
- Validates 12-bit ADC output range (0-4095)
- Tests both EspAdc and BaseAdc interface methods
- Verifies voltage readings are within expected ranges

### 4. Calibration Test
**Function**: `test_adc_calibration()`
- Tests ADC calibration initialization
- Validates calibration availability for different attenuation levels
- Tests raw-to-voltage conversion using calibration
- Handles cases where calibration may not be available

### 5. Multiple Channels Test
**Function**: `test_adc_multiple_channels()`
- Tests simultaneous reading from multiple ADC channels
- Validates multi-channel reading interface
- Ensures proper channel isolation
- Tests bulk reading operations

### 6. Averaging Test
**Function**: `test_adc_averaging()`
- Tests ADC averaging functionality with different sample counts
- Validates noise reduction through averaging
- Tests timing between samples
- Verifies averaged values are within expected bounds

### 7. Continuous Mode Test
**Function**: `test_adc_continuous_mode()`
- Tests ADC continuous (DMA) mode operation
- Validates callback-based data collection
- Tests ISR-safe callback implementation
- Measures continuous sampling performance
- Verifies proper start/stop functionality

### 8. Error Handling Test
**Function**: `test_adc_error_handling()`
- Tests proper error handling for invalid operations
- Validates rejection of invalid channel numbers
- Tests null pointer handling
- Ensures disabled channel access is properly rejected

### 9. Statistics and Diagnostics Test
**Function**: `test_adc_statistics()`
- Tests ADC statistics collection and reporting
- Validates diagnostic information gathering
- Tests statistics reset functionality
- Verifies performance metrics tracking

### 10. Performance Test
**Function**: `test_adc_performance()`
- Measures ADC conversion speed and performance
- Tests high-frequency reading capabilities
- Validates timing characteristics
- Provides performance benchmarking data

## Building and Running Tests

### Prerequisites
1. ESP-IDF v5.5+ installed and configured
2. ESP32-C6 development board connected
3. Hardware test setup as described above

### Build Commands
```bash
cd examples/esp32

# Build ADC test specifically
idf.py -DEXAMPLE_TYPE=adc_test build

# Flash and monitor
idf.py -DEXAMPLE_TYPE=adc_test flash monitor
```

### Alternative Build Methods
```bash
# Using build scripts
./build_example.sh adc_test

# Or PowerShell (Windows)
./build_example.ps1 adc_test
```

## Expected Test Output

### Successful Test Run
```
I (1234) ADC_Test: ╔══════════════════════════════════════════════════════════════════════════════╗
I (1235) ADC_Test: ║                    ESP32-C6 ADC COMPREHENSIVE TEST SUITE                    ║
I (1236) ADC_Test: ║                         HardFOC Internal Interface                          ║
I (1237) ADC_Test: ║                                                                              ║
I (1238) ADC_Test: ║  Hardware Setup Required:                                                    ║
I (1239) ADC_Test: ║  - GPIO0 (ADC1_CH0): Test input channel 1                                   ║
I (1240) ADC_Test: ║  - GPIO1 (ADC1_CH1): Test input channel 2                                   ║
I (1241) ADC_Test: ║  - GPIO2 (ADC1_CH2): Test input channel 3                                   ║
I (1242) ADC_Test: ║                                                                              ║
I (1243) ADC_Test: ║  Connect test voltages (0-3.3V) to GPIO pins for accurate testing          ║
I (1244) ADC_Test: ╚══════════════════════════════════════════════════════════════════════════════╝

... [Individual test results] ...

I (5678) ADC_Test: === ADC TEST SUMMARY ===
I (5679) ADC_Test: Total: 10, Passed: 10, Failed: 0, Success: 100.00%, Time: 1234.56 ms
I (5680) ADC_Test: [SUCCESS] ALL ADC TESTS PASSED!

I (5681) ADC_Test: ╔══════════════════════════════════════════════════════════════════════════════╗
I (5682) ADC_Test: ║                      🎉 ALL ADC TESTS PASSED! 🎉                           ║
I (5683) ADC_Test: ║                                                                              ║
I (5684) ADC_Test: ║  ESP32-C6 ADC system is working correctly with comprehensive testing        ║
I (5685) ADC_Test: ║  covering initialization, calibration, single/multi-channel reading,       ║
I (5686) ADC_Test: ║  continuous mode, error handling, statistics, and performance.              ║
I (5687) ADC_Test: ╚══════════════════════════════════════════════════════════════════════════════╝
```

### Typical Voltage Readings
With the recommended test setup:
- **GPIO0**: Variable (depends on your test input)
- **GPIO1**: ~1650mV (1.65V from voltage divider)
- **GPIO2**: ~0-100mV (near ground through resistor)

## Troubleshooting

### Common Issues

#### 1. Voltage Readings Out of Range
**Symptoms**: Voltage validation warnings
**Solutions**:
- Check hardware connections
- Verify 3.3V supply voltage
- Ensure proper grounding
- Check for floating inputs

#### 2. Continuous Mode Test Fails
**Symptoms**: No continuous mode data received
**Solutions**:
- Check if continuous mode is supported on your ESP-IDF version
- Verify callback registration
- Check for timing issues

#### 3. Calibration Not Available
**Symptoms**: Calibration warnings but tests still pass
**Solutions**:
- This is normal on some ESP32-C6 boards
- eFuse calibration data may not be available
- Tests should still pass with fallback linear conversion

#### 4. Performance Warnings
**Symptoms**: Slow ADC conversion warnings
**Solutions**:
- Check system load
- Verify ESP-IDF configuration
- Consider hardware-specific limitations

### Debug Tips

1. **Enable Debug Logging**:
   ```bash
   idf.py menuconfig
   # Component config → Log output → Default log verbosity → Debug
   ```

2. **Monitor Specific Components**:
   ```bash
   # Add to your build: -DLOG_LOCAL_LEVEL=ESP_LOG_DEBUG
   ```

3. **Hardware Verification**:
   - Use multimeter to verify test voltages
   - Check GPIO pin configurations
   - Verify power supply stability

## Test Configuration

### Modifiable Parameters
The test suite includes several configurable parameters in `AdcComprehensiveTest.cpp`:

```cpp
// Test channels (modify for different GPIO pins)
static constexpr hf_channel_id_t TEST_CHANNEL_1 = 0; // GPIO0
static constexpr hf_channel_id_t TEST_CHANNEL_2 = 1; // GPIO1
static constexpr hf_channel_id_t TEST_CHANNEL_3 = 2; // GPIO2

// Voltage validation ranges
static constexpr uint32_t VOLTAGE_TOLERANCE_MV = 200;  // ±200mV tolerance
static constexpr uint32_t MIN_VALID_VOLTAGE_MV = 100;  // 0.1V minimum
static constexpr uint32_t MAX_VALID_VOLTAGE_MV = 3200; // 3.2V maximum

// Continuous mode parameters
static constexpr uint32_t CONTINUOUS_TEST_DURATION_MS = 2000;    // 2 seconds
static constexpr uint32_t CONTINUOUS_SAMPLES_PER_FRAME = 64;     // 64 samples/frame
static constexpr uint32_t CONTINUOUS_MAX_STORE_FRAMES = 4;       // 4 frames max
```

### ADC Configuration
The test uses the following ADC settings:
- **Attenuation**: 12dB (full 0-3.3V range)
- **Bit Width**: 12-bit (0-4095 counts)
- **Sampling Frequency**: 1kHz (continuous mode)
- **Calibration**: Enabled when available

## Integration with CI/CD

### GitHub Actions
The test suite integrates with the existing GitHub Actions CI pipeline:

```yaml
# In .github/workflows/esp32.yml
example_type: [adc_test, ...]
```

### Automated Testing
- Build validation for all ESP-IDF versions
- Hardware-in-the-loop testing (when available)
- Performance regression detection

## Advanced Usage

### Custom Test Development
To add new ADC tests:

1. **Create Test Function**:
   ```cpp
   bool test_my_custom_adc_feature() noexcept {
       // Your test implementation
       return true; // or false on failure
   }
   ```

2. **Add to Test Suite**:
   ```cpp
   // In app_main()
   RUN_TEST(test_my_custom_adc_feature);
   ```

3. **Follow Test Patterns**:
   - Use consistent error handling
   - Provide detailed logging
   - Validate all return values
   - Clean up resources properly

### Hardware Variations
The test suite can be adapted for different hardware setups:
- Different GPIO pin assignments
- Various voltage reference levels  
- Alternative test signal sources
- Custom calibration procedures

## Support and Contribution

### Getting Help
- Check ESP-IDF documentation for ADC specifics
- Review ESP32-C6 datasheet for hardware limitations
- Consult HardFOC documentation for library specifics

### Contributing
- Follow the noexcept architecture strictly
- Add comprehensive test coverage for new features
- Update documentation for any changes
- Ensure backward compatibility

---

**Note**: This test suite provides comprehensive validation of the ESP32-C6 ADC system using the HardFOC EspAdc implementation. It ensures proper functionality across all major ADC operations while maintaining the noexcept architecture requirements.