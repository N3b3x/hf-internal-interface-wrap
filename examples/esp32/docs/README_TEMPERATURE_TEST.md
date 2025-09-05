# ESP32-C6 Temperature Sensor Comprehensive Test Suite

## Overview

The Temperature Sensor Comprehensive Test Suite provides extensive validation of the `EspTemperature` class for ESP32-C6 platforms using ESP-IDF v5.5+. This test suite demonstrates complete temperature sensing functionality, threshold monitoring, continuous monitoring, calibration, power management, and health diagnostics with a focus on embedded environments using `noexcept` functions.

**✅ Status: Successfully tested on ESP32-C6-DevKitM-1 hardware**

## Features Tested

### Core Temperature Functionality
- **Basic Temperature Reading**: Single-shot and continuous temperature measurements
- **Sensor Initialization**: Proper sensor startup and configuration
- **Temperature Calibration**: Offset and gain calibration with validation
- **Reading Validation**: Data integrity and range checking

### Advanced Monitoring Features
- **Threshold Monitoring**: High/low temperature threshold detection with callbacks
- **Continuous Monitoring**: Real-time temperature monitoring with configurable intervals
- **Range Management**: Dynamic temperature range configuration and validation
- **Statistics Collection**: Temperature history, min/max tracking, and trend analysis

### System Integration
- **Power Management**: Low-power modes and wake-up functionality
- **Health Monitoring**: Sensor health checks and diagnostic information
- **Error Handling**: Comprehensive error condition testing and recovery
- **Performance Testing**: Reading speed, accuracy, and resource usage optimization

### ESP32-C6 Specific Features
- **Built-in Temperature Sensor**: Internal temperature sensor validation
- **High Precision**: Enhanced accuracy with calibration
- **Low Power Operation**: Optimized for battery-powered applications
- **Interrupt Integration**: Hardware interrupt-driven monitoring

## Hardware Requirements

### Supported Platforms
- **Primary Target**: ESP32-C6-DevKitM-1
- **ESP-IDF Version**: v5.5 or later
- **Minimum Flash**: 4MB
- **Minimum RAM**: 256KB

### Temperature Sensor
- **Built-in Sensor**: ESP32-C6 internal temperature sensor
- **Range**: -40°C to +125°C (typical)
- **Resolution**: 0.1°C
- **Accuracy**: ±2°C (after calibration)

### Connections
- **USB**: For flashing and serial monitoring (built-in USB-JTAG)
- **No External Hardware Required**: Uses internal temperature sensor

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

# Build Temperature test
idf.py build -DEXAMPLE_TYPE=temperature_test -DBUILD_TYPE=Release

# Flash and monitor
idf.py -p /dev/ttyUSB0 flash monitor
```

### Alternative Build Methods

#### Using Build Scripts (Recommended)
```bash
# Source ESP-IDF environment
source /path/to/esp-idf/export.sh

# Build with optimization
./build_example.sh temperature_test Release

# Flash to device
idf.py -B build_temperature_test_Release flash monitor
```

#### Debug Build for Development
```bash
# Build with debug symbols and verbose output
idf.py build -DEXAMPLE_TYPE=temperature_test -DBUILD_TYPE=Debug

