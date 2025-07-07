# Portable ESP32 ADC Usage Guide

This guide demonstrates how to use the portable `EspAdc` class across different ESP32 variants.

## Overview

The `EspAdc` class is designed to be portable across all ESP32 variants while maintaining single ADC unit encapsulation. Each `EspAdc` instance represents one ADC unit, and higher-level applications should instantiate multiple objects for multi-unit boards.

## Supported ESP32 Variants

| Variant | ADC Units | Channels per Unit | Max Sampling | Notes |
|---------|-----------|-------------------|--------------|-------|
| ESP32-C6 | 1 | 7 | 100kSPS | Single unit, 7 channels |
| ESP32 Classic | 2 | 8 | 200kSPS | Dual units, 8 channels each |
| ESP32-S2 | 1 | 10 | 200kSPS | Single unit, 10 channels |
| ESP32-S3 | 2 | 10 | 200kSPS | Dual units, 10 channels each |
| ESP32-C3 | 1 | 6 | 100kSPS | Single unit, 6 channels |
| ESP32-C2 | 1 | 4 | 100kSPS | Single unit, 4 channels |
| ESP32-H2 | 1 | 6 | 100kSPS | Single unit, 6 channels |

## Configuration

### Step 1: Select Your ESP32 Variant

In `inc/mcu/utils/McuSelect.h`, uncomment exactly one target:

```cpp
// For ESP32-C6
#define HF_TARGET_MCU_ESP32C6

// For ESP32 Classic
// #define HF_TARGET_MCU_ESP32

// For ESP32-S2
// #define HF_TARGET_MCU_ESP32S2

// For ESP32-S3
// #define HF_TARGET_MCU_ESP32S3

// For ESP32-C3
// #define HF_TARGET_MCU_ESP32C3

// For ESP32-C2
// #define HF_TARGET_MCU_ESP32C2

// For ESP32-H2
// #define HF_TARGET_MCU_ESP32H2
```

### Step 2: Configure ADC Units

The configuration is automatically handled based on your ESP32 variant selection. The class will use the correct:
- Maximum number of channels
- GPIO mappings
- Sampling frequencies
- Hardware limits

## Usage Examples

### Example 1: Single ADC Unit (ESP32-C6, ESP32-S2, ESP32-C3, ESP32-C2, ESP32-H2)

```cpp
#include "EspAdc.h"

// Create ADC unit configuration
hf_adc_unit_config_t adc_config = {
    .unit_id = 0,  // ADC1 (only unit available)
    .mode = hf_adc_mode_t::ONESHOT,
    .bit_width = hf_adc_bitwidth_t::WIDTH_12_BIT,
    .calibration_config = {
        .enable_calibration = true
    },
    .channel_configs = {
        {.channel_id = 0, .enabled = true, .attenuation = hf_adc_atten_t::ATTEN_DB_11, .bitwidth = hf_adc_bitwidth_t::WIDTH_12_BIT},
        {.channel_id = 1, .enabled = true, .attenuation = hf_adc_atten_t::ATTEN_DB_11, .bitwidth = hf_adc_bitwidth_t::WIDTH_12_BIT},
        // ... configure other channels as needed
    }
};

// Create ADC instance
EspAdc adc(adc_config);

// Initialize
if (adc.EnsureInitialized()) {
    // Read voltage from channel 0
    float voltage;
    if (adc.ReadChannelV(0, voltage) == hf_adc_err_t::ADC_SUCCESS) {
        printf("Channel 0 voltage: %.3f V\n", voltage);
    }
    
    // Read raw count from channel 1
    uint32_t raw_count;
    if (adc.ReadChannelCount(1, raw_count) == hf_adc_err_t::ADC_SUCCESS) {
        printf("Channel 1 raw count: %lu\n", raw_count);
    }
}
```

### Example 2: Dual ADC Units (ESP32 Classic, ESP32-S3)

