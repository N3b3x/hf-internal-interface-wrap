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
```

## Class Definition

```cpp
class EspAdc : public BaseAdc {
public:
    // Constructor with configuration structure
    explicit EspAdc(const hf_adc_unit_config_t& config) noexcept;
    
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
    hf_u8_t GetMaxChannels() const noexcept override;
    bool IsChannelAvailable(hf_channel_id_t channel_id) const noexcept override;
    
    // Reading operations
    hf_adc_err_t ReadChannelV(hf_channel_id_t channel_id, float& channel_reading_v,
                              hf_u8_t numOfSamplesToAvg = 1,
                              hf_time_t timeBetweenSamples = 0) noexcept override;
    hf_adc_err_t ReadChannelCount(hf_channel_id_t channel_id, hf_u32_t& channel_reading_count,
                                  hf_u8_t numOfSamplesToAvg = 1,
                                  hf_time_t timeBetweenSamples = 0) noexcept override;
    hf_adc_err_t ReadChannel(hf_channel_id_t channel_id, hf_u32_t& channel_reading_count,
                             float& channel_reading_v, hf_u8_t numOfSamplesToAvg = 1,
                             hf_time_t timeBetweenSamples = 0) noexcept override;
    
    // Advanced operations
    hf_adc_err_t SetMode(hf_adc_mode_t mode) noexcept;
    hf_adc_err_t ConfigureChannel(hf_channel_id_t channel_id, hf_adc_atten_t attenuation,
                                  hf_adc_bitwidth_t bitwidth = hf_adc_bitwidth_t::WIDTH_DEFAULT) noexcept;
    hf_adc_err_t EnableChannel(hf_channel_id_t channel_id) noexcept;
    hf_adc_err_t DisableChannel(hf_channel_id_t channel_id) noexcept;
    
    // Continuous mode operations
    hf_adc_err_t ConfigureContinuous(const hf_adc_continuous_config_t& config) noexcept;
    hf_adc_err_t SetContinuousCallback(hf_adc_continuous_callback_t callback, void* user_data = nullptr) noexcept;
    hf_adc_err_t StartContinuous() noexcept;
    hf_adc_err_t StopContinuous() noexcept;
    hf_adc_err_t ReadContinuousData(hf_u8_t* buffer, hf_u32_t buffer_size, hf_u32_t& bytes_read,
                                    hf_time_t timeout_ms) noexcept;
    
    // Calibration operations
    hf_adc_err_t InitializeCalibration(hf_adc_atten_t attenuation,
                                       hf_adc_bitwidth_t bitwidth = hf_adc_bitwidth_t::WIDTH_DEFAULT) noexcept;
    bool IsCalibrationAvailable(hf_adc_atten_t attenuation) const noexcept;
    hf_adc_err_t RawToVoltage(hf_u32_t raw_count, hf_adc_atten_t attenuation, hf_u32_t& voltage_mv) noexcept;
    
    // Filter operations
    hf_adc_err_t ConfigureFilter(const hf_adc_filter_config_t& filter_config) noexcept;
    hf_adc_err_t SetFilterEnabled(hf_u8_t filter_id, bool enabled) noexcept;
    
    // Monitor operations
    hf_adc_err_t ConfigureMonitor(const hf_adc_monitor_config_t& monitor_config) noexcept;
    hf_adc_err_t SetMonitorCallback(hf_u8_t monitor_id, hf_adc_monitor_callback_t callback,
                                    void* user_data = nullptr) noexcept;
    hf_adc_err_t SetMonitorEnabled(hf_u8_t monitor_id, bool enabled) noexcept;
    
