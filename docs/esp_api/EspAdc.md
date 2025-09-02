# EspAdc API Reference

## Overview

`EspAdc` is the ESP32-C6 implementation of the `BaseAdc` interface, providing comprehensive ADC (Analog-to-Digital Converter) functionality specifically optimized for ESP32-C6 microcontrollers running ESP-IDF v5.5+. It offers both basic and advanced ADC features with hardware-specific optimizations.

## Features

- **ESP32-C6 SAR ADC** - Full support for ESP32-C6 SAR (Successive Approximation Register) ADC
- **12-bit Resolution** - High-resolution analog-to-digital conversion
- **Multiple Channels** - Support for multiple ADC channels
- **Multiple Units** - ADC1 and ADC2 unit support
- **Attenuation Control** - Configurable input attenuation
- **Calibration** - Built-in calibration support
- **Power Management** - Deep sleep compatibility
- **Performance Optimized** - Hardware-accelerated operations

## Header File

```cpp
#include "inc/mcu/esp32/EspAdc.h"
```

## Class Definition

```cpp
class EspAdc : public BaseAdc {
public:
    // Constructor with full configuration
    explicit EspAdc(
        hf_adc_unit_t unit = hf_adc_unit_t::HF_ADC_UNIT_1,
        hf_adc_channel_t channel = hf_adc_channel_t::HF_ADC_CHANNEL_0,
        hf_adc_atten_t attenuation = hf_adc_atten_t::HF_ADC_ATTEN_DB_11,
        hf_adc_width_t width = hf_adc_width_t::HF_ADC_WIDTH_BIT_12
    ) noexcept;

    // Destructor
    ~EspAdc() override;

    // BaseAdc implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // ADC operations
    hf_adc_err_t ReadRaw(hf_adc_raw_t* raw_value) noexcept override;
    hf_adc_err_t ReadVoltage(hf_adc_voltage_t* voltage) noexcept override;
    hf_adc_err_t ReadMillivolts(hf_adc_millivolts_t* millivolts) noexcept override;
    hf_adc_err_t GetResolution(hf_adc_resolution_t* resolution) const noexcept override;
    hf_adc_err_t GetMaxVoltage(hf_adc_voltage_t* max_voltage) const noexcept override;

    // Advanced features
    hf_adc_err_t SetAttenuation(hf_adc_atten_t attenuation) noexcept override;
    hf_adc_err_t GetAttenuation(hf_adc_atten_t* attenuation) const noexcept override;
    hf_adc_err_t SetWidth(hf_adc_width_t width) noexcept override;
    hf_adc_err_t GetWidth(hf_adc_width_t* width) const noexcept override;
    hf_adc_err_t Calibrate() noexcept override;
    hf_adc_err_t IsCalibrated(bool* calibrated) const noexcept override;
};
```

## Usage Examples

### Basic ADC Reading

```cpp
#include "inc/mcu/esp32/EspAdc.h"

// Create ADC instance
EspAdc adc(HF_ADC_UNIT_1, HF_ADC_CHANNEL_0, HF_ADC_ATTEN_DB_11, HF_ADC_WIDTH_BIT_12);

// Initialize
if (!adc.Initialize()) {
    printf("Failed to initialize ADC\n");
    return;
}

// Read raw value
hf_adc_raw_t raw_value;
hf_adc_err_t err = adc.ReadRaw(&raw_value);
if (err == HF_ADC_ERR_OK) {
    printf("Raw ADC value: %d\n", raw_value);
}

// Read voltage
hf_adc_voltage_t voltage;
err = adc.ReadVoltage(&voltage);
if (err == HF_ADC_ERR_OK) {
    printf("Voltage: %.3f V\n", voltage);
}

// Read millivolts
hf_adc_millivolts_t millivolts;
err = adc.ReadMillivolts(&millivolts);
if (err == HF_ADC_ERR_OK) {
    printf("Millivolts: %d mV\n", millivolts);
}
```

### Multiple ADC Channels

