# ⚡ McuAdc Class

[![MCU Layer](https://img.shields.io/badge/Layer-MCU-blue?style=flat-square)](../index.md#mcu-layer)
[![ADC Interface](https://img.shields.io/badge/Interface-ADC-orange?style=flat-square)](BaseAdc.md)
[![Platform Agnostic](https://img.shields.io/badge/Platform-Agnostic-purple?style=flat-square)](#platform-support)

## 📋 Overview

The `McuAdc` class provides a platform-agnostic MCU ADC driver that implements the unified `BaseAdc` interface. It automatically adapts to the current MCU platform while providing consistent, high-level ADC functionality with advanced features like sampling averaging and configurable timing.

### 🎯 Key Features

- ✅ **Platform Agnostic**: Unified interface across different MCU platforms
- ✅ **Multi-Channel Support**: Full support for all available ADC channels
- ✅ **Sample Averaging**: Built-in multi-sample averaging for noise reduction
- ✅ **Configurable Timing**: Adjustable timing between samples
- ✅ **Flexible Resolution**: Support for various bit widths (8, 10, 12-bit)
- ✅ **Attenuation Control**: Configurable input voltage range
- ✅ **Raw & Voltage Readings**: Support for both raw counts and calibrated voltage

## 🏗️ Class Hierarchy

```
BaseAdc (Abstract Base)
    └── McuAdc (MCU Implementation)
```

## 🚀 Quick Start

### Basic Single Reading

```cpp
#include "McuAdc.h"

// Create ADC for unit 1 with 11dB attenuation, 12-bit resolution
McuAdc adc(HF_ADC_UNIT_1, 11, hf_adc_resolution_t::Bits12);

// Initialize and read
if (adc.Initialize()) {
    uint32_t raw_count;
    HfAdcErr result = adc.ReadChannelCount(0, raw_count);
    if (result == HfAdcErr::ADC_OK) {
        printf("Channel 0 raw count: %lu\n", raw_count);
    }
}
```

### Voltage Reading with Averaging

```cpp
// Read voltage with 10 samples averaged
float voltage;
HfAdcErr result = adc.ReadChannelVoltage(
    0,           // Channel 0
    voltage,     // Output voltage
    10,          // Average 10 samples
    1000         // 1ms between samples
);

if (result == HfAdcErr::ADC_OK) {
    printf("Average voltage: %.3f V\n", voltage);
}
```

### Multi-Channel Monitoring

```cpp
class SensorMonitor {
private:
    McuAdc adc_;
    static const uint8_t SENSOR_CHANNELS[] = {0, 1, 2, 3};
    
public:
    SensorMonitor() : adc_(HF_ADC_UNIT_1, 11) {}
    
    bool Initialize() {
        return adc_.Initialize();
    }
    
    void ReadAllSensors() {
        for (uint8_t channel : SENSOR_CHANNELS) {
            if (adc_.IsChannelAvailable(channel)) {
                float voltage;
                if (adc_.ReadChannelVoltage(channel, voltage, 5) == HfAdcErr::ADC_OK) {
                    printf("Sensor %d: %.3f V\n", channel, voltage);
                }
            }
        }
    }
};
```

## 📚 Constructor

### McuAdc Constructor

```cpp
McuAdc(
    hf_adc_unit_t adc_unit,
    uint32_t attenuation,
    hf_adc_resolution_t width = hf_adc_resolution_t::Bits12
);
```

#### Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `adc_unit` | `hf_adc_unit_t` | - | ADC unit identifier |
| `attenuation` | `uint32_t` | - | Attenuation level (dB) |
| `width` | `hf_adc_resolution_t` | `Bits12` | ADC resolution |

#### Attenuation Values

| Value | Input Range | Use Case |
|-------|-------------|----------|
| `0` | 0-1.1V | Low voltage sensors |
| `2.5` | 0-1.5V | Battery monitoring |
| `6` | 0-2.2V | General purpose |
| `11` | 0-3.9V | High voltage sensors |

#### Resolution Options

| Resolution | Max Value | Precision |
|------------|-----------|-----------|
| `Bits8` | 255 | 8-bit |
| `Bits10` | 1023 | 10-bit |
| `Bits12` | 4095 | 12-bit |

## 🔧 Core Methods

### Initialization

#### Initialize()
```cpp
bool Initialize() noexcept override;
```
- **Purpose**: Initialize the ADC peripheral with current configuration
- **Returns**: `true` if successful, `false` otherwise
- **Thread Safety**: ❌ Not thread-safe

#### Deinitialize()
```cpp
bool Deinitialize() noexcept override;
```
- **Purpose**: Deinitialize the ADC peripheral
- **Returns**: `true` if successful, `false` otherwise
- **Thread Safety**: ❌ Not thread-safe

### Channel Information

#### GetMaxChannels()
```cpp
uint8_t GetMaxChannels() const noexcept override;
```
- **Purpose**: Get maximum number of channels supported
- **Returns**: Maximum channel count
- **Thread Safety**: ✅ Safe

#### IsChannelAvailable()
```cpp
bool IsChannelAvailable(uint8_t channel_num) const noexcept override;
```
- **Purpose**: Check if specific channel is available
- **Parameters**: `channel_num` - Channel number to check
- **Returns**: `true` if available, `false` otherwise
- **Thread Safety**: ✅ Safe

## 📊 Reading Methods

### Raw Count Reading

#### ReadChannelCount()
```cpp
HfAdcErr ReadChannelCount(
    uint8_t channel_num,
    uint32_t& channel_reading_count,
    uint8_t numOfSamplesToAvg = 1,
    uint32_t timeBetweenSamples = 0
) noexcept override;
```

**Parameters:**
- `channel_num`: ADC channel number
- `channel_reading_count`: Output raw ADC count
- `numOfSamplesToAvg`: Number of samples to average (1-255)
- `timeBetweenSamples`: Time between samples in microseconds

**Returns:** `HfAdcErr` status code

### Voltage Reading

#### ReadChannelVoltage()
```cpp
HfAdcErr ReadChannelVoltage(
    uint8_t channel_num,
    float& channel_reading_voltage,
    uint8_t numOfSamplesToAvg = 1,
    uint32_t timeBetweenSamples = 0
) noexcept override;
```

**Parameters:**
- `channel_num`: ADC channel number
- `channel_reading_voltage`: Output voltage in volts
- `numOfSamplesToAvg`: Number of samples to average
- `timeBetweenSamples`: Time between samples in microseconds

**Returns:** `HfAdcErr` status code

## 🎚️ Configuration Methods

### Attenuation Control

#### SetChannelAttenuation()
```cpp
HfAdcErr SetChannelAttenuation(
    uint8_t channel_num,
    uint32_t attenuation_db
) noexcept override;
```

### Resolution Control

#### SetResolution()
```cpp
HfAdcErr SetResolution(hf_adc_resolution_t resolution) noexcept override;
```

### Channel Configuration

#### ConfigureChannel()
```cpp
HfAdcErr ConfigureChannel(
    uint8_t channel_num,
    uint32_t attenuation_db
) noexcept override;
```

## 🛡️ Error Handling

All methods return appropriate error codes from the `HfAdcErr` enumeration:

| Error Code | Description |
|------------|-------------|
| `ADC_OK` | Operation completed successfully |
| `ADC_ERR_INVALID_CHANNEL` | Invalid channel number |
| `ADC_ERR_NOT_INITIALIZED` | ADC not initialized |
| `ADC_ERR_INVALID_CONFIG` | Invalid configuration |
| `ADC_ERR_HARDWARE_FAILURE` | Hardware operation failed |
| `ADC_ERR_TIMEOUT` | Operation timed out |
| `ADC_ERR_CALIBRATION_FAILED` | Calibration failed |

### Error Handling Example

```cpp
HfAdcErr result = adc.ReadChannelVoltage(0, voltage);
switch (result) {
    case HfAdcErr::ADC_OK:
        printf("Reading successful: %.3f V\n", voltage);
        break;
    case HfAdcErr::ADC_ERR_INVALID_CHANNEL:
        printf("Error: Invalid channel\n");
        break;
    case HfAdcErr::ADC_ERR_NOT_INITIALIZED:
        printf("Error: ADC not initialized\n");
        break;
    default:
        printf("Error: ADC operation failed (%d)\n", static_cast<int>(result));
        break;
}
```

## 🔄 Advanced Usage Patterns

### Continuous Monitoring

```cpp
class ContinuousMonitor {
private:
    McuAdc adc_;
    volatile bool monitoring_;
    
public:
    void StartMonitoring() {
        monitoring_ = true;
        while (monitoring_) {
            float voltage;
            if (adc_.ReadChannelVoltage(0, voltage, 5, 1000) == HfAdcErr::ADC_OK) {
                ProcessVoltage(voltage);
            }
            vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz sampling
        }
    }
    
    void StopMonitoring() {
        monitoring_ = false;
    }
};
```

### Calibrated Sensor Reading

```cpp
class TemperatureSensor {
private:
    McuAdc adc_;
    float offset_;
    float scale_;
    
public:
    TemperatureSensor(float offset, float scale) 
        : adc_(HF_ADC_UNIT_1, 11), offset_(offset), scale_(scale) {}
    
    float ReadTemperature() {
        float voltage;
        if (adc_.ReadChannelVoltage(0, voltage, 10, 500) == HfAdcErr::ADC_OK) {
            return (voltage - offset_) * scale_;
        }
        return NAN; // Error
    }
};
```

## 🖥️ Platform Support

The `McuAdc` class automatically adapts to different MCU platforms:

### ESP32-C6
- ✅ Full support for ADC1 and ADC2
- ✅ Hardware calibration support
- ✅ DMA mode support
- ✅ All attenuation levels

### Future Platforms
- 🔄 STM32 series (planned)
- 🔄 Nordic nRF series (planned)
- 🔄 Raspberry Pi Pico (planned)

## 💡 Best Practices

### 1. **Proper Initialization**
```cpp
McuAdc adc(HF_ADC_UNIT_1, 11);
if (!adc.Initialize()) {
    // Handle initialization failure
    ESP_LOGE(TAG, "ADC initialization failed");
    return;
}
```

### 2. **Use Appropriate Averaging**
```cpp
// For noisy signals, use more samples
float stable_reading;
adc.ReadChannelVoltage(0, stable_reading, 50, 100);

// For fast changing signals, use fewer samples
float fast_reading;
adc.ReadChannelVoltage(1, fast_reading, 1, 0);
```

### 3. **Channel Validation**
```cpp
uint8_t channel = 5;
if (!adc.IsChannelAvailable(channel)) {
    ESP_LOGW(TAG, "Channel %d not available", channel);
    return;
}
```

### 4. **Resource Management**
```cpp
class AdcManager {
private:
    McuAdc adc_;
    
public:
    AdcManager() : adc_(HF_ADC_UNIT_1, 11) {}
    
    ~AdcManager() {
        adc_.Deinitialize();
    }
};
```

## 📈 Performance Considerations

### Sampling Speed
- **Single Sample**: ~50µs
- **10 Samples Averaged**: ~500µs + timing delays
- **50 Samples Averaged**: ~2.5ms + timing delays

### Memory Usage
- **Class Instance**: ~64 bytes
- **Per Reading**: Minimal stack usage
- **Calibration Data**: Stored in platform-specific memory

## 🔗 Related Classes

- [`BaseAdc`](BaseAdc.md) - Base ADC interface
- [`SfAdc`](SfAdc.md) - Thread-safe ADC wrapper
- [`McuTypes`](#) - Platform-specific type definitions

---

*For more information about the HardFOC Internal Interface Wrapper, see the [main documentation](../index.md).*
