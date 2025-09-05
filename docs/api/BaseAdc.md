# 📊 BaseAdc API Reference

## 📋 Navigation

[← Previous: BaseGpio](BaseGpio.md) | [Back to API Index](README.md) |
[Next: BasePwm →](BasePwm.md)

---

## 🌟 Overview

`BaseAdc` is the abstract base class for all ADC (Analog-to-Digital
Converter) implementations in the HardFOC system. It provides a unified
interface for analog sensor reading with support for multi-channel
operation, averaging, and comprehensive error handling.

## ✨ Features

- **🔢 Multi-Channel Support** - Read from multiple analog channels
- **⚡ Voltage and Raw Count Readings** - Get calibrated voltage or raw ADC counts
- **📊 Averaging Support** - Built-in sample averaging for noise reduction
- **⏰ Flexible Timing** - Configurable time between samples
- **🔧 Lazy Initialization** - Resources allocated only when needed
- **🛡️ Comprehensive Error Handling** - 32 detailed error codes with descriptions

## Header File

```cpp
#include "inc/base/BaseAdc.h"
```text

## Type Definitions

### Error Codes

```cpp
enum class hf*adc*err*t : hf*u8*t {
    ADC*SUCCESS = 0,                         // ✅ Success
    ADC*ERR*FAILURE = 1,                     // ❌ General failure
    ADC*ERR*NOT*INITIALIZED = 2,             // ⚠️ Not initialized
    ADC*ERR*ALREADY*INITIALIZED = 3,         // ⚠️ Already initialized
    ADC*ERR*INVALID*PARAMETER = 4,           // 🚫 Invalid parameter
    ADC*ERR*NULL*POINTER = 5,                // 🚫 Null pointer
    ADC*ERR*OUT*OF*MEMORY = 6,               // 💾 Out of memory
    ADC*ERR*CHANNEL*NOT*FOUND = 7,           // 🔍 Channel not found
    ADC*ERR*CHANNEL*NOT*ENABLED = 8,         // ⚠️ Channel not enabled
    ADC*ERR*CHANNEL*NOT*CONFIGURED = 9,      // ⚙️ Channel not configured
    ADC*ERR*CHANNEL*ALREADY*REGISTERED = 10, // 📝 Channel already registered
    ADC*ERR*CHANNEL*READ*ERR = 11,           // 📖 Channel read error
    ADC*ERR*CHANNEL*WRITE*ERR = 12,          // ✍️ Channel write error
    ADC*ERR*INVALID*CHANNEL = 13,            // 🔍 Invalid channel
    ADC*ERR*CHANNEL*BUSY = 14,               // 🔄 Channel busy
    ADC*ERR*INVALID*SAMPLE*COUNT = 15,       // 📊 Invalid sample count
    ADC*ERR*SAMPLE*TIMEOUT = 16,             // ⏰ Sample timeout
    ADC*ERR*SAMPLE*OVERFLOW = 17,            // 📈 Sample overflow
    ADC*ERR*SAMPLE*UNDERFLOW = 18,           // 📉 Sample underflow
    ADC*ERR*HARDWARE*FAULT = 19,             // 💥 Hardware fault
    ADC*ERR*COMMUNICATION*FAILURE = 20,      // 📡 Communication failure
    ADC*ERR*DEVICE*NOT*RESPONDING = 21,      // 🔇 Device not responding
    ADC*ERR*CALIBRATION*FAILURE = 22,        // 🔧 Calibration failure
    ADC*ERR*VOLTAGE*OUT*OF*RANGE = 23,       // ⚡ Voltage out of range
    ADC*ERR*INVALID*CONFIGURATION = 24,      // ⚙️ Invalid configuration
    ADC*ERR*UNSUPPORTED*OPERATION = 25,      // 🚫 Unsupported operation
    ADC*ERR*RESOURCE*BUSY = 26,              // 🔄 Resource busy
    ADC*ERR*RESOURCE*UNAVAILABLE = 27,       // 🚫 Resource unavailable
    // Additional calibration errors (28-39)
    ADC*ERR*SYSTEM*ERROR = 40,               // 💻 System error
    ADC*ERR*PERMISSION*DENIED = 41,          // 🔒 Permission denied
    ADC*ERR*OPERATION*ABORTED = 42,          // 🛑 Operation aborted
    ADC*ERR*INITIALIZATION*FAILED = 43,      // 🚀 Initialization failed
    ADC*ERR*INVALID*PARAM = 44,              // 🚫 Invalid parameter
    ADC*ERR*TIMEOUT = 45,                    // ⏰ Operation timeout
    ADC*ERR*NOT*SUPPORTED = 46,              // 🚫 Not supported
    ADC*ERR*INVALID*STATE = 47,              // ⚠️ Invalid state
    ADC*ERR*DRIVER*ERROR = 48,               // 🔧 Driver error
    ADC*ERR*DMA*ERROR = 49,                  // 💾 DMA error
    ADC*ERR*FILTER*ERROR = 50,               // 🔧 Filter configuration error
    ADC*ERR*NO*CALLBACK = 51,                // 📞 No callback provided
    ADC*ERR*NOT*STARTED = 52,                // ⏸️ Operation not started
    ADC*ERR*CALIBRATION = 53,                // 🔧 Calibration error
    ADC*ERR*BUSY = 54,                       // 🔄 Resource busy
    ADC*ERR*HARDWARE*FAILURE = 55,           // 💥 Hardware failure
    ADC*ERR*CHANNEL*DISABLED = 56            // ⚠️ Channel disabled
};
```text

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
    virtual hf*u8*t GetMaxChannels() const noexcept = 0;
    virtual bool IsChannelAvailable(hf*channel*id*t channel*id) const noexcept = 0;
    
    // Reading methods
    virtual hf*adc*err*t ReadChannelV(hf*channel*id*t channel*id,
                                     float& channel*reading*v,
                                     hf*u8*t numOfSamplesToAvg = 1,
                                     hf*time*t timeBetweenSamples = 0) noexcept = 0;
                                     
    virtual hf*adc*err*t ReadChannelCount(hf*channel*id*t channel*id,
                                         hf*u32*t& channel*reading*count,
                                         hf*u8*t numOfSamplesToAvg = 1,
                                         hf*time*t timeBetweenSamples = 0) noexcept = 0;
                                         
    virtual hf*adc*err*t ReadChannelCountAndV(hf*channel*id*t channel*id,
                                             hf*u32*t& channel*reading*count,
                                             float& channel*reading*v,
                                             hf*u8*t numOfSamplesToAvg = 1,
                                             hf*time*t timeBetweenSamples = 0) noexcept = 0;
};
```text

## Reading Methods

### Voltage Reading

```cpp
hf*adc*err*t ReadChannelV(hf*channel*id*t channel*id,
                         float& channel*reading*v,
                         hf*u8*t numOfSamplesToAvg = 1,
                         hf*time*t timeBetweenSamples = 0) noexcept;
