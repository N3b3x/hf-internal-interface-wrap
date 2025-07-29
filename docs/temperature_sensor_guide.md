# ESP32-C6 Temperature Sensor Implementation Guide

## Overview

This guide covers the HardFOC temperature sensor implementation for ESP32-C6, which provides access to the internal chip temperature sensor through a clean, object-oriented C++ interface.

## Features

### BaseTemperature Abstract Class
- **Standardized Interface**: Consistent API across different temperature sensor implementations
- **Comprehensive Error Handling**: 44+ specific error codes with detailed descriptions
- **Multiple Temperature Units**: Support for Celsius, Fahrenheit, Kelvin, and Rankine
- **Advanced Monitoring**: Threshold monitoring and continuous reading capabilities
- **Power Management**: Sleep mode support for low-power applications
- **Calibration Support**: Manual and automatic calibration features
- **Self-Test Functionality**: Built-in sensor health checks

### EspTemperature Implementation
- **ESP32-C6 Optimized**: Leverages ESP-IDF temperature sensor driver
- **Multiple Measurement Ranges**: 5 predefined ranges with different accuracy levels
- **Thread-Safe**: Mutex-protected operations for multi-threaded applications
- **Automatic Range Selection**: Finds optimal range for given temperature requirements
- **Continuous Monitoring**: Timer-based periodic temperature readings
- **Threshold Interrupts**: Callback-based threshold violation detection

## Hardware Specifications

| Parameter | Value | Notes |
|-----------|--------|-------|
| **Temperature Range** | -40°C to +125°C | Full hardware range |
| **Resolution** | 0.25°C | Fixed hardware resolution |
| **Accuracy** | ±1°C to ±3°C | Depends on selected range |
| **Response Time** | ~50ms | Typical measurement time |
| **Sensor Type** | Internal chip sensor | Measures chip temperature, not ambient |

### Predefined Measurement Ranges

| Range | Temperature Range | Accuracy | Use Case |
|-------|------------------|----------|----------|
| `ESP_TEMP_RANGE_NEG10_80` | -10°C to 80°C | ±1°C | **Best accuracy** (default) |
| `ESP_TEMP_RANGE_20_100` | 20°C to 100°C | ±2°C | High temperature applications |
| `ESP_TEMP_RANGE_NEG30_50` | -30°C to 50°C | ±2°C | Cold environment monitoring |
| `ESP_TEMP_RANGE_50_125` | 50°C to 125°C | ±3°C | High temperature range |
| `ESP_TEMP_RANGE_NEG40_20` | -40°C to 20°C | ±3°C | Extreme cold applications |

## Quick Start Guide

### 1. Include Headers

```cpp
#include "BaseTemperature.h"
#include "EspTemperature.h"

using namespace HardFOC;
```

### 2. Basic Usage

```cpp
// Create temperature sensor instance
EspTemperature temp_sensor;

// Initialize with default configuration
HfTempConfig_t config = HF_TEMP_CONFIG_DEFAULT();
config.range_min_celsius = -10.0f;
config.range_max_celsius = 80.0f;

HfTempError_t error = temp_sensor.initialize(&config);
if (error != TEMP_SUCCESS) {
    ESP_LOGE("APP", "Failed to initialize: %s", hf_temp_get_error_string(error));
    return;
}

// Enable sensor
error = temp_sensor.enable();
if (error != TEMP_SUCCESS) {
    ESP_LOGE("APP", "Failed to enable: %s", hf_temp_get_error_string(error));
    return;
}

// Read temperature
float temperature;
error = temp_sensor.read_celsius(&temperature);
if (error == TEMP_SUCCESS) {
    ESP_LOGI("APP", "Temperature: %.2f°C", temperature);
}
```

### 3. ESP32-Specific Initialization

```cpp
// Alternative: Use ESP32-specific configuration
EspTempConfig_t esp_config = ESP_TEMP_CONFIG_DEFAULT();
esp_config.preferred_range = ESP_TEMP_RANGE_NEG10_80;  // Best accuracy
esp_config.enable_power_management = false;
esp_config.auto_range_selection = true;

error = temp_sensor.initialize_esp32(&esp_config);
```

## Advanced Features

### Continuous Monitoring

```cpp
// Callback function for temperature readings
void temperature_callback(const HfTempReading_t* reading, void* user_data) {
    if (reading->is_valid) {
        ESP_LOGI("APP", "Temperature: %.2f°C (±%.1f°C)", 
                 reading->temperature_celsius, reading->accuracy_celsius);
    }
}

// Start continuous monitoring at 2 Hz
uint32_t sample_rate = 2;
error = temp_sensor.start_continuous_monitoring(sample_rate, temperature_callback, nullptr);

// Stop monitoring when done
temp_sensor.stop_continuous_monitoring();
```

### Threshold Monitoring