```cpp
// Create multiple ADC channels
EspAdc adc1(HF_ADC_UNIT_1, HF_ADC_CHANNEL_0, HF_ADC_ATTEN_DB_11);
EspAdc adc2(HF_ADC_UNIT_1, HF_ADC_CHANNEL_1, HF_ADC_ATTEN_DB_11);
EspAdc adc3(HF_ADC_UNIT_1, HF_ADC_CHANNEL_2, HF_ADC_ATTEN_DB_11);

// Initialize all
if (!adc1.Initialize() || !adc2.Initialize() || !adc3.Initialize()) {
    printf("Failed to initialize ADC channels\n");
    return;
}

// Read from all channels
hf_adc_millivolts_t values[3];
if (adc1.ReadMillivolts(&values[0]) == HF_ADC_ERR_OK &&
    adc2.ReadMillivolts(&values[1]) == HF_ADC_ERR_OK &&
    adc3.ReadMillivolts(&values[2]) == HF_ADC_ERR_OK) {
    
    printf("ADC readings: %d mV, %d mV, %d mV\n", 
           values[0], values[1], values[2]);
}
```

### Attenuation Control

```cpp
// Set different attenuation levels
hf_adc_atten_t attenuations[] = {
    HF_ADC_ATTEN_DB_0,   // 0-1.1V
    HF_ADC_ATTEN_DB_2_5, // 0-1.5V
    HF_ADC_ATTEN_DB_6,   // 0-2.2V
    HF_ADC_ATTEN_DB_11   // 0-3.3V
};

for (int i = 0; i < 4; i++) {
    hf_adc_err_t err = adc.SetAttenuation(attenuations[i]);
    if (err == HF_ADC_ERR_OK) {
        hf_adc_voltage_t max_voltage;
        adc.GetMaxVoltage(&max_voltage);
        printf("Attenuation %d: Max voltage = %.1f V\n", i, max_voltage);
        
        hf_adc_millivolts_t reading;
        if (adc.ReadMillivolts(&reading) == HF_ADC_ERR_OK) {
            printf("  Current reading: %d mV\n", reading);
        }
    }
}
```

### Calibration

```cpp
// Calibrate ADC for better accuracy
hf_adc_err_t err = adc.Calibrate();
if (err == HF_ADC_ERR_OK) {
    printf("ADC calibrated successfully\n");
    
    // Check if calibrated
    bool calibrated;
    err = adc.IsCalibrated(&calibrated);
    if (err == HF_ADC_ERR_OK && calibrated) {
        printf("ADC is calibrated\n");
    }
} else {
    printf("ADC calibration failed: %d\n", err);
}
```

## ESP32-C6 Specific Features

### SAR ADC

The ESP32-C6 uses a SAR (Successive Approximation Register) ADC with 12-bit resolution.

### Multiple Units

Support for both ADC1 and ADC2 units with different channel configurations.

### Attenuation Control

Configurable input attenuation for different voltage ranges:
- 0dB: 0-1.1V
- 2.5dB: 0-1.5V  
- 6dB: 0-2.2V
- 11dB: 0-3.3V

### Built-in Calibration

Hardware calibration support for improved accuracy.

## Error Handling

The `EspAdc` class provides comprehensive error handling with specific error codes:

- `HF_ADC_ERR_OK` - Operation successful
- `HF_ADC_ERR_INVALID_ARG` - Invalid parameter
- `HF_ADC_ERR_NOT_INITIALIZED` - ADC not initialized
- `HF_ADC_ERR_INVALID_CHANNEL` - Invalid channel
- `HF_ADC_ERR_INVALID_UNIT` - Invalid unit
- `HF_ADC_ERR_READ_FAILED` - Read operation failed
- `HF_ADC_ERR_CALIBRATION_FAILED` - Calibration failed

## Performance Considerations

- **Sampling Rate**: Higher resolution reduces maximum sampling rate
- **Attenuation**: Choose appropriate attenuation for your voltage range
- **Calibration**: Calibrate for better accuracy in your application
- **Noise**: Use proper filtering for noisy signals

## Related Documentation

- [BaseAdc API Reference](../api/BaseAdc.md) - Base class interface
- [HardwareTypes Reference](../api/HardwareTypes.md) - Platform-agnostic type definitions
- [ESP-IDF ADC Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/peripherals/adc.html) - ESP-IDF documentation