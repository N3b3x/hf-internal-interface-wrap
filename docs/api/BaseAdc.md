# BaseAdc API Reference

## Overview

`BaseAdc` is the abstract base class for all ADC (Analog-to-Digital Converter) implementations in the HardFOC system. It provides a unified interface for analog sensor reading with support for multi-channel operation, averaging, and comprehensive error handling.

## Features

- **Multi-Channel Support** - Read from multiple analog channels
- **Voltage and Raw Count Readings** - Get calibrated voltage or raw ADC counts
- **Averaging Support** - Built-in sample averaging for noise reduction
- **Flexible Timing** - Configurable time between samples
- **Lazy Initialization** - Resources allocated only when needed
- **Comprehensive Error Handling** - 32 detailed error codes with descriptions

## Header File

```cpp
#include "inc/base/BaseAdc.h"
```

## Type Definitions

### Error Codes

```cpp
enum class hf_adc_err_t : hf_u8_t {
    ADC_SUCCESS = 0,                      // Success
    ADC_ERR_FAILURE = 1,                  // General failure
    ADC_ERR_NOT_INITIALIZED = 2,          // Not initialized
    ADC_ERR_ALREADY_INITIALIZED = 3,      // Already initialized
    ADC_ERR_INVALID_PARAMETER = 4,        // Invalid parameter
    ADC_ERR_NULL_POINTER = 5,             // Null pointer
    ADC_ERR_OUT_OF_MEMORY = 6,            // Out of memory
    ADC_ERR_CHANNEL_NOT_FOUND = 7,        // Channel not found
    ADC_ERR_CHANNEL_NOT_ENABLED = 8,      // Channel not enabled
    ADC_ERR_CHANNEL_NOT_CONFIGURED = 9,   // Channel not configured
    ADC_ERR_CHANNEL_BUSY = 10,            // Channel busy
    ADC_ERR_CHANNEL_TIMEOUT = 11,         // Channel timeout
    ADC_ERR_INVALID_CHANNEL = 12,         // Invalid channel
    ADC_ERR_INVALID_RESOLUTION = 13,      // Invalid resolution
    ADC_ERR_INVALID_REFERENCE = 14,       // Invalid reference
    ADC_ERR_INVALID_SAMPLE_TIME = 15,     // Invalid sample time
    ADC_ERR_INVALID_TRIGGER = 16,         // Invalid trigger
    ADC_ERR_INVALID_MODE = 17,            // Invalid mode
    ADC_ERR_CALIBRATION_FAILED = 18,      // Calibration failed
    ADC_ERR_CONVERSION_FAILED = 19,       // Conversion failed
    ADC_ERR_HARDWARE_ERROR = 20,          // Hardware error
    ADC_ERR_COMMUNICATION_ERROR = 21,     // Communication error
    ADC_ERR_TIMEOUT = 22,                 // Timeout
    ADC_ERR_BUSY = 23,                    // Busy
    ADC_ERR_NOT_SUPPORTED = 24,           // Not supported
    ADC_ERR_CONFIGURATION_LOCKED = 25,    // Configuration locked
    ADC_ERR_POWER_MANAGEMENT = 26,        // Power management
    ADC_ERR_VOLTAGE_OUT_OF_RANGE = 27,    // Voltage out of range
    ADC_ERR_OVERRUN = 28,                 // Overrun
    ADC_ERR_UNDERRUN = 29,                // Underrun
    ADC_ERR_DMA_ERROR = 30,               // DMA error
    ADC_ERR_FIFO_ERROR = 31               // FIFO error
};
```

## Class Interface

```cpp
class BaseAdc {
public:
    // Construction and destruction
    virtual ~BaseAdc() noexcept = default;
    BaseAdc(const BaseAdc&) = delete;
    BaseAdc& operator=(const BaseAdc&) = delete;
    BaseAdc(BaseAdc&&) noexcept = default;
    BaseAdc& operator=(BaseAdc&&) noexcept = default;

    // Initialization and status
    bool EnsureInitialized() noexcept;
    bool EnsureDeinitialized() noexcept;
    bool IsInitialized() const noexcept;

    // Pure virtual methods (implemented by derived classes)
    virtual bool Initialize() noexcept = 0;
    virtual bool Deinitialize() noexcept = 0;
    
    // Channel information
    virtual hf_u8_t GetMaxChannels() const noexcept = 0;
    virtual bool IsChannelAvailable(hf_channel_id_t channel_id) const noexcept = 0;
    
    // Reading methods
    virtual hf_adc_err_t ReadChannelV(hf_channel_id_t channel_id, 
                                     float& channel_reading_v,
                                     hf_u8_t numOfSamplesToAvg = 1,
                                     hf_time_t timeBetweenSamples = 0) noexcept = 0;
                                     
    virtual hf_adc_err_t ReadChannelCount(hf_channel_id_t channel_id, 
                                         hf_u32_t& channel_reading_count,
                                         hf_u8_t numOfSamplesToAvg = 1,
                                         hf_time_t timeBetweenSamples = 0) noexcept = 0;
                                         
    virtual hf_adc_err_t ReadChannelCountAndV(hf_channel_id_t channel_id,
                                             hf_u32_t& channel_reading_count,
                                             float& channel_reading_v,
                                             hf_u8_t numOfSamplesToAvg = 1,
                                             hf_time_t timeBetweenSamples = 0) noexcept = 0;
};
```