```cpp
#include "EspAdc.h"

// ADC1 configuration
hf_adc_unit_config_t adc1_config = {
    .unit_id = 0,  // ADC1
    .mode = hf_adc_mode_t::ONESHOT,
    .bit_width = hf_adc_bitwidth_t::WIDTH_12_BIT,
    .calibration_config = {
        .enable_calibration = true
    },
    .channel_configs = {
        {.channel_id = 0, .enabled = true, .attenuation = hf_adc_atten_t::ATTEN_DB_11, .bitwidth = hf_adc_bitwidth_t::WIDTH_12_BIT},
        {.channel_id = 1, .enabled = true, .attenuation = hf_adc_atten_t::ATTEN_DB_11, .bitwidth = hf_adc_bitwidth_t::WIDTH_12_BIT},
        // ... configure other channels
    }
};

// ADC2 configuration
hf_adc_unit_config_t adc2_config = {
    .unit_id = 1,  // ADC2
    .mode = hf_adc_mode_t::ONESHOT,
    .bit_width = hf_adc_bitwidth_t::WIDTH_12_BIT,
    .calibration_config = {
        .enable_calibration = true
    },
    .channel_configs = {
        {.channel_id = 0, .enabled = true, .attenuation = hf_adc_atten_t::ATTEN_DB_11, .bitwidth = hf_adc_bitwidth_t::WIDTH_12_BIT},
        {.channel_id = 1, .enabled = true, .attenuation = hf_adc_atten_t::ATTEN_DB_11, .bitwidth = hf_adc_bitwidth_t::WIDTH_12_BIT},
        // ... configure other channels
    }
};

// Create ADC instances
EspAdc adc1(adc1_config);
EspAdc adc2(adc2_config);

// Initialize both units
if (adc1.EnsureInitialized() && adc2.EnsureInitialized()) {
    // Read from ADC1 channel 0
    float voltage1;
    if (adc1.ReadChannelV(0, voltage1) == hf_adc_err_t::ADC_SUCCESS) {
        printf("ADC1 Channel 0 voltage: %.3f V\n", voltage1);
    }
    
    // Read from ADC2 channel 0
    float voltage2;
    if (adc2.ReadChannelV(0, voltage2) == hf_adc_err_t::ADC_SUCCESS) {
        printf("ADC2 Channel 0 voltage: %.3f V\n", voltage2);
    }
}
```

### Example 3: Continuous Mode with Callbacks

```cpp
#include "EspAdc.h"

// Continuous mode configuration
hf_adc_unit_config_t adc_config = {
    .unit_id = 0,
    .mode = hf_adc_mode_t::CONTINUOUS,
    .bit_width = hf_adc_bitwidth_t::WIDTH_12_BIT,
    .continuous_config = {
        .sample_freq_hz = 1000,      // 1kHz sampling
        .conv_frame_size = 1024,     // 1024 samples per frame
        .flush_pool = false
    },
    .channel_configs = {
        {.channel_id = 0, .enabled = true, .attenuation = hf_adc_atten_t::ATTEN_DB_11, .bitwidth = hf_adc_bitwidth_t::WIDTH_12_BIT},
        {.channel_id = 1, .enabled = true, .attenuation = hf_adc_atten_t::ATTEN_DB_11, .bitwidth = hf_adc_bitwidth_t::WIDTH_12_BIT},
    }
};

// Create ADC instance
EspAdc adc(adc_config);

// Continuous data callback
bool continuous_callback(const hf_adc_continuous_data_t* data, void* user_data) {
    printf("Received %lu bytes of continuous data\n", data->size);
    
    // Process the data here
    // data->buffer contains the raw ADC data
    // data->conversion_count contains number of conversions
    // data->timestamp_us contains timestamp
    
    return false; // Return false to continue receiving callbacks
}

// Initialize and start continuous mode
if (adc.EnsureInitialized()) {
    // Set callback
    adc.SetContinuousCallback(continuous_callback, nullptr);
    
    // Start continuous sampling
    if (adc.StartContinuous() == hf_adc_err_t::ADC_SUCCESS) {
        printf("Continuous mode started\n");
        
        // Let it run for a while
        vTaskDelay(pdMS_TO_TICKS(5000));
        
        // Stop continuous mode
        adc.StopContinuous();
        printf("Continuous mode stopped\n");
    }
}
```

### Example 4: Advanced Features (Filters and Monitors)

```cpp
#include "EspAdc.h"

// Configuration with advanced features
hf_adc_unit_config_t adc_config = {
    .unit_id = 0,
    .mode = hf_adc_mode_t::CONTINUOUS,
    .bit_width = hf_adc_bitwidth_t::WIDTH_12_BIT,
    .continuous_config = {
        .sample_freq_hz = 1000,
        .conv_frame_size = 1024,
        .flush_pool = false
    },
    .channel_configs = {
        {.channel_id = 0, .enabled = true, .attenuation = hf_adc_atten_t::ATTEN_DB_11, .bitwidth = hf_adc_bitwidth_t::WIDTH_12_BIT},
    }
};

EspAdc adc(adc_config);

// Monitor callback
bool monitor_callback(const hf_adc_monitor_event_t* event, void* user_data) {
    if (event->event_type == hf_adc_monitor_event_type_t::HIGH_THRESH) {
        printf("High threshold exceeded on monitor %d, channel %d, value: %lu\n", 
               event->monitor_id, event->channel_id, event->raw_value);
    } else {
        printf("Low threshold exceeded on monitor %d, channel %d, value: %lu\n", 
               event->monitor_id, event->channel_id, event->raw_value);
    }
    return false;
}

if (adc.EnsureInitialized()) {
    // Configure IIR filter for noise reduction
    hf_adc_filter_config_t filter_config = {
        .filter_id = 0,
        .channel_id = 0,
        .coefficient = hf_adc_iir_filter_coeff_t::IIR_FILTER_COEFF_2
    };
    adc.ConfigureFilter(filter_config);
    adc.SetFilterEnabled(0, true);
    
    // Configure threshold monitor
    hf_adc_monitor_config_t monitor_config = {
        .monitor_id = 0,
        .channel_id = 0,
        .high_threshold = 3000,  // High threshold
        .low_threshold = 1000    // Low threshold
    };
    adc.ConfigureMonitor(monitor_config);
    adc.SetMonitorCallback(0, monitor_callback, nullptr);
    adc.SetMonitorEnabled(0, true);
    
    // Start continuous mode
    adc.SetContinuousCallback(continuous_callback, nullptr);
    adc.StartContinuous();
}
```