    // Diagnostics
    hf_adc_err_t GetStatistics(hf_adc_statistics_t& statistics) noexcept override;
    hf_adc_err_t GetDiagnostics(hf_adc_diagnostics_t& diagnostics) noexcept override;
    hf_adc_err_t ResetStatistics() noexcept override;
};
```

## Configuration Structures

### ADC Unit Configuration

```cpp
struct hf_adc_unit_config_t {
    uint8_t unit_id;                                // ADC unit ID (0 for ADC1, 1 for ADC2)
    hf_adc_mode_t mode;                             // Operating mode (ONESHOT/CONTINUOUS)
    hf_adc_bitwidth_t bit_width;                    // ADC resolution
    hf_adc_channel_config_t channel_configs[7];     // Channel configurations
    hf_adc_continuous_config_t continuous_config;   // Continuous mode settings
    hf_adc_calibration_config_t calibration_config; // Calibration settings
};
```

### Channel Configuration

```cpp
struct hf_adc_channel_config_t {
    hf_channel_id_t channel_id;  // Channel ID (0-6 for ESP32-C6)
    hf_adc_atten_t attenuation;  // Input attenuation level
    hf_adc_bitwidth_t bitwidth;  // Resolution for this channel
    bool enabled;                // Channel enable flag
};
```

### Continuous Mode Configuration

```cpp
struct hf_adc_continuous_config_t {
    uint32_t sample_freq_hz;      // Sampling frequency (10Hz - 100kHz)
    uint32_t samples_per_frame;   // Samples per frame per channel (64-1024)
    uint32_t max_store_frames;    // Maximum frames to store (1-8)
    bool flush_pool;              // Flush pool flag
};
```

## Usage Examples

### Basic One-Shot Reading

```cpp
#include "mcu/esp32/EspAdc.h"

// Configure ADC unit
hf_adc_unit_config_t config = {};
config.unit_id = 0;  // ADC1
config.mode = hf_adc_mode_t::ONESHOT;
config.bit_width = hf_adc_bitwidth_t::WIDTH_12BIT;

// Configure channel 0
config.channel_configs[0].channel_id = 0;
config.channel_configs[0].attenuation = hf_adc_atten_t::ATTEN_DB_12;  // 0-3.3V range
config.channel_configs[0].bitwidth = hf_adc_bitwidth_t::WIDTH_12BIT;
config.channel_configs[0].enabled = true;

// Create and initialize ADC
EspAdc adc(config);
if (!adc.EnsureInitialized()) {
    printf("Failed to initialize ADC\n");
    return;
}

// Read voltage
float voltage;
hf_adc_err_t result = adc.ReadChannelV(0, voltage);
if (result == hf_adc_err_t::ADC_SUCCESS) {
    printf("Channel 0 voltage: %.3f V\n", voltage);
}
```

### Multi-Channel Reading with Averaging

```cpp
// Configure multiple channels
config.channel_configs[1].channel_id = 1;
config.channel_configs[1].attenuation = hf_adc_atten_t::ATTEN_DB_12;
config.channel_configs[1].enabled = true;

config.channel_configs[2].channel_id = 2;
config.channel_configs[2].attenuation = hf_adc_atten_t::ATTEN_DB_6;   // 0-2.2V range
config.channel_configs[2].enabled = true;

EspAdc adc(config);
adc.EnsureInitialized();

// Read multiple channels with averaging
hf_channel_id_t channels[] = {0, 1, 2};
uint32_t raw_readings[3];
float voltages[3];

hf_adc_err_t result = adc.ReadMultipleChannels(channels, 3, raw_readings, voltages);
if (result == hf_adc_err_t::ADC_SUCCESS) {
    for (int i = 0; i < 3; i++) {
        printf("Channel %d: %u counts, %.3f V\n", channels[i], raw_readings[i], voltages[i]);
    }
}

// Read with averaging for noise reduction
float averaged_voltage;
result = adc.ReadChannelV(0, averaged_voltage, 10, 5);  // 10 samples, 5ms between
if (result == hf_adc_err_t::ADC_SUCCESS) {
    printf("Averaged voltage: %.3f V\n", averaged_voltage);
}
```

### Continuous Mode with Callback

```cpp
// Global variables for continuous mode
static QueueHandle_t adc_queue;
static volatile bool data_ready = false;

// ISR-safe callback function
bool adc_continuous_callback(const hf_adc_continuous_data_t* data, void* user_data) {
    // Signal that new data is available
    data_ready = true;
    
    // Send notification to processing task
    BaseType_t higher_priority_task_woken = pdFALSE;
    xQueueSendFromISR(adc_queue, &data->conversion_count, &higher_priority_task_woken);
    
    return higher_priority_task_woken == pdTRUE;
}

// Configure continuous mode
hf_adc_unit_config_t config = {};
config.unit_id = 0;
config.mode = hf_adc_mode_t::CONTINUOUS;
config.continuous_config.sample_freq_hz = 1000;        // 1kHz sampling
config.continuous_config.samples_per_frame = 64;       // 64 samples per frame
config.continuous_config.max_store_frames = 4;         // 4 frame buffer

// Enable channels for continuous sampling
config.channel_configs[0].enabled = true;
config.channel_configs[1].enabled = true;

