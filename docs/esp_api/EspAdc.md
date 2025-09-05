# 📊 EspAdc API Reference

<div align="center">

**📋 Navigation**

[← Previous: EspGpio](EspGpio.md) | [Back to ESP API Index](README.md) | [Next: EspPwm →](EspPwm.md)

</div>

---

## Overview

`EspAdc` provides ESP32 ADC (Analog-to-Digital Converter) functionality with comprehensive support
for all ESP32 variants using ESP-IDF v5.5+.
It implements the `BaseAdc` interface with hardware-specific optimizations for one-shot and
continuous sampling modes, calibration, filtering, and threshold monitoring.

## Features

- **Multi-Variant Support** - ESP32-C6, ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C2, ESP32-H2
- **Dual Operation Modes** - One-shot and continuous (DMA) sampling
- **Hardware Calibration** - Automatic calibration using eFuse data
- **Digital Filtering** - Up to 2 IIR filters for noise reduction
- **Threshold Monitoring** - Up to 2 monitors with ISR callbacks
- **Multi-Channel Support** - Configurable channels with individual settings
- **Thread Safety** - Mutex-protected operations for multi-threaded access
- **Comprehensive Diagnostics** - Statistics tracking and error reporting

## Header File

```cpp
#include "mcu/esp32/EspAdc.h"
```text

## Class Definition

```cpp
class EspAdc : public BaseAdc {
public:
    // Constructor with configuration structure
    explicit EspAdc(const hf*adc*unit*config*t& config) noexcept;
    
    // Destructor with proper cleanup
    ~EspAdc() noexcept override;
    
    // Copy and move operations disabled for resource safety
    EspAdc(const EspAdc&) = delete;
    EspAdc& operator=(const EspAdc&) = delete;
    EspAdc(EspAdc&&) = delete;
    EspAdc& operator=(EspAdc&&) = delete;
    
    // BaseAdc interface implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    hf*u8*t GetMaxChannels() const noexcept override;
    bool IsChannelAvailable(hf*channel*id*t channel*id) const noexcept override;
    
    // Reading operations
    hf*adc*err*t ReadChannelV(hf*channel*id*t channel*id, float& channel*reading*v,
                              hf*u8*t numOfSamplesToAvg = 1,
                              hf*time*t timeBetweenSamples = 0) noexcept override;
    hf*adc*err*t ReadChannelCount(hf*channel*id*t channel*id, hf*u32*t& channel*reading*count,
                                  hf*u8*t numOfSamplesToAvg = 1,
                                  hf*time*t timeBetweenSamples = 0) noexcept override;
    hf*adc*err*t ReadChannel(hf*channel*id*t channel*id, hf*u32*t& channel*reading*count,
                             float& channel*reading*v, hf*u8*t numOfSamplesToAvg = 1,
                             hf*time*t timeBetweenSamples = 0) noexcept override;
    
    // Advanced operations
    hf*adc*err*t SetMode(hf*adc*mode*t mode) noexcept;
    hf*adc*err*t ConfigureChannel(hf*channel*id*t channel*id, hf*adc*atten*t attenuation,
                                  hf*adc*bitwidth*t bitwidth = hf*adc*bitwidth*t::WIDTH*DEFAULT) noexcept;
    hf*adc*err*t EnableChannel(hf*channel*id*t channel*id) noexcept;
    hf*adc*err*t DisableChannel(hf*channel*id*t channel*id) noexcept;
    
    // Continuous mode operations
    hf*adc*err*t ConfigureContinuous(const hf*adc*continuous*config*t& config) noexcept;
    hf*adc*err*t SetContinuousCallback(hf*adc*continuous*callback*t callback, void* user*data = nullptr) noexcept;
    hf*adc*err*t StartContinuous() noexcept;
    hf*adc*err*t StopContinuous() noexcept;
    hf*adc*err*t ReadContinuousData(hf*u8*t* buffer, hf*u32*t buffer*size, hf*u32*t& bytes*read,
                                    hf*time*t timeout*ms) noexcept;
    
    // Calibration operations
    hf*adc*err*t InitializeCalibration(hf*adc*atten*t attenuation,
                                       hf*adc*bitwidth*t bitwidth = hf*adc*bitwidth*t::WIDTH*DEFAULT) noexcept;
    bool IsCalibrationAvailable(hf*adc*atten*t attenuation) const noexcept;
    hf*adc*err*t RawToVoltage(hf*u32*t raw*count, hf*adc*atten*t attenuation, hf*u32*t& voltage*mv) noexcept;
    
    // Filter operations
    hf*adc*err*t ConfigureFilter(const hf*adc*filter*config*t& filter*config) noexcept;
    hf*adc*err*t SetFilterEnabled(hf*u8*t filter*id, bool enabled) noexcept;
    
    // Monitor operations
    hf*adc*err*t ConfigureMonitor(const hf*adc*monitor*config*t& monitor*config) noexcept;
    hf*adc*err*t SetMonitorCallback(hf*u8*t monitor*id, hf*adc*monitor*callback*t callback,
                                    void* user*data = nullptr) noexcept;
    hf*adc*err*t SetMonitorEnabled(hf*u8*t monitor*id, bool enabled) noexcept;
    
    // Diagnostics
    hf*adc*err*t GetStatistics(hf*adc*statistics*t& statistics) noexcept override;
    hf*adc*err*t GetDiagnostics(hf*adc*diagnostics*t& diagnostics) noexcept override;
    hf*adc*err*t ResetStatistics() noexcept override;
};
```text