# Run with detailed logging
idf.py -p /dev/ttyUSB0 flash monitor
```

## Test Categories

### 1. Sensor Initialization Tests
```cpp
bool test_temperature_sensor_initialization() noexcept;
```
- **Validates**: Proper sensor startup and state management
- **Tests**: 
  - Initial state verification (UNINITIALIZED)
  - Successful initialization process
  - State transition validation (INITIALIZED)
  - Error handling for initialization failures
- **Expected Results**: Clean initialization with proper state transitions

### 2. Basic Temperature Reading Tests
```cpp
bool test_temperature_reading() noexcept;
```
- **Validates**: Core temperature measurement functionality
- **Tests**:
  - Single temperature reading
  - Reading validation and range checking
  - Temperature accuracy verification
  - Reading consistency checks
- **Expected Results**: Accurate temperature readings within expected range

### 3. Sensor Information Tests
```cpp
bool test_sensor_info() noexcept;
```
- **Validates**: Sensor metadata and capability reporting
- **Tests**:
  - Sensor description retrieval
  - Range information (min/max temperatures)
  - Resolution and accuracy specifications
  - Sensor identification and version
- **Expected Results**: Complete and accurate sensor information

### 4. Range Management Tests
```cpp
bool test_range_management() noexcept;
```
- **Validates**: Dynamic temperature range configuration
- **Tests**:
  - Range setting and validation
  - Out-of-range handling
  - Range boundary testing
  - Invalid range rejection
- **Expected Results**: Proper range validation and boundary enforcement

### 5. Threshold Monitoring Tests
```cpp
bool test_threshold_monitoring() noexcept;
```
- **Validates**: Temperature threshold detection and alerting
- **Tests**:
  - High threshold configuration and detection
  - Low threshold configuration and detection
  - Threshold callback functionality
  - Multiple threshold management
- **Expected Results**: Accurate threshold detection with proper callback execution

### 6. Continuous Monitoring Tests
```cpp
bool test_continuous_monitoring() noexcept;
```
- **Validates**: Real-time temperature monitoring capabilities
- **Tests**:
  - Continuous monitoring start/stop
  - Monitoring interval configuration
  - Data streaming and buffering
  - Monitoring callback functionality
- **Expected Results**: Reliable continuous monitoring with configurable intervals

### 7. Calibration Tests
```cpp
bool test_calibration() noexcept;
```
- **Validates**: Temperature sensor calibration functionality
- **Tests**:
  - Offset calibration (temperature correction)
  - Gain calibration (scaling correction)
  - Calibration validation and persistence
  - Factory reset capabilities
- **Expected Results**: Improved accuracy through proper calibration

### 8. Power Management Tests
```cpp
bool test_power_management() noexcept;
```
- **Validates**: Low-power operation modes
- **Tests**:
  - Low-power mode entry/exit
  - Wake-up functionality
  - Power consumption optimization
  - Sleep mode compatibility
- **Expected Results**: Efficient power management with maintained functionality

### 9. Self-Test and Health Monitoring
```cpp
bool test_self_test_and_health() noexcept;
```
- **Validates**: Sensor health and diagnostic capabilities
- **Tests**:
  - Self-test execution and validation
  - Health status reporting
  - Diagnostic information collection
  - Error detection and reporting
- **Expected Results**: Comprehensive health monitoring with accurate diagnostics

### 10. Statistics and Diagnostics
```cpp
bool test_statistics_and_diagnostics() noexcept;
```
- **Validates**: Performance metrics and data analysis
- **Tests**:
  - Temperature statistics (min/max/average)
  - Reading count and frequency tracking
  - Trend analysis and history
  - Performance diagnostics
- **Expected Results**: Accurate statistics collection and analysis

### 11. ESP32-Specific Features
```cpp
bool test_esp32_specific_features() noexcept;
```
- **Validates**: ESP32-C6 specific temperature sensor features
- **Tests**:
  - ESP32-specific callback functions
  - Hardware interrupt integration
  - Advanced configuration options
  - Platform-specific optimizations
- **Expected Results**: Full utilization of ESP32-C6 temperature sensor capabilities

### 12. Error Handling Tests
```cpp
bool test_error_handling() noexcept;
```
- **Validates**: Robust error condition handling
- **Tests**:
  - Invalid parameter handling
  - Sensor failure scenarios
  - Recovery mechanisms
  - Error reporting accuracy
- **Expected Results**: Graceful error handling without system crashes

### 13. Performance and Stress Tests
```cpp
bool test_performance_and_stress() noexcept;
```
- **Validates**: Performance characteristics under load
- **Tests**:
  - Reading speed optimization
  - High-frequency sampling
  - Memory usage efficiency
  - Stress testing with rapid operations
- **Expected Results**: Optimal performance within embedded system constraints

## Expected Test Results

### Successful Execution Output
```
╔══════════════════════════════════════════════════════════════════════════════╗
║                ESP32-C6 TEMPERATURE COMPREHENSIVE TEST SUITE                ║
║                         HardFOC Internal Interface                          ║
╚══════════════════════════════════════════════════════════════════════════════╝

╔══════════════════════════════════════════════════════════════════════════════╗
║ Running: test_temperature_sensor_initialization                            ║
╚══════════════════════════════════════════════════════════════════════════════╝
[SUCCESS] Temperature sensor initialization successful
[SUCCESS] PASSED: test_temperature_sensor_initialization (0.85 ms)

╔══════════════════════════════════════════════════════════════════════════════╗
║ Running: test_temperature_reading                                          ║
╚══════════════════════════════════════════════════════════════════════════════╝
Current temperature: 23.45°C
[SUCCESS] Temperature reading within valid range
[SUCCESS] PASSED: test_temperature_reading (1.23 ms)

