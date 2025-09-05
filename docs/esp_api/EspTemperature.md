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
```text

## Class Definition

```cpp
class EspTemperature : public BaseTemperature {
public:
    // Constructor with configuration
    explicit EspTemperature(const hf*temperature*config*t& config) noexcept;
    
    // Destructor with proper cleanup
    ~EspTemperature() noexcept override;
    
    // BaseTemperature interface implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    
    // Temperature reading operations
    hf*temperature*err*t ReadTemperature(float& temperature*c) noexcept override;
    hf*temperature*err*t ReadTemperatureWithRange(float& temperature*c, 
                                                  hf*temperature*range*t range) noexcept override;
    
    // Threshold monitoring
    hf*temperature*err*t SetThreshold(hf*temperature*threshold*t threshold*type,
                                      float threshold*value*c) noexcept override;
    hf*temperature*err*t SetThresholdCallback(hf*temperature*threshold*callback*t callback,
                                              void* user*data = nullptr) noexcept override;
    
    // Continuous monitoring
    hf*temperature*err*t StartContinuousMonitoring(uint32*t interval*ms) noexcept override;
    hf*temperature*err*t StopContinuousMonitoring() noexcept override;
    bool IsContinuousMonitoringActive() const noexcept override;
    
    // Power management
    hf*temperature*err*t EnterSleepMode() noexcept override;
    hf*temperature*err*t ExitSleepMode() noexcept override;
    
    // Diagnostics
    hf*temperature*err*t GetStatistics(hf*temperature*statistics*t& statistics) noexcept override;
    hf*temperature*err*t GetDiagnostics(hf*temperature*diagnostics*t& diagnostics) noexcept override;
    hf*temperature*err*t ResetStatistics() noexcept override;
    
    // Self-test and health monitoring
    hf*temperature*err*t RunSelfTest() noexcept override;
    bool IsHealthy() const noexcept override;
};
```text

## Configuration Structure

### Temperature Configuration

```cpp
struct hf*temperature*config*t {
    hf*temperature*range*t default*range;        // Default measurement range
    bool enable*calibration;                     // Enable hardware calibration
    bool enable*continuous*monitoring;           // Enable continuous monitoring
    uint32*t continuous*interval*ms;             // Continuous monitoring interval
    float high*threshold*c;                      // High temperature threshold
    float low*threshold*c;                       // Low temperature threshold
    hf*temperature*threshold*callback*t callback; // Threshold callback function
    void* user*data;                             // User data for callbacks
};
```text

## Usage Examples

### Basic Temperature Reading

```cpp
#include "mcu/esp32/EspTemperature.h"

// Configure temperature sensor
hf*temperature*config*t config = {};
config.default*range = hf*temperature*range*t::RANGE*HIGH*ACCURACY;
config.enable*calibration = true;

// Create and initialize temperature sensor
EspTemperature temp*sensor(config);
if (!temp*sensor.EnsureInitialized()) {
    printf("Failed to initialize temperature sensor\n");
    return;
}

// Read current temperature
float temperature;
hf*temperature*err*t result = temp*sensor.ReadTemperature(temperature);
if (result == hf*temperature*err*t::TEMPERATURE*SUCCESS) {
    printf("Current temperature: %.2f°C\n", temperature);
}
```text

### Threshold Monitoring

```cpp
// Threshold callback function
void temperature*threshold*callback(hf*temperature*threshold*t threshold*type,
                                   float temperature*c, void* user*data) {
    if (threshold*type == hf*temperature*threshold*t::HIGH*THRESHOLD) {
        printf("High temperature warning: %.2f°C\n", temperature*c);
    } else {
        printf("Low temperature warning: %.2f°C\n", temperature*c);
    }
}

// Configure with thresholds
hf*temperature*config*t config = {};
config.high*threshold*c = 80.0f;  // 80°C high threshold
config.low*threshold*c = 0.0f;    // 0°C low threshold
config.callback = temperature*threshold*callback;