```text

**Parameters:**

- `channel*id` - ADC channel identifier (0-based)
- `channel*reading*v` - Reference to store voltage reading in volts
- `numOfSamplesToAvg` - Number of samples to average (default: 1)
- `timeBetweenSamples` - Time between samples in milliseconds (default: 0)

**Returns:** Error code indicating success or failure

### Raw Count Reading

```cpp
hf*adc*err*t ReadChannelCount(hf*channel*id*t channel*id,
                             hf*u32*t& channel*reading*count,
                             hf*u8*t numOfSamplesToAvg = 1,
                             hf*time*t timeBetweenSamples = 0) noexcept;
```text

**Parameters:**

- `channel*id` - ADC channel identifier
- `channel*reading*count` - Reference to store raw ADC count
- `numOfSamplesToAvg` - Number of samples to average
- `timeBetweenSamples` - Time between samples in milliseconds

**Returns:** Error code indicating success or failure

### Combined Reading

```cpp
hf*adc*err*t ReadChannelCountAndV(hf*channel*id*t channel*id,
                                 hf*u32*t& channel*reading*count,
                                 float& channel*reading*v,
                                 hf*u8*t numOfSamplesToAvg = 1,
                                 hf*time*t timeBetweenSamples = 0) noexcept;
```text

Reads both raw count and calibrated voltage in a single operation for
efficiency.

## Usage Examples

### Basic Voltage Reading

```cpp
#include "inc/mcu/esp32/EspAdc.h"

// Create ADC instance
EspAdc adc(ADC*UNIT*1, ADC*ATTEN*DB*11);

// Initialize ADC
if (!adc.EnsureInitialized()) {
    printf("Failed to initialize ADC\n");
    return;
}

// Read voltage from channel 0
float voltage;
hf*adc*err*t result = adc.ReadChannelV(0, voltage);
if (result == hf*adc*err*t::ADC*SUCCESS) {
    printf("Channel 0 voltage: %.3f V\n", voltage);
} else {
    printf("ADC Error: %s\n", HfAdcErrToString(result));
}
```text

### Multi-Sample Averaging

```cpp
// Read with averaging for noise reduction
float voltage;
hf*adc*err*t result = adc.ReadChannelV(0, voltage, 10, 5);  // 10 samples, 5ms between
if (result == hf*adc*err*t::ADC*SUCCESS) {
    printf("Averaged voltage: %.3f V\n", voltage);
}
```text

### Raw Count Reading

```cpp
// Read raw ADC counts
hf*u32*t raw*count;
hf*adc*err*t result = adc.ReadChannelCount(0, raw*count);
if (result == hf*adc*err*t::ADC*SUCCESS) {
    printf("Raw ADC count: %u\n", raw*count);
}
```text

### Combined Reading

```cpp
// Read both raw and calibrated values efficiently
hf*u32*t raw*count;
float voltage;
hf*adc*err*t result = adc.ReadChannelCountAndV(0, raw*count, voltage);
if (result == hf*adc*err*t::ADC*SUCCESS) {
    printf("Raw: %u, Voltage: %.3f V\n", raw*count, voltage);
}
```text

### Multi-Channel Sensor Reading

```cpp
class SensorReader {
private:
    EspAdc adc*;
    
public:
    SensorReader() : adc*(ADC*UNIT*1, ADC*ATTEN*DB*11) {}
    