## Reading Methods

### Voltage Reading

```cpp
hf_adc_err_t ReadChannelV(hf_channel_id_t channel_id, 
                         float& channel_reading_v,
                         hf_u8_t numOfSamplesToAvg = 1,
                         hf_time_t timeBetweenSamples = 0) noexcept;
```

**Parameters:**
- `channel_id` - ADC channel identifier (0-based)
- `channel_reading_v` - Reference to store voltage reading in volts
- `numOfSamplesToAvg` - Number of samples to average (default: 1)
- `timeBetweenSamples` - Time between samples in milliseconds (default: 0)

**Returns:** Error code indicating success or failure

### Raw Count Reading

```cpp
hf_adc_err_t ReadChannelCount(hf_channel_id_t channel_id, 
                             hf_u32_t& channel_reading_count,
                             hf_u8_t numOfSamplesToAvg = 1,
                             hf_time_t timeBetweenSamples = 0) noexcept;
```

**Parameters:**
- `channel_id` - ADC channel identifier
- `channel_reading_count` - Reference to store raw ADC count
- `numOfSamplesToAvg` - Number of samples to average
- `timeBetweenSamples` - Time between samples in milliseconds

**Returns:** Error code indicating success or failure

### Combined Reading

```cpp
hf_adc_err_t ReadChannelCountAndV(hf_channel_id_t channel_id,
                                 hf_u32_t& channel_reading_count,
                                 float& channel_reading_v,
                                 hf_u8_t numOfSamplesToAvg = 1,
                                 hf_time_t timeBetweenSamples = 0) noexcept;
```

Reads both raw count and calibrated voltage in a single operation for efficiency.

## Usage Examples

### Basic Voltage Reading

```cpp
#include "inc/mcu/esp32/EspAdc.h"

// Create ADC instance
EspAdc adc(ADC_UNIT_1, ADC_ATTEN_DB_11);

// Initialize ADC
if (!adc.EnsureInitialized()) {
    printf("Failed to initialize ADC\n");
    return;
}

// Read voltage from channel 0
float voltage;
hf_adc_err_t result = adc.ReadChannelV(0, voltage);
if (result == hf_adc_err_t::ADC_SUCCESS) {
    printf("Channel 0 voltage: %.3f V\n", voltage);
} else {
    printf("ADC Error: %s\n", HfAdcErrToString(result));
}
```

### Multi-Sample Averaging

```cpp
// Read with averaging for noise reduction
float voltage;
hf_adc_err_t result = adc.ReadChannelV(0, voltage, 10, 5);  // 10 samples, 5ms between
if (result == hf_adc_err_t::ADC_SUCCESS) {
    printf("Averaged voltage: %.3f V\n", voltage);
}
```

### Raw Count Reading

```cpp
// Read raw ADC counts
hf_u32_t raw_count;
hf_adc_err_t result = adc.ReadChannelCount(0, raw_count);
if (result == hf_adc_err_t::ADC_SUCCESS) {
    printf("Raw ADC count: %u\n", raw_count);
}
```

### Combined Reading

```cpp
// Read both raw and calibrated values efficiently
hf_u32_t raw_count;
float voltage;
hf_adc_err_t result = adc.ReadChannelCountAndV(0, raw_count, voltage);
if (result == hf_adc_err_t::ADC_SUCCESS) {
    printf("Raw: %u, Voltage: %.3f V\n", raw_count, voltage);
}
```

### Multi-Channel Sensor Reading