... (additional tests) ...

=== TEMPERATURE TEST SUMMARY ===
Total: 13, Passed: 13, Failed: 0, Success: 100.00%, Time: 245.67 ms
[SUCCESS] ALL TEMPERATURE TESTS PASSED!
```

### Performance Metrics
Typical performance on ESP32-C6 @ 160MHz:
- **Sensor Initialization**: ~1ms
- **Single Temperature Reading**: ~500µs
- **Continuous Monitoring**: Configurable intervals (10ms - 10s)
- **Calibration Operation**: ~2ms
- **Threshold Detection**: <100µs response time

### Accuracy Specifications
- **Raw Accuracy**: ±3°C (uncalibrated)
- **Calibrated Accuracy**: ±1°C (with offset/gain calibration)
- **Resolution**: 0.1°C
- **Measurement Range**: -40°C to +125°C
- **Stability**: ±0.1°C over 24 hours

### Memory Usage
- **Static Memory**: ~500 bytes for sensor instance
- **Dynamic Memory**: Variable based on monitoring configuration
- **Flash Usage**: ~6KB for test code
- **Stack Usage**: ~128 bytes per reading operation

## Configuration Options

### Temperature Sensor Configuration
```cpp
// Basic configuration
EspTemperature sensor;
sensor.EnsureInitialized();

// Set measurement range
sensor.SetTemperatureRange(-20.0f, 80.0f);

// Configure calibration
sensor.SetCalibration(offset_celsius, gain_factor);

// Set up threshold monitoring
sensor.SetThresholds(low_threshold, high_threshold);
```

### Monitoring Configuration
```cpp
// Continuous monitoring setup
sensor.StartContinuousMonitoring(interval_ms);

// Register monitoring callback
sensor.RegisterMonitoringCallback(monitoring_callback, user_data);

// Register threshold callback
sensor.RegisterThresholdCallback(threshold_callback, user_data);
```

### Power Management
```cpp
// Low-power mode configuration
sensor.EnableLowPowerMode(true);

// Configure wake-up settings
sensor.SetWakeUpInterval(wake_interval_ms);
```

## Troubleshooting

### Common Issues

#### Build Failures
```bash
# Missing ESP-IDF environment
source $IDF_PATH/export.sh

# Wrong target platform
idf.py set-target esp32c6

# Dependency issues
idf.py clean
idf.py build
```

#### Runtime Issues
- **Initialization Failures**: Check sensor availability and ESP-IDF version
- **Inaccurate Readings**: Perform calibration with known reference temperatures
- **Callback Issues**: Verify callback function signatures and user data handling
- **Memory Issues**: Monitor heap usage during continuous monitoring

#### Calibration Issues
```cpp
// Manual calibration example
float reference_temp = 25.0f;  // Known reference temperature
float measured_temp = sensor.ReadTemperature();
float offset = reference_temp - measured_temp;
sensor.SetCalibrationOffset(offset);
```

### Debug Mode Configuration
Enable enhanced debugging:
```bash
# Build with debug configuration
idf.py build -DEXAMPLE_TYPE=temperature_test -DBUILD_TYPE=Debug

# Enable verbose sensor logging
idf.py menuconfig
# Component config → Temperature Sensor → Enable debug output
```

## Integration Examples

### Basic Temperature Monitoring
```cpp
#include "mcu/esp32/EspTemperature.h"

// Create temperature sensor instance
EspTemperature temp_sensor;

// Initialize sensor
if (temp_sensor.EnsureInitialized()) {
    // Read current temperature
    hf_temp_reading_t reading = {};
    auto result = temp_sensor.ReadTemperature(&reading);
    
    if (result == hf_temp_err_t::TEMP_SUCCESS) {
        ESP_LOGI("APP", "Temperature: %.2f°C", reading.temperature_celsius);
    }
}
```

### Advanced Monitoring with Callbacks
```cpp
// Threshold callback function
void temperature_alert(EspTemperature* sensor, float temperature, bool is_high) {
    if (is_high) {
        ESP_LOGW("TEMP", "High temperature alert: %.2f°C", temperature);
    } else {
        ESP_LOGW("TEMP", "Low temperature alert: %.2f°C", temperature);
    }
}