## GPIO Mappings

The GPIO mappings are automatically handled based on your ESP32 variant. Here are the mappings for reference:

### ESP32-C6
- Channel 0 → GPIO0
- Channel 1 → GPIO1
- Channel 2 → GPIO2
- Channel 3 → GPIO3
- Channel 4 → GPIO4
- Channel 5 → GPIO5
- Channel 6 → GPIO6

### ESP32 Classic
**ADC1:**
- Channel 0 → GPIO36
- Channel 1 → GPIO37
- Channel 2 → GPIO38
- Channel 3 → GPIO39
- Channel 4 → GPIO32
- Channel 5 → GPIO33
- Channel 6 → GPIO34
- Channel 7 → GPIO35

**ADC2:**
- Channel 0 → GPIO4
- Channel 1 → GPIO0
- Channel 2 → GPIO2
- Channel 3 → GPIO15
- Channel 4 → GPIO13
- Channel 5 → GPIO12
- Channel 6 → GPIO14
- Channel 7 → GPIO27

### ESP32-S2
- Channel 0 → GPIO1
- Channel 1 → GPIO2
- Channel 2 → GPIO3
- Channel 3 → GPIO4
- Channel 4 → GPIO5
- Channel 5 → GPIO6
- Channel 6 → GPIO7
- Channel 7 → GPIO8
- Channel 8 → GPIO9
- Channel 9 → GPIO10

### ESP32-S3
**ADC1:**
- Channel 0 → GPIO1
- Channel 1 → GPIO2
- Channel 2 → GPIO3
- Channel 3 → GPIO4
- Channel 4 → GPIO5
- Channel 5 → GPIO6
- Channel 6 → GPIO7
- Channel 7 → GPIO8
- Channel 8 → GPIO9
- Channel 9 → GPIO10

**ADC2:**
- Channel 0 → GPIO11
- Channel 1 → GPIO12
- Channel 2 → GPIO13
- Channel 3 → GPIO14
- Channel 4 → GPIO15
- Channel 5 → GPIO16
- Channel 6 → GPIO17
- Channel 7 → GPIO18
- Channel 8 → GPIO19
- Channel 9 → GPIO20

### ESP32-C3
- Channel 0 → GPIO0
- Channel 1 → GPIO1
- Channel 2 → GPIO2
- Channel 3 → GPIO3
- Channel 4 → GPIO4
- Channel 5 → GPIO5

### ESP32-C2
- Channel 0 → GPIO0
- Channel 1 → GPIO1
- Channel 2 → GPIO2
- Channel 3 → GPIO3

### ESP32-H2
- Channel 0 → GPIO0
- Channel 1 → GPIO1
- Channel 2 → GPIO2
- Channel 3 → GPIO3
- Channel 4 → GPIO4
- Channel 5 → GPIO5

## Best Practices

1. **Always check return values** from ADC operations
2. **Use proper error handling** for robust applications
3. **Initialize calibration** for accurate voltage readings
4. **Configure channels before use** with appropriate attenuation
5. **Use filters** for noise reduction in continuous mode
6. **Use monitors** for threshold detection
7. **Handle multi-unit boards** by creating separate EspAdc instances
8. **Check hardware limits** for your specific ESP32 variant

## Error Handling

The EspAdc class provides comprehensive error codes. Always check return values:

```cpp
hf_adc_err_t result = adc.ReadChannelV(0, voltage);
if (result != hf_adc_err_t::ADC_SUCCESS) {
    printf("ADC error: %s\n", HfAdcErrToString(result).data());
    // Handle error appropriately
}
```

## Thread Safety

The EspAdc class is thread-safe and can be used from multiple threads. All operations are protected by mutexes internally.

## Performance Considerations

- **One-shot mode**: Best for occasional readings
- **Continuous mode**: Best for high-frequency sampling
- **Filters**: Reduce noise but add latency
- **Calibration**: Improves accuracy but uses more memory
- **Multi-unit**: Each unit operates independently

This portable design allows you to write code once and use it across all ESP32 variants by simply changing the MCU selection in `McuSelect.h`. 