EspTemperature temp*sensor(config);
temp*sensor.EnsureInitialized();

// Set threshold callback
temp*sensor.SetThresholdCallback(temperature*threshold*callback, nullptr);
```text

### Continuous Monitoring

```cpp
// Configure continuous monitoring
hf*temperature*config*t config = {};
config.enable*continuous*monitoring = true;
config.continuous*interval*ms = 1000;  // 1 second intervals

EspTemperature temp*sensor(config);
temp*sensor.EnsureInitialized();

// Start continuous monitoring
hf*temperature*err*t result = temp*sensor.StartContinuousMonitoring(1000);
if (result == hf*temperature*err*t::TEMPERATURE*SUCCESS) {
    printf("Continuous monitoring started\n");
}

// Check if monitoring is active
if (temp*sensor.IsContinuousMonitoringActive()) {
    printf("Temperature monitoring is active\n");
}

// Stop monitoring when done
temp*sensor.StopContinuousMonitoring();
```text

### Power Management

```cpp
EspTemperature temp*sensor(config);
temp*sensor.EnsureInitialized();

// Enter sleep mode to save power
hf*temperature*err*t result = temp*sensor.EnterSleepMode();
if (result == hf*temperature*err*t::TEMPERATURE*SUCCESS) {
    printf("Temperature sensor in sleep mode\n");
}

// Exit sleep mode for measurements
temp*sensor.ExitSleepMode();

// Read temperature after waking up
float temperature;
temp*sensor.ReadTemperature(temperature);
```text

### Diagnostics and Health Monitoring

```cpp
EspTemperature temp*sensor(config);
temp*sensor.EnsureInitialized();

// Run self-test
hf*temperature*err*t result = temp*sensor.RunSelfTest();
if (result == hf*temperature*err*t::TEMPERATURE*SUCCESS) {
    printf("Temperature sensor self-test passed\n");
}

// Check health status
if (temp*sensor.IsHealthy()) {
    printf("Temperature sensor is healthy\n");
}

// Get statistics
hf*temperature*statistics*t stats;
temp*sensor.GetStatistics(stats);
printf("Total readings: %u\n", stats.total*readings);
printf("Successful readings: %u\n", stats.successful*readings);
printf("Average temperature: %.2f°C\n", stats.average*temperature*c);

// Get diagnostics
hf*temperature*diagnostics*t diagnostics;
temp*sensor.GetDiagnostics(diagnostics);
printf("Last error: %d\n", diagnostics.last*error*code);
printf("Consecutive errors: %u\n", diagnostics.consecutive*errors);
```text

## Temperature Ranges

| Range | Accuracy | Use Case |

|-------|----------|----------|

| `RANGE*LOW*ACCURACY` | ±2°C | General monitoring |

| `RANGE*MEDIUM*ACCURACY` | ±1°C | Standard applications |

| `RANGE*HIGH*ACCURACY` | ±0.5°C | Precision applications |

## Error Handling

The `EspTemperature` class provides comprehensive error reporting through the `hf*temperature*err*t`
enumeration:

- `TEMPERATURE*SUCCESS` - Operation completed successfully
- `TEMPERATURE*ERR*NOT*INITIALIZED` - Sensor not initialized
- `TEMPERATURE*ERR*CALIBRATION` - Calibration error
- `TEMPERATURE*ERR*THRESHOLD` - Threshold configuration error
- `TEMPERATURE*ERR*TIMEOUT` - Operation timeout
- `TEMPERATURE*ERR*HARDWARE` - Hardware failure

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
- **[ESP-IDF Temperature Sensor Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/temp_sensor.html)** - ESP-IDF documentation
## # <<<<<<< Current (Your changes)
<div align="center">

**📋 Navigation**

[← Previous: EspPeriodicTimer](EspPeriodicTimer.md) | [Back to ESP API Index](README.md) | [Next:
EspLogger →](EspLogger.md)

</div>
>>>>>>> Incoming (Background Agent changes)