    bool initialize() {
        return adc*.EnsureInitialized();
    }
    
    void read*all*sensors() {
        // Read current sensor (channel 0)
        float current*voltage;
        if (adc*.ReadChannelV(0, current*voltage, 5) == hf*adc*err*t::ADC*SUCCESS) {
            float current*amps = (current*voltage - 2.5f) / 0.1f;  // ACS712 conversion
            printf("Motor current: %.2f A\n", current*amps);
        }
        
        // Read position sensor (channel 1)
        float position*voltage;
        if (adc*.ReadChannelV(1, position*voltage, 3) == hf*adc*err*t::ADC*SUCCESS) {
            float position*degrees = (position*voltage / 3.3f) * 360.0f;
            printf("Motor position: %.1f degrees\n", position*degrees);
        }
        
        // Read temperature sensor (channel 2)
        float temp*voltage;
        if (adc*.ReadChannelV(2, temp*voltage) == hf*adc*err*t::ADC*SUCCESS) {
            float temperature*c = (temp*voltage - 0.5f) / 0.01f;  // TMP36 conversion
            printf("Temperature: %.1f °C\n", temperature*c);
        }
    }
    
    bool check*channel*availability() {
        printf("Available ADC channels:\n");
        for (hf*u8*t ch = 0; ch < adc*.GetMaxChannels(); ch++) {
            if (adc*.IsChannelAvailable(ch)) {
                printf("  Channel %u: Available\n", ch);
            }
        }
        return true;
    }
};
```text

### Error Handling Best Practices

```cpp
hf*adc*err*t read*sensor*with*retry(BaseAdc& adc, hf*channel*id*t channel, float& voltage) {
    const int max*retries = 3;
    int retry*count = 0;
    
    while (retry*count < max*retries) {
        hf*adc*err*t result = adc.ReadChannelV(channel, voltage, 5, 2);
        
        switch (result) {
            case hf*adc*err*t::ADC*SUCCESS:
                return result;  // Success, return immediately
                
            case hf*adc*err*t::ADC*ERR*BUSY:
            case hf*adc*err*t::ADC*ERR*TIMEOUT:
                // Transient errors - retry
                retry*count++;
                vTaskDelay(pdMS*TO*TICKS(10));  // Wait before retry
                break;
                
            case hf*adc*err*t::ADC*ERR*NOT*INITIALIZED:
                // Try to initialize
                if (!adc.EnsureInitialized()) {
                    return result;  // Initialization failed
                }
                retry*count++;
                break;
                
            default:
                // Permanent error - don't retry
                printf("ADC Error: %s\n", HfAdcErrToString(result));
                return result;
        }
    }
    
    printf("ADC read failed after %d retries\n", max*retries);
    return hf*adc*err*t::ADC*ERR*TIMEOUT;
}
```text

## Utility Functions

```cpp
// Convert error code to string
const char* HfAdcErrToString(hf*adc*err*t err) noexcept;
```text

## Performance Considerations

### Sample Averaging

- Use averaging (`numOfSamplesToAvg > 1`) to reduce noise in noisy
  environments
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

The BaseAdc class is **not thread-safe**. If you need to access ADC from
multiple threads, you must provide your own synchronization mechanisms.

## Implementation Notes

- **Lazy Initialization**: Hardware resources are only allocated when
  `EnsureInitialized()` is called
- **Calibration**: Voltage readings are automatically calibrated based on
  reference voltage
- **Resolution**: Actual resolution depends on the underlying hardware
  (12-bit typical)
- **Reference Voltage**: Configurable reference voltage affects measurement
  range and accuracy

## Derived Classes

The following concrete implementations are available:

- **EspAdc** - ESP32-C6 on-chip ADC implementation
- **I2cAdc** - I2C-based external ADC support
- **SpiAdc** - SPI-based external ADC support

## Related Documentation

- [EspAdc API Reference](../esp_api/EspAdc.md) - ESP32-C6 implementation
- [HardwareTypes Reference](HardwareTypes.md) - Platform-agnostic type
  definitions

---

## 📋 Navigation

[← Previous: BaseGpio](BaseGpio.md) | [Back to API Index](README.md) |
[Next: BasePwm →](BasePwm.md)
