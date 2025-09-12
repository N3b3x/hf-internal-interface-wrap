---
layout: default
title: "🌡️ EspTemperature"
description: "ESP32-C6 Temperature implementation with internal sensor and threshold monitoring"
nav_order: 14
parent: "🔧 ESP32 Implementations"
permalink: /docs/esp_api/EspTemperature/
---

# 🌡️ EspTemperature API Reference

<div align="center">

**📋 Navigation**

[← Previous: EspPeriodicTimer](EspPeriodicTimer.md) | [Back to ESP API Index](README.md) | [Next:
EspLogger →](EspLogger.md)

</div>

---

## Overview

`EspTemperature` provides ESP32-C6 internal temperature sensor functionality with comprehensive
monitoring capabilities.
It implements the `BaseTemperature` interface with hardware-specific optimizations for accurate
temperature measurement,
threshold monitoring, and continuous monitoring.

## Features

- **Internal Temperature Sensor** - ESP32-C6 built-in temperature sensor support
- **Multiple Measurement Ranges** - Different accuracy levels for various use cases
- **Hardware Calibration** - Automatic offset compensation and calibration
- **Threshold Monitoring** - Configurable high/low temperature thresholds with callbacks
- **Continuous Monitoring** - Timer-based sampling with configurable intervals
- **Power Management** - Sleep/wake modes for power efficiency
- **Thread Safety** - Mutex-protected operations for multi-threaded access
- **Comprehensive Diagnostics** - Health monitoring and statistics tracking

## Header File

```cpp
#include "mcu/esp32/EspTemperature.h"
```

## Class Definition

```cpp
class EspTemperature : public BaseTemperature {
public:
    // Constructor with configuration
    explicit EspTemperature(const hf_temperature_config_t& config) noexcept;
    
    // Destructor with proper cleanup
    ~EspTemperature() noexcept override;
    
    // BaseTemperature interface implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    
    // Temperature reading operations
    hf_temperature_err_t ReadTemperature(float& temperature_c) noexcept override;
    hf_temperature_err_t ReadTemperatureWithRange(float& temperature_c, 
                                                  hf_temperature_range_t range) noexcept override;
    
    // Threshold monitoring
    hf_temperature_err_t SetThreshold(hf_temperature_threshold_t threshold_type,
                                      float threshold_value_c) noexcept override;
    hf_temperature_err_t SetThresholdCallback(hf_temperature_threshold_callback_t callback,
                                              void* user_data = nullptr) noexcept override;
    
    // Continuous monitoring
    hf_temperature_err_t StartContinuousMonitoring(uint32_t interval_ms) noexcept override;
    hf_temperature_err_t StopContinuousMonitoring() noexcept override;
    bool IsContinuousMonitoringActive() const noexcept override;
    
    // Power management
    hf_temperature_err_t EnterSleepMode() noexcept override;
    hf_temperature_err_t ExitSleepMode() noexcept override;
    
    // Diagnostics
    hf_temperature_err_t GetStatistics(hf_temperature_statistics_t& statistics) noexcept override;
    hf_temperature_err_t GetDiagnostics(hf_temperature_diagnostics_t& diagnostics) noexcept override;
    hf_temperature_err_t ResetStatistics() noexcept override;
    
    // Self-test and health monitoring
    hf_temperature_err_t RunSelfTest() noexcept override;
    bool IsHealthy() const noexcept override;
};
```

## Configuration Structure

### Temperature Configuration

```cpp
struct hf_temperature_config_t {
    hf_temperature_range_t default_range;        // Default measurement range
    bool enable_calibration;                     // Enable hardware calibration
    bool enable_continuous_monitoring;           // Enable continuous monitoring
    uint32_t continuous_interval_ms;             // Continuous monitoring interval
    float high_threshold_c;                      // High temperature threshold
    float low_threshold_c;                       // Low temperature threshold
    hf_temperature_threshold_callback_t callback; // Threshold callback function
    void* user_data;                             // User data for callbacks
};
```

## Usage Examples

### Basic Temperature Reading

```cpp
#include "mcu/esp32/EspTemperature.h"

// Configure temperature sensor
hf_temperature_config_t config = {};
config.default_range = hf_temperature_range_t::RANGE_HIGH_ACCURACY;
config.enable_calibration = true;

// Create and initialize temperature sensor
EspTemperature temp_sensor(config);
if (!temp_sensor.EnsureInitialized()) {
    printf("Failed to initialize temperature sensor\n");
    return;
}

// Read current temperature
float temperature;
hf_temperature_err_t result = temp_sensor.ReadTemperature(temperature);
if (result == hf_temperature_err_t::TEMPERATURE_SUCCESS) {
    printf("Current temperature: %.2f°C\n", temperature);
}
```

### Threshold Monitoring