## Configuration Structures

### ADC Unit Configuration

```cpp
struct hf*adc*unit*config*t {
    uint8*t unit*id;                                // ADC unit ID (0 for ADC1, 1 for ADC2)
    hf*adc*mode*t mode;                             // Operating mode (ONESHOT/CONTINUOUS)
    hf*adc*bitwidth*t bit*width;                    // ADC resolution
    hf*adc*channel*config*t channel*configs[7];     // Channel configurations
    hf*adc*continuous*config*t continuous*config;   // Continuous mode settings
    hf*adc*calibration*config*t calibration*config; // Calibration settings
};
```text

### Channel Configuration

```cpp
struct hf*adc*channel*config*t {
    hf*channel*id*t channel*id;  // Channel ID (0-6 for ESP32-C6)
    hf*adc*atten*t attenuation;  // Input attenuation level
    hf*adc*bitwidth*t bitwidth;  // Resolution for this channel
    bool enabled;                // Channel enable flag
};
```text

### Continuous Mode Configuration

```cpp
struct hf*adc*continuous*config*t {
    uint32*t sample*freq*hz;      // Sampling frequency (10Hz - 100kHz)
    uint32*t samples*per*frame;   // Samples per frame per channel (64-1024)
    uint32*t max*store*frames;    // Maximum frames to store (1-8)
    bool flush*pool;              // Flush pool flag
};
```text

## Usage Examples

### Basic One-Shot Reading

```cpp
#include "mcu/esp32/EspAdc.h"

// Configure ADC unit
hf*adc*unit*config*t config = {};
config.unit*id = 0;  // ADC1
config.mode = hf*adc*mode*t::ONESHOT;
config.bit*width = hf*adc*bitwidth*t::WIDTH*12BIT;

// Configure channel 0
config.channel*configs[0].channel*id = 0;
config.channel*configs[0].attenuation = hf*adc*atten*t::ATTEN*DB*12;  // 0-3.3V range
config.channel*configs[0].bitwidth = hf*adc*bitwidth*t::WIDTH*12BIT;
config.channel*configs[0].enabled = true;

// Create and initialize ADC
EspAdc adc(config);
if (!adc.EnsureInitialized()) {
    printf("Failed to initialize ADC\n");
    return;
}

// Read voltage
float voltage;
hf*adc*err*t result = adc.ReadChannelV(0, voltage);
if (result == hf*adc*err*t::ADC*SUCCESS) {
    printf("Channel 0 voltage: %.3f V\n", voltage);
}
```text

### Multi-Channel Reading with Averaging