EspAdc adc(config);
adc.EnsureInitialized();

// Configure continuous mode and set callback
adc.ConfigureContinuous(config.continuous_config);
adc.SetContinuousCallback(adc_continuous_callback, nullptr);

// Start continuous sampling
adc.StartContinuous();

// Process data in main loop
uint8_t buffer[256];
uint32_t bytes_read;
while (true) {
    if (data_ready) {
        data_ready = false;
        
        // Read latest data with zero timeout (non-blocking)
        hf_adc_err_t result = adc.ReadContinuousData(buffer, sizeof(buffer), bytes_read, 0);
        if (result == hf_adc_err_t::ADC_SUCCESS) {
            // Process the data buffer
            process_adc_data(buffer, bytes_read);
        }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
}

// Stop continuous mode
adc.StopContinuous();
```

### Threshold Monitoring

```cpp
// Monitor callback function
void monitor_callback(const hf_adc_monitor_event_t* event, void* user_data) {
    if (event->event_type == hf_adc_monitor_event_type_t::HIGH_THRESH) {
        printf("High threshold exceeded: %u mV\n", event->raw_value);
    } else {
        printf("Below low threshold: %u mV\n", event->raw_value);
    }
}

// Configure continuous mode for monitoring
hf_adc_unit_config_t config = {};
config.unit_id = 0;
config.mode = hf_adc_mode_t::CONTINUOUS;
config.channel_configs[0].enabled = true;

EspAdc adc(config);
adc.EnsureInitialized();
adc.ConfigureContinuous(config.continuous_config);
adc.SetContinuousCallback(adc_continuous_callback, nullptr);

// Configure threshold monitor
hf_adc_monitor_config_t monitor_config = {};
monitor_config.monitor_id = 0;
monitor_config.channel_id = 0;
monitor_config.high_threshold = 3000;  // Raw ADC counts
monitor_config.low_threshold = 1000;   // Raw ADC counts

adc.ConfigureMonitor(monitor_config);
adc.SetMonitorCallback(0, monitor_callback, nullptr);
adc.SetMonitorEnabled(0, true);

// Start continuous mode with monitoring
adc.StartContinuous();
```

### Calibration and Precise Measurements

```cpp
// Initialize calibration for specific attenuation
hf_adc_err_t result = adc.InitializeCalibration(hf_adc_atten_t::ATTEN_DB_12);
if (result == hf_adc_err_t::ADC_SUCCESS) {
    printf("Calibration initialized successfully\n");
} else {
    printf("Calibration not available, using linear conversion\n");
}

// Check calibration availability
if (adc.IsCalibrationAvailable(hf_adc_atten_t::ATTEN_DB_12)) {
    // Read raw value and convert using calibration
    uint32_t raw_value;
    adc.ReadSingleRaw(0, raw_value);
    
    uint32_t calibrated_voltage_mv;
    result = adc.RawToVoltage(raw_value, hf_adc_atten_t::ATTEN_DB_12, calibrated_voltage_mv);
    if (result == hf_adc_err_t::ADC_SUCCESS) {
        printf("Calibrated voltage: %u mV\n", calibrated_voltage_mv);
    }
}
```

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

The `EspAdc` class provides comprehensive error reporting through the `hf_adc_err_t` enumeration:

- `ADC_SUCCESS` - Operation completed successfully
- `ADC_ERR_NOT_INITIALIZED` - ADC not initialized
- `ADC_ERR_INVALID_CHANNEL` - Invalid channel ID
- `ADC_ERR_CHANNEL_NOT_ENABLED` - Channel not enabled
- `ADC_ERR_CALIBRATION` - Calibration error
- `ADC_ERR_TIMEOUT` - Operation timeout
- `ADC_ERR_BUSY` - Resource busy
- `ADC_ERR_HARDWARE_FAILURE` - Hardware failure

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
- **[EspTypes_ADC.h](../../inc/mcu/esp32/utils/EspTypes_ADC.h)** - Type definitions and utilities
- **[ADC Test Suite](../../examples/esp32/docs/README_ADC_TEST.md)** - Comprehensive testing documentation
<!-- markdownlint-disable-next-line MD013 -->
- **[ESP-IDF ADC Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/adc.html)** - ESP-IDF docs

---

<div align="center">

**📋 Navigation**

[← Previous: EspGpio](EspGpio.md) | [Back to ESP API Index](README.md) | [Next: EspPwm →](EspPwm.md)

</div>