```cpp
// Threshold callback function
void temperature_threshold_callback(hf_temperature_threshold_t threshold_type,
                                   float temperature_c, void* user_data) {
    if (threshold_type == hf_temperature_threshold_t::HIGH_THRESHOLD) {
        printf("High temperature warning: %.2f°C\n", temperature_c);
    } else {
        printf("Low temperature warning: %.2f°C\n", temperature_c);
    }
}

// Configure with thresholds
hf_temperature_config_t config = {};
config.high_threshold_c = 80.0f;  // 80°C high threshold
config.low_threshold_c = 0.0f;    // 0°C low threshold
config.callback = temperature_threshold_callback;

EspTemperature temp_sensor(config);
temp_sensor.EnsureInitialized();

// Set threshold callback
temp_sensor.SetThresholdCallback(temperature_threshold_callback, nullptr);
```

### Continuous Monitoring

```cpp
// Configure continuous monitoring
hf_temperature_config_t config = {};
config.enable_continuous_monitoring = true;
config.continuous_interval_ms = 1000;  // 1 second intervals

EspTemperature temp_sensor(config);
temp_sensor.EnsureInitialized();

// Start continuous monitoring
hf_temperature_err_t result = temp_sensor.StartContinuousMonitoring(1000);
if (result == hf_temperature_err_t::TEMPERATURE_SUCCESS) {
    printf("Continuous monitoring started\n");
}

// Check if monitoring is active
if (temp_sensor.IsContinuousMonitoringActive()) {
    printf("Temperature monitoring is active\n");
}

// Stop monitoring when done
temp_sensor.StopContinuousMonitoring();
```

### Power Management

```cpp
EspTemperature temp_sensor(config);
temp_sensor.EnsureInitialized();

// Enter sleep mode to save power
hf_temperature_err_t result = temp_sensor.EnterSleepMode();
if (result == hf_temperature_err_t::TEMPERATURE_SUCCESS) {
    printf("Temperature sensor in sleep mode\n");
}

// Exit sleep mode for measurements
temp_sensor.ExitSleepMode();

// Read temperature after waking up
float temperature;
temp_sensor.ReadTemperature(temperature);
```

### Diagnostics and Health Monitoring

```cpp
EspTemperature temp_sensor(config);
temp_sensor.EnsureInitialized();

// Run self-test
hf_temperature_err_t result = temp_sensor.RunSelfTest();
if (result == hf_temperature_err_t::TEMPERATURE_SUCCESS) {
    printf("Temperature sensor self-test passed\n");
}

// Check health status
if (temp_sensor.IsHealthy()) {
    printf("Temperature sensor is healthy\n");
}

// Get statistics
hf_temperature_statistics_t stats;
temp_sensor.GetStatistics(stats);
printf("Total readings: %u\n", stats.total_readings);
printf("Successful readings: %u\n", stats.successful_readings);
printf("Average temperature: %.2f°C\n", stats.average_temperature_c);

// Get diagnostics
hf_temperature_diagnostics_t diagnostics;
temp_sensor.GetDiagnostics(diagnostics);
printf("Last error: %d\n", diagnostics.last_error_code);
printf("Consecutive errors: %u\n", diagnostics.consecutive_errors);
```

## Temperature Ranges

| Range | Accuracy | Use Case |

|-------|----------|----------|

| `RANGE_LOW_ACCURACY` | ±2°C | General monitoring |

| `RANGE_MEDIUM_ACCURACY` | ±1°C | Standard applications |

| `RANGE_HIGH_ACCURACY` | ±0.5°C | Precision applications |

## Error Handling

The `EspTemperature` class provides comprehensive error reporting through the `hf_temperature_err_t`
enumeration:

- `TEMPERATURE_SUCCESS` - Operation completed successfully
- `TEMPERATURE_ERR_NOT_INITIALIZED` - Sensor not initialized
- `TEMPERATURE_ERR_CALIBRATION` - Calibration error
- `TEMPERATURE_ERR_THRESHOLD` - Threshold configuration error
- `TEMPERATURE_ERR_TIMEOUT` - Operation timeout
- `TEMPERATURE_ERR_HARDWARE` - Hardware failure

## Performance Characteristics

- **Measurement Time**: ~100µs per reading
- **Accuracy**: ±0.5°C (high accuracy range)
- **Range**: -10°C to +80°C (typical)
- **Continuous Monitoring**: Up to 10Hz sampling rate
- **Power Consumption**: ~1mA active, ~1µA sleep

## Thread Safety

The `EspTemperature` class uses mutex protection for thread-safe operation.
Multiple threads can safely call temperature sensor methods simultaneously.

## Related Documentation

- **[BaseTemperature API Reference](../api/BaseTemperature.md)** - Base class interface
<!-- markdownlint-disable-next-line MD013 -->
- **[ESP-IDF Temperature Sensor Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/temp_sensor.html)** - ESP-IDF docs
---
<div align="center">

**📋 Navigation**

[← Previous: EspPeriodicTimer](EspPeriodicTimer.md) | [Back to ESP API Index](README.md) | [Next:
EspLogger →](EspLogger.md)

</div>
>>>>>>> Incoming (Background Agent changes)