```cpp
// Configure multiple channels
config.channel*configs[1].channel*id = 1;
config.channel*configs[1].attenuation = hf*adc*atten*t::ATTEN*DB*12;
config.channel*configs[1].enabled = true;

config.channel*configs[2].channel*id = 2;
config.channel*configs[2].attenuation = hf*adc*atten*t::ATTEN*DB*6;   // 0-2.2V range
config.channel*configs[2].enabled = true;

EspAdc adc(config);
adc.EnsureInitialized();

// Read multiple channels with averaging
hf*channel*id*t channels[] = {0, 1, 2};
uint32*t raw*readings[3];
float voltages[3];

hf*adc*err*t result = adc.ReadMultipleChannels(channels, 3, raw*readings, voltages);
if (result == hf*adc*err*t::ADC*SUCCESS) {
    for (int i = 0; i < 3; i++) {
        printf("Channel %d: %u counts, %.3f V\n", channels[i], raw*readings[i], voltages[i]);
    }
}

// Read with averaging for noise reduction
float averaged*voltage;
result = adc.ReadChannelV(0, averaged*voltage, 10, 5);  // 10 samples, 5ms between
if (result == hf*adc*err*t::ADC*SUCCESS) {
    printf("Averaged voltage: %.3f V\n", averaged*voltage);
}
```text

### Continuous Mode with Callback

```cpp
// Global variables for continuous mode
static QueueHandle*t adc*queue;
static volatile bool data*ready = false;

// ISR-safe callback function
bool adc*continuous*callback(const hf*adc*continuous*data*t* data, void* user*data) {
    // Signal that new data is available
    data*ready = true;
    
    // Send notification to processing task
    BaseType*t higher*priority*task*woken = pdFALSE;
    xQueueSendFromISR(adc*queue, &data->conversion*count, &higher*priority*task*woken);
    
    return higher*priority*task*woken == pdTRUE;
}

// Configure continuous mode
hf*adc*unit*config*t config = {};
config.unit*id = 0;
config.mode = hf*adc*mode*t::CONTINUOUS;
config.continuous*config.sample*freq*hz = 1000;        // 1kHz sampling
config.continuous*config.samples*per*frame = 64;       // 64 samples per frame
config.continuous*config.max*store*frames = 4;         // 4 frame buffer

// Enable channels for continuous sampling
config.channel*configs[0].enabled = true;
config.channel*configs[1].enabled = true;

EspAdc adc(config);
adc.EnsureInitialized();

// Configure continuous mode and set callback
adc.ConfigureContinuous(config.continuous*config);
adc.SetContinuousCallback(adc*continuous*callback, nullptr);

// Start continuous sampling
adc.StartContinuous();

// Process data in main loop
uint8*t buffer[256];
uint32*t bytes*read;
while (true) {
    if (data*ready) {
        data*ready = false;
        
        // Read latest data with zero timeout (non-blocking)
        hf*adc*err*t result = adc.ReadContinuousData(buffer, sizeof(buffer), bytes*read, 0);
        if (result == hf*adc*err*t::ADC*SUCCESS) {
            // Process the data buffer
            process*adc*data(buffer, bytes*read);
        }
    }
    vTaskDelay(pdMS*TO*TICKS(10));
}

// Stop continuous mode
adc.StopContinuous();
```text

### Threshold Monitoring

```cpp
// Monitor callback function
void monitor*callback(const hf*adc*monitor*event*t* event, void* user*data) {
    if (event->event*type == hf*adc*monitor*event*type*t::HIGH*THRESH) {
        printf("High threshold exceeded: %u mV\n", event->raw*value);
    } else {
        printf("Below low threshold: %u mV\n", event->raw*value);
    }
}

// Configure continuous mode for monitoring
hf*adc*unit*config*t config = {};
config.unit*id = 0;
config.mode = hf*adc*mode*t::CONTINUOUS;
config.channel*configs[0].enabled = true;

EspAdc adc(config);
adc.EnsureInitialized();
adc.ConfigureContinuous(config.continuous*config);
adc.SetContinuousCallback(adc*continuous*callback, nullptr);

// Configure threshold monitor
hf*adc*monitor*config*t monitor*config = {};
monitor*config.monitor*id = 0;
monitor*config.channel*id = 0;
monitor*config.high*threshold = 3000;  // Raw ADC counts
monitor*config.low*threshold = 1000;   // Raw ADC counts

adc.ConfigureMonitor(monitor*config);
adc.SetMonitorCallback(0, monitor*callback, nullptr);
adc.SetMonitorEnabled(0, true);

// Start continuous mode with monitoring
adc.StartContinuous();
```text