```cpp
class SensorReader {
private:
    EspAdc adc_;
    
public:
    SensorReader() : adc_(ADC_UNIT_1, ADC_ATTEN_DB_11) {}
    
    bool initialize() {
        return adc_.EnsureInitialized();
    }
    
    void read_all_sensors() {
        // Read current sensor (channel 0)
        float current_voltage;
        if (adc_.ReadChannelV(0, current_voltage, 5) == hf_adc_err_t::ADC_SUCCESS) {
            float current_amps = (current_voltage - 2.5f) / 0.1f;  // ACS712 conversion
            printf("Motor current: %.2f A\n", current_amps);
        }
        
        // Read position sensor (channel 1)
        float position_voltage;
        if (adc_.ReadChannelV(1, position_voltage, 3) == hf_adc_err_t::ADC_SUCCESS) {
            float position_degrees = (position_voltage / 3.3f) * 360.0f;
            printf("Motor position: %.1f degrees\n", position_degrees);
        }
        
        // Read temperature sensor (channel 2)
        float temp_voltage;
        if (adc_.ReadChannelV(2, temp_voltage) == hf_adc_err_t::ADC_SUCCESS) {
            float temperature_c = (temp_voltage - 0.5f) / 0.01f;  // TMP36 conversion
            printf("Temperature: %.1f °C\n", temperature_c);
        }
    }
    
    bool check_channel_availability() {
        printf("Available ADC channels:\n");
        for (hf_u8_t ch = 0; ch < adc_.GetMaxChannels(); ch++) {
            if (adc_.IsChannelAvailable(ch)) {
                printf("  Channel %u: Available\n", ch);
            }
        }
        return true;
    }
};
```

### Error Handling Best Practices

```cpp
hf_adc_err_t read_sensor_with_retry(BaseAdc& adc, hf_channel_id_t channel, float& voltage) {
    const int max_retries = 3;
    int retry_count = 0;
    
    while (retry_count < max_retries) {
        hf_adc_err_t result = adc.ReadChannelV(channel, voltage, 5, 2);
        
        switch (result) {
            case hf_adc_err_t::ADC_SUCCESS:
                return result;  // Success, return immediately
                
            case hf_adc_err_t::ADC_ERR_BUSY:
            case hf_adc_err_t::ADC_ERR_TIMEOUT:
                // Transient errors - retry
                retry_count++;
                vTaskDelay(pdMS_TO_TICKS(10));  // Wait before retry
                break;
                
            case hf_adc_err_t::ADC_ERR_NOT_INITIALIZED:
                // Try to initialize
                if (!adc.EnsureInitialized()) {
                    return result;  // Initialization failed
                }
                retry_count++;
                break;
                
            default:
                // Permanent error - don't retry
                printf("ADC Error: %s\n", HfAdcErrToString(result));
                return result;
        }
    }
    
    printf("ADC read failed after %d retries\n", max_retries);
    return hf_adc_err_t::ADC_ERR_TIMEOUT;
}
```

## Utility Functions

```cpp
// Convert error code to string
const char* HfAdcErrToString(hf_adc_err_t err) noexcept;
```

## Performance Considerations

### Sample Averaging
- Use averaging (`numOfSamplesToAvg > 1`) to reduce noise in noisy environments
- Higher averaging improves accuracy but increases conversion time
- Typical values: 1-10 samples for most applications

### Timing Between Samples
- Use `timeBetweenSamples` when reading sensors that need settling time
- Useful for multiplexed inputs or high-impedance sources
- Typical values: 0-10ms depending on source impedance

### Channel Selection
- Check channel availability with `IsChannelAvailable()` before use
- Some channels may be reserved for internal use
- Channel count varies by platform (ESP32-C6: up to 7 channels)

## Thread Safety

The BaseAdc class is **not thread-safe**. If you need to access ADC from multiple threads, you must provide your own synchronization mechanisms.

## Implementation Notes

- **Lazy Initialization**: Hardware resources are only allocated when `EnsureInitialized()` is called
- **Calibration**: Voltage readings are automatically calibrated based on reference voltage
- **Resolution**: Actual resolution depends on the underlying hardware (12-bit typical)
- **Reference Voltage**: Configurable reference voltage affects measurement range and accuracy

## Derived Classes

The following concrete implementations are available:

- **EspAdc** - ESP32-C6 on-chip ADC implementation
- **I2cAdc** - I2C-based external ADC support
- **SpiAdc** - SPI-based external ADC support

## Related Documentation

- [EspAdc API Reference](EspAdc.md) - ESP32-C6 implementation
- [HardwareTypes Reference](HardwareTypes.md) - Platform-agnostic type definitions 