// Continuous monitoring callback
void temperature_monitor(EspTemperature* sensor, float temperature, hf_u64_t timestamp) {
    ESP_LOGI("TEMP", "Temperature: %.2f°C at %llu µs", temperature, timestamp);
}

// Setup advanced monitoring
temp_sensor.SetThresholds(15.0f, 35.0f);  // 15°C low, 35°C high
temp_sensor.RegisterThresholdCallback(temperature_alert);
temp_sensor.RegisterMonitoringCallback(temperature_monitor);
temp_sensor.StartContinuousMonitoring(1000);  // 1 second interval
```

### Calibration and Accuracy Improvement
```cpp
// Perform calibration with known reference
float known_temp = 25.0f;  // Reference temperature
float measured = temp_sensor.ReadTemperatureValue();
float offset = known_temp - measured;

// Apply calibration
temp_sensor.SetCalibrationOffset(offset);

// Verify improved accuracy
measured = temp_sensor.ReadTemperatureValue();
ESP_LOGI("TEMP", "Calibrated reading: %.2f°C (expected: %.2f°C)", 
         measured, known_temp);
```

## API Reference

### Core Functions
```cpp
class EspTemperature {
public:
    // Lifecycle management
    bool EnsureInitialized() noexcept;
    bool EnsureDeinitialized() noexcept;
    hf_temp_state_t GetState() const noexcept;
    
    // Basic temperature reading
    hf_temp_err_t ReadTemperature(hf_temp_reading_t* reading) noexcept;
    float ReadTemperatureValue() noexcept;
    
    // Configuration
    hf_temp_err_t SetTemperatureRange(float min_celsius, float max_celsius) noexcept;
    hf_temp_err_t SetCalibration(float offset, float gain) noexcept;
};
```

### Advanced Functions
```cpp
// Monitoring and callbacks
hf_temp_err_t SetThresholds(float low_celsius, float high_celsius) noexcept;
hf_temp_err_t StartContinuousMonitoring(hf_u32_t interval_ms) noexcept;
hf_temp_err_t StopContinuousMonitoring() noexcept;

// ESP32-specific callbacks
void RegisterThresholdCallback(EspTempThresholdCallback callback) noexcept;
void RegisterMonitoringCallback(EspTempMonitoringCallback callback) noexcept;

// Statistics and diagnostics
hf_temp_stats_t GetStatistics() const noexcept;
hf_temp_health_t GetHealthStatus() const noexcept;
```

## Embedded Development Best Practices

### Performance Optimization
- Use appropriate monitoring intervals for your application
- Enable low-power mode for battery applications
- Cache temperature readings when high-frequency access is needed
- Use callbacks instead of polling for event-driven applications

### Memory Management
- All functions are `noexcept` - no exception handling overhead
- Minimal dynamic allocation during operation
- Configurable monitoring buffers
- Stack usage optimization for interrupt contexts

### Real-time Considerations
- Temperature readings are non-blocking
- Callback execution in interrupt context
- Predictable response times for threshold detection
- Suitable for real-time temperature control applications

## Applications and Use Cases

### Environmental Monitoring
```cpp
// Monitor ambient temperature
temp_sensor.SetThresholds(18.0f, 28.0f);  // Comfort zone
temp_sensor.StartContinuousMonitoring(30000);  // 30-second updates
```

### Thermal Protection
```cpp
// CPU/system thermal protection
temp_sensor.SetThresholds(60.0f, 80.0f);  // Warning/critical temps
temp_sensor.RegisterThresholdCallback(thermal_protection_handler);
```

### Data Logging
```cpp
// Regular temperature logging
temp_sensor.StartContinuousMonitoring(60000);  // 1-minute intervals
temp_sensor.RegisterMonitoringCallback(log_temperature_data);
```

## CI/CD Integration

The temperature test is automatically included in the continuous integration pipeline:

```yaml
matrix:
  example_type: [temperature_test, ...]
  build_type: [Release, Debug]
```

### Automated Testing
- **Build Verification**: Compile-time validation
- **Runtime Testing**: Automated test execution
- **Accuracy Validation**: Temperature reading verification
- **Performance Benchmarking**: Response time and accuracy testing

## References

- [ESP32-C6 Temperature Sensor Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/temp_sensor.html)
- [ESP32-C6 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c6_technical_reference_manual_en.pdf)
- [ESP-IDF v5.5 Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/)
- [Temperature Sensor Calibration Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-guides/temp_sensor.html)