### Calibration and Precise Measurements

```cpp
// Initialize calibration for specific attenuation
hf*adc*err*t result = adc.InitializeCalibration(hf*adc*atten*t::ATTEN*DB*12);
if (result == hf*adc*err*t::ADC*SUCCESS) {
    printf("Calibration initialized successfully\n");
} else {
    printf("Calibration not available, using linear conversion\n");
}

// Check calibration availability
if (adc.IsCalibrationAvailable(hf*adc*atten*t::ATTEN*DB*12)) {
    // Read raw value and convert using calibration
    uint32*t raw*value;
    adc.ReadSingleRaw(0, raw*value);
    
    uint32*t calibrated*voltage*mv;
    result = adc.RawToVoltage(raw*value, hf*adc*atten*t::ATTEN*DB*12, calibrated*voltage*mv);
    if (result == hf*adc*err*t::ADC*SUCCESS) {
        printf("Calibrated voltage: %u mV\n", calibrated*voltage*mv);
    }
}
```text

## ESP32 Variant Specifications

### ESP32-C6

- **ADC Units**: 1 (ADC1)
- **Channels**: 7 (0-6)
- **Resolution**: 12-bit (4096 levels)
- **Sampling Rate**: 10 Hz - 100 kHz
- **Input Range**: 0-3.3V (with 12dB attenuation)
- **Filters**: 2 IIR filters
- **Monitors**: 2 threshold monitors

### ESP32 Classic

- **ADC Units**: 2 (ADC1, ADC2)
- **Channels**: 8 per unit (0-7)
- **Resolution**: 12-bit (4096 levels)
- **Sampling Rate**: 10 Hz - 200 kHz
- **Input Range**: 0-3.3V (with 12dB attenuation)
- **Filters**: 2 IIR filters
- **Monitors**: 2 threshold monitors

## Attenuation Levels

| Attenuation | Input Range | Use Case |

|-------------|-------------|----------|

| 0dB | 0-0.95V | Low voltage sensors |

| 2.5dB | 0-1.32V | 1.2V logic levels |

| 6dB | 0-1.98V | 1.8V logic levels |

| 12dB | 0-3.3V | Full voltage range |

## Error Handling

The `EspAdc` class provides comprehensive error reporting through the `hf*adc*err*t` enumeration:

- `ADC*SUCCESS` - Operation completed successfully
- `ADC*ERR*NOT*INITIALIZED` - ADC not initialized
- `ADC*ERR*INVALID*CHANNEL` - Invalid channel ID
- `ADC*ERR*CHANNEL*NOT*ENABLED` - Channel not enabled
- `ADC*ERR*CALIBRATION` - Calibration error
- `ADC*ERR*TIMEOUT` - Operation timeout
- `ADC*ERR*BUSY` - Resource busy
- `ADC*ERR*HARDWARE*FAILURE` - Hardware failure

## Performance Considerations

- **One-Shot Mode**: ~50µs per conversion (including calibration)
- **Continuous Mode**: Sustained sampling up to maximum frequency
- **Calibration**: Improves accuracy by ±10mV typically
- **Filtering**: Reduces noise at the cost of response time
- **Multi-Channel**: Round-robin sampling in continuous mode

## Thread Safety

The `EspAdc` class uses mutex protection for thread-safe operation.
Multiple threads can safely call ADC methods simultaneously.

## Related Documentation

- **[BaseAdc API Reference](../api/BaseAdc.md)** - Base class interface
- **[EspTypesADC.h](../../inc/mcu/esp32/utils/EspTypesADC.h)** - Type definitions and utilities
- **[ADC Test Suite](../../examples/esp32/docs/README_ADC_TEST.md)** - Comprehensive testing documentation
- **[ESP-IDF ADC Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/adc.html)** - ESP-IDF documentation

---

<div align="center">

**📋 Navigation**

[← Previous: EspGpio](EspGpio.md) | [Back to ESP API Index](README.md) | [Next: EspPwm →](EspPwm.md)

</div>