```cpp
// Threshold callback function
void threshold_callback(float temperature, uint32_t threshold_type, void* user_data) {
    const char* type = (threshold_type == 0) ? "LOW" : "HIGH";
    ESP_LOGW("APP", "%s threshold exceeded: %.2f°C", type, temperature);
}

// Set thresholds
error = temp_sensor.set_thresholds(10.0f, 50.0f);  // 10°C to 50°C

// Enable threshold monitoring
error = temp_sensor.enable_threshold_monitoring(threshold_callback, nullptr);
```

### Calibration

```cpp
// Automatic calibration with known reference
float reference_temperature = 25.0f;  // Known ambient temperature
error = temp_sensor.calibrate(reference_temperature);

// Manual calibration offset
error = temp_sensor.set_calibration_offset(2.5f);  // Add 2.5°C offset

// Get current offset
float offset;
error = temp_sensor.get_calibration_offset(&offset);

// Reset calibration
error = temp_sensor.reset_calibration();
```

### Range Management

```cpp
// Find optimal range for temperature requirements
EspTempRange_t optimal = temp_sensor.find_optimal_range(0.0f, 60.0f);

// Set specific measurement range
error = temp_sensor.set_measurement_range(ESP_TEMP_RANGE_NEG10_80);

// Get range information
float min_temp, max_temp, accuracy;
error = temp_sensor.get_range_info(ESP_TEMP_RANGE_NEG10_80, 
                                   &min_temp, &max_temp, &accuracy);
```

### Power Management

```cpp
// Enter sleep mode
error = temp_sensor.enter_sleep_mode();

// Check if sleeping
bool is_sleeping = temp_sensor.is_sleeping();

// Exit sleep mode
error = temp_sensor.exit_sleep_mode();

// Re-enable after sleep
error = temp_sensor.enable();
```

### Multiple Temperature Units

```cpp
float celsius, fahrenheit, kelvin;

// Read in different units
error = temp_sensor.read_celsius(&celsius);
error = temp_sensor.read_fahrenheit(&fahrenheit);
error = temp_sensor.read_kelvin(&kelvin);

// Read in specific unit
error = temp_sensor.read_temperature_unit(&temperature, TEMP_UNIT_FAHRENHEIT);
```

### Self-Test and Health Check

```cpp
// Perform comprehensive self-test
error = temp_sensor.self_test();
if (error == TEMP_SUCCESS) {
    ESP_LOGI("APP", "Self-test passed");
} else {
    ESP_LOGE("APP", "Self-test failed: %s", hf_temp_get_error_string(error));
}

// Quick health check
error = temp_sensor.check_health();
```

## Error Handling

### Error Codes
The implementation provides 44+ specific error codes covering all possible failure scenarios:

```cpp
// Common error codes
TEMP_SUCCESS                    // Operation successful
TEMP_ERR_NOT_INITIALIZED       // Sensor not initialized
TEMP_ERR_SENSOR_DISABLED        // Sensor is disabled
TEMP_ERR_READ_FAILED           // Failed to read temperature
TEMP_ERR_OUT_OF_RANGE          // Temperature outside valid range
TEMP_ERR_INVALID_PARAMETER     // Invalid parameter provided
TEMP_ERR_TIMEOUT               // Operation timeout
// ... many more specific errors
```

### Error Handling Best Practices

```cpp
// Always check return values
HfTempError_t error = temp_sensor.read_celsius(&temperature);
if (error != TEMP_SUCCESS) {
    ESP_LOGE("APP", "Temperature read failed: %s", hf_temp_get_error_string(error));
    
    // Handle specific errors
    switch (error) {
        case TEMP_ERR_NOT_INITIALIZED:
            // Reinitialize sensor
            break;
        case TEMP_ERR_SENSOR_DISABLED:
            // Re-enable sensor
            break;
        case TEMP_ERR_OUT_OF_RANGE:
            // Adjust measurement range
            break;
        default:
            // General error handling
            break;
    }
}

// Set error callback for automatic error handling
void error_callback(HfTempError_t error, const char* description, void* user_data) {
    ESP_LOGE("TEMP", "Error: %s", description);
}

temp_sensor.set_error_callback(error_callback, nullptr);
```

## Integration with ESP-IDF

### CMakeLists.txt Dependencies

```cmake
# Add temperature sensor component dependency
target_link_libraries(${COMPONENT_LIB} PRIVATE 
    esp_driver_tsens
    freertos
)

# Include directories
target_include_directories(${COMPONENT_LIB} PUBLIC
    inc/base
    src/mcu/esp32
)
```

### Kconfig Configuration

```kconfig
# Enable temperature sensor support
CONFIG_TEMP_SENSOR_ISR_IRAM_SAFE=y

# Optional: Enable power management
CONFIG_PM_ENABLE=y
```

### Required ESP-IDF Components

- `esp_driver_tsens` - Temperature sensor driver
- `freertos` - FreeRTOS RTOS
- `esp_timer` - High-resolution timer
- `esp_log` - Logging system

## Thread Safety

The EspTemperature implementation is **thread-safe** with the following guarantees:

- **Safe operations**: `read_celsius()`, `read_temperature()`, `initialize()`, `enable()`, `disable()`
- **Atomic state changes**: All state modifications are mutex-protected
- **Callback safety**: Callbacks are executed in timer task context

```cpp
// Safe to call from multiple threads
void task1(void* params) {
    while (true) {
        float temp;
        temp_sensor.read_celsius(&temp);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task2(void* params) {
    while (true) {
        HfTempReading_t reading;
        temp_sensor.read_temperature(&reading);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

## Performance Considerations

### Memory Usage
- **Static allocation**: No dynamic memory allocation during operation
- **Small footprint**: ~2KB RAM for instance state
- **Configurable**: IRAM usage configurable via Kconfig

### Timing Characteristics
- **Reading time**: ~50ms per measurement
- **Initialization**: ~100ms typical
- **Callback latency**: <1ms for threshold/monitoring callbacks

### Best Practices
1. **Initialize once**: Avoid repeated initialization
2. **Batch operations**: Use continuous monitoring for frequent readings
3. **Error handling**: Always check return values
4. **Resource cleanup**: Call `deinitialize()` when done
5. **Range selection**: Use optimal range for best accuracy

## Troubleshooting

### Common Issues

#### "Temperature sensor not available"
```cpp
// Check if ESP32-C6 chip supports temperature sensor
HfTempSensorInfo_t info;
if (temp_sensor.get_sensor_info(&info) != TEMP_SUCCESS) {
    ESP_LOGE("APP", "Temperature sensor not supported on this chip");
}
```

#### "Invalid range" errors
```cpp
// Ensure range is within hardware limits
if (min_temp < ESP_TEMP_MIN_CELSIUS || max_temp > ESP_TEMP_MAX_CELSIUS) {
    ESP_LOGE("APP", "Range [%.1f, %.1f] exceeds hardware limits [%.1f, %.1f]",
             min_temp, max_temp, ESP_TEMP_MIN_CELSIUS, ESP_TEMP_MAX_CELSIUS);
}
```

#### Inconsistent readings
```cpp
// Check sensor health
if (temp_sensor.check_health() != TEMP_SUCCESS) {
    ESP_LOGW("APP", "Sensor health check failed - consider recalibration");
    temp_sensor.reset_calibration();
}
```

### Debug Tips

1. **Enable verbose logging**:
   ```cpp
   esp_log_level_set("EspTemperature", ESP_LOG_DEBUG);
   ```

2. **Check sensor state**:
   ```cpp
   ESP_LOGI("APP", "Sensor state: %d", temp_sensor.get_state());
   ESP_LOGI("APP", "Initialized: %s", temp_sensor.is_initialized() ? "Yes" : "No");
   ESP_LOGI("APP", "Enabled: %s", temp_sensor.is_enabled() ? "Yes" : "No");
   ```

3. **Monitor last error**:
   ```cpp
   HfTempError_t last_error = temp_sensor.get_last_error();
   if (last_error != TEMP_SUCCESS) {
       ESP_LOGW("APP", "Last error: %s", hf_temp_get_error_string(last_error));
   }
   ```

## API Reference Summary

### Core Operations
- `initialize()` / `initialize_esp32()` - Initialize sensor
- `deinitialize()` - Cleanup resources
- `enable()` / `disable()` - Control sensor state
- `read_celsius()` / `read_temperature()` - Read temperature

### Configuration
- `set_range()` / `get_range()` - Measurement range
- `set_resolution()` / `get_resolution()` - Resolution control
- `set_measurement_range()` - ESP32-specific ranges

### Monitoring
- `start_continuous_monitoring()` / `stop_continuous_monitoring()` - Continuous reading
- `set_thresholds()` / `enable_threshold_monitoring()` - Threshold alerts

### Calibration
- `calibrate()` - Automatic calibration
- `set_calibration_offset()` / `get_calibration_offset()` - Manual calibration
- `reset_calibration()` - Reset to defaults

### Information
- `get_sensor_info()` - Hardware information
- `get_capabilities()` - Feature support
- `self_test()` / `check_health()` - Diagnostics

### Power Management
- `enter_sleep_mode()` / `exit_sleep_mode()` - Power control
- `is_sleeping()` - Sleep state query

## Example Projects

Complete examples are available in the `examples/` directory:

- **`temperature_sensor_example.cpp`** - Comprehensive demonstration of all features
- Basic reading and multiple units
- Continuous monitoring with statistics
- Threshold monitoring with callbacks
- Calibration procedures
- Power management
- Self-test and health checks
- Error handling patterns

## License and Support

This implementation is part of the HardFOC project and follows the same licensing terms. For support, issues, or feature requests, please refer to the main HardFOC repository.

---

**Note**: The internal temperature sensor measures chip temperature, which is typically higher than ambient temperature due to chip power consumption and thermal characteristics. For ambient temperature measurement, consider using external temperature sensors.